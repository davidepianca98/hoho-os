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

int start_proc(char *name) {
    process_t *proc = (process_t *) kmalloc(sizeof(process_t));
    strcpy(proc->name, name);
    proc->id = pid++;
    proc->priority = 1;
    proc->state = PROC_NEW;

    if(!elf_loader(name, proc)) {
        printk("Can't load the executable\n");
        proc->state = PROC_STOPPED;
        return 0;
    }
    
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
    
    void *stack = get_phys_addr(cur->pdir, (uint32_t) cur->esp);
    
    vmm_unmap_phys_addr(cur->pdir, (uint32_t) cur->esp);
    pmm_free(stack);
    
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

