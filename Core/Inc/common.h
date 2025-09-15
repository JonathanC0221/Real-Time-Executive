/*
 * common.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: If you feel that there are common
 *      C functions corresponding to this
 *      header, then any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#define TID_NULL 0 //predefined Task ID for the NULL task
#define MAX_TASKS 16 //maximum number of tasks in the system
#define STACK_SIZE 0x200 //min. size of each task’s stack
#define DORMANT 0 //state of terminated task
#define READY 1 //state of task that can be scheduled but is not running
#define RUNNING 2 //state of running task
#define SLEEPING 3 //state of task that is waiting for period of time to expire

#define RTX_OK 0
#define RTX_ERR 1

typedef unsigned int task_t;
extern task_t current_tid;
extern int kernel_started;

#endif /* INC_COMMON_H_ */
