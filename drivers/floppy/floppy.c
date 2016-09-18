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

#include <drivers/floppy.h>
#include <hal/hal.h>
#include <hal/device.h>
#include <drivers/video.h>
#include <lib/string.h>
#include <fs/fat.h>

#define FLOPPY_DMA_LEN 0x4800
#define FLOPPY_DMA_CHANNEL 2

uint8_t floppy_irq_done = 0;
int cur_drive = 0;
static device_t dev_info[4];

static const char floppy_dmabuf[FLOPPY_DMA_LEN] __attribute__((aligned(0x8000)));
uint32_t *dma_buffer = (uint32_t *) &floppy_dmabuf;

static char *drive_types[8] = {
    "none",
    "360kB 5.25",
    "1.2MB 5.25",
    "720kB 3.5",

    "1.44MB 3.5",
    "2.88MB 3.5",
    "unknown type",
    "unknown type"
};

static const char *status[] = {
    0,
    "error",
    "invalid",
    "drive"
};

extern void floppy_int();

void floppy_init() {
    install_ir(38, 0x80 | 0x0E, 0x8, &floppy_int);
    int ndrives = floppy_detect_drives();
    if(ndrives > 0) {
        floppy_reset();
        floppy_drive_data(13, 1, 0xF, 1);
    }
}

void floppy_wait_irq() {
    while(floppy_irq_done == 0);
    floppy_irq_done = 0;
}

void floppy_dma_init() {
    union {
        uint8_t byte[4];
        uint32_t l;
    } a, c;
    
    a.l = (uint32_t) &floppy_dmabuf;
    c.l = FLOPPY_DMA_LEN - 1;
    
    if((a.l >> 24) || (c.l >> 16) || (((a.l & 0xFFFF) + c.l) >> 16)) {
        printk("Wrong DMA address\n");
        return;
    }
    dma_reset();
    dma_mask_channel(FLOPPY_DMA_CHANNEL);
    dma_reset_flipflop(0);
    dma_set_address(FLOPPY_DMA_CHANNEL, a.byte[0], a.byte[1]);
    dma_set_external_page_register(FLOPPY_DMA_CHANNEL, a.byte[2]);
    dma_reset_flipflop(0);
    dma_set_count(FLOPPY_DMA_CHANNEL, c.byte[0], c.byte[1]);
    dma_set_read(FLOPPY_DMA_CHANNEL);
    dma_unmask_all();
}

void floppy_write_dor(uint8_t val) {
    outportb(FLOPPY_DOR, val);
}

uint8_t floppy_read_status() {
    return inportb(FLOPPY_MSR);
}

void floppy_send_cmd(uint8_t cmd) {
    int i;
    for(i = 0; i < 500; i++)
        if(floppy_read_status() & FLOPPY_MSR_MASK_DATAREG)
            return outportb(FLOPPY_FIFO, cmd);
}

uint8_t floppy_read_data() {
    int i;
    for(i = 0; i < 500; i++)
        if(floppy_read_status() & FLOPPY_MSR_MASK_DATAREG)
            return inportb(FLOPPY_FIFO);
    return NULL;
}

void floppy_write_ccr(uint8_t val) {
    outportb(FLOPPY_CTRL, val);
}

void floppy_read_sector_imp(uint8_t head, uint8_t track, uint8_t sector) {
    uint32_t st0, cyl;
    
    floppy_dma_init();
    dma_set_read(FLOPPY_DMA_CHANNEL);
    floppy_send_cmd(FLOPPY_CMD_READ_SECT | FLOPPY_CMD_EXT_MULTITRACK | FLOPPY_CMD_EXT_SKIP | FLOPPY_CMD_EXT_DENSITY);
    floppy_send_cmd((head << 2) | cur_drive);
    floppy_send_cmd(track);
    floppy_send_cmd(head);
    floppy_send_cmd(sector);
    floppy_send_cmd(FLOPPY_SECTOR_DTL_512);
    floppy_send_cmd(((sector + 1) >= FLOPPY_SECTORS_PER_TRACK) ? FLOPPY_SECTORS_PER_TRACK : sector + 1);
    floppy_send_cmd(FLOPPY_GAP3_LENGTH_3_5);
    floppy_send_cmd(0xFF);
    
    floppy_wait_irq();
    for(int i = 0; i < 7; i++)
        floppy_read_data();
    floppy_check_int(&st0, &cyl);
    
    if(st0 & 0x08) {
        printk("floppy_read_sector: drive not ready\n");
    }
}

