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
#include <drivers/io.h>
#include <hal/tss.h>
#include <elf.h>
#include <drivers/video.h>
#include <proc/sched.h>
#include <hal/hal.h>
#include <lib/string.h>

static int pid = 2;

int start_proc(char *name, char *arguments) {
    process_t *proc = (process_t *) kmalloc(sizeof(process_t));
    strcpy(proc->name, name);
    proc->id = pid++;
    proc->priority = 1;
    proc->state = PROC_NEW;

    file f;
    
    struct page_directory *addr_space = create_address_space();
    if(!addr_space) {
        printk("Failed finding address space\n");
        return 0;
    }
    
    f = vfs_file_open(name, 0);
    if((f.type == FS_NULL) || ((f.type & FS_DIR) == FS_DIR)) {
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
    
    // build the heap
    void *heap = (void *) proc->stack_limit + 512;
    void *heap_phys = (void *) pmm_malloc();
    
    vmm_map_phys(proc->pdir, (uint32_t) heap, (uint32_t) heap_phys, PAGE_PRESENT_FLAG | PAGE_RW_FLAG | PAGE_MODE_FLAG);
    
    proc->heap = (uint32_t) heap;
    proc->heap_limit = ((uint32_t) heap + 4096);
    
    heap_init((vmm_addr_t *) heap);
    
    // fill the stack
    uint32_t *stackp = (uint32_t *) proc->stack_limit;
    
    // arguments
    uint32_t argc = 1;
    char **argv = (char **) umalloc(strlen(name) + strlen(arguments), (vmm_addr_t *) heap);
    strcpy(argv[0], name);
    
    while(*arguments) {
        char *p = strchr(arguments, ' ');
        if(p == NULL)
            break;
        int strl = strlen(arguments) - strlen(p);
        strncpy(argv[argc], arguments, strl);
        argc++;
        while(strl > 0) {
            arguments++;
            strl--;
        }
        arguments++;
    }
    
    *--stackp = (uint32_t) argv;
    *--stackp = argc;

    *--stackp = (uint32_t) &end_process; // the process needs to know where to return
    *--stackp = 0x23;                 // ss
    *--stackp = proc->stack_limit - 12; // esp
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
    
    proc->state = PROC_ACTIVE;
    
    sched_add_proc(proc);
    return proc->id;
}

void end_proc(int ret) {
    sched_state(0);
    if(ret)
        printk("The process returned with error: %d\n", ret);
    process_t *cur = get_cur_proc();
    if((cur == NULL) || (cur->id == PROC_NULL)) {
        printk("Process not found\n");
        sched_state(1);
        return;
    }
    
    cur->state = PROC_STOPPED;
    
    void *stack = get_phys_addr(cur->pdir, cur->esp);
    vmm_unmap_phys_addr(cur->pdir, cur->esp);
    pmm_free(stack);
    
    void *heap = get_phys_addr(cur->pdir, cur->heap);
    vmm_unmap_phys_addr(cur->pdir, cur->heap);
    pmm_free(heap);
    
    for(uint32_t page = 0; page < cur->image_size / PAGE_SIZE; page++) {
        uint32_t virt = cur->image_base + (page * PAGE_SIZE);

        uint32_t phys = (uint32_t) get_phys_addr(cur->pdir, virt);

		vmm_unmap_phys_addr(cur->pdir, virt);
		pmm_free((void *) phys);
	}
	uint32_t *phys = get_phys_addr(cur->pdir, (vmm_addr_t) cur->pdir);
	pmm_free(phys);
	//sched_remove_proc(cur->id);
    sched_state(1);
    while(1);
}

int proc_state(int id) {
    process_t *cur = get_proc_by_id(id);
    return cur->state;
}

