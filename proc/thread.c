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
#include <proc/thread.h>
#include <drivers/io.h>
#include <drivers/video.h>
#include <proc/sched.h>
#include <hal/hal.h>
#include <lib/string.h>

extern void fork_eip();

static int pid = 2;

/* Allocates space for a new thread */
thread_t *create_thread() {
    thread_t *thread = (thread_t *) kmalloc(sizeof(thread_t));
    if(thread == NULL)
        return NULL;
    thread->pid = pid++;
    thread->main = 0;
    thread->time = 20;
    thread->state = PROC_NEW;
    thread->next = thread;
    thread->prec = thread;
    return thread;
}

/* Starts a new thread (fork) */
int start_thread() {
    sched_state(0);
    disable_int();
    process_t *cur = get_cur_proc();
    
    thread_t *thread = create_thread();
    if(!thread)
        return -1;
    
    thread_t *parent = cur->thread_list;
    
    thread->image_base = cur->thread_list->image_base;
    thread->image_size = cur->thread_list->image_size;
    thread->parent = (void *) cur;
    
    if(!build_stack(thread, cur->pdir, cur->threads + 1)) {
        kfree(thread);
        sched_state(1);
        enable_int();
        return -1;
    }
    
    if(!stack_fill(thread, 0, 0)) {
        kfree(thread);
        sched_state(1);
        enable_int();
        return -1;
    }
    
    memcpy((void *) thread->stack_limit - PAGE_SIZE, (void *) cur->thread_list->stack_limit - PAGE_SIZE, PAGE_SIZE);

    if(!build_heap(thread, cur->pdir, cur->threads + 1)) {
        kfree(thread);
        sched_state(1);
        enable_int();
        return -1;
    }
    memcpy((void *) thread->heap, (void *) cur->thread_list->heap, PAGE_SIZE);
    
    cur->threads++;
    
    thread->prec = cur->thread_list;
    thread->next = cur->thread_list->next;
    cur->thread_list->next->prec = thread;
    cur->thread_list->next = thread;
    
    // TODO fix splitting
    fork_eip();
    if(cur->thread_list == parent) {
        thread->state = PROC_ACTIVE;
        sched_state(1);
        enable_int();
        return thread->pid;
    } else {
        return 0;
    }
}

/* Terminates the calling thread */
void stop_thread(int code) {
    sched_state(0);

    process_t *cur = get_cur_proc();
    if(cur == NULL) {
        printk("Process not found\n");
        sched_state(1);
        enable_int();
        while(1);
    }
    
    // Terminating the main thread will terminate the process
    if(cur->thread_list->main == 1)
        end_proc(code);
    
    thread_t *thread = cur->thread_list;
    
    cur->thread_list->next->prec = cur->thread_list->prec;
    cur->thread_list->prec->next = cur->thread_list->next;
    
    vmm_unmap(cur->pdir, cur->thread_list->stack_limit - PAGE_SIZE);
    vmm_unmap(cur->pdir, cur->thread_list->stack_kernel_limit - PAGE_SIZE);
    for(int i = 0; i < 4; i++) {
        vmm_unmap(cur->pdir, cur->thread_list->heap + (i * PAGE_SIZE));
    }
    
    kfree(thread);
    
    sched_state(1);
    enable_int();
    while(1);
}

