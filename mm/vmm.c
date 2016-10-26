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
#include <mm/memory.h>
#include <lib/string.h>
#include <drivers/video.h>

/*
 * |------------------------------------------------|
 * | 0x0 - 0x400000 -> identity mapped kernel space |
 * | kernel_end - 0x200000 -> kernel heap           |
 * | 0x200000 - 0x400000 -> paging structures       |
 * | -----------------------------------------------|
 * | 0x400000 - 0x700000 -> free space              |
 * |------------------------------------------------|
 * | 0x700000 - 0x800000 -> elf loading space       |
 * |------------------------------------------------|
 * | 0x800000 - end -> programs address space       |
 * |------------------------------------------------|
 */

page_dir_t kern_dir[1024] __attribute__((aligned(4096)));
page_dir_t *current_dir = 0;

extern uint32_t kernel_start;
extern uint32_t kernel_end;

/**
 * Initializes the Virtual Memory Manager
 */
void vmm_init() {
    memset(kern_dir, 0, PAGEDIR_SIZE);
    memset((void *) get_page_table_bitmap(), 0, 0x10);
    map_kernel(kern_dir);
    change_page_directory(kern_dir);
    enable_paging();
}

/**
 * Maps the kernel in the given page directory
 */
void map_kernel(page_dir_t *pdir) {
    vmm_addr_t virt = 0x00000000;
    mm_addr_t phys = 0x0;
    
    // Identity map first 4MB
    for(int i = 0; i < 1024; i++, virt += PAGE_SIZE, phys += PAGE_SIZE) {
        vmm_map_phys(pdir, virt, phys, PAGE_PRESENT | PAGE_RW);
    }
}

/**
 * Switches page directory with the given one
 */
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

/**
 * Creates a page table for the given virtual address
 */
int vmm_create_page_table(page_dir_t *pdir, vmm_addr_t virt, uint32_t flags) {
    void *pt = page_table_malloc();
    if(!pt)
        return NULL;
    pdir[virt >> 22] = ((uint32_t) pt) | flags;
    return 1;
}

/**
 * Allocates a chunk of memory and maps it to the virtual address
 */
int vmm_map(page_dir_t *pdir, vmm_addr_t virt, uint32_t flags) {
    // Get a memory block
    mm_addr_t phys = (mm_addr_t) pmm_malloc();
    if(!phys) {
        printk("VMM: Failed allocating memory %x\n", phys);
        return NULL;
    }
    
    // If the page table is not present, create it
    if(!pdir[virt >> 22]) {
        if(!vmm_create_page_table(pdir, virt, flags)) {
            return NULL;
        }
    }
    // Map the address to the page table
    // Use the virtual address to get the index in the page directory and keep only the first 12 bits
    // which is the page table and use the virtual address to find the index in the page table
    ((uint32_t *) (pdir[virt >> 22] & ~0xFFF))[virt << 10 >> 10 >> 12] = phys | flags;
    flush_tlb(virt);
    return 1;
}

/**
 * Maps the physical address to the virtual one
 */
int vmm_map_phys(page_dir_t *pdir, vmm_addr_t virt, mm_addr_t phys, uint32_t flags) {
    // If the page table is not present, create it
    if(pdir[virt >> 22] == 0) {
        if(!vmm_create_page_table(pdir, virt, flags)) {
            return NULL;
        }
    }
    // Map the address to the page table
    // Use the virtual address to get the index in the page directory and keep only the first 12 bits
    // which is the page table and use the virtual address to find the index in the page table
    ((uint32_t *) (pdir[virt >> 22] & ~0xFFF))[virt << 10 >> 10 >> 12] = phys | flags;
    flush_tlb(virt);
    return 1;
}

/**
 * Gets the physical address from the given virtual address
 */
void *get_phys_addr(page_dir_t *pdir, vmm_addr_t virt) {
    if(pdir[virt >> 22] == NULL)
        return NULL;
    return (void *) (((uint32_t *) (pdir[virt >> 22] & ~0xFFF))[virt << 10 >> 10 >> 12] >> 12 << 12);
}

/**
 * Creates a page directory to be used with a process
 */
page_dir_t *create_address_space() {
    // Allocate space for a page directory
    page_dir_t *pdir = (page_dir_t *) page_table_malloc();
    if(!pdir)
        return NULL;
    // Clone page directory
    memcpy(pdir, kern_dir, PAGEDIR_SIZE);
    return pdir;
}

/**
 * Deletes a page directory
 */
void delete_address_space(page_dir_t *pdir) {
    for(int i = 0; i < PAGEDIR_SIZE; i++) {
        if(pdir[i * PAGE_SIZE >> 22]) {
            vmm_unmap_page_table(pdir, i * PAGE_SIZE);
        }
    }
}

/**
 * Unmaps the page table and frees the memory block
 */
void vmm_unmap_page_table(page_dir_t *pdir, vmm_addr_t virt) {
    void *frame = (void *) (pdir[virt >> 22] & PAGE_FRAME_MASK);
    page_table_free(frame);
    pdir[virt >> 22] = NULL;
    flush_tlb(virt);
}

/**
 * Unmaps the physical address from the virtual and deallocates memory
 */
void vmm_unmap(page_dir_t *pdir, vmm_addr_t virt) {
    if(pdir[virt >> 22] != NULL) {
        void *addr = get_phys_addr(pdir, virt);
        if(addr) {
            pmm_free(addr);
            ((uint32_t *) (pdir[virt >> 22] & ~0xFFF))[virt << 10 >> 10 >> 12] = 0;
        } else {
            printk("Error unmapping memory\n");
        }
    }
}

/**
 * Unmaps a physical address from the virtual
 */
void vmm_unmap_phys(page_dir_t *pdir, vmm_addr_t virt) {
    if(pdir[virt >> 22] != NULL) {
        ((uint32_t *) (pdir[virt >> 22] & ~0xFFF))[virt << 10 >> 10 >> 12] = 0;
    }
}

