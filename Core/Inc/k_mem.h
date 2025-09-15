/*
 * k_mem.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_MEM_H_
#define INC_K_MEM_H_

#include <stddef.h>
#define ORIGIN_MEMORY 0x20000000 // Starting address of the memory pool

int k_mem_init();              // Initialize memory manager
void* k_mem_alloc(size_t size); // Allocate memory
int k_mem_dealloc(void* ptr);   // Deallocate memory
int k_mem_count_extfrag(size_t size); // Count external fragmentation

#endif /* INC_K_MEM_H_ */
