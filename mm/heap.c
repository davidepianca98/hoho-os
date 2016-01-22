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
#include <mm/heap.h>
#include <mm/mm.h>
#include <mm/paging.h>
#include <drivers/video.h>

heap_info_t *heap_info;

void heap_init(vmm_addr_t *addr) {
    heap_info = (heap_info_t *) addr;
    heap_info->start = addr + sizeof(heap_info_t);
    heap_info->size = 4096;
    heap_info->used = sizeof(heap_header_t) + sizeof(heap_info_t);
    heap_info->first_header = (heap_header_t *) addr;
    heap_info->first_header->magic = HEAP_MAGIC;
    heap_info->first_header->size = heap_info->size;
    heap_info->first_header->is_free = 1;
    heap_info->first_header->next = NULL;
}

void *umalloc(size_t len) {
    void *ptr = first_free_usr(len);
    if(ptr)
        return ptr;
    return NULL;
}

void ufree(void *ptr) {
    heap_header_t *head = ptr - sizeof(heap_header_t);
    if((head->is_free == 0) && (head->magic == HEAP_MAGIC)) {
        head->is_free = 1;
        heap_info->used -= head->size;
        
        // merge contiguous free sections
        heap_header_t *app = head->next;
        while((app != NULL) && (app->is_free == 1)) {
            head->size += app->size + sizeof(heap_header_t);
            head->next = app->next;
            
            app = app->next;
        }
    }
}

void *first_free_usr(size_t len) {
    heap_header_t *head = (heap_header_t *) heap_info->first_header;
    
    if(heap_info->used >= heap_info->size)
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
            heap_info->used += len + sizeof(heap_header_t);
            return (void *) head + sizeof(heap_header_t);
        }
        head = head->next;
    }
    return NULL;
}

