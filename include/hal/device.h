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

#ifndef DEVICE_H
#define DEVICE_H

#include <fs/vfs.h>
#include <fs/fat_mount.h>

typedef struct device {
    int id;
    int type; // 0 floppy, 1 ata hdd
    char mount[4];
    char* (*read) (int lba);
    int (*write) (int lba);
    filesystem fs;
    fat_mount_info_t minfo;
} device_t;

void device_register(device_t *dev);
device_t *get_dev_by_name(char *name);
device_t *get_dev_by_id(int id);
int get_dev_id_by_name(char *name);

#endif

