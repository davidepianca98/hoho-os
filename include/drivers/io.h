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

#ifndef ASM_H
#define ASM_H

#include <mm/mm.h>
#include <mm/paging.h>
#include <types.h>

extern uint8_t inportb(uint16_t port);
extern uint16_t inportw(uint16_t port);
extern void outportb(uint16_t port, uint8_t val);
extern void halt();
void enable_int();
void disable_int();

#endif
