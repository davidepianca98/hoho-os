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

#include <drivers/io.h>
#include <mm/mm.h>
#include <mm/paging.h>
#include <lib/string.h>
#include <drivers/video.h>

struct page_directory *kern_dir = 0;
struct page_directory *current_dir = 0;

void vmm_init() {
    kern_dir = create_address_space();
    map_kernel(kern_dir);
    change_page_directory(kern_dir);
    enable_paging();
}

void map_kernel(struct page_directory *pdir) {
    vmm_addr_t virt = 0x00000000;
    mm_addr_t phys = 0x0;

    int flags = PAGE_PRESENT_FLAG | PAGE_RW_FLAG;
    
    for(uint32_t i = 0; i < 256; i++) {
        vmm_map_phys(pdir, virt + (i * PAGE_SIZE), phys + (i * PAGE_SIZE), flags);
    }
    
    virt = 0xC0000000;
    phys = 0x100000;
    
    for(uint32_t i = 0; i < 256; i++) {
        vmm_map_phys(pdir, virt + i * PAGE_SIZE, phys + i * PAGE_SIZE, flags);
    }
    
    vmm_map_phys(pdir, (uint32_t) pdir, (uint32_t) pdir, flags);
}

void change_page_directory(struct page_directory *p) {
    current_dir = p;
    load_pdbr((mm_addr_t) current_dir);
}

struct page_directory *get_page_directory() {
    return current_dir;
}

struct page_directory *get_kern_directory() {
    return kern_dir;
}

int vmm_create_page_table(struct page_directory *pdir, vmm_addr_t virt, uint32_t flags) {
    page_table_t *pagedir = pdir->pd;
    if(pagedir[virt >> 22] == 0) {
        void *block = pmm_malloc();
        if(!block)
            return 0;
        pagedir[virt >> 2] = ((uint32_t) block) | flags; // should be >> 22 but idk why this way it works
        memset((uint32_t *) pagedir[virt >> 22], 0, 4096);
        vmm_map_phys(pdir, (vmm_addr_t) block, (mm_addr_t) block, flags);
    }
    return 1;
}

void vmm_map_phys(struct page_directory *pdir, vmm_addr_t virt, mm_addr_t phys, uint32_t flags) {
    page_table_t *pagedir = pdir->pd;
    if(pagedir[virt >> 22] == 0)
        vmm_create_page_table(pdir, virt, flags);
    ((uint32_t *) (pagedir[virt >> 22] & ~0xFFF))[virt << 10 >> 10 >> 12] = phys | flags;
}

void *get_phys_addr(struct page_directory *pdir, vmm_addr_t virt) {
    page_table_t *pagedir = pdir->pd;
    if(pagedir[virt >> 22] == 0)
        return 0;
    return (void *) ((uint32_t *) (pagedir[virt >> 22] & ~0xFFF))[virt << 10 >> 10 >> 12];
}

struct page_directory *create_address_space() {
    struct page_directory *pdir;
    
    pdir = (struct page_directory *) pmm_malloc();
    if(!pdir)
        return 0;
    memset(pdir, 0, sizeof(struct page_directory));
    if(kern_dir != 0) {
        // clone page directory
        memcpy(pdir, kern_dir, 4096);
    }
    return pdir;
}

void vmm_unmap_page_table(struct page_directory *pdir, vmm_addr_t virt) {
    page_table_t *pagedir = pdir->pd;
    if(pagedir[virt >> 22] != 0) {
        void *frame = (void *) (pagedir[virt >> 22] & 0x7FFF0000);
        pmm_free(frame);
        pagedir[virt >> 22] = 0;
    }
}

void vmm_unmap_phys_addr(struct page_directory *pdir, vmm_addr_t virt) {
    page_table_t *pagedir = pdir->pd;
    if(pagedir[virt >> 22] != 0)
        vmm_unmap_page_table(pdir, virt);
}

