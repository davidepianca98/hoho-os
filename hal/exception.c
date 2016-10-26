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

#include <hal/hal.h>
#include <panic.h>
#include <drivers/video.h>
#include <proc/proc.h>
#include <proc/thread.h>

void default_ir_handler() {
    disable_int();
    printk("Unhandled exception\n");
    panic();
}

void ex_divide_by_zero() {
    printk("Division by zero\n");
    panic();
}

void ex_single_step() {
    printk("Single step\n");
    panic();
}

void ex_nmi() {
    printk("NMI trap\n");
    panic();
}

void ex_breakpoint() {
    printk("Breakpoint\n");
    panic();
}

void ex_overflow() {
    printk("Overflow\n");
    panic();
}

void ex_bounds_check() {
    printk("Bounds check\n");
    panic();
}

void ex_invalid_opcode(struct regs_error *re) {
    printk("Invalid opcode\nFaulty instruction: 0x%x\n", re->eip);
    printk("eip: %x cs: %x\neax: %d ebx: %d ecx: %d edx: %d\nesp: %x ebp: %x esi: %d edi: %d\nds: %x es: %x fs: %x gs: %x\n", re->eip, re->cs, re->eax, re->ebx, re->ecx, re->edx, re->esp, re->ebp, re->esi, re->edi, re->ds, re->es, re->fs, re->gs);
    panic();
}

void ex_device_not_available() {
    printk("Device not available\n");
    panic();
}

void ex_double_fault() {
    printk("Double fault\n");
    panic();
}

void ex_invalid_tss() {
    printk("Invalid TSS\n");
    panic();
}

void ex_segment_not_present() {
    printk("Segment not present\n");
    panic();
}

void ex_stack_fault() {
    printk("Stack fault\n");
    panic();
}

void ex_gpf(struct regs_error *re) {
    printk("\nGeneral protection fault\nError code: %b\n", re->error);
    printk("eip: %x cs: %x\neax: %d ebx: %d ecx: %d edx: %d\nesp: %x ebp: %x esi: %d edi: %d\nds: %x es: %x fs: %x gs: %x\n", re->eip, re->cs, re->eax, re->ebx, re->ecx, re->edx, re->esp, re->ebp, re->esi, re->edi, re->ds, re->es, re->fs, re->gs);
    printk("cr2: %x cr3: %x\n", get_cr2(), get_pdbr());
    
    // If a GPF occurs in kernel mode, we don't really want to continue
    if(re->es == 0x10) {
        panic();
    } else {
        // If we were in user mode, just kill that thread or process
        change_page_directory(get_kern_directory());
        stop_thread(1);
    }
}

void ex_page_fault() {
    int virt_addr = get_cr2();
    mm_addr_t phys_addr = (mm_addr_t) get_phys_addr(get_page_directory(), virt_addr);
    
    printk("Page fault %x\n", virt_addr);
    // If the physical address is not mapped
    if(!(phys_addr)) {
        if(!vmm_map(get_page_directory(), virt_addr, PAGE_PRESENT | PAGE_RW | PAGE_USER)) {
            printk("Couldn't create page table\n");
            panic();
        }
    } else {
        printk("\nPage fault at addr: 0x%x\n", virt_addr);
        printk("Phys addr: 0x%x\n", phys_addr);
        panic();
    }
}

void ex_fpu_error() {
    printk("FPU error\n");
    panic();
}

void ex_alignment_check() {
    printk("Alignment check\n");
    panic();
}

void ex_machine_check() {
    printk("Machine check\n");
    panic();
}

void ex_simd_fpu() {
    printk("SIMD FPU error\n");
    panic();
}

