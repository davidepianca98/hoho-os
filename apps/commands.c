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
#include <drivers/keyboard.h>
#include <drivers/video.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <proc/proc.h>

char senddir[64];

extern uint32_t kernel_start;
extern uint32_t kernel_end;

/**
 * Change directory
 */
void console_cd(char *dir, char *command) {
    memset(senddir, 0, 64);
    strcpy(senddir, dir);
    strcat(senddir, "/");
    char *arg = get_argument(command, 1);
    if(arg) {
        strcat(senddir, arg);
        if(vfs_cd(senddir)) {
            strcpy(dir, senddir);
        } else {
            printk("cd %s: directory not found\n", senddir);
        }
    } else {
        memset(dir, 0, 64);
    }
}

/**
 * Starts a program
 */
void console_start(char *dir, char *command) {
    memset(senddir, 0, 64);
    strcpy(senddir, dir);
    strcat(senddir, "/");
    strcat(senddir, get_argument(command, 1));
    // Cut the arguments from the path
    if(get_argument(senddir, 1)) {
        strncpy(senddir, senddir, strlen(senddir) - strlen(get_argument(senddir, 1)) - 1);
    }
    
    char *arguments = kmalloc(128);
    memset(arguments, 0, 128);
    strcpy(arguments, get_argument(command, 2));
    
    int procn = start_proc(senddir, arguments);
    if(procn != PROC_STOPPED) {
        while(proc_state(procn) != PROC_STOPPED);
        remove_proc(procn);
    }
}

/**
 * Outputs a file into the console
 */
void console_read(char *dir, char *command) {
    memset(senddir, 0, 64);
    strcpy(senddir, dir);
    strcat(senddir, "/");
    strcat(senddir, get_argument(command, 1));
    file f = vfs_file_open(senddir, "r");
    if(f.type != FS_FILE) {
        printk("read: file %s not found\n", senddir);
    } else {
        print_file(f);
    }
    printk("\n");
}

/**
 * Writes the input to the file
 */
void console_write(char *dir, char *command) {
    memset(senddir, 0, 64);
    strcpy(senddir, dir);
    strcat(senddir, "/");
    strcat(senddir, get_argument(command, 1));
    
    file f = vfs_file_open(senddir, "w");
    if(f.type != FS_FILE) {
        printk("write: file %s not found\n", senddir);
    } else {
        vfs_file_write(&f, get_argument(command, 2));
    }
}

/**
 * Gets the current working directory
 */
char *console_pwd() {
    return get_dir();
}

/**
 * Prints the content of the file
 */
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

/**
 * Prints memory info
 */
void print_meminfo() {
    printk("Total mem: %d MB\nFree mem: %d MB\n", get_mem_size() / 1024, (get_max_blocks() - get_used_blocks()) * 4 / 1024);
    printk("Heap size: %d KB Free heap: %d KB\n", get_heap_size() / 1024, (get_heap_size() - get_used_heap()) / 1024);
    printk("cr0: %x cr2: %x cr3: %x\n", get_cr0(), get_cr2(), get_pdbr());
}

/**
 * Returns the next argument
 */
char *get_argument(char *command, int n) {
    for(int i = 0; i < n; i++) {
        command = strchr(command, ' ');
        if(command) {
            command++;
        } else {
            return command;
        }
    }
    return command;
}
