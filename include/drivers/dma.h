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

#ifndef DMA_H
#define DMA_H

#include <hal/hal.h>
#include <types.h>

#define DMA0_STATUS_REG             0x08
#define DMA0_COMMAND_REG            0x08
#define DMA0_REQUEST_REG            0x09
#define DMA0_CHANMASK_REG           0x0A
#define DMA0_MODE_REG               0x0B
#define DMA0_CLEARBYTE_FLIPFLOP_REG 0x0C
#define DMA0_TEMP_REG               0X0D
#define DMA0_MASTER_CLEAR_REG       0X0D
#define DMA0_CLEAR_MASK_REG         0X0E
#define DMA0_MASK_REG               0X0F

#define DMA1_STATUS_REG             0xD0
#define DMA1_COMMAND_REG            0xD0
#define DMA1_REQUEST_REG            0xD2
#define DMA1_CHANMASK_REG           0xD4
#define DMA1_MODE_REG               0xD6
#define DMA1_CLEARBYTE_FLIPFLOP_REG 0xD8
#define DMA1_INTER_REG              0XDA
#define DMA1_UNMASK_ALL_REG         0XDC
#define DMA1_MASK_REG               0XDE

#define DMA0_CHAN0_ADDR_REG         0
#define DMA0_CHAN0_COUNT_REG        1
#define DMA0_CHAN1_ADDR_REG         2
#define DMA0_CHAN1_COUNT_REG        3
#define DMA0_CHAN2_ADDR_REG         4
#define DMA0_CHAN2_COUNT_REG        5
#define DMA0_CHAN3_ADDR_REG         6
#define DMA0_CHAN3_COUNT_REG        7

#define DMA1_CHAN4_ADDR_REG         0xC0
#define DMA1_CHAN4_COUNT_REG        0xC2
#define DMA1_CHAN5_ADDR_REG         0xC4
#define DMA1_CHAN5_COUNT_REG        0xC6
#define DMA1_CHAN6_ADDR_REG         0xC8
#define DMA1_CHAN6_COUNT_REG        0xCA
#define DMA1_CHAN7_ADDR_REG         0xCC
#define DMA1_CHAN7_COUNT_REG        0xCE

#define DMA_PAGE_EXTRA0             0x80
#define DMA_PAGE_CHAN2_ADDRBYTE2    0x81
#define DMA_PAGE_CHAN3_ADDRBYTE2    0x82
#define DMA_PAGE_CHAN1_ADDRBYTE2    0x83
#define DMA_PAGE_EXTRA1             0x84
#define DMA_PAGE_EXTRA2             0x85
#define DMA_PAGE_EXTRA3             0x86
#define DMA_PAGE_CHAN6_ADDRBYTE2    0x87
#define DMA_PAGE_CHAN7_ADDRBYTE2    0x88
#define DMA_PAGE_CHAN5_ADDRBYTE2    0x89
#define DMA_PAGE_EXTRA4             0x8C
#define DMA_PAGE_EXTRA5             0x8D
#define DMA_PAGE_EXTRA6             0x8E
#define DMA_PAGE_DRAM_REFRESH       0x8F

#define DMA_CMD_MASK_MEMTOMEM       1
#define DMA_CMD_MASK_CHAN0ADDRHOLD  2
#define DMA_CMD_MASK_ENABLE         4
#define DMA_CMD_MASK_TIMING         8
#define DMA_CMD_MASK_PRIORITY       0x10
#define DMA_CMD_MASK_WRITESEL       0x20
#define DMA_CMD_MASK_DREQ           0x40
#define DMA_CMD_MASK_DACK           0x80

#define DMA_MODE_MASK_SEL           3

#define DMA_MODE_MASK_TRA           0xC
#define DMA_MODE_SELF_TEST          0
#define DMA_MODE_READ_TRANSFER      4
#define DMA_MODE_WRITE_TRANSFER     8

#define DMA_MODE_MASK_AUTO          0x10
#define DMA_MODE_MASK_IDEC          0x20

#define DMA_MODE_MASK               0xC0
#define DMA_MODE_TRANSFER_ON_DEMAND 0
#define DMA_MODE_TRANSFER_SINGLE    0x40
#define DMA_MODE_TRANSFER_BLOCK     0x80
#define DMA_MODE_TRANSFER_CASCADE   0xC0

void dma_set_address(uint8_t channel, uint8_t low, uint8_t high);
void dma_set_count(uint8_t channel, uint8_t low, uint8_t high);
void dma_set_external_page_register(uint8_t reg, uint8_t val);
void dma_set_mode(uint8_t channel, uint8_t mode);
void dma_set_read(uint8_t channel);
void dma_set_write(uint8_t channel);
void dma_mask_channel(uint8_t channel);
void dma_unmask_channel(uint8_t channel);
void dma_reset_flipflop(int dma);
void dma_reset();
void dma_unmask_all();

#endif

