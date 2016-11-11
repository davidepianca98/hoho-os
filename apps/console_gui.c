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

#include <console.h>
#include <drivers/io.h>
#include <hal/hal.h>
#include <drivers/keyboard.h>
#include <lib/string.h>
#include <fs/vfs.h>
#include <gui/window.h>
#include <drivers/video.h>
#include <proc/proc.h>
#include <proc/sched.h>

char *user;

window_t *window;
text_area_t *text_area;

/**
 * Sets up the console
 */
void console_init_gui(char *usr) {
    window = window_create("Console", 50, 50, 700, 500);
    text_area = create_text_area(10, 10, 660, 440);
    add_component(window, text_area);
    text_area_set_text(text_area, "Console started\n");
    
    user = kmalloc(sizeof(strlen(usr) + 1));
    memset(user, 0, strlen(usr) + 1);
    strcpy(user, usr);
    console_run_gui();
    kfree(user);
}

char str[256];

char *to_string(char *buffer, ...) {
    va_list args;
    memset(str, 0, 256);
    va_start(args, buffer);
    vsprintf(str, buffer, args);
    va_end(args);
    return str;
}

/**
 * Console loop
 */
void console_run_gui() {
    char *buffer = kmalloc(64);
    char c = 0;
    
    text_area_append(text_area, to_string("%s  $ ", user));
    int buffer_counter = 0;
    while(1) {
        c = keyboard_get_lastkey();
        if(c == NULL)
            continue;
        keyboard_invalidate_lastkey();
        if(character_check(c)) {
            buffer[buffer_counter++] = c;
        } else if(c == '\b') { // Backspace
            if(buffer_counter > 0) {
                buffer_counter--;
            } else {
                continue;
            }
        }
        str[0] = c;
        str[1] = 0;
        text_area_append(text_area, str);
        if(c == '\n') { // Enter pressed
            buffer[buffer_counter] = '\0';
            console_exec(buffer);
            buffer_counter = 0;
            text_area_append(text_area, to_string("%s %s $ ", user, get_dir()));
        }
    }
    kfree(buffer);
}
