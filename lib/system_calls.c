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

void system(char *arg) {
    if(strcmp(arg, "clear") == 0) {
        asm volatile("mov $2, %eax; \
	                  int $0x72");
    }
}

char *pwd() {
    char *ret;
    asm volatile("mov $7, %eax; \
	                  int $0x72");
    asm volatile("mov %%eax, %0" : "=r" (ret));
    return ret;
}

void end_process_return() {
    asm volatile("mov %eax, %ebx; \
                  mov $5, %eax; \
                  int $0x72");
}
