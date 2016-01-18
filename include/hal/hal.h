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

#ifndef HAL_H
#define HAL_H

#include <drivers/ata.h>
#include <drivers/cpu.h>
#include <drivers/dma.h>
#include <hal/device.h>
#include <hal/exception.h>
#include <drivers/floppy.h>
#include <hal/gdt.h>
#include <hal/idt.h>
#include <hal/syscall.h>
#include <hal/tss.h>
#include <drivers/io.h>
#include <drivers/keyboard.h>
#include <mm/memory.h>
#include <drivers/pic.h>
#include <drivers/pit.h>
#include <types.h>

extern uint32_t kernel_start;
extern uint32_t kernel_end;

void hal_init();
void sleep(int s);
void reboot();

#endif

