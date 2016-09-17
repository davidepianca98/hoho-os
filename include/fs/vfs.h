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

#ifndef VFS_H
#define VFS_H

#include <types.h>

#define MAX_DEVICES 26

typedef struct {
    char name[32];
    uint32_t flags;
    uint32_t len;
    uint32_t eof;
    uint32_t dev;
    uint32_t current_cluster;
    uint32_t type;
} file;

typedef struct {
    file (*directory) (char *dir, int devid);
    void (*mount) ();
    void (*read) (file *f, char *str);
    void (*write) (file *f, char *str);
    void (*close) (file *f);
    file (*open) (char *name);
    void (*ls) (char *dir);
    file (*cd) (char *dir);
} filesystem;

#define FS_FILE     0
#define FS_DIR      1
#define FS_NULL     2

void vfs_init();
void vfs_ls();
void vfs_ls_dir(char *dir);
int vfs_cd(char *name);
file vfs_file_open(char *name, char *mode);
void vfs_file_read(file *f, char *str);
void vfs_file_write(file *f, char *str);
void vfs_file_close(file *f);
int vfs_get_dev(char *name);
void vfs_mount(char *name);
void vfs_unmount(char *name);

#endif

