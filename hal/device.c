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

#include <hal/device.h>
#include <lib/string.h>
#include <fs/vfs.h>
#include <drivers/video.h>

static device_t *devices[8];

void device_register(device_t *dev) {
    if(dev->id < 8) {
        devices[dev->id] = dev;
        vfs_mount(dev->mount);
    }
}

device_t *get_dev_by_name(char *name) {
    if(name[0] == '/')
        name++;
    for(int i = 0; i < 8; i++) {
        if(strncmp(devices[i]->mount, name, 3) == 0)
            return devices[i];
    }
    return NULL;
}

device_t *get_dev_by_id(int id) {
    for(int i = 0; i < 8; i++) {
        if(devices[i]->id == id)
            return devices[i];
    }
    return NULL;
}

int get_dev_id_by_name(char *name) {
    if(name[0] == '/')
        name++;
    for(int i = 0; i < 8; i++) {
        if(strncmp(devices[i]->mount, name, 3) == 0)
            return devices[i]->id;
    }
    return -1;
}

