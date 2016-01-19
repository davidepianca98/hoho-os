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

extern uint32_t kernel_start;
extern uint32_t kernel_end;

char *user;
char dir[11];

int buffer_c = 0;

void print_meminfo() {
    printk("Total mem: %d MB\nFree mem: %d MB\n", get_mem_size() / 1024 / 1024, (get_max_blocks() - get_used_blocks()) * 4096 / 1024 / 1024);
    printk("Heap size: %d MB Used heap: %d MB Free heap: %d MB\n", get_heap_size() / 1024 / 1024, get_used_heap() / 1024 / 1024, (get_heap_size() - get_used_heap()) / 1024 / 1024);
    printk("Kernel start: 0x%x Kernel end: 0x%x Kernel size: %d\n", &kernel_start, &kernel_end, (uint32_t) (&kernel_end - &kernel_start));
}

void print_file(file f) {
    if(f.flags == FS_NULL) {
        printk("Cannot open file\n");
        return;
    }

    if((f.flags & FS_DIR) == FS_DIR) {
        printk("Cannot display content of directory.\n");
        return;
    }

    printk("len : %d\n", f.len);
    while(f.eof != 1) {
        char buf[512];
        vfs_file_read(&f, buf);

        for(int i = 0; i < 512; i++)
            printk("%c", buf[i]);

        if(f.eof != 1) {
        	getchar();
        }
    }
}

void console_init(char *usr) {
    printk("Console started\n");
    user = kmalloc(sizeof(strlen(usr) + 1));
    memset(user, 0, strlen(usr) + 1);
    strcpy(user, usr);
    dir[0] = 0;
    console_run();
}

void console_run() {
    char *buffer = kmalloc(sizeof(char) * 64);
    char c = 0;
    printk("%s $ ", user);
    buffer_c = 0;
    while(1) {
        c = keyboard_get_lastkey();
        if(c == NULL)
            continue;
        keyboard_invalidate_lastkey();
        if(((int) c >= 32) && ((int) c <= 122)) {
            buffer[buffer_c++] = c;
        } else if(c == '\r') {
            if(buffer_c > 0)
                buffer_c--;
            else
                continue;
        }
        printk("%c", c);
        if(c == '\n') {
            buffer[buffer_c] = '\0';
            console_exec(buffer);
            buffer_c = 0;
            printk("%s %s $ ", user, dir);
        }
    }
}

void console_exec(char *buf) {
    char *arg = strchr(buf, ' ');
    if(arg) {
        arg++;
        if(strncmp(buf, "cd", 2) == 0) {
            if(vfs_cd(arg))
                strcpy(dir, arg);
            else
                printk("Directory not found\n");
        } else if(strncmp(buf, "start", 5) == 0) {
            int procn = start_proc(arg);
            while(proc_state(procn) != PROC_STOPPED);
        } else if(strncmp(buf, "read", 4) == 0) {
            file f = vfs_file_open(arg, 0);
            print_file(f);
            printk("\n");
        } else if(strncmp(buf, "write", 5) == 0) {
            char *buf2 = kmalloc(128);
            strcpy(buf2, buf + 6);
            char *arg2 = strchr(buf2, ' ');
            file f = vfs_file_open("fda/file.txt", 1);
            vfs_file_write(&f, arg2 + 1);
        } else
            printk("Command not found\n");
    } else {
        if(strcmp(buf, "hoho") == 0)
            printk("hoho\n");
        else if(strcmp(buf, "help") == 0)
            printk("Help:\nhoho - prints hoho\nhelp - shows help\nmeminfo - prints RAM info\ncpuinfo - shows CPU info\nls - shows filesystem devices\nread - reads a file\nstart - starts a program\nclear - clears the screen\nhalt - shuts down\nreboot - reboots the pc\n");
        else if(strcmp(buf, "meminfo") == 0)
            print_meminfo();
        else if(strcmp(buf, "cpuinfo") == 0)
            printk("%s\n", get_cpu_vendor(0));
        else if(strcmp(buf, "ls") == 0) {
            if(dir[0] == 0)
                vfs_ls();
            else
                vfs_ls_dir(dir);
        } else if(strcmp(buf, "clear") == 0)
            clear();
        else if(strcmp(buf, "cd") == 0)
            dir[0] = 0;
        else if(strcmp(buf, "proc") == 0)
            print_procs();
        else if(strcmp(buf, "halt") == 0) {
            printk("Shutting down\n");
            halt();
            while(1);
        } else if(strcmp(buf, "reboot") == 0) {
            printk("Rebooting\n");
            reboot();
        } else
            printk("Command not found\n");
    }
}

