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

#include <proc/sched.h>
#include <console.h>
#include <mm/memory.h>
#include <hal/hal.h>
#include <lib/string.h>
#include <drivers/io.h>

process_t *list;
static int n_proc = 1;

process_t *get_cur_proc() {
    return list;
}

process_t *get_proc_by_id(int id) {
    process_t *app = list;
    for(int i = 0; i < n_proc; i++) {
        if(app->id == id) {
            return app;
        }
        app = app->next;
    }
    return NULL;
}

void main_proc() {
    console_init("Hoho");
}

uint32_t schedule(uint32_t esp) {
    if(get_sched_state() == 0)
        return list->esp;
    list->esp = esp;
    do {
        list = list->next;
    } while(list->state == PROC_STOPPED);
    change_page_directory(list->pdir);
    return list->esp;
}

void sched_add_proc(process_t *proc) {
    sched_state(0);
    n_proc++;
    proc->prec = list;
    proc->next = list->next;
    proc->next->prec = proc;
    list->next = proc;
    sched_state(1);
}

void sched_set_null_proc(int id) {
    process_t *app = list;
    for(int i = 0; i < n_proc; i++) {
        if(app->id == id) {
            app->state = PROC_STOPPED;
            break;
        }
        app = app->next;
    }
}

void sched_remove_proc(int id) {
    process_t *app = get_proc_by_id(id);
    if(app->id == id) {
        app->prec->next = app->next;
        app->next->prec = app->prec;
        n_proc--;
        // free heap memory
        list = app->next;
    }
}

void sched_init() {
    process_t *proc = (process_t *) kmalloc(sizeof(process_t));
    strcpy(proc->name, "console");
    proc->id = 1;
    proc->priority = 1;
    proc->state = PROC_ACTIVE;
    proc->pdir = get_kern_directory();
    proc->eip = (uint32_t) &main_proc;
    void *stack = (void *) pmm_malloc();
    
    vmm_map_phys(proc->pdir, (uint32_t) stack, (uint32_t) stack, PAGE_PRESENT_FLAG | PAGE_RW_FLAG | PAGE_MODE_FLAG);

    proc->esp = (uint32_t) stack;
    proc->stack_limit = ((uint32_t) proc->esp + 4096);
    
    uint32_t *stackp = (uint32_t *) proc->stack_limit;
    *--stackp = 0x202;              // eflags
    *--stackp = 0x8;                // cs
    *--stackp = proc->eip;          // eip
    *--stackp = 0;                  // eax
    *--stackp = 0;                  // ebx
    *--stackp = 0;                  // ecx
    *--stackp = 0;                  // edx
    *--stackp = 0;                  // esi
    *--stackp = 0;                  // edi
    *--stackp = proc->esp + 4096;   // ebp
    *--stackp = 0x10;               // ds
    *--stackp = 0x10;               // es
    *--stackp = 0x10;               // fs
    *--stackp = 0x10;               // gs
    proc->esp = (uint32_t) stackp;
    
    proc->next = proc;
    proc->prec = proc;
    list = proc;
    sched_state(1);
    disable_int();
    change_page_directory(proc->pdir);
    switch_usermode_start(proc->esp, proc->eip);
}

int get_nproc() {
    return n_proc;
}

void print_procs() {
    process_t *app = list;
    for(int i = 0; i < n_proc; i++) {
        printk("Name: %s id: %d priority: %d page directory: 0x%x state: %d\n", app->name, app->id, app->priority, app->pdir, app->state);
        printk("    eip: 0x%x esp: 0x%x stack limit: 0x%x\nimage base: 0x%x image size: %x\n\n", app->eip, app->esp, app->stack_limit, app->image_base, app->image_size);
        app = app->next;
    }
}

