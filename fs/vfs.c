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
#include <lib/string.h>
#include <fs/vfs.h>
#include <fs/fat.h>
#include <drivers/video.h>
#include <hal/device.h>

static filesystem *devs[MAX_DEVICES];

void vfs_init() {
    for(int i = 0; i < MAX_DEVICES; i++)
        devs[i] = NULL;
}

void vfs_ls() {
    for(int i = 0; i < MAX_DEVICES; i++) {
        if(devs[i] != NULL) {
            device_t *device = get_dev_by_id(i);
            if(device)
                printk("%s\n", device->mount);
        }
    }
}

void vfs_ls_dir(char *dir) {
    char buf[512];
    int device = get_dev_id_by_name(dir);
    if(device >= 0) {
        if(devs[device])
            return devs[device]->ls(dir, buf);
    }
}

int vfs_cd(char *name) {
    int device = get_dev_id_by_name(name);
    if(device >= 0) {
        if(devs[device])
            return 1;
    }
    return 0;
}

file vfs_file_open(char *name, int w) {
    int device = get_dev_id_by_name(name);
    file f;
    if(device >= 0) {
        if(devs[device]) {
            f = devs[device]->open(name);
            if(w)
                f.len = 0;
            return f;
        }
    }
    f.flags = FS_NULL;
    return f;
}

void vfs_file_read(file *f, char *str) {
    if(f) {
        if(devs[f->dev])
            devs[f->dev]->read(f, str);
    } 
}

void vfs_file_write(file *f, char *str) {
    if(f) {
        if(devs[f->dev])
            devs[f->dev]->write(f, str);
    } 
}

void vfs_file_close(file *f) {
    if(f) {
        if(devs[f->dev])
            devs[f->dev]->close(f);
    }
}

int vfs_get_dev(char *name) {
    if(name[3] == '\\') {
        if(strncmp(name, "hd", 2) == 0)
            return 1;
        else if(strncmp(name, "fd", 2) == 0)
            return 0;
    }
    return -1;
}

void vfs_mount(char *name) {
    device_t *dev = get_dev_by_name(name);
    if(&dev->fs) {
        devs[dev->id] = &dev->fs;
        fat_mount(dev);
    }
}

void vfs_unmount(char *name) {
    device_t *dev = get_dev_by_name(name);
    if(&dev->fs)
        devs[dev->id] = NULL;
}

