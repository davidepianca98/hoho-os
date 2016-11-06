;
;  Copyright 2016 Davide Pianca
;
;  Licensed under the Apache License, Version 2.0 (the "License");
;  you may not use this file except in compliance with the License.
;  You may obtain a copy of the License at
;
;      http://www.apache.org/licenses/LICENSE-2.0
;
;  Unless required by applicable law or agreed to in writing, software
;  distributed under the License is distributed on an "AS IS" BASIS,
;  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;  See the License for the specific language governing permissions and
;  limitations under the License.
;

global loader
extern kmain

MODULEALIGN equ 1<<0
MEMINFO     equ 1<<1
VIDMOD      equ 1<<2
FLAGS       equ MODULEALIGN | MEMINFO | VIDMOD
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .mbheader
align 4

multiboot_header:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    dd multiboot_header
    dd loader
    dd stack
    dd stack
    dd loader
    dd 0
    dd 1024
    dd 768
    dd 32

section .text

STACKSIZE   equ 0x4000

loader:
    mov esp, stack + STACKSIZE
    push ebx                    ; pointer to multiboot structure
    call kmain
    cli
    hlt

.hang:
    jmp .hang
    
section .bss
align 4

stack:
    resb STACKSIZE

