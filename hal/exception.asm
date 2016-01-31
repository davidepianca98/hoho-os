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

extern ex_gpf

global gpf_handle
gpf_handle:
    pusha
    push gs
    push fs
    push es
    push ds
    push esp
    call ex_gpf
    ret

extern ex_invalid_opcode

global invop_handle
invop_handle:
    pusha
    push gs
    push fs
    push es
    push ds
    push eax
    push esp
    call ex_invalid_opcode
    ret

extern ex_page_fault

global pf_handle
pf_handle:
    pusha
    push gs
    push fs
    push es
    push ds
    push esp
    call ex_page_fault
    ret

