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

#ifndef HEAP_H
#define HEAP_H

#include <types.h>
#include <mm/paging.h>

#define HEAP_MAGIC      0xA0B0C0

typedef struct heap_header {
    int magic; // sanity check: 0xA0B0C0
    size_t size;
    int is_free;
    struct heap_header *next;
} heap_header_t;

typedef struct {
    size_t size;
    vmm_addr_t *start;
    size_t used;
    heap_header_t *first_header;
} heap_info_t;

void kheap_init();
void *kmalloc(size_t len);
void kfree(void *ptr);

void *first_free(size_t len);

int get_heap_size();
int get_used_heap();

void print_header(heap_header_t *head);

#endif

