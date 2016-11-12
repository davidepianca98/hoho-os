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

#include <console.h>
#include <hal/hal.h>
#include <panic.h>
#include <drivers/video.h>
#include <proc/proc.h>
#include <proc/thread.h>

void (*return_error)() = (void *) RETURN_ADDR;

void default_ir_handler() {
    disable_int();
    console_print("Unhandled exception\n");
    panic();
}

void ex_divide_by_zero() {
    console_print("Division by zero\n");
    panic();
}

void ex_single_step() {
    console_print("Single step\n");
    panic();
}

void ex_nmi() {
    console_print("NMI trap\n");
    panic();
}

void ex_breakpoint() {
    console_print("Breakpoint\n");
    panic();
}

void ex_overflow() {
    console_print("Overflow\n");
    panic();
}

void ex_bounds_check() {
    console_print("Bounds check\n");
    panic();
}

void ex_invalid_opcode(struct regs *re) {
    console_print("Invalid opcode\n");
    console_print("eip: %x cs: %x\neax: %d ebx: %d ecx: %d edx: %d\nesp: %x ebp: %x esi: %d edi: %d\nds: %x es: %x fs: %x gs: %x\n", re->eip, re->cs, re->eax, re->ebx, re->ecx, re->edx, re->esp, re->ebp, re->esi, re->edi, re->ds, re->es, re->fs, re->gs);
    if(re->es == 0x10) {
        // If an Invalid Opcode occurs in kernel mode, we don't really want to continue
        panic();
    } else {
        // If we were in user mode, just kill that thread or process
        return_exception();
    }
}

void ex_device_not_available() {
    console_print("Device not available\n");
    panic();
}

void ex_double_fault() {
    console_print("Double fault\n");
    panic();
}

void ex_invalid_tss() {
    console_print("Invalid TSS\n");
    panic();
}

void ex_segment_not_present() {
    console_print("Segment not present\n");
    panic();
}

void ex_stack_fault() {
    console_print("Stack fault\n");
    panic();
}

void ex_gpf(struct regs_error *re) {
    console_print("\nGeneral protection fault\nError code: %b\n", re->error);
    console_print("eip: %x cs: %x\neax: %d ebx: %d ecx: %d edx: %d\nesp: %x ebp: %x esi: %d edi: %d\nds: %x es: %x fs: %x gs: %x\n", re->eip, re->cs, re->eax, re->ebx, re->ecx, re->edx, re->esp, re->ebp, re->esi, re->edi, re->ds, re->es, re->fs, re->gs);
    console_print("cr2: %x cr3: %x\n", get_cr2(), get_pdbr());
    
    // If a GPF occurs in kernel mode, we don't really want to continue
    if(re->es == 0x10) {
        panic();
    } else {
        // If we were in user mode, just kill that thread or process
        return_exception();
    }
}

void ex_page_fault(struct regs_error *re) {
    int virt_addr = get_cr2();
    mm_addr_t phys_addr = (mm_addr_t) get_phys_addr(get_page_directory(), virt_addr);
    
    console_print("\nPage fault at addr: 0x%x\n", virt_addr);
    console_print("Phys addr: 0x%x\n", phys_addr);
    // If a Page Fault occurs in kernel mode, we don't really want to continue
    if(re->es == 0x10) {
        panic();
    } else {
        // If we were in user mode, just kill that thread or process
        return_exception();
    }
}

void ex_fpu_error() {
    console_print("FPU error\n");
    panic();
}

void ex_alignment_check() {
    console_print("Alignment check\n");
    panic();
}

void ex_machine_check() {
    console_print("Machine check\n");
    panic();
}

void ex_simd_fpu() {
    console_print("SIMD FPU error\n");
    panic();
}

void return_exception() {
    int error = 1;
    asm volatile("mov %0, %%eax" : : "r" (error));
    (*return_error)();
}
