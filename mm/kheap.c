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
#include <mm/kheap.h>
#include <mm/mm.h>
#include <mm/paging.h>
#include <drivers/video.h>

#define HEAP_SIZE 256 // 256 pages, 256 * 4 KB = 1 MB

heap_info_t heap_info;

/**
 * Init the kernel heap memory
 */
void kheap_init() {
    heap_info.start = (vmm_addr_t *) ROUNDUP((uint32_t) get_mem_map() + get_max_blocks(), 4096);
    heap_info.size = HEAP_SIZE * BLOCKS_LEN;
    heap_info.used = sizeof(heap_header_t);
    heap_info.first_header = (heap_header_t *) heap_info.start;
    heap_info.first_header->magic = HEAP_MAGIC;
    heap_info.first_header->size = heap_info.size;
    heap_info.first_header->is_free = 1;
    heap_info.first_header->next = NULL;
    
    // Reserve the area for the kernel heap
    pmm_deinit_reg((mm_addr_t) heap_info.start, heap_info.size);
}

void *kmalloc(size_t len) {
    return first_free(len);
}

void kfree(void *ptr) {
    heap_header_t *head = ptr - sizeof(heap_header_t);
    if((head->is_free == 0) && (head->magic == HEAP_MAGIC)) {
        head->is_free = 1;
        //print_header(head);
        heap_info.used -= head->size;
        
        // merge contiguous free sections
        heap_header_t *app = head->next;
        while((app != NULL) && (app->is_free == 1)) {
            head->size += app->size + sizeof(heap_header_t);
            head->next = app->next;
            
            app = app->next;
        }
    }
}

void *first_free(size_t len) {
    heap_header_t *head = (heap_header_t *) heap_info.first_header;
    
    if(heap_info.used >= heap_info.size)
        return NULL;
    
    while(head != NULL) {
        if((head->size >= len) && (head->is_free == 1) && (head->magic == HEAP_MAGIC)) {
            head->is_free = 0;
            heap_header_t *head2 = (heap_header_t *) head + len + sizeof(heap_header_t);
            head2->size = head->size - len - sizeof(heap_header_t);
            head2->magic = HEAP_MAGIC;
            head2->is_free = 1;
            head2->next = NULL;
            head->next = head2;
            head->size = len;
            //print_header(head);
            heap_info.used += len + sizeof(heap_header_t);
            return (void *) head + sizeof(heap_header_t);
        }
        head = head->next;
    }
    return NULL;
}

int get_heap_size() {
    return heap_info.size;
}

int get_used_heap() {
    return heap_info.used;
}

void print_header(heap_header_t *head) {
    printk("Size: %d Is free: %d Next: %x\n", head->size, head->is_free, head->next);
}

