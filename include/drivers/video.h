#ifndef VIDEO_H
#define VIDEO_H

#include <types.h>

void video_init(int h, int w);
void printk(char *buffer, ...);
void printk_string(char *buffer);
void check();
void scroll();
void clear();

struct video_mem {
    int heigth;
    int width;
    uint16_t *ram;
};

typedef struct vbe_info_block {
    uint8_t signature[4];      // "VESA"
    uint16_t version;          // Either 0x0200 (VBE 2.0) or 0x0300 (VBE 3.0)
    uint32_t oem_string;       // Far pointer to OEM name
    uint8_t capabilities[4];   // capabilities
    uint32_t video_modes_ptr;  // Far pointer to video mode list
    uint16_t total_memory;     // Memory size in 64K blocks
    uint16_t oem_software_rev;
    uint32_t oem_vendor_name_ptr;
    uint32_t oem_product_name_ptr;
    uint32_t oem_roduct_rev_ptr;
    uint8_t reserved[222];
    uint8_t oem_data[256];
} __attribute__((__packed__)) vbe_info_block_t;

#endif

