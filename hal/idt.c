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
#include <mm/mm.h>
#include <panic.h>
#include <lib/string.h>
#include <drivers/video.h>

struct idt_ptr idtr;
struct idt_info idt[MAX_INTERRUPTS];

extern void gpf_handle();
extern void invop_handle();

void idt_init(uint16_t code) {
    int i;
    
    idtr.limit = sizeof(struct idt_info) * MAX_INTERRUPTS - 1;
    idtr.base = (uint32_t) &idt;
    
    memset(&idt, 0, idtr.limit);
    
    for(i = 0; i < MAX_INTERRUPTS; i++)
        install_ir(i, 0x80 | 0x0E, code, &default_ir_handler);
    
    install_ir(0, 0x80 | 0x0E, code, &ex_divide_by_zero);
    install_ir(1, 0x80 | 0x0E, code, &ex_single_step);
    install_ir(2, 0x80 | 0x0E, code, &ex_nmi);
    install_ir(3, 0x80 | 0x0E, code, &ex_breakpoint);
    install_ir(4, 0x80 | 0x0E, code, &ex_overflow);
    install_ir(5, 0x80 | 0x0E, code, &ex_bounds_check);
    install_ir(6, 0x80 | 0x0E, code, &invop_handle);
    install_ir(7, 0x80 | 0x0E, code, &ex_device_not_available);
    install_ir(8, 0x80 | 0x0E, code, &ex_double_fault);
    install_ir(10, 0x80 | 0x0E, code, &ex_invalid_tss);
    install_ir(11, 0x80 | 0x0E, code, &ex_segment_not_present);
    install_ir(12, 0x80 | 0x0E, code, &ex_stack_fault);
    install_ir(13, 0x80 | 0x0E, code, &gpf_handle);
    install_ir(14, 0x80 | 0x0E, code, &ex_page_fault);
    install_ir(16, 0x80 | 0x0E, code, &ex_fpu_error);
    install_ir(17, 0x80 | 0x0E, code, &ex_alignment_check);
    install_ir(18, 0x80 | 0x0E, code, &ex_machine_check);
    install_ir(19, 0x80 | 0x0E, code, &ex_simd_fpu);
    
    idt_set(&idtr);
}

void install_ir(uint32_t i, uint16_t flags, uint16_t sel, void *irq) {
    uint32_t ir_addr = (uint32_t) irq;
    
    idt[i].base_low = (uint16_t) ir_addr & 0xFFFF;
	idt[i].base_high = (uint16_t) (ir_addr >> 16) & 0xFFFF;
	idt[i].reserved = 0;
	idt[i].flags = (uint8_t) flags;
	idt[i].sel = sel;
}

