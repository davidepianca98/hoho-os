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

#ifndef THREAD_H
#define THREAD_H

#include <types.h>

typedef struct thread {
    pid_t pid;                      // thread id
    int time;                       // thread's time slice in ms
    int main;                       // if it's the main thread
    int state;                      // thread's state
    void *parent;                   // pointer to proc
    uint32_t eip;                   // start instruction pointer
    uint32_t esp;                   // start thread's stack pointer
    uint32_t stack_limit;           // thread's stack limit pointer
    uint32_t esp_kernel;            // thread's kernel stack pointer
    uint32_t stack_kernel_limit;    // thread's kernel stack limit
    uint32_t heap;                  // thread's heap pointer
    uint32_t heap_limit;            // thread's heap limit pointer
    uint32_t image_base;
    uint32_t image_size;
    struct thread *next;
    struct thread *prec;
} thread_t;

thread_t *create_thread();
int start_thread();
void stop_thread(int code);

#endif

