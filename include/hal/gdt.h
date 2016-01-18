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

#ifndef GDT_H
#define GDT_H

#include <types.h>

struct gdt_info {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t flags;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((__packed__));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((__packed__));

void gdt_init();
void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access);
extern void gdt_set(struct gdt_ptr *ptr);

#endif
