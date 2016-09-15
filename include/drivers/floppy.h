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

#ifndef FLOPPY_H
#define FLOPPY_H

#include <types.h>

enum floppy_io {
    FLOPPY_DOR     = 0x3F2,
    FLOPPY_MSR     = 0x3F4,
    FLOPPY_FIFO    = 0x3F5,
    FLOPPY_CTRL    = 0x3F7
};

enum floppy_cmd {
    FLOPPY_CMD_READ_TRACK = 2,
    FLOPPY_CMD_SPECIFY = 3,
    FLOPPY_CMD_CHECK_STAT = 4,
    FLOPPY_CMD_WRITE_SECT = 5,
    FLOPPY_CMD_READ_SECT = 6,
    FLOPPY_CMD_CALIBRATE = 7,
    FLOPPY_CMD_CHECK_INT = 8,
    FLOPPY_CMD_WRITE_DEL_S = 9,
    FLOPPY_CMD_READ_ID_S = 0xA,
    FLOPPY_CMD_READ_DEL_S = 0xC,
    FLOPPY_CMD_FORMAT_TRACK = 0xD,
    FLOPPY_CMD_SEEK = 0xF
};

enum floppy_cmd_ext {
    FLOPPY_CMD_EXT_SKIP = 0x20,
    FLOPPY_CMD_EXT_DENSITY = 0x40,
    FLOPPY_CMD_EXT_MULTITRACK = 0x80
};

enum floppy_gap3_length {
    FLOPPY_GAP3_LENGTH_STD = 42,
    FLOPPY_GAP3_LENGTH_5_14 = 32,
    FLOPPY_GAP3_LENGTH_3_5 = 27
};

enum floppy_sector_dtl {
    FLOPPY_SECTOR_DTL_128 = 0,
    FLOPPY_SECTOR_DTL_256 = 1,
    FLOPPY_SECTOR_DTL_512 = 2,
    FLOPPY_SECTOR_DTL_1024 = 4
};

#define FLOPPY_SECTORS_PER_TRACK 18

// DOR Reg
#define FLOPPY_DOR_MASK_DRIVE0          0
#define FLOPPY_DOR_MASK_DRIVE1          1
#define FLOPPY_DOR_MASK_DRIVE2          2
#define FLOPPY_DOR_MASK_DRIVE3          3
#define FLOPPY_DOR_MASK_RESET           4
#define FLOPPY_DOR_MASK_DMA             8
#define FLOPPY_DOR_MASK_DRIVE0_MOTOR    16
#define FLOPPY_DOR_MASK_DRIVE1_MOTOR    32
#define FLOPPY_DOR_MASK_DRIVE2_MOTOR    64
#define FLOPPY_DOR_MASK_DRIVE3_MOTOR    128

// MSR Reg
#define FLOPPY_MSR_MASK_DRIVE0_POS_MODE     1
#define FLOPPY_MSR_MASK_DRIVE1_POS_MODE     2
#define FLOPPY_MSR_MASK_DRIVE2_POS_MODE     4
#define FLOPPY_MSR_MASK_DRIVE3_POS_MODE     8
#define FLOPPY_MSR_MASK_BUSY                16
#define FLOPPY_MSR_MASK_DMA                 32
#define FLOPPY_MSR_MASK_DATAIO              64
#define FLOPPY_MSR_MASK_DATAREG             128

void floppy_init();
void floppy_wait_irq();
void floppy_dma_init();
void floppy_write_dor(uint8_t val);
uint8_t floppy_read_status();
void floppy_send_cmd(uint8_t cmd);
uint8_t floppy_read_data();
void floppy_write_ccr(uint8_t val);
void floppy_read_sector_imp(uint8_t head, uint8_t track, uint8_t sector);
char *floppy_read_sector(int lba);
int floppy_write_sector_imp(uint8_t head, uint8_t track, uint8_t sector);
int floppy_write_sector(int lba);
void floppy_drive_data(uint32_t stepr, uint32_t loadt, uint32_t unloadt, int dma);
int floppy_calibrate(uint32_t drive);
void floppy_check_int(uint32_t * st0, uint32_t *cyl);
int floppy_seek(uint32_t cyl, uint32_t head);
void floppy_disable();
void floppy_enable();
void floppy_reset();
void floppy_control_motor(int on);
void floppy_lba_to_chs(int lba, int *head, int *track, int *sector);
int floppy_detect_drives();
void floppy_set_cur_drive(int drive);

#endif

