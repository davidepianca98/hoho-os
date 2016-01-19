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
#include <hal/tss.h>
#include <lib/string.h>
#include <drivers/video.h>

tss_t tss;

void flush_tss() {
    asm volatile("mov $0x2B, %ax; \
                  ltr %ax;");
}

void install_tss() {
    uint32_t base = (uint32_t) &tss;
    gdt_set_entry(5, base, base + sizeof(tss_t), 0xE9);
    memset((void *) &tss, 0, sizeof(tss_t));
    
    tss.ss0 = 0x10;
    
    void *kernel_stack = pmm_malloc();
    vmm_map_phys(get_kern_directory(), (uint32_t) &kernel_end, (uint32_t) kernel_stack, PAGE_PRESENT_FLAG | PAGE_RW_FLAG);
    tss.esp0 = (uint32_t) &kernel_end;
    tss.cs = 0x0B;
    tss.ss = 0x13;
    tss.es = 0x13;
    tss.ds = 0x13;
    tss.fs = 0x13;
    tss.gs = 0x13;
    
    flush_tss();
}

