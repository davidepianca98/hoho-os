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

/**
 * Initializes the physical memory manager
 * Every bit in the pmm.map indicates if a 4096 byte block is free or not
 */
void pmm_init(uint32_t mem_size, mm_addr_t *mmap_addr, uint32_t mmap_len) {
    uint32_t kernel_size = &kernel_end - &kernel_start;

    // Get the blocks number
    pmm.max_blocks = pmm.used_blocks = mem_size / BLOCKS_LEN;
    // Set the address for the memory map
    pmm.map = (uint32_t *) 0x100000 + (ROUNDUP(kernel_size, 4096));
    // Set all the blocks as used
    memset(pmm.map, 0xF, pmm.max_blocks / BLOCKS_PER_BYTE);
    
    mem_region_t *mm_reg = (mem_region_t *) mmap_addr;
    mem_region_t *mm_lim = (mem_region_t *) mmap_addr + mmap_len;
    
    // Parse the memory map
    while(mm_reg < mm_lim) {
        // Check if the memory region is available
        if(mm_reg[0].type > 4 || mm_reg[0].type == 1)
            pmm_init_reg(mm_reg[0].addr_low, mm_reg[0].len_low);
        
        // Increment the pointer to the next map entry
        mm_reg++;
        
        // Check if it's finished
        if(mm_reg[0].addr_low == 0)
            break;
    }
    pmm_deinit_reg(0x100000, kernel_size);
    pmm_deinit_reg(0x0, 0x10000);
    pmm_deinit_reg((uint32_t) pmm.map, pmm.max_blocks);
    pmm.size = (pmm.max_blocks - pmm.used_blocks) * BLOCKS_LEN;
}

/**
 * Sets the block as used
 */
void pmm_set_bit(int bit) {
    if((bit < 0) || ((uint32_t) bit > pmm.max_blocks)) {
        printk("ERROR\n");
        return;
    }
    pmm.map[bit / 32] |= 1 << (bit % 32);
}

/**
 * Sets the block as free
 */
void pmm_unset_bit(int bit) {
    pmm.map[bit / 32] &= ~(1 << (bit % 32));
}

int pmm_get_bit(int bit) {
    return pmm.map[bit / 32] & 1 << (bit % 32);
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
 * Gets the first free n contiguous blocks
 */
int pmm_first_free_contig(int n) {
    uint32_t i;
    int j, temp, free;
    
    if(n <= 0)
        return -1;
    else if(n == 1)
        return pmm_first_free();
    
    for(i = 0; i < pmm.max_blocks / 32; i++) {
        if(pmm.map[i] != BYTE_SET) {
            for(j = 0; j < 32; j++) {
                if(!(pmm.map[i] & (1 << j))) {
                    temp = (i * 32) + (1 << j);
                    free = 0;
                    for(int k = 0; k <= n; k++) {
                        if(!pmm_get_bit(temp + k)) {
                            free++;
                        }
                        if(free == n) {
                            return (i * 32) + j;
                        }
                    }
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
    uint32_t align = addr / BLOCKS_LEN;
    for(i = 0; i < blocks; i++) {
        pmm_set_bit(align++);
        pmm.used_blocks++;
    }
    pmm_set_bit(0);
}

/**
 * Returns a usable block
 */
void *pmm_malloc() {
    int p = pmm_first_free();
    if(p == -1)
        return NULL;
    pmm_set_bit(p);
    pmm.used_blocks++;
    return (void *) (BLOCKS_LEN * p);
}

/**
 * Returns n contiguous usable blocks
 */
void *pmm_malloc_blocks(int n) {
    int i;
    int p = pmm_first_free_contig(n);
    if(p == -1)
        return NULL;
    for(i = 0; i < n; i++) {
        pmm_set_bit(p + i);
        pmm.used_blocks++;
    }
    return (void *) (BLOCKS_LEN * p);
}

/**
 * Frees a block
 */
void pmm_free(mm_addr_t *frame) {
    pmm_unset_bit((uint32_t) frame / BLOCKS_LEN);
    pmm.used_blocks--;
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
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r" (cr0));
}

/**
 * Loads the pdbr register with a physical address to a page directory
 */
void load_pdbr(mm_addr_t addr) {
    asm volatile("mov %%eax, %%cr3" : : "a" (addr));
}

/**
 * Gets the pdbr value
 */
mm_addr_t get_pdbr() {
    mm_addr_t ret;
    asm volatile("mov %%cr3, %%eax" : "=a" (ret));
    return ret;
}

/**
 * Flushes the TLB cache
 */
void flush_tlb(vmm_addr_t addr) {
    asm volatile("cli; invlpg (%0); sti" : : "r" (addr));
}

/**
 * Gets the value of the cr2 register
 */
int get_cr2() {
    int ret;
    asm volatile("mov %%cr2, %%eax" : "=a" (ret));
    return ret;
}

