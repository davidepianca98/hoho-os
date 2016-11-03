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
 
#include <lib/string.h>
#include <lib/system_calls.h>

void system(char *arg) {
    if(strcmp(arg, "clear") == 0) {
        syscall_call(2);
    }
}

char *pwd() {
    return (char *) syscall_call(8);
}

void end_process_return() {
    asm volatile("mov %eax, %ebx");
    syscall_call(5);
}

void *syscall_call(int n) {
    void *ret;
    asm volatile("mov %0, %%eax; \
	              int $0x72" : : "a" (n));
    asm volatile("mov %%eax, %0" : "=r" (ret));
    return ret;
}
