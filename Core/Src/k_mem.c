#include <stdint.h>
#include <stddef.h>
#include "common.h"
#include "k_mem.h"


extern uint32_t _estack;
extern uint32_t _Min_Stack_Size;
extern uint32_t _img_end;

#define FREE_TID 0xFFFFFFFF

typedef struct Block {
    size_t size;
    task_t tid;
    struct Block* next;
} Block;

Block* freelist = NULL;
Block* allocated_blocks = NULL;

int k_mem_init() {
    if (freelist != NULL) {
        return RTX_ERR;
    }

    uintptr_t heap_start = (uintptr_t)&_img_end;
    uintptr_t heap_end = (uintptr_t)&_estack - (uintptr_t)&_Min_Stack_Size;
    uintptr_t heap_size = heap_end - heap_start;

    Block* initial_block = (Block*)heap_start;
    if (heap_size < sizeof(Block)) return RTX_ERR;

    initial_block->size = heap_size - sizeof(Block);
    initial_block->tid = FREE_TID;
    initial_block->next = NULL;

    freelist = initial_block;
    allocated_blocks = NULL;

    return RTX_OK;
}

void* k_mem_alloc(size_t size) {
    if (freelist == NULL) {
        printf("freelist NULL");
        return NULL;
    }

    if (size == 0) {
        return NULL; // No allocation for zero size - no space for allocation
    }

    size = (size + 3) & ~3; // Align to 4 bytes by adding 3 and clearing the last 2 bits

    Block* current = freelist;
    Block* previous = NULL;

    while (current != NULL) {
        if (current->size >= size) {
            if (previous != NULL) {
                previous->next = current->next; // Remove current from freelist
            } else {
                freelist = current->next; // Set head of freelist to next block
            }

            // If the block is larger than needed, split it
            if (__builtin_expect(current->size >= size + sizeof(Block), 1)) {  // Check if the block can be split, expecting true because most allocations will be smaller than the block size
                Block* remaining = (Block*)((uintptr_t)current + sizeof(Block) + size);
                remaining->size = current->size - size - sizeof(Block);
                remaining->tid = FREE_TID;
                remaining->next = current->next;

                current->size = size; // Resize the current block to the requested size

                if (previous != NULL) {
                    previous->next = remaining; // Link the remaining block to the previous block
                } else {
                    freelist = remaining; // Set head of freelist to next block
                }
            }

            current->tid = current_tid;

            // Add the block to the allocated blocks list
            current->next = allocated_blocks;
            allocated_blocks = current;

            // Return a pointer to the memory after the block's metadata
            return (void*)((uintptr_t)current + sizeof(Block));
        }

        previous = current;
        current = current->next;
    }

    return NULL;
}

int k_mem_dealloc(void* ptr) {
    if (((uintptr_t)ptr - sizeof(Block)) < (uintptr_t)&_img_end || ((uintptr_t)ptr - sizeof(Block)) >= (uintptr_t)&_estack - _Min_Stack_Size) {
        return RTX_ERR;
    }

    Block* block = (Block*)((uintptr_t)ptr - sizeof(Block));

    // if (block->tid != current_tid) {
    //     printf("Block tid mismatch: expected %u, got %u\n", current_tid, block->tid);
    //     return RTX_ERR;
    // }

    // Check if the block is in the allocated blocks list
    Block* current = allocated_blocks;
    Block* previous = NULL;

    while (current != NULL) {
        if (current == block) {
            if (previous != NULL) {
                previous->next = current->next;
            } else {
                allocated_blocks = current->next;
            }
            break;
        }
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        // print the address of the block that was not found
        printf("Block not found in allocated blocks: %p\n", block);
        return RTX_ERR;
    }

    block->tid = FREE_TID;

    if (freelist == NULL || (uintptr_t)block < (uintptr_t)freelist) {
        block->next = freelist;
        freelist = block;
        // Coalesce with next block if possible
        if (freelist->next != NULL && (uintptr_t)freelist + freelist->size + sizeof(Block) == (uintptr_t)freelist->next) {
            freelist->size += freelist->next->size + sizeof(Block);
            freelist->next = freelist->next->next;
        }
        return RTX_OK;
    }

    current = freelist;

    while (current->next != NULL && (uintptr_t)current->next < (uintptr_t)block) {
        current = current->next;
    }

    block->next = current->next;
    current->next = block;

    if ((uintptr_t)current + current->size + sizeof(Block) == (uintptr_t)block) {
        current->size += block->size + sizeof(Block);
        current->next = block->next;
        block = current; // Update block pointer
    }

    if (block->next != NULL && (uintptr_t)block + block->size + sizeof(Block) == (uintptr_t)block->next) {
        block->size += block->next->size + sizeof(Block);
        block->next = block->next->next;
    }

    return RTX_OK;
}

int k_mem_count_extfrag(size_t size) {
    if (freelist == NULL) {
        return 0;
    }

    int count = 0;
    Block* current = freelist;

    while (current != NULL) {
        if (current->size + sizeof(Block) < size) {
            count++;
        }
        current = current->next;
    }

    return count;
}
