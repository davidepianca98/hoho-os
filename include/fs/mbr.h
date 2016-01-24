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

#ifndef MBR_H
#define MBR_H

#include <types.h>

#define FAT12_SYSTEM_ID             0x1
#define FAT16_SYSTEM_ID             0x4
#define NTFS_SYSTEM_ID              0x7
#define FAT32_SYSTEM_ID             0xC //0xB

// 16 bytes
typedef struct partition {
    uint8_t bootable;
    uint8_t start_head;
    uint16_t start_sect_cyl;
    uint8_t sys_id;
    uint8_t end_head;
    uint16_t end_sect_cyl;
    uint32_t lba_start;
    uint32_t total_sectors;
} __attribute__((__packed__)) partition_t;

typedef struct mbr {
    uint8_t fill[436];
    uint8_t id[10];
    partition_t partition_table[4];
    uint8_t signature[2];
} __attribute__((__packed__)) mbr_t;

#endif

