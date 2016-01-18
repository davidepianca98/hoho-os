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

#ifndef ATA_H
#define ATA_H

#include <types.h>

#define ATA_PRIMARY_DATA            0x1F0
#define ATA_PRIMARY_ERR             0x1F1
#define ATA_PRIMARY_SECTORS         0x1F2
#define ATA_PRIMARY_LBA_LOW         0x1F3
#define ATA_PRIMARY_LBA_MID         0x1F4
#define ATA_PRIMARY_LBA_HIGH        0x1F5
#define ATA_PRIMARY_DRIVE_SEL       0x1F6
#define ATA_PRIMARY_STATUS          0x1F7
#define ATA_PRIMARY_IRQ             14

#define ATA_SECONDARY_DATA          0x170
#define ATA_SECONDARY_ERR           0x171
#define ATA_SECONDARY_SECTORS       0x172
#define ATA_SECONDARY_LBA_LOW       0x173
#define ATA_SECONDARY_LBA_MID       0x174
#define ATA_SECONDARY_LBA_HIGH      0x175
#define ATA_SECONDARY_DRIVE_SEL     0x176
#define ATA_SECONDARY_STATUS        0x177
#define ATA_SECONDARY_IRQ           15

#define ATA_IDENTIFY                0xEC

typedef struct ata_drive {
    int present;
    int type; //master or slave
    uint32_t data_reg;
    uint32_t err_reg;
    uint32_t sectors_reg;
    uint32_t lba_low_reg;
    uint32_t lba_mid_reg;
    uint32_t lba_high_reg;
    uint32_t sel_reg;
    uint32_t status_reg;
    uint32_t irq_num;
    int lba_mode; // 1 LBA48 else LBA28
    uint32_t total_sectors_28;
    uint64_t total_sectors_48;
} drive_t;

typedef struct ata_drives {
    drive_t cur_hdd;
    drive_t primary_master;
    drive_t primary_slave;
    drive_t secondary_master;
    drive_t secondary_slave;
} ata_drives_t;

void ata_init();
void ata_wait_for_irq();
void ata_info_fill(drive_t *drive, int type, uint32_t data, uint32_t err, uint32_t sect, uint32_t lba_low, uint32_t lba_mid, uint32_t lba_high, uint32_t sel, uint32_t status, uint32_t irq);
void identify(drive_t *drive);
void delay_400ns();
char *ata_read_sector(int lba);

#endif