char *floppy_read_sector(int lba) {
    if(cur_drive > 3)
        return (char *) -1;
    int head = 0, track = 0, sector = 1;
    floppy_lba_to_chs(lba, &head, &track, &sector);
    floppy_control_motor(1);
    if(floppy_seek((uint8_t) track, (uint8_t) head) != 0) {
        floppy_control_motor(0);
        return 0;
    }
    floppy_read_sector_imp((uint8_t) head, (uint8_t) track, (uint8_t) sector);
    floppy_control_motor(0);
    return (char *) &floppy_dmabuf;
}

int floppy_write_sector_imp(uint8_t head, uint8_t track, uint8_t sector) {
    uint32_t st0, cyl;
    
    floppy_dma_init();
    dma_set_write(FLOPPY_DMA_CHANNEL);
    floppy_send_cmd(FLOPPY_CMD_WRITE_SECT | FLOPPY_CMD_EXT_MULTITRACK | FLOPPY_CMD_EXT_SKIP | FLOPPY_CMD_EXT_DENSITY);
    floppy_send_cmd((head << 2) | cur_drive);
    floppy_send_cmd(track);
    floppy_send_cmd(head);
    floppy_send_cmd(sector);
    floppy_send_cmd(FLOPPY_SECTOR_DTL_512);
    floppy_send_cmd(((sector + 1) >= FLOPPY_SECTORS_PER_TRACK) ? FLOPPY_SECTORS_PER_TRACK : sector + 1);
    floppy_send_cmd(FLOPPY_GAP3_LENGTH_3_5);
    floppy_send_cmd(0xFF);
    
    floppy_wait_irq();
    for(int i = 0; i < 7; i++)
        floppy_read_data();
    floppy_check_int(&st0, &cyl);
    
    if(st0 & 0xC0) {
        printk("floppy_write_sector: status = %s\n", status[st0 >> 6]);
        return 0;
    }
    if(st0 & 0x08) {
        printk("floppy_write_sector: drive not ready\n");
        return 0;
    }
    return 1;
}

int floppy_write_sector(int lba) {
    if(cur_drive > 3)
        return -1;
    int head = 0, track = 0, sector = 1;
    floppy_lba_to_chs(lba, &head, &track, &sector);
    floppy_control_motor(1);
    if(floppy_seek((uint8_t) track, (uint8_t) head) != 0) {
        floppy_control_motor(0);
        return -1;
    }
    if(!floppy_write_sector_imp((uint8_t) head, (uint8_t) track, (uint8_t) sector)) {
        floppy_control_motor(0);
        return -1;
    }
    floppy_control_motor(0);
    return 1;
}

void floppy_drive_data(uint32_t stepr, uint32_t loadt, uint32_t unloadt, int dma) {
    if(cur_drive > 3)
        return;
    uint32_t data = 0;
    floppy_send_cmd(FLOPPY_CMD_SPECIFY);
    data = ((stepr & 0xF) << 4) | (unloadt & 0xF);
    floppy_send_cmd(data);
    data = (loadt << 1) | ((dma == 1) ? 1 : 0);
    floppy_send_cmd(data);
}

int floppy_calibrate(uint32_t drive) {
    uint32_t st0, cyl;
    
    if(drive > 3)
        return -1;

    floppy_control_motor(1);
    
    for(int i = 0; i < 10; i++) {
        floppy_send_cmd(FLOPPY_CMD_CALIBRATE);
        floppy_send_cmd(drive);
        floppy_wait_irq();
        floppy_check_int(&st0, &cyl);
        
        if(st0 & 0xC0) {
            printk("floppy_calibrate: status = %s\n", status[st0 >> 6]);
            continue;
        }
        
        if(!cyl) {
            floppy_control_motor(0);
            return 0;
        }
    }
    
    floppy_control_motor(0);
    printk("Error calibrating floppy\n");
    return -1;
}

void floppy_check_int(uint32_t * st0, uint32_t *cyl) {
    floppy_send_cmd(FLOPPY_CMD_CHECK_INT);
    *st0 = floppy_read_data();
    *cyl = floppy_read_data();
}

