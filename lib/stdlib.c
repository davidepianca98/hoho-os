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

#include <mm/mm.h>
#include <mm/paging.h>
#include <lib/system_calls.h>
#include <lib/stdlib.h>

void *malloc(size_t len) {
    asm volatile("mov %0, %%ebx" : : "b" (len));
    return syscall_call(9);
}

void free(void *ptr) {
    asm volatile("lea (%0), %%ebx" : : "b" (ptr));
    syscall_call(10);
}
