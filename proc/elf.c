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

#include <proc/proc.h>
#include <mm/memory.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <elf.h>
#include <drivers/video.h>

#define MEMORY_LOAD_ADDRESS 0x700000

/**
 * Checks if the file can be executed in this OS
 */
int elf_validate(elf_header_t *eh) {
    if(eh == NULL)
        return 0;
    
    if(!((eh->magic[0] == 0x7F) && (eh->magic[1] == 'E') && (eh->magic[2] == 'L') && (eh->magic[3] == 'F'))) {
        printk("Magic number wrong\n");
        return 0;
    }
    
    if(eh->instruction_set != ELF_X86) {
        printk("Not an x86 executable\n");
        return 0;
    }
    
    if(eh->arch != ELF_32BIT) {
        printk("Not a 32 bit executable\n");
        return 0;
    }
    
    if(eh->machine != ELF_LITTLE_ENDIAN) {
        printk("Not a little endian executable\n");
        return 0;
    }
    
    if(eh->elf_version2 != 1) {
        printk("Wrong ELF version\n");
        return 0;
    }
    return 1;
}

/**
 * Loads an ELF executable in memory and partially builds threads' info
 */
int load_elf(char *name, thread_t *thread, page_dir_t *pdir) {
    // Load the file into memory
    uint32_t file_size = load_elf_file(name);
    if(!file_size) {
        printk("Error loading file\n");
        return -1;
    }
    
    // Check the elf header
    elf_header_t *eh = (elf_header_t *) MEMORY_LOAD_ADDRESS;
    if(!elf_validate(eh)) {
        printk("Failed validating elf\n");
        return -1;
    }
    
    // Relocate executable parts
    if(!load_elf_relocate(thread, pdir, eh)) {
        printk("Error relocating\n");
        return -1;
    }
    
    // Unmap executable from kernel directory
    for(uint32_t i = 0; i < file_size; i++) {
        vmm_unmap(get_kern_directory(), (uint32_t) MEMORY_LOAD_ADDRESS + (i * PAGE_SIZE));
    }
    
    return 1;
}

/**
 * Moves the file from the disk to RAM
 */
int load_elf_file(char *name) {
    // Open the executable
    file f = vfs_file_open(name, "r");
    if((f.type == FS_NULL) || (f.type & FS_DIR)) {
        printk("Failed opening file\n");
        return -1;
    }
    
    // Load the executable in memory
    uint32_t j = 0;
    while(f.eof != 1) {
        // The executable needs more memory, so reserve it
        if(((j + 8) % 8) == 0) {
            if(!vmm_map(get_kern_directory(), (uint32_t) MEMORY_LOAD_ADDRESS + (j * 512), PAGE_PRESENT | PAGE_RW)) {
                printk("Error mapping memory");
                return -1;
            }
        }
        // Copy the file into memory
        vfs_file_read(&f, (char *) MEMORY_LOAD_ADDRESS + (j * 512));
        j++;
    }
    vfs_file_close(&f);
    return j;
}

/**
 * Moves the executable parts to the correct virtual address for execution
 */
int load_elf_relocate(thread_t *thread, page_dir_t *pdir, elf_header_t *eh) {
    // Get the program header
    program_header_t *ph = (program_header_t *) ((uint32_t) eh + eh->program_header);
    // Get the entry point of the program
    thread->eip = eh->entry;
    // Get the base image virtual address
    thread->image_base = ph[0].p_vaddr;
    
    // Relocate the executable program parts into the correct memory locations
    uint32_t i, last;
    for(i = 0; i < eh->entry_number_prog_header; i++) {
        // If the part is executable
        if(ph[i].p_type == 1) {
            if(ph[i].p_mem_size == 0)
                continue;
            
            // Allocate pages for the program executable
            for(uint32_t j = 0; j <= ph[i].p_file_size / PAGE_SIZE; j++) {
                // Map executable in kernel and proc page directory
                if(!vmm_map(get_kern_directory(), ph[i].p_vaddr + (j * PAGE_SIZE), PAGE_PRESENT | PAGE_RW) ||
                   !vmm_map_phys(pdir, ph[i].p_vaddr + (j * PAGE_SIZE), (uint32_t) get_phys_addr(get_kern_directory(), ph[i].p_vaddr), PAGE_PRESENT | PAGE_RW | PAGE_USER)) {
                    printk("Error mapping memory");
                    return -1;
                }
            }
            // Copy the executable into the correct memory location
            memcpy((uint32_t *) ph[i].p_vaddr, (uint32_t *) ((uint32_t) MEMORY_LOAD_ADDRESS + ph[i].p_offset), ph[i].p_file_size);
            memset((void *) ph[i].p_vaddr + ph[i].p_file_size, 0, ph[i].p_mem_size - ph[i].p_file_size);
            // Unmap from kernel directory
            for(uint32_t j = 0; j <= ph[i].p_file_size / PAGE_SIZE; j++) {
                vmm_unmap_phys(get_kern_directory(), ph[i].p_vaddr + (j * PAGE_SIZE));
            }
            last = i;
        }
    }
    // The size of the executable in memory is equal to the virtual address of the last section + the offset - the start
    thread->image_size = ph[last].p_vaddr + ph[last].p_mem_size - thread->eip;
    // Round up the image size
    while((thread->image_size % PAGE_SIZE) != 0) {
        thread->image_size++;
    }
    return 1;
}
