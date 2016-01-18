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

#include <lib/stdio.h>
#include <types.h>
#include <lib/string.h>

void printf(char *buffer, ...) {
    char str[1024];
    va_list args;
    
    va_start(args, buffer);
    vsprintf(str, buffer, args);
    va_end(args);
    
    // call printk
    asm volatile("lea (%0), %%ebx" : : "b" (str));
	asm volatile("xor %eax, %eax; \
	              int $0x72");
}

void scanf(char *format, ...) {
    va_list args;
    int *d;
    char *c;
    char str[1024];
    
    va_start(args, format);
    for(int i = 0; i < strlen(format); i++) {
        switch(format[i]) {
            case '%':
                i++;
                switch(format[i]) {
                    case 'c':
                        c = va_arg(args, char *);
                        asm volatile("mov $1, %eax; \
	                                  int $0x72");
	                    int val;
	                    asm volatile("mov %%eax, %0" : "=r" (val));
	                    *c = (char) val;
                        break;
                    case 'd':
                    case 'i':
                        d = va_arg(args, int *);
                        asm volatile("lea (%0), %%ebx" : : "b" (str));
                        asm volatile("mov $1, %eax; \
	                                  int $0x72");
	                    *d = atoi(str);
                        break;
                    case 's':
                        c = va_arg(args, char *);
                        //strcpy(s, gets());
                        break;
                }
                break;
        }
    }
    va_end(args);
}

