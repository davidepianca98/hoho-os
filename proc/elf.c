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
#include <string.h>
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

int elf_loader(char *name, process_t *proc) {
    file f;
    
    struct page_directory *addr_space = create_address_space();
    if(!addr_space) {
        printk("Failed finding address space\n");
        return 0;
    }
    
    f = vfs_file_open(name);
    if((f.flags == FS_NULL) || ((f.flags & FS_DIR) == FS_DIR)) {
        printk("Failed opening file\n");
        return 0;
    }

    uint32_t i = 0;
    char *memory = (char *) pmm_malloc();
    char *buf = (char *) pmm_malloc();
    memset(memory, 0, 4096);
    while(f.eof != 1) {
        vfs_file_read(&f, buf + (i * 512));
        i++;
    }
    vfs_file_close(&f);
    elf_header_t *eh = (elf_header_t *) buf;
    if(!elf_validate(eh)) {
        printk("Failed validating elf\n");
        return 0;
    }
    
    program_header_t *ph = (program_header_t *) ((uint32_t) eh + eh->program_header);
    
    proc->pdir = addr_space;
    proc->eip = eh->entry;
    
    uint32_t totsize = 0;
    uint32_t memsize, filesize, vaddr, offset;
    for(i = 0; i < eh->entry_number_prog_header; i++) {
        if(ph[i].p_type == 1) {
            memsize = ph[i].p_mem_size;
            filesize = ph[i].p_file_size;
            vaddr = ph[i].p_vaddr;
            offset = ph[i].p_offset;
            //printk("%d %d 0x%x\n", memsize, filesize, vaddr);
            if(memsize == 0)
                continue;
            //printk("%x\n", memory+totsize);
            
            if(i == 0)
                proc->image_base = vaddr;
            vmm_map_phys(proc->pdir, vaddr, (uint32_t) memory, PAGE_PRESENT_FLAG | PAGE_RW_FLAG | PAGE_MODE_FLAG);
            memcpy((uint32_t *) vaddr, (uint32_t *) ((uint32_t) buf + offset), filesize);
            memset((void *) vaddr + filesize, 0, memsize - filesize);
            totsize += memsize;
        }
    }
    
    proc->image_size = totsize;
    
    // build the stack
    void *stack = (void *) (proc->image_base + proc->image_size + PAGE_SIZE);
    void *stack_phys = (void *) pmm_malloc();
    
    vmm_map_phys(proc->pdir, (uint32_t) stack, (uint32_t) stack_phys, PAGE_PRESENT_FLAG | PAGE_RW_FLAG | PAGE_MODE_FLAG);

    proc->esp = (uint32_t) stack;
    proc->stack_limit = ((uint32_t) proc->esp + 4096);
    
    uint32_t *stackp = (uint32_t *) proc->stack_limit;
    *--stackp = (uint32_t) &end_proc; // the process needs to know where to return
    *--stackp = 0x23;                 // ss
    *--stackp = proc->stack_limit - 4;// esp
    *--stackp = 0x202;                // eflags
    *--stackp = 0x1B;                 // cs
    *--stackp = proc->eip;            // eip
    *--stackp = 0;                    // eax
    *--stackp = 0;                    // ebx
    *--stackp = 0;                    // ecx
    *--stackp = 0;                    // edx
    *--stackp = 0;                    // esi
    *--stackp = 0;                    // edi
    *--stackp = proc->esp + 4096;     // ebp
    *--stackp = 0x23;                 // ds
    *--stackp = 0x23;                 // es
    *--stackp = 0x23;                 // fs
    *--stackp = 0x23;                 // gs
    proc->esp = (uint32_t) stackp;

    return 1;
}

