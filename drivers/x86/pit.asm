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

extern pit_ticks
extern sched_on

extern schedule

global pit_int
pit_int:

    ; push registers
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    push ds
    push es
    push fs
    push gs
    
    mov ebx, esp            ; save stack pointer
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    inc byte [pit_ticks]    ; increment PIT ticks
    
    mov eax, 0              ; check if scheduling is on
    cmp [sched_on], eax
    je .restore             ; scheduling off, end of interrupt
    
    push ebx
    call schedule           ; switch task
    
    mov esp, eax            ; change stack pointer

.restore:
    mov al, 0x20            ; PIC acknowledge
    out 0x20, al
    
    pop gs
    pop fs
    pop es
    pop ds
    
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    iretd

global fork_eip
fork_eip:
    pop eax
    jmp eax
