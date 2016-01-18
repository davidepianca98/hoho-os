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

void dma_set_address(uint8_t channel, uint8_t low, uint8_t high) {
    if(channel > 8)
        return;
    
    uint16_t port = 0;
    switch(channel) {
        case 0:
            port = DMA0_CHAN0_ADDR_REG;
            break;
        case 1:
            port = DMA0_CHAN1_ADDR_REG;
            break;
        case 2:
            port = DMA0_CHAN2_ADDR_REG;
            break;
        case 3:
            port = DMA0_CHAN3_ADDR_REG;
            break;
        case 4:
            port = DMA1_CHAN4_ADDR_REG;
            break;
        case 5:
            port = DMA1_CHAN5_ADDR_REG;
            break;
        case 6:
            port = DMA1_CHAN6_ADDR_REG;
            break;
        case 7:
            port = DMA1_CHAN7_ADDR_REG;
            break;
    }
    
    outportb(port, low);
    outportb(port, high);
}

void dma_set_count(uint8_t channel, uint8_t low, uint8_t high) {
    if(channel > 8)
        return;
    
    uint16_t port = 0;
    switch(channel) {
        case 0:
            port = DMA0_CHAN0_COUNT_REG;
            break;
        case 1:
            port = DMA0_CHAN1_COUNT_REG;
            break;
        case 2:
            port = DMA0_CHAN2_COUNT_REG;
            break;
        case 3:
            port = DMA0_CHAN3_COUNT_REG;
            break;
        case 4:
            port = DMA1_CHAN4_COUNT_REG;
            break;
        case 5:
            port = DMA1_CHAN5_COUNT_REG;
            break;
        case 6:
            port = DMA1_CHAN6_COUNT_REG;
            break;
        case 7:
            port = DMA1_CHAN7_COUNT_REG;
            break;
    }
    
    outportb(port, low);
    outportb(port, high);
}

void dma_set_external_page_register(uint8_t reg, uint8_t val) {
    if(reg > 14)
        return;
    
    uint16_t port = 0;
    
    switch(reg) {
        case 1:
            port = DMA_PAGE_CHAN1_ADDRBYTE2;
            break;
        case 2:
            port = DMA_PAGE_CHAN2_ADDRBYTE2;
            break;
        case 3:
            port = DMA_PAGE_CHAN3_ADDRBYTE2;
            break;
        case 4:
            return;
        case 5:
            port = DMA_PAGE_CHAN5_ADDRBYTE2;
            break;
        case 6:
            port = DMA_PAGE_CHAN6_ADDRBYTE2;
            break;
        case 7:
            port = DMA_PAGE_CHAN7_ADDRBYTE2;
            break;
    }
    outportb(port, val);
}

void dma_set_mode(uint8_t channel, uint8_t mode) {
    int dma = (channel < 4) ? 0 : 1;
    int chan = (dma == 0) ? channel : channel - 4;

    dma_mask_channel(channel);
    outportb((channel < 4) ? (DMA0_MODE_REG) : DMA1_MODE_REG, chan | (mode));
    dma_unmask_all();
}

void dma_set_read(uint8_t channel) {
    dma_set_mode(channel, DMA_MODE_READ_TRANSFER | DMA_MODE_TRANSFER_SINGLE);
}

void dma_set_write(uint8_t channel) {
    dma_set_mode (channel, DMA_MODE_WRITE_TRANSFER | DMA_MODE_TRANSFER_SINGLE);
}

void dma_mask_channel(uint8_t channel) {
	if (channel <= 4)
		outportb(DMA0_CHANMASK_REG, (1 << (channel-1)));
	else
		outportb(DMA1_CHANMASK_REG, (1 << (channel-5)));
}

void dma_unmask_channel(uint8_t channel) {
    if(channel <= 4)
        outportb(DMA0_CHANMASK_REG, channel);
    else
        outportb(DMA1_CHANMASK_REG, channel);
}

void dma_reset_flipflop(int dma) {
	if(dma > 1)
		return;
    outportb((dma == 0) ? DMA0_CLEARBYTE_FLIPFLOP_REG : DMA1_CLEARBYTE_FLIPFLOP_REG, 0xFF);
}

void dma_reset() {
    outportb(DMA0_TEMP_REG, 0xFF);
}

void dma_unmask_all() {
    outportb(DMA1_UNMASK_ALL_REG, 0xFF);
}

