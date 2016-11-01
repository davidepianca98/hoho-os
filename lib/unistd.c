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
#include <lib/string.h>
#include <lib/unistd.h>
#include <lib/system_calls.h>
#include <proc/proc.h>
#include <proc/sched.h>

/* Creates a child thread */
pid_t fork() {
    return (pid_t) syscall_call(3);
}

/* Terminates a thread */
void exit(int code) {
    asm volatile("mov %0, %%ebx" : : "b" (code));
    syscall_call(4);
}

/* Waits until a thread ends */
pid_t wait(int *x) {
    // TODO implement
    x = x;
    return 0;
}

/* Waits until a certain thread ends
pid_t wait(pid_t proc, int *x, int code) {
    return 0;
}*/

/* Gets the thread's pid */
pid_t getpid() {
    return 0;
}

/* Gets the parent's pid */
pid_t getppid() {
    return 0;
}

