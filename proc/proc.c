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

/**
 * Starts a new process
 */
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
        printk("Failed loading file\n");
        sched_state(1);
        return PROC_STOPPED;
    }
    
    if(!build_stack(proc->thread_list, proc->pdir, 0)) {
        printk("Failed allocating memory, error 1\n");
        sched_state(1);
        return PROC_STOPPED;
    }
    
    if(!build_heap(proc->thread_list, proc->pdir, 0)) {
        printk("Failed allocating memory, error 2\n");
        sched_state(1);
        return PROC_STOPPED;
    }
    
    uint32_t argc, argv;
    
    if(!heap_fill(proc->thread_list, name, arguments, &argc, &argv)) {
        printk("Failed allocating memory, error 3\n");
        sched_state(1);
        return PROC_STOPPED;
    }
    
    if(!stack_fill(proc->thread_list, argc, argv)) {
        printk("Failed allocating memory, error 4\n");
        sched_state(1);
        return PROC_STOPPED;
    }
    
    proc->threads = 1;
    
    proc->thread_list->state = PROC_ACTIVE;
    proc->state = PROC_ACTIVE;
    
    sched_add_proc(proc);
    return proc->thread_list->pid;
}

/**
 *Builds the stack for a thread
 */
int build_stack(thread_t *thread, page_dir_t *pdir, int nthreads) {
    // build the user stack
    void *stack = (void *) (thread->image_base + thread->image_size + (PAGE_SIZE * 3 * nthreads));

    thread->esp = (uint32_t) stack;
    thread->stack_limit = ((uint32_t) thread->esp + PAGE_SIZE);
    
    void *mem = pmm_malloc();
    if(!mem) {
        printk("Failed allocating memory\n");
        return -1;
    }
    vmm_map_phys(pdir, thread->esp, (mm_addr_t) mem, PAGE_PRESENT | PAGE_RW | PAGE_USER);
    vmm_map_phys(get_page_directory(), thread->esp, (uint32_t) get_phys_addr(pdir, thread->esp), PAGE_PRESENT | PAGE_RW);
    
    // build the kernel stack
    thread->esp_kernel = thread->stack_limit;
    thread->stack_kernel_limit = thread->esp_kernel + PAGE_SIZE;
    
    mem = pmm_malloc();
    if(!mem) {
        printk("Failed allocating memory\n");
        return -1;
    }
    vmm_map_phys(pdir, thread->esp_kernel, (mm_addr_t) mem, PAGE_PRESENT | PAGE_RW | PAGE_USER);
    vmm_map_phys(get_page_directory(), thread->esp_kernel, (uint32_t) get_phys_addr(pdir, thread->esp_kernel), PAGE_PRESENT | PAGE_RW);
    
    return 1;
}

/**
 * Builds the heap for a userspace thread
 */
int build_heap(thread_t *thread, page_dir_t *pdir, int nthreads) {
    // build the heap
    vmm_addr_t heap = thread->stack_kernel_limit + (PAGE_SIZE * 3 * nthreads);
    
    void *mem = pmm_malloc();
    if(!mem) {
        printk("Failed allocating memory\n");
        return -1;
    }
    vmm_map_phys(pdir, heap, (uint32_t) mem, PAGE_PRESENT | PAGE_RW | PAGE_USER);
    
    vmm_map_phys(get_page_directory(), (uint32_t) heap, (uint32_t) get_phys_addr(pdir, heap), PAGE_PRESENT | PAGE_RW);
    
    thread->heap = heap;
    thread->heap_limit = heap + PAGE_SIZE;
    
    heap_init((vmm_addr_t *) heap);

    return 1;
}

/**
 * Fills the heap with arguments
 */
