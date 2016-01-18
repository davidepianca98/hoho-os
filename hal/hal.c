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

void hal_init() {
    disable_int();
    gdt_init();
    idt_init(0x8);
    pic_init(0x20, 0x28);
    pit_init();
    pit_start_counter(100, PIT_COUNTER_0, PIT_MODE_SQUAREWAVEGEN);
    enable_int();
}

void sleep(int s) {
    int ticks = get_tick_count() + s;
    while(get_tick_count() < ticks);
}

void reboot() {
    outportb(0x64, 0xFE);
}

