#ifndef VIDEO_H
#define VIDEO_H

#include <types.h>
#include <multiboot.h>

extern void int32(uint8_t intnum, regs16_t *regs);

void video_init(int h, int w);
void printk(char *buffer, ...);
void printk_string(char *buffer);
void check();
void scroll();
void clear();
void vbe_init(multiboot_info_t *info);
void refresh_screen();
void put_pixel(int x, int y, int color);
void put_rect(int x, int y, int w, int h, int color);

struct video_mem {
    int heigth;
    int width;
    uint16_t *ram;
};

#define VIDEO_MEM_BUFFER 0x400000

struct vbe_mem {
    uint32_t buffer_size;
    void *mem;
    void *buffer;
    uint16_t xres;
    uint16_t yres;
    uint8_t bpp;
    uint16_t pitch;
};

typedef struct vbe_controller_info {
    uint8_t signature[4];      // "VESA"
    uint16_t version;          // Either 0x0200 (VBE 2.0) or 0x0300 (VBE 3.0)
    uint32_t oem_string;       // Far pointer to OEM name
    uint8_t capabilities[4];   // capabilities
    uint32_t video_modes_ptr;  // Far pointer to video mode list
    uint16_t total_memory;     // Memory size in 64K blocks
    uint16_t oem_software_rev;
    uint32_t oem_vendor_name_ptr;
    uint32_t oem_product_name_ptr;
    uint32_t oem_product_rev_ptr;
    uint8_t reserved[222];
    uint8_t oem_data[256];
} __attribute__((__packed__)) vbe_controller_info_t;

typedef struct vbe_mode_info {
    uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
    uint8_t window_a;			// deprecated
    uint8_t window_b;			// deprecated
    uint16_t granularity;		// deprecated; used while calculating bank numbers
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
    uint16_t pitch;			// number of bytes per horizontal line
    uint16_t width;			// width in pixels
    uint16_t height;			// height in pixels
    uint8_t w_char;			// unused...
    uint8_t y_char;			// ...
    uint8_t planes;
    uint8_t bpp;			// bits per pixel in this mode
    uint8_t banks;			// deprecated; total number of banks in this mode
    uint8_t memory_model;
    uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
    uint8_t reserved1[206];
} __attribute__((packed)) vbe_mode_info_t;

#endif

