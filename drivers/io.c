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

#include <drivers/io.h>

uint8_t inportb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %%dx, %%al" : "=a" (ret) : "d" (port));
    return ret;
}

uint16_t inportw(uint16_t port) {
    uint16_t ret;
    asm volatile("inw %%dx, %%ax" : "=a" (ret) : "d" (port));
    return ret;
}

void outportb(uint16_t port, uint8_t val) {
    asm volatile("outb %%al, %%dx" : : "d" (port), "a" (val));
}

void halt() {
    asm volatile("hlt");
}

void enable_int() {
    asm volatile("sti");
}

void disable_int() {
    asm volatile("cli");
}

