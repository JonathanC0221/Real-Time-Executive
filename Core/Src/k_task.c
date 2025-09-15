#include "common.h"
#include "k_task.h"
#include "stm32f4xx_hal.h"
#include "k_mem.h"

TCB tasks_array[MAX_TASKS];

int init_flag = 0;
int kernel_started = 0;
task_t current_tid = 0;

U32* stackptr;
U32* MSP_INIT_VAL;

void dormant(void *args) {
	while (1);
}

void osKernelInit(void) {
	if (init_flag) return;
	k_mem_init();

	SHPR3 = (SHPR3 & ~(0xFFU << 24)) | (0xF0U << 24);//SysTick is lowest priority (highest number)
	SHPR3 = (SHPR3 & ~(0xFFU << 16)) | (0xE0U << 16);//PendSV is in the middle
	SHPR2 = (SHPR2 & ~(0xFFU << 24)) | (0xD0U << 24);//SVC is highest priority (lowest number)

	// NVIC_SetPriority(PendSV_IRQn, 0xFF); // Set PendSV priority to the lowest
	// NVIC_SetPriority(SVCall_IRQn, 0);  // 0 is highest priority

	for (int i = 0; i < MAX_TASKS; i++) {
		tasks_array[i].ptask = NULL;
		tasks_array[i].stack_high = -1;
		tasks_array[i].tid = -1;
		tasks_array[i].state = DORMANT;
		tasks_array[i].stack_size = 0x0;
		tasks_array[i].deadline = 0;
		tasks_array[i].time_remaining = 0;
	}

	init_flag = 1;

	MSP_INIT_VAL = *(U32**)0x0;
	// printf("MSP: %p", MSP_INIT_VAL);

	TCB dormant_task = {.ptask = dormant, .stack_size = STACK_SIZE};
	osCreateDeadlineTask(INT32_MAX, &dormant_task);
}

int osCreateTask(TCB *task) {
    // Create a deadline task with a default deadline of 5ms
    return osCreateDeadlineTask(5, task);
}

int osCreateDeadlineTask(int deadline, TCB *task) {

	task_t tid = -1;


	if (task->stack_size < STACK_SIZE) return RTX_ERR;

	void* new_stack = k_mem_alloc(task->stack_size);

	if (new_stack == NULL) return RTX_ERR;

	task->stack_high = (U32)new_stack + task->stack_size;

	for (int i = 0; i < MAX_TASKS; i++) {
		if (tasks_array[i].state == DORMANT) {
			tid = i;
			break;
		}
	}

	if (tid == -1) return RTX_ERR; //no free slots to create

	//updating given TCB
	task->tid = tid;
	task->state = READY;

	//updating kernel level
	tasks_array[tid].tid = tid;
	tasks_array[tid].stack_size = task->stack_size;
	tasks_array[tid].ptask = task->ptask;
	tasks_array[tid].state = READY;
	tasks_array[tid].deadline = deadline;
	tasks_array[tid].time_remaining = deadline;
	tasks_array[tid].stack_high = task->stack_high;

	U32* stackptr = (U32*)tasks_array[tid].stack_high;
	*(--stackptr) = 1<<24; //This is xPSR, should be Thumb mode
	*(--stackptr) = (U32)tasks_array[tid].ptask; //the function name
	*(--stackptr) = 0xFFFFFFFD;
	for (int i = 0; i < 13; i++){
		*(--stackptr) = 0xDEADBEEF;
	}

	// Store the current stack pointer in the TCB
	tasks_array[tid].stack_pointer = (U32)stackptr;

	if (deadline < tasks_array[current_tid].time_remaining) {
		if (!kernel_started) return;
    	tasks_array[current_tid].state = READY; //set to ready
    	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set the PendSV bit to trigger a context switch
        __asm("ISB");
	}

	return RTX_OK;
}

