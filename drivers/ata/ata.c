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
#include <hal/device.h>
#include <drivers/video.h>
#include <lib/string.h>
#include <fs/fat.h>

static ata_drives_t ata_info;
uint8_t ata_irq_done = 0;

static device_t dev_info[4];

extern void ata_int();

void ata_init() {
    install_ir(46, 0x80 | 0x0E, 0x8, &ata_int);
    install_ir(47, 0x80 | 0x0E, 0x8, &ata_int);
    ata_info_fill(&ata_info.primary_master, 1, ATA_PRIMARY_DATA, ATA_PRIMARY_ERR, ATA_PRIMARY_SECTORS, ATA_PRIMARY_LBA_LOW, ATA_PRIMARY_LBA_MID, ATA_PRIMARY_LBA_HIGH, ATA_PRIMARY_DRIVE_SEL, ATA_PRIMARY_STATUS, ATA_PRIMARY_IRQ);
    ata_info_fill(&ata_info.primary_slave, 0, ATA_PRIMARY_DATA, ATA_PRIMARY_ERR, ATA_PRIMARY_SECTORS, ATA_PRIMARY_LBA_LOW, ATA_PRIMARY_LBA_MID, ATA_PRIMARY_LBA_HIGH, ATA_PRIMARY_DRIVE_SEL, ATA_PRIMARY_STATUS, ATA_PRIMARY_IRQ);
    ata_info_fill(&ata_info.secondary_master, 1, ATA_SECONDARY_DATA, ATA_SECONDARY_ERR, ATA_SECONDARY_SECTORS, ATA_SECONDARY_LBA_LOW, ATA_SECONDARY_LBA_MID, ATA_SECONDARY_LBA_HIGH, ATA_SECONDARY_DRIVE_SEL, ATA_SECONDARY_STATUS, ATA_SECONDARY_IRQ);
    ata_info_fill(&ata_info.secondary_slave, 0, ATA_SECONDARY_DATA, ATA_SECONDARY_ERR, ATA_SECONDARY_SECTORS, ATA_SECONDARY_LBA_LOW, ATA_SECONDARY_LBA_MID, ATA_SECONDARY_LBA_HIGH, ATA_SECONDARY_DRIVE_SEL, ATA_SECONDARY_STATUS, ATA_SECONDARY_IRQ);
    ata_info.cur_hdd = ata_info.primary_master;
    drive_t *temp_info = &ata_info.primary_master;
    for(int i = 0; i < 4; i++) {
        if(temp_info->present == 1) {
            dev_info[i].id = i;
            dev_info[i].type = 1;
            strcpy(dev_info[i].mount, "hd");
            dev_info[i].mount[2] = i + 'a';
            dev_info[i].mount[3] = 0;
            dev_info[i].read = &ata_read_sector;
            fat_init(&dev_info[i].fs);
            device_register(&dev_info[i]);
        }
        temp_info++;
    }
}

void ata_wait_for_irq() {
    while(ata_irq_done == 0);
    ata_irq_done = 0;
}

void ata_info_fill(drive_t *drive, int type, uint32_t data, uint32_t err, uint32_t sect, uint32_t lba_low, uint32_t lba_mid, uint32_t lba_high, uint32_t sel, uint32_t status, uint32_t irq) {
    drive->type = type;
    drive->data_reg = data;
    drive->err_reg = err;
    drive->sectors_reg = sect;
    drive->lba_low_reg = lba_low;
    drive->lba_mid_reg = lba_mid;
    drive->lba_high_reg = lba_high;
    drive->sel_reg = sel;
    drive->status_reg = status;
    drive->irq_num = irq;
    identify(drive);
}

void identify(drive_t *drive) {
    if(drive->type == 1)
        outportb(drive->sel_reg, 0xA0);
    else
        outportb(drive->sel_reg, 0xB0);
    outportb(drive->lba_low_reg, 0);
    outportb(drive->lba_mid_reg, 0);
    outportb(drive->lba_high_reg, 0);
    outportb(drive->status_reg, ATA_IDENTIFY);
    if(inportb(drive->status_reg) == 0) {
        drive->present = NULL;
        //printk("Drive not present\n");
    } else {
        if((inportb(drive->lba_mid_reg) != 0) || (inportb(drive->lba_high_reg) != 0)) {
            drive->present = NULL;
            //printk("Not an ATA drive\n");
        } else {
            while((inportb(drive->status_reg) & 0x80) != 0);
            while(((inportb(drive->status_reg) & 0x8) != 8) && ((inportb(drive->status_reg) & 0x0) == 0));
            if((inportb(drive->status_reg) & 0x0) == 1) {
                drive->present = NULL;
                //printk("Error bit set\n");
            } else {
                drive->present = 1;
                //printk("Drive available\n");
                for(int i = 0; i < 256; i++) {
                    switch(i) {
                        case 83:
                            if(inportw(drive->data_reg) & 0x400)
                                drive->lba_mode = 1;
                            else
                                drive->lba_mode = 0;
                            break;
                        case 60:
                            drive->total_sectors_28 = inportw(drive->data_reg);
                            drive->total_sectors_28 <<= 16;
                            break;
                        case 61:
                            drive->total_sectors_28 |= inportw(drive->data_reg);
                            break;
                        case 100:
                            drive->total_sectors_48 = inportw(drive->data_reg);
                            drive->total_sectors_48 <<= 16;
                            break;
                        case 101:
                        case 102:
                            drive->total_sectors_48 |= inportw(drive->data_reg);
                            drive->total_sectors_48 <<= 16;
                            break;
                        case 103:
                            drive->total_sectors_48 |= inportw(drive->data_reg);
                            break;
                        default:
                            inportw(drive->data_reg);
                            break;
                    }
                }
                //printk("Drive lba mode: %d\n", drive->lba_mode);
                //printk("secs 28: %d secs 48: %d\n", drive->total_sectors_28, drive->total_sectors_48);
            }
        }
    }
}

void delay_400ns() {
    
}

char *ata_read_sector(int lba) {
    char *buf = (char *) kmalloc(512);
    outportb(ata_info.cur_hdd.sel_reg, 0xE0 | (ata_info.cur_hdd.type << 4) | ((lba >> 24) & 0x0F));
    outportb(ata_info.cur_hdd.err_reg, 0x00);
    outportb(ata_info.cur_hdd.sectors_reg, (uint8_t) 1);
    outportb(ata_info.cur_hdd.lba_low_reg, (uint8_t) lba);
    outportb(ata_info.cur_hdd.lba_mid_reg, (uint8_t) (lba >> 8));
    outportb(ata_info.cur_hdd.lba_high_reg, (uint8_t) (lba >> 16));
    outportb(ata_info.cur_hdd.status_reg, 0x20);
    //ata_wait_for_irq();
    
    for(int i = 0; i < 256; i++) {
        uint16_t tmp = inportw(ata_info.cur_hdd.data_reg);
        buf[i * 2] = (char) tmp;
        buf[i * 2 + 1] = (char) (tmp >> 8);
    }
    delay_400ns();
    return buf;
}

