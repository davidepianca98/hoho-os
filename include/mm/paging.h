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

#ifndef PAGING_H
#define PAGING_H

#include <mm/mm.h>
#include <types.h>

#define PAGE_SIZE           4096

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

#define PAGE_PRESENT_FLAG   1
#define PAGE_RW_FLAG        2
#define PAGE_MODE_FLAG      4
#define PAGE_ACCESSED_FLAG  32
#define PAGE_DIRTY_FLAG     64
#define PAGE_FRAME_MASK     0x7FFFF000

typedef uint32_t page_t;

#define PAGE_TABLE_PRESENT_FLAG   1
#define PAGE_TABLE_RW_FLAG        2
#define PAGE_TABLE_MODE_FLAG      4
#define PAGE_TABLE_WRITETHROUGH_FLAG 8
#define PAGE_TABLE_CACHEDISABLED_FLAG 16
#define PAGE_TABLE_ACCESSED_FLAG  32
#define PAGE_TABLE_SIZE_FLAG      128
#define PAGE_TABLE_FRAME_MASK     0x7FFFF000

typedef uint32_t page_table_t;

struct page_table {
    page_t pt[1024];
};

struct page_directory {
    page_table_t pd[1024];
};

void vmm_init();

void map_kernel(struct page_directory *pdir);

void change_page_directory(struct page_directory *p);
struct page_directory *get_page_directory();
struct page_directory *get_kern_directory();

int vmm_create_page_table(struct page_directory *pdir, vmm_addr_t virt, uint32_t flags);
void vmm_map_phys(struct page_directory *pdir, vmm_addr_t virt, mm_addr_t phys, uint32_t flags);
void *get_phys_addr(struct page_directory *pdir, vmm_addr_t virt);
struct page_directory *create_address_space();
void vmm_unmap_page_table(struct page_directory *pdir, vmm_addr_t virt);
void vmm_unmap_phys_addr(struct page_directory *pdir, vmm_addr_t virt);

#endif

