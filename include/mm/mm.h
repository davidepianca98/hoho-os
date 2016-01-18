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

#ifndef MM_H
#define MM_H

#include <types.h>

#define BLOCKS_PER_BYTE 8
#define BLOCKS_LEN 4096
#define BYTE_SET 0xFFFFFFFF

typedef uint32_t mm_addr_t;

typedef uint32_t vmm_addr_t;

typedef struct mem_info {
    uint32_t size;
    uint32_t used_blocks;
    uint32_t max_blocks;
    mm_addr_t *map;
} mem_info_t;

typedef struct memory_region {
    uint32_t size;
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type;
} __attribute__((__packed__)) mem_region_t;

void pmm_init(uint32_t mem_size, mm_addr_t *mmap_addr);
void pmm_set_bit(int bit);
void pmm_unset_bit(int bit);
int pmm_get_bit(int bit);
int pmm_first_free();
int pmm_first_free_contig(int n);
void pmm_init_reg(mm_addr_t addr, uint32_t size);
void pmm_deinit_reg(mm_addr_t addr, uint32_t size);
void *pmm_malloc();
void *pmm_malloc_blocks(int n);
void pmm_free(mm_addr_t *frame);

uint32_t get_mem_size();
uint32_t get_used_blocks();
uint32_t get_max_blocks();

void enable_paging();
void load_pdbr(mm_addr_t addr);
mm_addr_t get_pdbr();
void flush_tlb(vmm_addr_t addr);
int get_cr2();

#endif

