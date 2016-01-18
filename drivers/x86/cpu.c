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
#include <mm/memory.h>
#include <lib/string.h>

#define cpuid(in, a, b, c, d) asm volatile("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));

char *get_cpu_vendor() {
    int eax, ebx, ecx, edx;
    char *v = (char *) kmalloc(sizeof(char) * 32);
    
    cpuid(0, eax, ebx, ecx, edx);
    
    switch(ebx) {
		case 0x756e6547:
	        strcpy(v, "Intel");
	        break;
	    case 0x68747541:
	        strcpy(v, "AMD");
	        break;
	}
	return v;
}

