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

#ifndef PROC_H
#define PROC_H

#include <hal/tss.h>
#include <mm/memory.h>
#include <types.h>

#define PROC_NULL       -1

#define PROC_STOPPED    0
#define PROC_ACTIVE     1
#define PROC_NEW        2

struct regs {
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t cs;
};

struct regs_error {
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t error;
    uint32_t eip;
    uint32_t cs;
};

typedef struct thread {
    pid_t pid;
    int main;
    int state;
    uint32_t eip;
    uint32_t esp;
    uint32_t stack_limit;
    uint32_t heap;
    uint32_t heap_limit;
    uint32_t image_base;
    uint32_t image_size;
    struct thread *next;
    struct thread *prec;
} thread_t;

typedef struct proc {
    char name[16];
    int state;
    struct page_directory *pdir;
    int threads;
    thread_t *thread_list;
    struct proc *next;
    struct proc *prec;
} process_t;

extern void end_process();

int start_proc(char *name, char *arguments);
thread_t *create_thread();
int load_elf(char *name, thread_t *thread, struct page_directory *pdir);
int build_stack(thread_t *thread, struct page_directory *pdir, int nthreads);
int build_heap(thread_t *thread, struct page_directory *pdir, int nthreads);
int start_thread(uint32_t eip);
void stop_thread(int code);
void end_proc(int ret);
int proc_state(int id);

#endif

