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

page_dir_t *kern_dir = 0;
page_dir_t *current_dir = 0;

extern uint32_t kernel_start;
extern uint32_t kernel_end;

void vmm_init() {
    kern_dir = create_address_space();
    map_kernel(kern_dir);
    change_page_directory(kern_dir);
    //enable_paging();
}

void map_kernel(page_dir_t *pdir) {
    vmm_addr_t virt = 0x00000000;
    mm_addr_t phys = 0x0;
    
    // identity map first 1MB and kernel
    while(phys < ((uint32_t) &kernel_end + PAGE_SIZE * 64)) { // should be without * 64 TODO fix
        vmm_map_phys(pdir, virt, phys, PAGE_PRESENT | PAGE_RW);
        virt += PAGE_SIZE;
        phys += PAGE_SIZE;
    }
    
    vmm_map_phys(pdir, (uint32_t) pdir, (uint32_t) pdir, PAGE_PRESENT | PAGE_RW);
}

void change_page_directory(page_dir_t *p) {
    current_dir = p;
    load_pdbr((mm_addr_t) current_dir);
}

page_dir_t *get_page_directory() {
    return current_dir;
}

page_dir_t *get_kern_directory() {
    return kern_dir;
}

int vmm_create_page_table(page_dir_t *pdir, vmm_addr_t virt, uint32_t flags) {
    if(pdir[virt >> 22] == 0) {
        void *block = pmm_malloc();
        if(!block)
            return 0;
        pdir[virt >> 22] = ((uint32_t) block) | flags;
        memset((uint32_t *) pdir[virt >> 22], 0, PAGE_SIZE);
        vmm_map_phys(pdir, (vmm_addr_t) block, (mm_addr_t) block, flags);
    } else {
        pdir[virt >> 22] |= flags;
    }
    return 1;
}

int vmm_map_phys(page_dir_t *pdir, vmm_addr_t virt, mm_addr_t phys, uint32_t flags) {
    // if the physical address isn't given, we allocate a chunk of memory
    if(phys == 0)
        phys = (mm_addr_t) pmm_malloc();
    if(phys == 0)
        return NULL;
    // if the page table is not present, create it
    if(pdir[virt >> 22] == 0)
        if(!vmm_create_page_table(pdir, virt, flags))
            return NULL;
    ((uint32_t *) (pdir[virt >> 22] & ~0xFFF))[virt << 10 >> 10 >> 12] = phys | flags;
    return 1;
}

void *get_phys_addr(page_dir_t *pdir, vmm_addr_t virt) {
    if(pdir[virt >> 22] == 0)
        return 0;
    return (void *) ((uint32_t *) (pdir[virt >> 22] & ~0xFFF))[virt << 10 >> 10 >> 12];
}

page_dir_t *create_address_space() {
    // allocate space for a page directory
    page_dir_t *pdir = (page_dir_t *) pmm_malloc();
    if(!pdir)
        return 0;
    vmm_map_phys(get_page_directory(), (uint32_t) pdir, (uint32_t) pdir, PAGE_PRESENT | PAGE_RW);
    memset(pdir, 0, PAGEDIR_SIZE);
    if(kern_dir != 0) {
        // clone page directory
        memcpy(pdir, kern_dir, PAGEDIR_SIZE);
    }
    return pdir;
}

void vmm_unmap_page_table(page_dir_t *pdir, vmm_addr_t virt) {
    if(pdir[virt >> 22] != 0) {
        void *frame = (void *) (pdir[virt >> 22] & PAGE_FRAME_MASK);
        pmm_free(frame);
        pdir[virt >> 22] = 0;
    }
}

void vmm_unmap_phys_addr(page_dir_t *pdir, vmm_addr_t virt) {
    if(pdir[virt >> 22] != 0)
        //vmm_unmap_page_table(pdir, virt);
        ((uint32_t *) (pdir[virt >> 22] & ~0xFFF))[virt << 10 >> 10 >> 12] = 0;
}

