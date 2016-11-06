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
struct vbe_mem vbemem;

void video_init(int h, int w) {
    x = 0;
    y = 0;
    vram.heigth = h;
    vram.width = w;
    vram.ram = (uint16_t *) 0xB8000;
}

void printk(char *buffer, ...) {
    char str[256];
    va_list args;
    
    va_start(args, buffer);
    vsprintf(str, buffer, args);
    va_end(args);
    
    printk_string(str);
}

void printk_string(char *buffer) {
    int i = 0;
    
    check();
    
    while(buffer[i] != '\0') {
        switch(buffer[i]) {
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
                vram.ram[(y * vram.width) + x++] = (uint16_t) (3840 | buffer[i++]);
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
    x = y = 0;
    for(int i = 0; i < vram.heigth * vram.width; i++) {
        *(vram.ram + i) = (uint16_t) 3872;
    }
}

void vbe_init() {
    vbe_mode_info_t *vbe_mode = (vbe_mode_info_t *) 0x2000;
    int ptr = (int) vbe_mode;
    
    regs16_t regs = { 0 };
    regs.ax = 0x4F01;
    regs.di = ptr & 0xF;
    regs.es = (ptr >> 4) & 0xFFFF;
    regs.cx = 0x118;
    int32(0x10, &regs);
    
    vbemem.buffer_size = vbe_mode->width * vbe_mode->height * (vbe_mode->bpp / 8);
    vbemem.mem = (void *) vbe_mode->framebuffer;
    vbemem.buffer = (void *) VIDEO_MEM_BUFFER;
    memset(vbemem.buffer, 0, vbemem.buffer_size);
    vbemem.xres = vbe_mode->width;
    vbemem.yres = vbe_mode->height;
    vbemem.bpp = vbe_mode->bpp;
    vbemem.pitch = vbe_mode->pitch;
    printk("%x\n", vbe_mode->framebuffer);
    regs.ax = 0x4F02;
    regs.bx = 0x4118;
    regs.cx = regs.es = regs.di = 0;
    //int32(0x10, &regs);
    // TODO start process refresh_screen
    //put_rect(100, 100, 100, 100, 8);
    //memcpy(vbemem.mem, vbemem.buffer, vbemem.buffer_size);
}

void refresh_screen() {
    while(1) {
        memcpy(vbemem.mem, vbemem.buffer, vbemem.buffer_size);
    }
}

void put_pixel(int x, int y, int color) {
    if(x < 0 || x > vbemem.xres || y < 0 || y > vbemem.yres)
        return;
    x = x * (vbemem.bpp / 8);
    y = y * vbemem.pitch;
    
    register char *c_temp;
    c_temp = (char *) ((int) vbemem.buffer) + x + y;
    c_temp[0] = color & 0xFF;
    c_temp[1] = (color >> 8) & 0xFF;
    c_temp[2] = (color >> 16) & 0xFF;
}

void put_rect(int x, int y, int w, int h, int color) {
    for(int i = 0; i < w; i++) {
        for(int j = 0; j < h; j++) {
            put_pixel(x, y, color);
        }
    }
}
