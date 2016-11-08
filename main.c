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
#include <fs/fat.h>
#include <hal/hal.h>
#include <drivers/keyboard.h>
#include <mm/memory.h>
#include <multiboot.h>
#include <fs/vfs.h>
#include <drivers/video.h>
#include <proc/sched.h>

int kmain(multiboot_info_t *info) {
    
    video_init(25, 80);
    clear();
    
    pmm_init(info->mem_high + info->mem_low, (uint32_t *) info->mmap_addr, info->mmap_len);
    vmm_init();
    kheap_init();
    
    vbe_init(info);
    
    hal_init();
    
    keyboard_init();
    
    syscall_init();
    install_tss();
    
    vfs_init();
    floppy_init();
    ata_init();
    
    sched_init();
    
    while(1)
        halt();
    
    return 0;
}