int osKernelStart(void) {
	if (!init_flag || kernel_started) return RTX_ERR;

	SysTick->VAL = 0; // Reset SysTick counter
	task_t priority_tid = 0;
	int highest_priority = INT32_MAX;

	// Find the next task to run based on priority(earliest deadline)
	for (int i = 1; i < MAX_TASKS; i++) {
        if (tasks_array[i].state == READY && tasks_array[i].time_remaining < highest_priority) {
            highest_priority = tasks_array[i].time_remaining;
            priority_tid = i;
        }
    }

	current_tid = priority_tid;

	// No tasks are ready to run
	if (tasks_array[current_tid].state != READY) return RTX_ERR;

	//start first ready task
	kernel_started = 1;

	__set_PSP((U32)tasks_array[current_tid].stack_pointer);
	tasks_array[current_tid].state = RUNNING;

	__enable_irq();
	__asm("SVC #0");

	// OH SHIT, IT FUCKED UP!
	return RTX_ERR;
}

void osYield(void){
	if (!kernel_started) return;
	tasks_array[current_tid].state = READY; //set to ready
	tasks_array[current_tid].time_remaining = tasks_array[current_tid].deadline; //reset time remaining when yielding
	// __asm("SVC #1");
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set the PendSV bit to trigger a context switch
    __asm("ISB");
}

void osPeriodYield() {
    tasks_array[current_tid].state = SLEEPING; //set to sleep

    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set the PendSV bit to trigger a context switch
    __asm("ISB");
}

void osSleep(int timeInMs){
    if (!kernel_started || timeInMs <= 0) return;
    tasks_array[current_tid].state = SLEEPING; //set to sleep
    tasks_array[current_tid].time_remaining = timeInMs;

    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set the PendSV bit to trigger a context switch
    __asm("ISB");
}

void osScheduler(void){
	if (!kernel_started) {return;}

	tasks_array[current_tid].stack_pointer = __get_PSP();

	if (tasks_array[current_tid].state == RUNNING) {
		tasks_array[current_tid].state = READY;
	}

	task_t priority_tid = 0;
	int highest_priority = INT32_MAX;

	// Find the next task to run based on priority(earliest deadline)
	for (int i = 1; i < MAX_TASKS; i++) {
        if (tasks_array[i].state == READY && tasks_array[i].time_remaining < highest_priority) {
            highest_priority = tasks_array[i].time_remaining;
            priority_tid = i;
        }
    }

	current_tid = priority_tid; //set tid to next available task, if none then go to dormant
	tasks_array[current_tid].state = RUNNING; //set new task to running state
	__set_PSP(tasks_array[current_tid].stack_pointer); //update stack pointer to the next task's stack
}

int osTaskInfo(task_t TID, TCB* task_copy) {
	if (TID < 0 || TID >= MAX_TASKS || task_copy == NULL) return RTX_ERR;

	//copying task info
	task_copy->tid = tasks_array[TID].tid;
	task_copy->ptask = tasks_array[TID].ptask;
	task_copy->stack_high = tasks_array[TID].stack_high;
	task_copy->state = tasks_array[TID].state;
	task_copy->stack_size = tasks_array[TID].stack_size;
	task_copy->deadline = tasks_array[TID].deadline;
	task_copy->time_remaining = tasks_array[TID].time_remaining;

	return RTX_OK;
}

int osSetDeadline(int deadline, task_t TID) {
	__disable_irq();
    if (TID < 0 || TID >= MAX_TASKS || tasks_array[TID].state != READY || deadline <= 0) {
        __enable_irq();
        return RTX_ERR;
    }

    tasks_array[TID].deadline = deadline;
    tasks_array[TID].time_remaining = deadline;


	if (tasks_array[TID].deadline < tasks_array[current_tid].time_remaining) {
	    __enable_irq();
	    if (!kernel_started) return;
	    tasks_array[current_tid].state = READY; //set to ready
	    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set the PendSV bit to trigger a context switch
	    __asm("ISB");
	}
	__enable_irq();

    return RTX_OK;
}

int osTaskExit(void) {
    __disable_irq();
	if (tasks_array[current_tid].state != RUNNING) {
	    __enable_irq();
	    return RTX_ERR;
	}
	tasks_array[current_tid].state = DORMANT;
	tasks_array[current_tid].stack_high = tasks_array[current_tid].stack_high - tasks_array[current_tid].stack_size; //reset stack high
	int status = k_mem_dealloc((void*)tasks_array[current_tid].stack_high);
	if(status == RTX_ERR) {
	    __enable_irq();
	    return RTX_ERR;
	}
	__enable_irq();
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set the PendSV bit to trigger a context switch
    __asm("ISB");
}

task_t osGetTID(void){
	if (!kernel_started) return TID_NULL;
	return current_tid;
}
