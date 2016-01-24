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
    proc->state = PROC_NEW;

    // Create a new page directory
    proc->pdir = create_address_space();
    if(!proc->pdir) {
        printk("Failed finding address space\n");
        return PROC_STOPPED;
    }
    
    proc->thread_list = create_thread();
    if(proc->thread_list == NULL)
        return PROC_STOPPED;
    proc->thread_list->main = 1;
    proc->thread_list->parent = (void *) proc;

    if(load_elf(name, proc->thread_list, proc->pdir) == -1) {
        sched_state(1);
        return PROC_STOPPED;
    }
    
    if(!build_stack(proc->thread_list, proc->pdir, 0)) {
        sched_state(1);
        return PROC_STOPPED;
    }
    
    if(!build_heap(proc->thread_list, proc->pdir, 0)) {
        sched_state(1);
        return PROC_STOPPED;
    }
    
    // fill the stack
    uint32_t *stackp = (uint32_t *) proc->thread_list->stack_limit;
    
    // arguments
    uint32_t argc = 1;
    char **argv = (char **) umalloc(strlen(name) + strlen(arguments), (vmm_addr_t *) proc->thread_list->heap);
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

    *--stackp = (uint32_t) &end_process;             // the process needs to know where to return
    *--stackp = 0x23;                                // ss
    *--stackp = proc->thread_list->stack_limit - 12; // esp
    *--stackp = 0x202;                               // eflags
    *--stackp = 0x1B;                                // cs
    *--stackp = proc->thread_list->eip;              // eip
    *--stackp = 0;                                   // eax
    *--stackp = 0;                                   // ebx
    *--stackp = 0;                                   // ecx
    *--stackp = 0;                                   // edx
    *--stackp = 0;                                   // esi
    *--stackp = 0;                                   // edi
    *--stackp = proc->thread_list->esp + 4096;       // ebp
    *--stackp = 0x23;                                // ds
    *--stackp = 0x23;                                // es
    *--stackp = 0x23;                                // fs
    *--stackp = 0x23;                                // gs
    proc->thread_list->esp = (uint32_t) stackp;
    
    proc->threads = 1;
    
    proc->thread_list->state = PROC_ACTIVE;
    proc->state = PROC_ACTIVE;
    
    sched_add_proc(proc);
    return proc->thread_list->pid;
}

thread_t *create_thread() {
    thread_t *thread = (thread_t *) kmalloc(sizeof(thread_t));
    thread->pid = pid++;
    thread->main = 0;
    thread->state = PROC_NEW;
    thread->next = thread;
    thread->prec = thread;
    return thread;
}

int load_elf(char *name, thread_t *thread, struct page_directory *pdir) {
    file f;
    
    // Open the executable
    f = vfs_file_open(name, 0);
    if((f.type == FS_NULL) || ((f.type & FS_DIR) == FS_DIR)) {
        printk("Failed opening file\n");
        return -1;
    }

    // Load the executable in memory
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
        return -1;
    }
    
    program_header_t *ph = (program_header_t *) ((uint32_t) eh + eh->program_header);
    
    thread->eip = eh->entry;
    
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
                thread->image_base = vaddr;
            vmm_map_phys(pdir, vaddr, (uint32_t) memory, PAGE_PRESENT_FLAG | PAGE_RW_FLAG | PAGE_MODE_FLAG);
            memcpy((uint32_t *) vaddr, (uint32_t *) ((uint32_t) buf + offset), filesize);
            memset((void *) vaddr + filesize, 0, memsize - filesize);
            totsize += memsize;
        }
    }
    pmm_free((void *) buf);
    
    thread->image_size = totsize;
    return 1;
}

int build_stack(thread_t *thread, struct page_directory *pdir, int nthreads) {
    // build the stack
    void *stack = (void *) thread->image_base + thread->image_size + PAGE_SIZE + (9216 * nthreads);
    void *stack_phys = (void *) pmm_malloc();
    if(stack_phys == NULL)
        return 0;
    
    vmm_map_phys(pdir, (uint32_t) stack, (uint32_t) stack_phys, PAGE_PRESENT_FLAG | PAGE_RW_FLAG | PAGE_MODE_FLAG);

    thread->esp = (uint32_t) stack;
    thread->stack_limit = ((uint32_t) thread->esp + 4096);
    return 1;
}

int build_heap(thread_t *thread, struct page_directory *pdir, int nthreads) {
    // build the heap
    void *heap = (void *) thread->stack_limit + 512 + (9216 * nthreads);
    void *heap_phys = (void *) pmm_malloc();
    if(heap_phys == NULL)
        return 0;
    
    vmm_map_phys(pdir, (uint32_t) heap, (uint32_t) heap_phys, PAGE_PRESENT_FLAG | PAGE_RW_FLAG | PAGE_MODE_FLAG);
    
    thread->heap = (uint32_t) heap;
    thread->heap_limit = ((uint32_t) heap + 4096);
    
    heap_init((vmm_addr_t *) heap);
    return 1;
}

