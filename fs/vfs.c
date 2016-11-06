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
#include <proc/sched.h>

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
    int device = get_dev_id_by_name(dir);
    if(device >= 0) {
        if(devs[device])
            return devs[device]->ls(dir + 1);
    }
}

int vfs_cd(char *name) {
    int device = get_dev_id_by_name(name);
    if(device >= 0) {
        if(devs[device]) {
            char *p = strchr(name + 1, '/');
            if(p) {
                file f = devs[device]->cd(name + 1);
                if(f.type == FS_DIR) {
                    return 1;
                } else {
                    return 0;
                }
            }
            return 1;
        }
    }
    return 0;
}

int vfs_touch(char *name) {
    int device = get_dev_id_by_name(name);
    if(device >= 0) {
        if(devs[device]) {
            return devs[device]->touch(name + 1);
        }
    }
    return 0;
}

int vfs_delete(char *name) {
    int device = get_dev_id_by_name(name);
    if(device >= 0) {
        if(devs[device]) {
            return devs[device]->delete(name + 1);
        }
    }
    return 0;
}

file *vfs_file_open(char *name, char *mode) {
    int device = get_dev_id_by_name(name);
    file *f = kmalloc(sizeof(file));
    if(device >= 0) {
        if(devs[device]) {
            *f = devs[device]->open(name + 1);
            if(f->type == FS_FILE) {
                if(strcmp(mode, "w") == 0) {
                    f->len = 0;
                }
                return f;
            }
        }
    }
    f->type = FS_NULL;
    return f;
}

file *vfs_file_open_user(char *name, char *mode) {
    process_t *cur = get_cur_proc();
    if(cur && cur->thread_list) {
        file *f = (file *) umalloc(sizeof(file), (vmm_addr_t *) cur->thread_list->heap);
        int device = get_dev_id_by_name(name);
        if(device >= 0 && devs[device]) {
            file fil = devs[device]->open(name + 1);
            memcpy(f, &fil, sizeof(file));
            if(f->type == FS_FILE) {
                if(strcmp(mode, "w") == 0) {
                    f->len = 0;
                }
                return f;
            }
        }
    }
    return NULL;
}

void vfs_file_read(file *f, char *str) {
    if(f) {
        if(devs[f->dev]) {
            devs[f->dev]->read(f, str);
        }
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
        if(devs[f->dev]) {
            devs[f->dev]->close(f);
            kfree(f);
        }
    }
}

void vfs_file_close_user(file *f) {
    if(f) {
        if(devs[f->dev]) {
            devs[f->dev]->close(f);
            process_t *cur = get_cur_proc();
            if(cur && cur->thread_list) {
                ufree(f, (vmm_addr_t *) cur->thread_list->heap);
            }
        }
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

