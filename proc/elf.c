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
        printk("Not an x86 machine\n");
        return 0;
    }
    
    if(eh->arch != ELF_32BIT) {
        printk("Not a 32 bit executable\n");
        return 0;
    }
    
    if(eh->machine != ELF_LITTLE_ENDIAN) {
        printk("We only support little endian\n");
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
    // Open the executable
    file f = vfs_file_open(name, 0);
    if((f.type == FS_NULL) || (f.type & FS_DIR)) {
        printk("Failed opening file\n");
        return -1;
    }

    // Reserve some memory for the executable
    char *vbuf = (char *) pmm_malloc();
    if(!vbuf) {
        printk("Failed allocating memory\n");
        return -1;
    }
    vmm_map_phys(get_page_directory(), (uint32_t) MEMORY_LOAD_ADDRESS, (uint32_t) vbuf, PAGE_PRESENT | PAGE_RW);
    
    // Load the executable in memory
    uint32_t file_size = 0;
    while(f.eof != 1) {
        // Copy the file into memory
        vfs_file_read(&f, (char *) MEMORY_LOAD_ADDRESS + (file_size * 512));
        file_size++;
        
        // The executable needs more memory, so reserve it
        if((file_size % 8) == 0) {
            char *file_mem = (char *) pmm_malloc();
            if(!file_mem) {
                printk("Failed allocating memory\n");
                return -1;
            }
            vmm_map_phys(get_page_directory(), (uint32_t) MEMORY_LOAD_ADDRESS + (PAGE_SIZE * (file_size / 8)), (uint32_t) file_mem, PAGE_PRESENT | PAGE_RW);
        }
    }
    vfs_file_close(&f);
    
    // Check the elf header
    elf_header_t *eh = (elf_header_t *) MEMORY_LOAD_ADDRESS;
    if(!elf_validate(eh)) {
        printk("Failed validating elf\n");
        return -1;
    }
    
    // Get the program header
    program_header_t *ph = (program_header_t *) ((uint32_t) eh + eh->program_header);
    // Get the entry point of the program
    thread->eip = eh->entry;
    // Get the base image virtual address
    thread->image_base = ph[0].p_vaddr;
    
    // Relocate the executable program parts into the correct memory locations
    int index = 0;
    for(uint32_t i = 0; i < eh->entry_number_prog_header; i++) {
        // If the part is executable
        if(ph[i].p_type == 1) {
            if(ph[i].p_mem_size == 0)
                continue;
            
            for(uint32_t j = 0; j < (ph[i].p_file_size / PAGE_SIZE) + 1; j++) {
                // Reserve some memory
                char *exec_mem = (char *) pmm_malloc();
                if(!exec_mem) {
                    printk("Failed allocating memory\n");
                    return -1;
                }
                // Map executable in proc page directory
                vmm_map_phys(pdir, ph[i].p_vaddr + (j * PAGE_SIZE), (uint32_t) exec_mem, PAGE_PRESENT | PAGE_RW | PAGE_USER);

                // Map executable in kernel page directory
                vmm_map_phys(get_page_directory(), ph[i].p_vaddr + (j * PAGE_SIZE), (uint32_t) get_phys_addr(pdir, ph[i].p_vaddr), PAGE_PRESENT | PAGE_RW);
            }
            // Copy the executable into correct memory
            memcpy((uint32_t *) ph[i].p_vaddr, (uint32_t *) ((uint32_t) vbuf + ph[i].p_offset), ph[i].p_file_size);
            memset((void *) ph[i].p_vaddr + ph[i].p_file_size, 0, ph[i].p_mem_size - ph[i].p_file_size);
            index = i;
        }
    }
    // The size of the executable in memory is equal to the virtual address of the last section + the offset
    // also round up
    thread->image_size = (ph[index].p_vaddr + ph[index].p_file_size - thread->eip + PAGE_SIZE) >> 12 << 12;
    
    // Unmap executable from kernel directory
    for(uint32_t i = 0; i < file_size; i++) {
        vmm_unmap_phys_addr(get_page_directory(), (uint32_t) thread->image_base + (i * PAGE_SIZE));
        void *buf = get_phys_addr(get_page_directory(), (uint32_t) vbuf + (i + PAGE_SIZE));
        pmm_free(buf);
        vmm_unmap_phys_addr(get_page_directory(), (uint32_t) vbuf + (i * PAGE_SIZE));
    }
    
    return 1;
}