/* Creates a new thread (fork) */
int start_thread(uint32_t eip) {
    sched_state(0);
    process_t *cur = get_cur_proc();
    if(cur->thread_list->pid == 1)
        return -1;

    thread_t *thread = create_thread();

    // Copy executable info
    thread->image_base = cur->thread_list->image_base;
    thread->image_size = cur->thread_list->image_size;
    thread->parent = (void *) cur;
    
    if(!build_stack(thread, cur->pdir, cur->threads))
        return -1;
    
    if(!build_heap(thread, cur->pdir, cur->threads))
        return -1;

    // fill the stack
    uint32_t *stackp = (uint32_t *) thread->stack_limit;
    *--stackp = 0x23;                     // ss
    *--stackp = thread->stack_limit;      // esp
    *--stackp = 0x202;                    // eflags
    *--stackp = 0x1B;                     // cs
    *--stackp = eip;                      // eip
    *--stackp = 0;                        // eax
    *--stackp = 0;                        // ebx
    *--stackp = 0;                        // ecx
    *--stackp = 0;                        // edx
    *--stackp = 0;                        // esi
    *--stackp = 0;                        // edi
    *--stackp = thread->esp + 4096;       // ebp
    *--stackp = 0x23;                     // ds
    *--stackp = 0x23;                     // es
    *--stackp = 0x23;                     // fs
    *--stackp = 0x23;                     // gs
    thread->esp = (uint32_t) stackp;
    
    thread->eip = eip;
    
    cur->threads++;
    
    thread->prec = cur->thread_list;
    thread->next = cur->thread_list->next;
    cur->thread_list->next->prec = thread;
    cur->thread_list->next = thread;
    
    thread->state = PROC_ACTIVE;
    sched_state(1);
    return 1;
}

void stop_thread(int code) {
    sched_state(0);

    process_t *cur = get_cur_proc();
    if(cur == NULL) {
        printk("Process not found\n");
        sched_state(1);
        return;
    }
    
    // Terminating the main thread will terminate the process
    if(cur->thread_list->main == 1)
        end_proc(code);
    
    cur->thread_list->next->prec = cur->thread_list->prec;
    cur->thread_list->prec->next = cur->thread_list->next;
    
    void *stack = get_phys_addr(cur->pdir, cur->thread_list->esp);
    vmm_unmap_phys_addr(cur->pdir, cur->thread_list->esp);
    pmm_free(stack);
    
    void *heap = get_phys_addr(cur->pdir, cur->thread_list->heap);
    vmm_unmap_phys_addr(cur->pdir, cur->thread_list->heap);
    pmm_free(heap);
    
    for(uint32_t page = 0; page < cur->thread_list->image_size / PAGE_SIZE; page++) {
        uint32_t virt = cur->thread_list->image_base + (page * PAGE_SIZE);

        uint32_t phys = (uint32_t) get_phys_addr(cur->pdir, virt);

        vmm_unmap_phys_addr(cur->pdir, virt);
        pmm_free((void *) phys);
    }
    uint32_t *phys = get_phys_addr(cur->pdir, (vmm_addr_t) cur->pdir);
    pmm_free(phys);
    
    sched_state(1);
    enable_int();
}

void end_proc(int ret) {
    sched_state(0);

    process_t *cur = get_cur_proc();
    if(cur == NULL) {
        printk("Process not found\n");
        sched_state(1);
        enable_int();
        return;
    }
    
    if(ret)
        printk("The process %d returned with error: %d\n", cur->thread_list->pid, ret);
    
    cur->state = PROC_STOPPED;
    
    for(int i = 0; i < cur->threads; i++) {
        void *stack = get_phys_addr(cur->pdir, cur->thread_list->esp);
        vmm_unmap_phys_addr(cur->pdir, cur->thread_list->esp);
        pmm_free(stack);
    
        void *heap = get_phys_addr(cur->pdir, cur->thread_list->heap);
        vmm_unmap_phys_addr(cur->pdir, cur->thread_list->heap);
        pmm_free(heap);
    
        for(uint32_t page = 0; page < cur->thread_list->image_size / PAGE_SIZE; page++) {
            uint32_t virt = cur->thread_list->image_base + (page * PAGE_SIZE);

            uint32_t phys = (uint32_t) get_phys_addr(cur->pdir, virt);

		    vmm_unmap_phys_addr(cur->pdir, virt);
		    pmm_free((void *) phys);
	    }
	    uint32_t *phys = get_phys_addr(cur->pdir, (vmm_addr_t) cur->pdir);
	    pmm_free(phys);
	    cur->thread_list = cur->thread_list->next;
	}
	//sched_remove_proc(cur->id);
    sched_state(1);
    enable_int();
    while(1);
}

int proc_state(int id) {
    process_t *cur = get_proc_by_id(id);
    return cur->state;
}

