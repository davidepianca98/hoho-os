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

#include <drivers/video.h>
#include <lib/string.h>

static int x;
static int y;
struct video_mem vram;

void video_init(int h, int w) {
    x = 0;
    y = 0;
    vram.heigth = h;
    vram.width = w;
    vram.ram = (uint16_t *) 0xB8000;
}

void printk(char *buffer, ...) {
    int i = 0;
    char str[256];
    va_list args;
    
    va_start(args, buffer);
    vsprintf(str, buffer, args);
    va_end(args);
    
    check();
    
    while(str[i] != '\0') {
        switch(str[i]) {
            case '\0':
                return;
            case '\b':
                vram.ram[(y * vram.width) + --x] = (uint16_t) 3872;
                i++;
                break;
            case '\n':
                i++;
                y++;
                x = 0;
                check();
                break;
            case '\r':
                i++;
                x = 0;
                break;
            default:
                check();
                vram.ram[(y * vram.width) + x++] = (uint16_t) (3840 | str[i++]);
                break;
        }
    }
}

void check() {
    if(x >= vram.width) {
        x = 0;
        y++;
    }
    if(y >= vram.heigth) {
        y--;
        scroll();
    }
}

void scroll() {
    for(int i = 0; i < vram.width * (vram.heigth - 1); i++) {
        vram.ram[i] = vram.ram[i + vram.width];
    }
    for(int i = 0; i < vram.width; i++) {
        vram.ram[vram.width * (vram.heigth - 1) + i] = 3872;
    }
}

void clear() {
    for(int i = 0; i < vram.heigth * vram.width; i++) {
        *(vram.ram + i) = (uint16_t) 3872;
    }
}

