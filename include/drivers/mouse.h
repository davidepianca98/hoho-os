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

#ifndef MOUSE_H
#define MOUSE_H

#include <types.h>

#define MOUSE_PORT   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_ABIT   0x02
#define MOUSE_BBIT   0x01
#define MOUSE_WRITE  0xD4
#define MOUSE_F_BIT  0x20
#define MOUSE_V_BIT  0x08

#define LEFT_CLICK      0x1
#define RIGHT_CLICK     0x2
#define MIDDLE_CLICK    0x4

typedef struct mouse_info {
    int x;
    int y;
    uint32_t button;
} mouse_info_t;

void mouse_wait(uint8_t type);
void mouse_write(uint8_t write);
uint8_t mouse_read();
mouse_info_t *get_mouse_info();
void mouse_handler();
void mouse_check_bounds();
void mouse_init();

#endif
