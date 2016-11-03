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
#include <lib/stdlib.h>
#include <types.h>
#include <lib/string.h>
#include <lib/system_calls.h>

void printf(char *buffer, ...) {
    char *str = (char *) malloc(256);
    va_list args;
    
    va_start(args, buffer);
    vsprintf(str, buffer, args);
    va_end(args);
    
    // Call printk
    asm volatile("lea (%0), %%ebx" : : "b" (str));
    asm volatile("xor %eax, %eax; \
                  int $0x72");
    free(str);
}

void scanf(char *format, ...) {
    va_list args;
    int *d;
    char *c;
    char *str = (char *) malloc(256);
    
    va_start(args, format);
    for(int i = 0; i < strlen(format); i++) {
        switch(format[i]) {
            case '%':
                i++;
                switch(format[i]) {
                    case 'c':
                        c = va_arg(args, char *);
                        asm volatile("lea (%0), %%ebx" : : "b" (str));
                        syscall_call(1);
	                    *c = (char) str[0];
                        break;
                    case 'd':
                    case 'i':
                        d = va_arg(args, int *);
                        asm volatile("lea (%0), %%ebx" : : "b" (str));
                        syscall_call(1);
	                    *d = atoi(str);
                        break;
                    case 's':
                        c = va_arg(args, char *);
                        asm volatile("lea (%0), %%ebx" : : "b" (str));
                        syscall_call(1);
                        strcpy(c, str);
                        break;
                }
                break;
        }
    }
    va_end(args);
    free(str);
}

FILE *fopen(char *filename, char *mode) {
    char *file = (char *) malloc(256);
    strcpy(file, pwd());
    strcat(file, "/");
    strcat(file, filename);
    
    asm volatile("lea (%0), %%ebx" : : "b" (file));
    asm volatile("lea (%0), %%ecx" : : "c" (mode));
    FILE *f = (FILE *) syscall_call(6);
    free(file);
    return f;
}

void fclose(FILE *f) {
    asm volatile("lea (%0), %%ebx" : : "b" (f));
    syscall_call(7);
}
