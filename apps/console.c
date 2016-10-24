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
#include <drivers/video.h>
#include <proc/proc.h>
#include <proc/sched.h>

char *user;
char *dir;

char privilege = '$';

/**
 * Sets up the console
 */
void console_init(char *usr) {
    printk("Console started\n");
    user = kmalloc(sizeof(strlen(usr) + 1));
    memset(user, 0, strlen(usr) + 1);
    strcpy(user, usr);
    dir = kmalloc(64);
    memset(dir, 0, 64);
    console_run();
    kfree(user);
}

/**
 * Console loop
 */
void console_run() {
    char *buffer = kmalloc(64);
    char c = 0;
    printk("%s $ ", user);
    int buffer_counter = 0;
    while(1) {
        c = keyboard_get_lastkey();
        if(c == NULL)
            continue;
        keyboard_invalidate_lastkey();
        if(character_check(c)) {
            buffer[buffer_counter++] = c;
        } else if(c == '\b') { // backspace
            if(buffer_counter > 0) {
                buffer_counter--;
            } else {
                continue;
            }
        }
        printk("%c", c);
        if(c == '\n') { // enter pressed
            buffer[buffer_counter] = '\0';
            console_exec(buffer);
            buffer_counter = 0;
            printk("%s %s %c ", user, dir, privilege);
        }
    }
    kfree(buffer);
}

/**
 * Checks if the character is printable
 */
int character_check(char c) {
    if(((int) c >= 32) && ((int) c <= 122)) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * Executes the issued command
 */
void console_exec(char *buf) {
    if(strncmp(buf, "cd", 2) == 0) {
        console_cd(dir, buf);
    } else if(strncmp(buf, "start", 5) == 0) {
        console_start(dir, buf);
    } else if(strncmp(buf, "read", 4) == 0) {
        console_read(dir, buf);
    } else if(strncmp(buf, "write", 5) == 0) {
        console_write(dir, buf);
    } else if(strcmp(buf, "hoho") == 0) {
        printk("hoho\n");
    } else if(strcmp(buf, "help") == 0) {
        printk("Help:\nhoho - prints hoho\nhelp - shows help\nmeminfo - prints RAM info\ncpuinfo - shows CPU info\nls - shows filesystem devices\nread - reads a file\nstart - starts a program\nclear - clears the screen\nhalt - shuts down\nreboot - reboots the pc\n");
    } else if(strcmp(buf, "meminfo") == 0) {
        print_meminfo();
    } else if(strcmp(buf, "cpuinfo") == 0) {
        printk("%s\n", get_cpu_vendor(0));
    } else if(strcmp(buf, "ls") == 0) {
        if(dir[0] == 0) {
            vfs_ls();
        } else {
            vfs_ls_dir(dir);
        }
    } else if(strcmp(buf, "clear") == 0) {
        clear();
    } else if(strcmp(buf, "proc") == 0) {
        print_procs();
    } else if(strcmp(buf, "halt") == 0) {
        printk("Shutting down\n");
        halt();
        while(1);
    } else if(strcmp(buf, "reboot") == 0) {
        printk("Rebooting\n");
        reboot();
    } else {
        printk("Command not found\n");
    }
}

/**
 * Gets the current working directory
 */
char *get_dir() {
    return dir;
}
