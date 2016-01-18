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
#include <hal/syscall.h>
#include <proc/proc.h>
#include <drivers/keyboard.h>

#define MAX_SYSCALL 2

static void *syscalls[] = {
    &printk,
    &gets
};

void syscall_init() {
    install_ir(0x72, 0x80 | 0x0E | 0x60, 0x8, &syscall_disp);
}

void syscall_disp() {
    int index = 0;
    asm volatile("mov %%eax, %0" : "=r" (index));
    if(index >= MAX_SYSCALL)
        return;
    void *func = syscalls[index];
    asm volatile("push %%edi; \
                  push %%esi; \
                  push %%edx; \
                  push %%ecx; \
                  push %%ebx; \
                  call *%0;   \
                  pop %%ebx;  \
                  pop %%ebx;  \
                  pop %%ebx;  \
                  pop %%ebx;  \
                  pop %%ebx;  \
                  sti" : : "r" (func));
}

