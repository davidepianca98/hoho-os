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
char *dir;

int buffer_c = 0;

char privilege = '$';

void print_meminfo() {
    printk("Total mem: %d MB\nFree mem: %d MB\n", get_mem_size() / 1024 / 1024, (get_max_blocks() - get_used_blocks()) * 4096 / 1024 / 1024);
    printk("Heap size: %d MB Used heap: %d MB Free heap: %d MB\n", get_heap_size() / 1024 / 1024, get_used_heap() / 1024 / 1024, (get_heap_size() - get_used_heap()) / 1024 / 1024);
    printk("Kernel start: 0x%x Kernel end: 0x%x Kernel size: %d\n", &kernel_start, &kernel_end, (uint32_t) (&kernel_end - &kernel_start));
    uint32_t cr0 = 0;
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    printk("cr0: %x ", cr0);
    printk("cr2: %x cr3: %x\n", get_cr2(), get_pdbr());
}

void print_file(file f) {
    if(f.type == FS_NULL) {
        printk("Cannot open file\n");
        return;
    }

    if((f.type & FS_DIR) == FS_DIR) {
        printk("Cannot display content of directory.\n");
        return;
    }

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
    dir = kmalloc(64);
    memset(dir, 0, 64);
    console_run();
    kfree(user);
}

void console_run() {
    char *buffer = kmalloc(64);
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
            printk("%s %s %c ", user, dir, privilege);
        }
    }
    kfree(buffer);
}

void console_exec(char *buf) {
    char *senddir = kmalloc(64);
    memset(senddir, 0, 64);
    char *arg = strchr(buf, ' ');
    if(arg) {
        arg++;
        if(strncmp(buf, "cd", 2) == 0) {
            strcpy(senddir, dir);
            strcat(senddir, "/");
            strcat(senddir, arg);
            if(vfs_cd(senddir)) {
                strcpy(dir, senddir);
            } else
                printk("cd %s: directory not found\n", senddir);
        } else if(strncmp(buf, "start", 5) == 0) {
            char *buf2 = kmalloc(128);
            memset(buf2, 0, 128);
            strcpy(buf2, buf + 6);
            char *arg2 = strchr(buf2, ' ');
            strcpy(senddir, dir);
            strcat(senddir, "/");
            strncpy(senddir + strlen(dir) + 1, buf2, strlen(buf2) - (strlen(arg2) - 2));
            int procn = start_proc(senddir, arg2 + 1);
            if(procn != PROC_STOPPED)
                while(proc_state(procn) != PROC_STOPPED);
        } else if(strncmp(buf, "read", 4) == 0) {
            strcpy(senddir, dir);
            strcat(senddir, "/");
            strcat(senddir, arg);
            file f = vfs_file_open(senddir, 0);
            if(f.type != FS_FILE)
                printk("read: file %s not found\n", buf);
            else
                print_file(f);
            printk("\n");
        } else if(strncmp(buf, "write", 5) == 0) {
            char *buf2 = kmalloc(128);
            memset(buf2, 0, 128);
            strcpy(buf2, buf + 6);
            char *arg2 = strchr(buf2, ' ');
            strcpy(senddir, dir);
            strcat(senddir, "/");
            strncpy(senddir + strlen(dir) + 1, buf2, strlen(buf2) - strlen(arg2));
            file f = vfs_file_open(senddir, 1);
            if(f.type != FS_FILE)
                printk("write: file %s not found\n", senddir);
            else
                vfs_file_write(&f, arg2 + 1);
            kfree(buf2);
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
            memset(dir, 0, 64);
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
    kfree(senddir);
}

