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
#include <drivers/keyboard.h>
#include <drivers/video.h>
#include <fs/vfs.h>
#include <hal/hal.h>
#include <lib/string.h>
#include <proc/proc.h>
#include <proc/sched.h>

char senddir[64];
char dir[64];

extern uint32_t kernel_start;
extern uint32_t kernel_end;

/**
 * Gets the current working directory
 */
char *get_dir() {
    return dir;
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
 * Prints either to text or to gui console
 */
void console_print(char *buffer, ...) {
    char str[256];
    va_list args;
    
    va_start(args, buffer);
    vsprintf(str, buffer, args);
    va_end(args);
    
    if(is_text_mode()) {
        printk_string(str);
    } else {
        console_gui_print(str);
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
    } else if(strncmp(buf, "touch", 5) == 0) {
        console_touch(dir, buf);
    } else if(strncmp(buf, "delete", 6) == 0) {
        console_delete(dir, buf);
    } else if(strcmp(buf, "hoho") == 0) {
        console_print("hoho\n");
    } else if(strcmp(buf, "help") == 0) {
        console_print("Help:\nhoho - prints hoho\nhelp - shows help\nmeminfo - prints RAM info\ncpuinfo - shows CPU info\nls - shows filesystem devices\nread - reads a file\nstart - starts a program\nclear - clears the screen\nhalt - shuts down\nreboot - reboots the pc\n");
    } else if(strcmp(buf, "meminfo") == 0) {
        print_meminfo();
    } else if(strcmp(buf, "cpuinfo") == 0) {
        console_print("%s\n", get_cpu_vendor(0));
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
        console_print("Shutting down\n");
        halt();
        while(1);
    } else if(strcmp(buf, "reboot") == 0) {
        console_print("Rebooting\n");
        reboot();
    } else {
        console_print("Command not found\n");
    }
}

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
            console_print("cd %s: directory not found\n", senddir);
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
        console_print("\n");
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
    file *f = vfs_file_open(senddir, "r");
    if(f->type != FS_FILE) {
        console_print("read: file %s not found\n", senddir);
    } else {
        print_file(f);
    }
    vfs_file_close(f);
}

/**
 * Writes the input to the file
 */
void console_write(char *dir, char *command) {
    memset(senddir, 0, 64);
    strcpy(senddir, dir);
    strcat(senddir, "/");
    strcat(senddir, get_argument(command, 1));
    
    file *f = vfs_file_open(senddir, "w");
    if(f->type != FS_FILE) {
        console_print("write: file %s not found\n", senddir);
    } else {
        vfs_file_write(f, get_argument(command, 2));
    }
    vfs_file_close(f);
}

/**
 * Creates a file
 */
void console_touch(char *dir, char *command) {
    memset(senddir, 0, 64);
    strcpy(senddir, dir);
    strcat(senddir, "/");
    strcat(senddir, get_argument(command, 1));
    
    if(!vfs_touch(senddir)) {
        console_print("touch: Error creating file\n");
    }
}

/**
 * Deletes a file
 */
void console_delete(char *dir, char *command) {
    memset(senddir, 0, 64);
    strcpy(senddir, dir);
    strcat(senddir, "/");
    strcat(senddir, get_argument(command, 1));
    
    if(!vfs_delete(senddir)) {
        console_print("delete: Error deleting file\n");
    }
}

/**
 * Gets the current working directory
 */
char *console_pwd() {
    return get_dir();
}

char *console_pwd_user() {
    process_t *cur = get_cur_proc();
    if(cur && cur->thread_list) {
        char *dir = console_pwd();
        char *pwd_dir = (char *) umalloc(strlen(dir), (vmm_addr_t *) cur->thread_list->heap);
        strcpy(pwd_dir, dir);
        return pwd_dir;
    }
    return NULL;
}

/**
 * Prints the content of the file
 */
void print_file(file *f) {
    if(f->type == FS_NULL) {
        console_print("Cannot open file\n");
        return;
    }

    if((f->type & FS_DIR) == FS_DIR) {
        console_print("Cannot display content of directory.\n");
        return;
    }

    while(f->eof != 1) {
        char buf[512];
        vfs_file_read(f, buf);

        for(int i = 0; i < 512; i++)
            console_print("%c", buf[i]);

        if(f->eof != 1) {
        	getchar();
        }
    }
}

/**
 * Prints memory info
 */
void print_meminfo() {
    console_print("Total mem: %d MB\nFree mem: %d MB\n", get_mem_size() / 1024, (get_max_blocks() - get_used_blocks()) * 4 / 1024);
    console_print("Heap size: %d KB Free heap: %d KB\n", get_heap_size() / 1024, (get_heap_size() - get_used_heap()) / 1024);
    console_print("cr0: %x cr2: %x cr3: %x\n", get_cr0(), get_cr2(), get_pdbr());
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
