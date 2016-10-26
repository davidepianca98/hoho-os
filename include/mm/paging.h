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

#define PAGE_PRESENT        0x1
#define PAGE_RW             0x2
#define PAGE_USER           0x4
#define PAGE_ACCESSED       0x20
#define PAGE_FRAME_MASK     0x7FFFF000

typedef uint32_t page_dir_t;

#define PAGEDIR_SIZE        1024

void vmm_init();

void map_kernel(page_dir_t *pdir);

void change_page_directory(page_dir_t *p);
page_dir_t *get_page_directory();
page_dir_t *get_kern_directory();

int vmm_create_page_table(page_dir_t *pdir, vmm_addr_t virt, uint32_t flags);
int vmm_map(page_dir_t *pdir, vmm_addr_t virt, uint32_t flags);
int vmm_map_phys(page_dir_t *pdir, vmm_addr_t virt, mm_addr_t phys, uint32_t flags);
void *get_phys_addr(page_dir_t *pdir, vmm_addr_t virt);
page_dir_t *create_address_space();
void delete_address_space(page_dir_t *pdir);
void vmm_unmap_page_table(page_dir_t *pdir, vmm_addr_t virt);
void vmm_unmap(page_dir_t *pdir, vmm_addr_t virt);
void vmm_unmap_phys(page_dir_t *pdir, vmm_addr_t virt);

void *page_table_malloc();
void paging_set_bit(int bit);
void paging_unset_bit(int bit);
int paging_first_free();
void page_table_free(void *addr);
uint32_t *get_page_table_bitmap();

#endif

