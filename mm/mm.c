/*
 *  Copyright 2016 Davide Pianca
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <hal/hal.h>
#include <lib/string.h>
#include <drivers/video.h>

mem_info_t pmm;

// 32KB for the bitmap to reserve up to 4GB
static uint32_t bitmap[BITMAP_LEN] __attribute__((aligned(BLOCKS_LEN)));

/**
 * Initializes the physical memory manager
 * Every bit in the pmm.map indicates if a 4096 byte block is free or not
 * mem_size is in KB
 */
void pmm_init(uint32_t mem_size, mm_addr_t *mmap_addr, uint32_t mmap_len) {
    // Get the blocks number
    pmm.max_blocks = pmm.used_blocks = mem_size / 4;
    // Set the address for the memory map
    pmm.map = bitmap;
    // Set all the blocks as used
    memset(pmm.map, BYTE_SET, BITMAP_LEN);
    
    mem_region_t *mm_reg = (mem_region_t *) mmap_addr;
    
    // Parse the memory map
    while(mm_reg < (mem_region_t *) mmap_addr + mmap_len) {
        // Check if the memory region is available
        if(mm_reg->type == 1)
            pmm_init_reg(mm_reg->addr_low, mm_reg->len_low);
        
        // Increment the pointer to the next map entry
        mm_reg = (mem_region_t *) ((uint32_t) mm_reg + mm_reg->size + sizeof(mm_reg->size));
    }
    pmm_deinit_reg(0x0, KERNEL_SPACE_END);
    pmm.size = (pmm.max_blocks - pmm.used_blocks) * 4;
}

/**
 * Sets the block as used
 */
void pmm_set_bit(int bit) {
    pmm.map[bit / 32] |= (1 << (bit % 32));
}

/**
 * Sets the block as free
 */
void pmm_unset_bit(int bit) {
    pmm.map[bit / 32] &= ~(1 << (bit % 32));
}

/**
 * Gets the first free block
 */
int pmm_first_free() {
    uint32_t i;
    int j;
    
    for(i = 0; i < pmm.max_blocks / 32; i++) {
        if(pmm.map[i] != BYTE_SET) {
            for(j = 0; j < 32; j++) {
                if(!(pmm.map[i] & (1 << j))) {
                    return (i * 32) + j;
                }
            }
        }
    }
    return -1;
}

/**
 * Initializes a memory region to be used
 */
void pmm_init_reg(mm_addr_t addr, uint32_t size) {
    uint32_t i;
    uint32_t blocks = size / BLOCKS_LEN;
    uint32_t align = addr / BLOCKS_LEN;
    for(i = 0; i < blocks; i++) {
        pmm_unset_bit(align++);
        pmm.used_blocks--;
    }
    pmm_set_bit(0);
}

/**
 * Deinitializes a reserved memory region
 */
void pmm_deinit_reg(mm_addr_t addr, uint32_t size) {
    uint32_t i;
    uint32_t blocks = size / BLOCKS_LEN;
    uint32_t align;
    if(addr == 0) {
        align = 0;
    } else {
        align = addr / BLOCKS_LEN;
    }
    for(i = 0; i < blocks; i++) {
        pmm_set_bit(align++);
        pmm.used_blocks++;
    }
}

/**
 * Returns a usable block
 */
void *pmm_malloc() {
    int p = pmm_first_free();
    if(!p)
        return NULL;
    pmm_set_bit(p);
    pmm.used_blocks++;
    return (void *) (BLOCKS_LEN * p);
}

/**
 * Frees a block
 */
void pmm_free(mm_addr_t *addr) {
    if((uint32_t) addr < KERNEL_SPACE_END)
        return;
    pmm_unset_bit((uint32_t) addr / BLOCKS_LEN);
    pmm.used_blocks--;
}

mm_addr_t *get_mem_map() {
    return pmm.map;
}

uint32_t get_mem_size() {
    return pmm.size;
}

uint32_t get_used_blocks() {
    return pmm.used_blocks;
}

uint32_t get_max_blocks() {
    return pmm.max_blocks;
}

/**
 * Enables paging
 */
void enable_paging() {
    uint32_t reg;
    // Enable paging
    asm volatile("mov %%cr0, %0" : "=r" (reg));
    reg |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r" (reg));
}

/**
 * Loads the pdbr register with a physical address to a page directory
 */
void load_pdbr(mm_addr_t addr) {
    asm volatile("mov %0, %%cr3" : : "r" (addr));
    enable_paging();
}

/**
 * Gets the pdbr value
 */
mm_addr_t get_pdbr() {
    mm_addr_t ret;
    asm volatile("mov %%cr3, %0" : "=r" (ret));
    return ret;
}

/**
 * Flushes the TLB cache
 */
void flush_tlb(vmm_addr_t addr) {
    asm volatile("cli; invlpg (%0); sti" : : "r" (addr));
}

/**
 * Gets the value of the cr0 register
 */
int get_cr0() {
    int ret;
    asm volatile("mov %%cr0, %0" : "=r" (ret));
    return ret;
}

/**
 * Gets the value of the cr2 register
 */
int get_cr2() {
    int ret;
    asm volatile("mov %%cr2, %0" : "=r" (ret));
    return ret;
}
