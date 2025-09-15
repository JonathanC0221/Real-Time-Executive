/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
#include <stdio.h>
#include "common.h"
#include "k_task.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
	printf("I'm very hard right now\r\n");
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  printf("i have dementia\r\n");
  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */
  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */
  printf("you're using me wrong\r\n");
  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
// void SVC_Handler(void)
//{
//   /* USER CODE BEGIN SVCall_IRQn 0 */
//
//   /* USER CODE END SVCall_IRQn 0 */
//   /* USER CODE BEGIN SVCall_IRQn 1 */
//
//   /* USER CODE END SVCall_IRQn 1 */
// }

// extern void SVC_0(void);
// extern void SVC_1(void);

void SVC_Handler_Main(unsigned int *svc_args)
{
  unsigned int svc_number;

  svc_number = ((char *)svc_args[6])[-2];

  switch (svc_number)
  {
  case 0:
    // start tasks
    __asm volatile(
        "MRS r0, PSP\n"
        "LDMIA r0!, {r4-r11}\n"
        "MSR PSP, r0\n"
        "MOV LR, #0xFFFFFFFD\n"
        "BX LR\n");
    break;

  case 1:
    // printf("svc 1\r\n");
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set the PendSV bit to trigger a context switch
    __asm("ISB");
    break;
  }
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
 * @brief This function handles Pendable request for system service.
 */
// void PendSV_Handler(void)
// {
//   /* USER CODE BEGIN PendSV_IRQn 0 */
//   //	printf("entered pendSV\r\n");
//   __asm volatile(
//       "MRS R0, PSP\n"         // Load PSP into R0
//       "STMDB R0!, {R4-R11}\n" // Push R4–R11 onto current stack (PSP)
//       "MSR PSP, R0\n"
//       "BL osScheduler\n" // Call the scheduler function
//       "MRS R0, PSP\n"
//       "LDMIA R0!, {R4-R11}\n"
//       "MSR PSP, R0\n"
//       "MOV LR, #0xFFFFFFFD\n"
//       "BX LR\n");
//   /* USER CODE END PendSV_IRQn 0 */

//   /* USER CODE END PendSV_IRQn 1 */
// }

/**
 * @brief This function handles System tick timer.
 */

extern TCB tasks_array[MAX_TASKS];
extern task_t current_tid;

void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  if (!kernel_started) return;
  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();

  int higher_priority_task_ready = 0;
  TCB *current_task = &tasks_array[current_tid];

  if (current_tid != 0) {
      current_task->time_remaining--;
      if (current_task->time_remaining <= 0) {
        current_task->time_remaining = current_task->deadline;
      }
  }

  for (int i = 1; i < MAX_TASKS; i++) {
    TCB *task = &tasks_array[i];

    if (task->state != DORMANT && i != current_tid) {
      task->time_remaining--;

      if (task->time_remaining <= 0) {
        task->state = READY;
        task->time_remaining = task->deadline;
      }
    }

    // Check if there's a higher priority (lower time_remaining) ready task
    if (task->state == READY && task->time_remaining < current_task->time_remaining) {
      higher_priority_task_ready = 1;
    }
  }

  if (higher_priority_task_ready) {
      current_task->state = READY;
      SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set the PendSV bit to trigger a context switch
    __asm("ISB");
  }
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
