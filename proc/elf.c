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

/* Loads an ELF executable in memory and partially builds threads' info */
int load_elf(char *name, thread_t *thread, page_dir_t *pdir) {
    // Open the executable
    file f = vfs_file_open(name, 0);
    if((f.type == FS_NULL) || (f.type & FS_DIR)) {
        printk("Failed opening file\n");
        return -1;
    }

    // Load the executable in memory
    uint32_t i = 0;
    char *vbuf = (char *) 0x700000; // TODO fix this
    vmm_map_phys(get_page_directory(), (uint32_t) vbuf, 0, PAGE_PRESENT | PAGE_RW);
    while(f.eof != 1) {
        vfs_file_read(&f, vbuf + (i * 512));
        i++;
        if((i * 512) >= PAGE_SIZE) {
            vmm_map_phys(get_page_directory(), (uint32_t) vbuf + (PAGE_SIZE * (i / 8)), 0, PAGE_PRESENT | PAGE_RW);
        }
    }
    vfs_file_close(&f);
    
    elf_header_t *eh = (elf_header_t *) vbuf;
    if(!elf_validate(eh)) {
        printk("Failed validating elf\n");
        return -1;
    }
    
    program_header_t *ph = (program_header_t *) ((uint32_t) eh + eh->program_header);
    
    thread->eip = eh->entry;
    
    int index = 0;
    for(i = 0; i < eh->entry_number_prog_header; i++) {
        if(ph[i].p_type == 1) {
            //printk("0x%x 0x%x 0x%x\n", ph[i].p_mem_size, ph[i].p_file_size, ph[i].p_vaddr);
            if(ph[i].p_mem_size == 0)
                continue;
            
            if(i == 0)
                thread->image_base = ph[i].p_vaddr;
            
            for(uint32_t j = 0; j < (ph[i].p_file_size / PAGE_SIZE) + 1; j++) {
                // map executable in proc page directory
                vmm_map_phys(pdir, ph[i].p_vaddr + (j * PAGE_SIZE), 0, PAGE_PRESENT | PAGE_RW | PAGE_USER);

                // map executable in kernel page directory
                vmm_map_phys(get_page_directory(), ph[i].p_vaddr + (j * PAGE_SIZE), (uint32_t) get_phys_addr(pdir, ph[i].p_vaddr), PAGE_PRESENT | PAGE_RW);
            }
            // copy the executable into correct memory
            memcpy((uint32_t *) ph[i].p_vaddr, (uint32_t *) ((uint32_t) vbuf + ph[i].p_offset), ph[i].p_file_size);
            memset((void *) ph[i].p_vaddr + ph[i].p_file_size, 0, ph[i].p_mem_size - ph[i].p_file_size);
            index = i;
        }
    }
    // the size of the executable in memory is equal to the virtual address of the last section + the offset
    // also round up
    thread->image_size = (ph[index].p_vaddr + ph[index].p_file_size - thread->eip + PAGE_SIZE) >> 12 << 12;
    
    // unmap executable from kernel directory
    for(i = 0; i < thread->image_size / PAGE_SIZE; i++) {
        vmm_unmap_phys_addr(get_page_directory(), (uint32_t) thread->image_base + (i * PAGE_SIZE));
        void *buf = get_phys_addr(get_page_directory(), (uint32_t) vbuf + (i + PAGE_SIZE));
        pmm_free(buf);
        vmm_unmap_phys_addr(get_page_directory(), (uint32_t) vbuf + (i * PAGE_SIZE));
    }
    
    return 1;
}

