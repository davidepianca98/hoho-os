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

void ex_invalid_opcode(struct regs *re, uint32_t eip) {
    printk("Invalid opcode\nFaulty instruction: 0x%x\n", eip);
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
    printk("\n\nGeneral protection fault\nError code: %b\n\n", re->error);
    printk("eip: %x cs: %x\neax: %d ebx: %d ecx: %d edx: %d\nesp: %x ebp: %x esi: %d edi: %d\nds: %x es: %x fs: %x gs: %x\n", re->eip, re->cs, re->eax, re->ebx, re->ecx, re->edx, re->esp, re->ebp, re->esi, re->edi, re->ds, re->es, re->fs, re->gs);
    if(re->es == 0x10)
        panic();
    else
        stop_thread(1);
}

void ex_page_fault() {
    int addr = get_cr2();
    printk("Page fault at addr: 0x%x\n", addr);
    panic();
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