int heap_fill(thread_t *thread, char *name, char *arguments, uint32_t *argc, uint32_t *argv1) {
    *argc = 1;
    char **argv = (char **) umalloc(strlen(name) + strlen(arguments) + 1, (vmm_addr_t *) thread->heap);
    strcpy(argv[0], name);
    
    while(*arguments) {
        char *p = strchr(arguments, ' ');
        if(p == NULL) {
            strcpy(argv[*argc], arguments);
            (*argc)++;
            break;
        }
        int strl = strlen(arguments) - strlen(p);
        strncpy(argv[*argc], arguments, strl);
        (*argc)++;
        while(strl > 0) {
            arguments++;
            strl--;
        }
        arguments++;
    }
    *argv1 = (uint32_t) argv;
    vmm_unmap_phys_addr(get_page_directory(), (uint32_t) thread->heap);
    
    return 1;
}

/**
 * Fills the stack with register values
 */
int stack_fill(thread_t *thread, uint32_t argc, uint32_t argv) {
    // fill kernel stack
    uint32_t *stackp = (uint32_t *) thread->stack_kernel_limit;
    *--stackp = 0x23;                                       // ss
    *--stackp = thread->stack_limit - 12;                   // esp
    *--stackp = 0x202;                                      // eflags
    *--stackp = 0x1B;                                       // cs
    *--stackp = thread->eip;                                // eip
    *--stackp = 0;                                          // eax
    *--stackp = 0;                                          // ebx
    *--stackp = 0;                                          // ecx
    *--stackp = 0;                                          // edx
    *--stackp = 0;                                          // esi
    *--stackp = 0;                                          // edi
    *--stackp = thread->esp + PAGE_SIZE;                    // ebp
    *--stackp = 0x23;                                       // ds
    *--stackp = 0x23;                                       // es
    *--stackp = 0x23;                                       // fs
    *--stackp = 0x23;                                       // gs
    thread->esp_kernel = (uint32_t) stackp;
    
    // fill user stack
    stackp = (uint32_t *) thread->stack_limit;
    *--stackp = argv;
    *--stackp = argc;
    *--stackp = (uint32_t) &end_process;                    // the process needs to know where to return
    thread->esp = (uint32_t) stackp;
    
    return 1;
}

/**
 * Terminates a process and frees all the memory 
 */
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
        printk("Process %d returned with error: %d\n", cur->thread_list->pid, ret);
    
    cur->state = PROC_STOPPED;
    
    for(int i = 0; i < cur->threads; i++) {
        void *stack = get_phys_addr(cur->pdir, cur->thread_list->stack_limit - PAGE_SIZE);
        vmm_unmap_phys_addr(cur->pdir, cur->thread_list->stack_limit - PAGE_SIZE);
        pmm_free(stack);
        
        void *kernel_stack = get_phys_addr(cur->pdir, cur->thread_list->stack_kernel_limit - PAGE_SIZE);
        vmm_unmap_phys_addr(cur->pdir, cur->thread_list->stack_kernel_limit - PAGE_SIZE);
        pmm_free(kernel_stack);
    
        void *heap = get_phys_addr(cur->pdir, cur->thread_list->heap);
        vmm_unmap_phys_addr(cur->pdir, cur->thread_list->heap);
        pmm_free(heap);
        
        cur->thread_list = cur->thread_list->next;
    }
	
    // remove the executable
    for(uint32_t page = 0; page < cur->thread_list->image_size / PAGE_SIZE; page++) {
        uint32_t virt = cur->thread_list->image_base + (page * PAGE_SIZE);

        uint32_t phys = (uint32_t) get_phys_addr(cur->pdir, virt);

        vmm_unmap_phys_addr(cur->pdir, virt);
        pmm_free((void *) phys);
        // TODO need to remove threads from kheap
    }

    // delete the page directory
    // TODO remove also page tables
    uint32_t *phys = get_phys_addr(cur->pdir, (vmm_addr_t) cur->pdir);
    pmm_free(phys);

    sched_state(1);
    enable_int();
    while(1);
}

/**
 * Returns given id process state
 */
int proc_state(int id) {
    process_t *cur = get_proc_by_id(id);
    return cur->state;
}
