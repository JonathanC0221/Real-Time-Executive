/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include <stdio.h> //You are permitted to use this library, but currently only printf is implemented. Anything else is up to you!
#include "k_task.h"
/**
  * @brief  The application entry point.
  * @retval int
  */

// uint32_t* stackptr;
//uint32_t* MSP_INIT_VAL;

void print_balls(void *args){
	task_t test = osGetTID();
//	printf("Balls 1 tid %d\r\n", test);
//	osYield();
//	printf("BigBalls 1 tid %d\r\n", test);
//	osTaskExit();
	// for (int i = 1; i <= 10; i++) {
	// 	printf("Iter: %d TID: %d\r\n", i, test);
	// 	if (i == 5) {
	// 		osYield();
	// 	}
	// }
	// osTaskExit();
  int i = 1; 
	while (1) {
    if (i == 200 || i == 500 || i == 5000|| i == 30000000 || i == 60000000){
      printf("task 1 running: i = %d\r\n", i);
    }
    i += 1;
	}
}

void print_balls2(void *args) {
	task_t test = osGetTID();
	// printf("Balls2 TID: %d\r\n", test);
	// osTaskExit();
  int i = 1; 
	while (1) {
		if (i == 200 || i == 500 || i == 5000|| i == 30000000 || i == 60000000){
      printf("task 2 running: i = %d\r\n", i);
    }
    i += 1;
	}
}

void print_balls3(void *args) {
	task_t test = osGetTID();
	// printf("Balls3 TID: %d\r\n", test);
	// osTaskExit();
  int i = 1; 
	while (1) {
		if (i == 200 || i == 500 || i == 5000|| i == 30000000 || i == 60000000){
      printf("task 3 running: i = %d\r\n", i);
      // yield early 3 times (before timeout) then use full time slice
      printf("yield\r\n");
      osYield();
    }
    i += 1;
	}
}

int main(void)
{

  /* MCU Configuration: Don't change this or the whole chip won't work!*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  // extern uint32_t _img_end;
  // printf("end of image: %x\r\n", &_img_end);

  // uint32_t *heap_begin = (uint32_t*)&_img_end + 0x80; // add 0x200 / 4 (bc uint32)

  // extern uint32_t _estack;
  // extern uint32_t _Min_Stack_Size;

  // uint32_t stack_end = &_estack;
  // uint32_t stack_size = &_Min_Stack_Size;
  // uint32_t *heap_end = (uint32_t*) (stack_end - stack_size);
  // printf("estack: %x\r\n", &_estack);
  // printf("beginning of heap: %p\r\n", heap_begin);
  // printf("end of heap: %p\r\n", heap_end);


  osKernelInit();

  TCB task = {.ptask = print_balls, .stack_size = 0x200};
  osCreateTask(&task);
  TCB task2 = {.ptask = print_balls2, .stack_size = 0x200};
  osCreateTask(&task2);
  TCB task3 = {.ptask = print_balls3, .stack_size = 0x200};
  osCreateTask(&task3);

  osKernelStart();
  /* MCU Configuration is now complete. Start writing your code below this line */
//  MSP_INIT_VAL = *(uint32_t**)0x0;
//  printf("MSP Init is: %p\r\n", MSP_INIT_VAL); //note the %p to print a pointer. It will be in hex

//  __asm("SVC #6");
//  __asm("SVC #11");

  // stackptr = (int)MSP_INIT_VAL - 0x400;
  // *(--stackptr) = 1<<24; //This is xPSR, should be Thumb mode
  // *(--stackptr) = (uint32_t)print_continuously; //the function name
  // for (int i = 0; i < 14; i++){
	// *(--stackptr) = 0xA; //An arbitrary number
  // }
  // __asm("SVC #0");

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  printf("you fucked up\n");
    /* USER CODE END WHILE */
//	  printf("Hello, world!\r\n");
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

