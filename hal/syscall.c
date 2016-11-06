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
#include <hal/syscall.h>
#include <proc/proc.h>
#include <proc/thread.h>
#include <drivers/keyboard.h>

#define MAX_SYSCALL 11

typedef uint32_t (*syscall_call_func)(uint32_t, ...);

static void *syscalls[] = {
    &printk_string,             // printf   0
    &gets,                      // scanf    1
    &clear,                     // clear    2
    &start_thread,              // fork     3
    &stop_thread,               // exit     4
    &end_process,               // return n 5
    &vfs_file_open_user,        // fopen    6
    &vfs_file_close_user,       // fclose   7
    &console_pwd_user,          // PWD      8
    &umalloc_sys,               // malloc   9
    &ufree_sys                  // free     10
};

void syscall_init() {
    install_ir(0x72, 0x80 | 0x0E | 0x60, 0x8, &syscall_handle);
}

void syscall_disp(struct regs *re) {
    if(re->eax >= MAX_SYSCALL) {
        re->eax = -1;
        return;
    }
    syscall_call_func func = syscalls[re->eax];
    re->eax = func(re->ebx, re->ecx, re->edx, re->esi, re->edi);
}
