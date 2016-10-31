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

/*
 * Process in memory
 * |-----------image_start---------------|
 * |                                     |
 * |-------image_start + image_size------|
 * |              padding                |
 * |------------user stack---------------| ---|
 * |               4096B                 |    |
 * |-----------kernel stack--------------|    |
 * |               4096B                 |    | x number of threads
 * |-------------user heap---------------|    |
 * |               4096B                 |    |
 * |-------------------------------------| ---|
 */

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

    if(!load_elf(name, proc->thread_list, proc->pdir)) {
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
    // Build the user stack
    thread->esp = (uint32_t) (thread->image_base + thread->image_size + (PAGE_SIZE * 3 * nthreads));
    thread->stack_limit = ((uint32_t) thread->esp + PAGE_SIZE);
    
    if(!vmm_map(get_kern_directory(), thread->esp, PAGE_PRESENT | PAGE_RW) ||
        !vmm_map_phys(pdir, thread->esp, (uint32_t) get_phys_addr(get_kern_directory(), thread->esp), PAGE_PRESENT | PAGE_RW | PAGE_USER))
        return 0;
    
    // Build the kernel stack
    thread->esp_kernel = thread->stack_limit;
    thread->stack_kernel_limit = thread->esp_kernel + PAGE_SIZE;
    
    if(!vmm_map(get_kern_directory(), thread->esp_kernel, PAGE_PRESENT | PAGE_RW) ||
        !vmm_map_phys(pdir, thread->esp_kernel, (uint32_t) get_phys_addr(get_kern_directory(), thread->esp_kernel), PAGE_PRESENT | PAGE_RW | PAGE_USER))
        return 0;
    
    return 1;
}

/**
 * Builds the heap for a userspace thread
 */
int build_heap(thread_t *thread, page_dir_t *pdir, int nthreads) {
    // build the heap
    vmm_addr_t heap = thread->stack_kernel_limit + (PAGE_SIZE * 3 * nthreads);
    
    if(!vmm_map(get_kern_directory(), heap, PAGE_PRESENT | PAGE_RW) ||
        !vmm_map_phys(pdir, heap, (uint32_t) get_phys_addr(get_kern_directory(), heap), PAGE_PRESENT | PAGE_RW | PAGE_USER))
        return 0;
    
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
    char *argv = (char *) umalloc(strlen(name) + strlen(arguments) + 10, (vmm_addr_t *) thread->heap);
    strcpy(argv, name);
    
    char *argv_c = argv;
    // TODO fix arguments
    while(*arguments) {
        char *p = strchr(arguments, ' ');
        if(p == NULL) {
            strcpy(argv_c, arguments);
            (*argc)++;
            break;
        }
        int strl = strlen(arguments) - strlen(p);
        strncpy(argv_c, arguments, strl);
        (*argc)++;
        while(strl > 0) {
            arguments++;
            argv_c++;
            strl--;
        }
        strcat(argv_c, "\n");
        arguments++;
    }
    *argv1 = (uint32_t) argv;
    vmm_unmap_phys(get_kern_directory(), (uint32_t) thread->heap);
    
    return 1;
}

/**
 * Fills the stack with register values
 */
int stack_fill(thread_t *thread, uint32_t argc, uint32_t argv) {
    // Fill user stack
    uint32_t *stackp = (uint32_t *) thread->stack_limit;
    *--stackp = argv;
    *--stackp = argc;
    *--stackp = (uint32_t) RETURN_ADDR;     // The process needs to know where to return
    thread->esp = (uint32_t) stackp;
    
    // Fill kernel stack
    stackp = (uint32_t *) thread->stack_kernel_limit;
    *--stackp = 0x23;                                       // ss
    *--stackp = thread->esp;                                // esp
    *--stackp = 0x202;                                      // eflags
    *--stackp = 0x1B;                                       // cs
    *--stackp = thread->eip;                                // eip
    *--stackp = 0;                                          // eax
    *--stackp = 0;                                          // ebx
    *--stackp = 0;                                          // ecx
    *--stackp = 0;                                          // edx
    *--stackp = 0;                                          // esi
    *--stackp = 0;                                          // edi
    *--stackp = thread->stack_limit;                        // ebp
    *--stackp = 0x23;                                       // ds
    *--stackp = 0x23;                                       // es
    *--stackp = 0x23;                                       // fs
    *--stackp = 0x23;                                       // gs
    thread->esp_kernel = (uint32_t) stackp;
    
    vmm_unmap_phys(get_kern_directory(), (uint32_t) thread->esp);
    
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
        while(1);
    }
    
    if(ret)
        printk("Process %d returned with error: %d\n", cur->thread_list->pid, ret);
    
    cur->state = PROC_STOPPED;

    // Remove the executable
    for(uint32_t page = 0; page < cur->thread_list->image_size / PAGE_SIZE; page++) {
        vmm_unmap(cur->pdir, cur->thread_list->image_base + (page * PAGE_SIZE));
    }
    
    for(int i = 0; i < cur->threads; i++) {
        vmm_unmap(cur->pdir, cur->thread_list->stack_limit - PAGE_SIZE);
        vmm_unmap(cur->pdir, cur->thread_list->stack_kernel_limit - PAGE_SIZE);
        vmm_unmap(cur->pdir, cur->thread_list->heap);
        
        thread_t *thread = cur->thread_list;
        cur->thread_list = cur->thread_list->next;
        kfree(thread);
    }
    
    change_page_directory(get_kern_directory());
    delete_address_space(cur->pdir);
    kfree(cur);

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
