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

#include <hal/gdt.h>
#include <drivers/video.h>

#define GDT_LEN 8

struct gdt_info gdt_tab[GDT_LEN];
struct gdt_ptr ptr;

void gdt_init() {
    gdt_set_entry(0, 0, 0, 0);
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A);
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92);
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA);
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2);
    
    ptr.base = (uint32_t) &gdt_tab;
    ptr.limit = sizeof(struct gdt_info) * GDT_LEN - 1;
    
    gdt_set(&ptr);
}

void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access) {
    gdt_tab[index].base_low = base & 0xFFFF;
    gdt_tab[index].base_middle = (base >> 16) & 0xFF;
    gdt_tab[index].base_high = (base >> 24) & 0xFF;
    gdt_tab[index].limit_low = limit & 0xFFFF;
    gdt_tab[index].granularity = ((limit >> 16) & 0x0F) | (0xCF & 0xF0);
    gdt_tab[index].flags = access;
}

