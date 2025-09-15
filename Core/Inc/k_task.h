/*
 * k_task.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_TASK_H_
#define INC_K_TASK_H_

#include <stdio.h>
#include "common.h"

#define SHPR2 (*((volatile uint32_t*)0xE000ED1C))//SVC is bits 31-28
#define SHPR3 (*((volatile uint32_t*)0xE000ED20))//SysTick is bits 31-28, and PendSV is bits 23-20

typedef unsigned int U32;
typedef unsigned short U16;
typedef char U8;
typedef struct task_control_block{
	U32 stack_pointer; //current stack pointer for context switching
	void (*ptask)(void* args); //entry address
	U32 stack_high; //starting address of stack (high address)
	task_t tid; //task ID
	U8 state; //task's state
	U16 stack_size; //stack size. Must be a multiple of 8
	//your own fields at the end
	int deadline; //deadline for the task, in milliseconds
	int time_remaining; //time remaining before the current deadline or sleep_time expires
}TCB;

void osKernelInit(void);
int osCreateTask(TCB* task);
int osKernelStart(void);
void osYield(void);
void osSleep(int timeInMs);
void osPeriodYield(void);
void osScheduler(void);
int osTaskInfo(task_t TID, TCB* task_copy);
task_t osGetTID(void);
int osSetDeadline(int deadline, task_t TID);
int osCreateDeadlineTask(int deadline, TCB* task);
int osTaskExit(void);

#endif /* INC_K_TASK_H_ */
