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
#include <drivers/keyboard.h>
#include <drivers/video.h>
#include <drivers/io.h>

enum KBD_PORTS {
	KBD_CHECK = 0x64,
	KBD_IN = 0x60,
};

static uint8_t lastkey = 0;
static uint8_t key_loc = 0;
static uint8_t kbd_enabled = 0;
/*
static uint8_t caps_lock = 0;
static uint8_t num_pad = 0;
static uint8_t scroll_led = 0;

static uint8_t ctrl = 0;
static uint8_t shift = 0;
static uint8_t alt = 0;*/

uint8_t *keycache;

void keyboard_irq() {
    asm volatile("add $12, %esp");
    asm volatile("pusha");
    asm volatile("cli");
    keyboard_read_key();
    irq_done(1);
    asm volatile("sti");
    asm volatile("popa");
    asm volatile("iret");
}

void keyboard_init() {
    outportb(KBD_CHECK, 0xAE);
    install_ir(33, 0x80 | 0x0E, 0x8, &keyboard_irq);
    keycache = kmalloc(sizeof(uint8_t) * 256);
    printk("Keyboard enabled\n");
	kbd_enabled = 1;
}

void keyboard_read_key() {
    lastkey = 0;
    if(inportb(KBD_CHECK) & 1) {
        lastkey = keyboard_to_ascii(inportb(KBD_IN));
        keycache[key_loc] = lastkey;
        key_loc++;
	}
}

char keyboard_get_key() {
    char c = 0;
	if(key_loc == 0)
	    return c;
	c = keycache[0];
	key_loc--;
	for(int i = 0; i < 255; i++) {
		keycache[i] = keycache[i+1];
	}
	return c;
}

char keyboard_get_lastkey() {
    return lastkey;
}

void keyboard_invalidate_lastkey() {
    lastkey = NULL;
}

static char* qwertzuiop = "qwertzuiop"; // 0x10-0x1c
static char* asdfghjkl = "asdfghjkl";
static char* yxcvbnm = "yxcvbnm";
static char* num = "123456789";
uint8_t keyboard_to_ascii(uint8_t key){
	if(key == 0x1C) return '\n';
	if(key == 0x39) return ' ';
	if(key == 0xE) return '\r';
	if(key == POINT_RELEASED) return '.';
	if(key == SLASH_RELEASED) return '/';
	if(key == ZERO_PRESSED) return '0';
	if(key == BACKSLASH_PRESSED) return '\\';
	if(key >= ONE_PRESSED && key <= NINE_PRESSED)
		return num[key - ONE_PRESSED];
	if(key >= 0x10 && key <= 0x1C)
		return qwertzuiop[key - 0x10];
	else if(key >= 0x1E && key <= 0x26)
		return asdfghjkl[key - 0x1E];
	else if(key >= 0x2C && key <= 0x32)
		return yxcvbnm[key - 0x2C];
	return 0;
}

void keyboard_set_leds(int num, int capslock, int scroll) {
    uint8_t cmd = 0;
    cmd = (scroll) ? (cmd | 1) : (cmd & 1);
    cmd = (num) ? (cmd | 2) : (cmd & 2);
    cmd = (capslock) ? (cmd | 4) : (cmd & 4);
    outportb(KBD_IN, 0xED);
    outportb(KBD_IN, cmd);
}

char getchar() {
    asm volatile("sti");
    char c = NULL;
    while(1) {
        c = keyboard_get_lastkey();
        if(c == NULL)
            continue;
        keyboard_invalidate_lastkey();
        return c;
    }
}

// TODO fix
void gets(char *str) {
    int count = 0;
    char c;
    while(1) {
        asm volatile("sti");
        c = keyboard_get_lastkey();
        //asm volatile("cli");
        if(c == NULL)
            continue;
        keyboard_invalidate_lastkey();
        if(((int) c >= 32) && ((int) c <= 122))
            str[count++] = c;
        else if(c == '\r')
            count--;
        printk("%c", c);
        if(c == '\n') {
            str[count] = '\0';
            break;
        }
    }
}

