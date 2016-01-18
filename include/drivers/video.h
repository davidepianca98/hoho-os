#ifndef VIDEO_H
#define VIDEO_H

#include <types.h>

void video_init(int h, int w);
void printk(char *buffer, ...);
void check();
void scroll();
void clear();

struct video_mem {
    int heigth;
    int width;
    uint16_t *ram;
    int **ram_mat;
};

#endif