int floppy_seek(uint32_t cyl, uint32_t head) {
    uint32_t st0, cyl0 = -1;
    
    if(cur_drive > 3)
        return -1;
    
    floppy_control_motor(1);
    
    for(int i = 0; i < 10; i++) {
        floppy_send_cmd(FLOPPY_CMD_SEEK);
        floppy_send_cmd((head) << 2 | cur_drive);
        floppy_send_cmd(cyl);
        
        floppy_wait_irq();
        floppy_check_int(&st0, &cyl0);
        
        if(st0 & 0xC0) {
            printk("floppy_seek: status = %s\n", status[st0 >> 6]);
            continue;
        }
        
        if(cyl0 == cyl) {
            floppy_control_motor(0);
            return 0;
        }
    }
    floppy_control_motor(0);
    printk("Floppy seek failed\n");
    return -1;
}

void floppy_disable() {
    floppy_write_dor(0);
}

void floppy_enable() {
    floppy_write_dor(FLOPPY_DOR_MASK_RESET | FLOPPY_DOR_MASK_DMA);
}

void floppy_reset() {
    if(cur_drive > 3)
        return;
    uint32_t st0, cyl;
    floppy_disable();
    floppy_enable();
    floppy_wait_irq();
    for(int i = 0; i < 4; i++)
        floppy_check_int(&st0, &cyl);
    
    floppy_write_ccr(0);
    floppy_drive_data(3, 16, 240, 1);
    floppy_calibrate(cur_drive);
}

void floppy_control_motor(int on) {
    if(cur_drive > 3)
        return;
    
    uint32_t motor = 0;
    
    switch(cur_drive) {
        case 0:
            motor = FLOPPY_DOR_MASK_DRIVE0_MOTOR;
            break;
        case 1:
            motor = FLOPPY_DOR_MASK_DRIVE1_MOTOR;
            break;
        case 2:
            motor = FLOPPY_DOR_MASK_DRIVE2_MOTOR;
            break;
        case 3:
            motor = FLOPPY_DOR_MASK_DRIVE3_MOTOR;
            break;
    }
    
    if(on)
        floppy_write_dor(cur_drive | motor | FLOPPY_DOR_MASK_RESET | FLOPPY_DOR_MASK_DMA);
    else
        floppy_write_dor(FLOPPY_CMD_READ_DEL_S);
}

void floppy_lba_to_chs(int lba, int *head, int *track, int *sector) {
    *head = (lba % (FLOPPY_SECTORS_PER_TRACK * 2)) / FLOPPY_SECTORS_PER_TRACK;
    *track = lba / (FLOPPY_SECTORS_PER_TRACK * 2);
    *sector = (lba % FLOPPY_SECTORS_PER_TRACK) + 1;
}

int floppy_detect_drives() {
    outportb(0x70, 0x10);
    sleep(100);
    uint8_t drives = inportb(0x71);
    int ndrives = 0;
    
    if(strcmp(drive_types[drives >> 4], "1.44MB 3.5") == 0) {
        dev_info[0].id = 4;
        dev_info[0].type = 0;
        strcpy(dev_info[0].mount, "fd");
        dev_info[0].mount[2] = 'a';
        dev_info[0].mount[3] = 0;
        dev_info[0].read = &floppy_read_sector;
        dev_info[0].write = &floppy_write_sector;
        fat_init(&dev_info[0].fs);
        device_register(&dev_info[0]);
        cur_drive = 0;
        ndrives++;
    }
    if(strcmp(drive_types[drives & 0xF], "1.44MB 3.5") == 0) {
        dev_info[1].id = 5;
        dev_info[1].type = 0;
        strcpy(dev_info[1].mount, "fd");
        dev_info[1].mount[2] = 'b';
        dev_info[1].mount[3] = 0;
        dev_info[1].read = &floppy_read_sector;
        dev_info[1].write = &floppy_write_sector;
        fat_init(&dev_info[1].fs);
        device_register(&dev_info[1]);
        cur_drive = 1;
        ndrives++;
    }

    //printk(" - Floppy drive 0: %s\n", drive_types[drives >> 4]);
    //printk(" - Floppy drive 1: %s\n", drive_types[drives & 0xF]);
    return ndrives;
}

void floppy_set_cur_drive(int drive) {
    if((drive < 0) || (drive > 3))
        return;
    cur_drive = drive;
}

