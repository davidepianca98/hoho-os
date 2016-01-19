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
#include <fs/fat.h>
#include <fs/fat_mount.h>
#include <drivers/video.h>

#define SECTOR_SIZE 512

extern uint32_t *dma_buffer;

uint8_t FAT[SECTOR_SIZE * 2];

void fat_mount(device_t *dev) {
    bootsector_t *bs = (bootsector_t *) dev->read(0);
    if((bs->ignore[0] != 0xEB) || (bs->ignore[2] != 0x90)) // not a FAT fs
        return;
    dev->minfo.n_sectors = (bs->bpb.n_sectors == 0) ? bs->bpb.long_sectors : bs->bpb.n_sectors;
    dev->minfo.fat_offset = bs->bpb.reserved_sectors;
    dev->minfo.fat_size = (bs->bpb.fat_sectors == 0) ? bs->bpb_ext.fat_sectors : bs->bpb.fat_sectors;
    dev->minfo.fat_entry_size = 8;
    dev->minfo.n_root_entries = bs->bpb.n_dir_entries;
    dev->minfo.root_offset = (bs->bpb.n_fats * dev->minfo.fat_size) + 1;
    dev->minfo.root_size = ((bs->bpb.n_dir_entries * 32) + (bs->bpb.sector_bytes - 1)) / bs->bpb.sector_bytes;
    dev->minfo.first_data_sector = dev->minfo.fat_offset + (bs->bpb.n_fats * dev->minfo.fat_size) + dev->minfo.root_size;
    dev->minfo.data_sectors = bs->bpb.n_sectors - (bs->bpb.reserved_sectors + (bs->bpb.n_fats * dev->minfo.fat_size) + dev->minfo.root_size);
    
    uint32_t total_clusters = dev->minfo.data_sectors / bs->bpb.cluster_sectors;
    if(total_clusters < 4085)
        dev->minfo.type = FAT12;
    else if(total_clusters < 65525)
        dev->minfo.type = FAT16;
    else if(total_clusters < 268435445)
        dev->minfo.type = FAT32;
    else
        dev->minfo.type = EXFAT;
    //printk("%x %x %x\n", bs->ignore[0], bs->ignore[1], bs->ignore[2]);
    //printk("sect bytes: %d clus sect: %d res sect: %d n fats: %d\n n dir entr: %d n sect: %d media: %d fat sect: %d\n", bs->bpb.sector_bytes, bs->bpb.cluster_sectors, bs->bpb.reserved_sectors, bs->bpb.n_fats, bs->bpb.n_dir_entries, bs->bpb.n_sectors, bs->bpb.media, bs->bpb.fat_sectors);
    //printk("n secs:%d fat offs:%d fat size:%d fat en size:%d\nn root en:%d root offs:%d root size:%d first data: %d\n", dev->minfo.n_sectors, dev->minfo.fat_offset, dev->minfo.fat_size, dev->minfo.fat_entry_size, dev->minfo.n_root_entries, dev->minfo.root_offset, dev->minfo.root_size, dev->minfo.first_data_sector);
}

void to_dos_file_name(char *name, char *str, int len) {
    int i;
    
    if((len > 11) || (!name) || (!str))
        return;
    
    memset(str, ' ', len);
    for(i = 0; i < strlen(name) - 1 && i < len; i++) {
        if((name[i] == '.') || (i == 8))
            break;
        str[i] = toupper(name[i]);
    }
    
    if(name[i] == '.') {
        for(int j = 0; j < 3; j++) {
            i++;
            if(name[i])
                str[8 + j] = toupper(name[i]);
        }
    }
}

void print_dir(directory_t *dir) {
    printk("%s %s %d %d %d\n", dir->filename, dir->extension, dir->attrs, dir->first_cluster, dir->file_size);
}

file fat_directory(char *dir_name, int devid) {
    file f;
    unsigned char *buf;
    directory_t *dir;
    char *file_name = kmalloc(sizeof(char) * 11);
    to_dos_file_name(dir_name, file_name, 11);
    file_name[11] = 0;
    
    device_t *dev = get_dev_by_id(devid);
    
    for(int i = 0; i < 14; i++) {
        buf = (unsigned char *) dev->read(dev->minfo.root_offset + i);
        dir = (directory_t *) buf;
        for(int j = 0; j < 16; j++) {
            char name[11];
            memcpy(name, dir->filename, 11);
            name[11] = 0;
            if(strncmp(file_name, name, 11) == 0) {
                strcpy(f.name, dir_name);
                f.current_cluster = dir->first_cluster;
                f.len = dir->file_size;
                f.eof = 0;
                f.dev = devid;
                if(dir->attrs == 0x10)
                    f.type = FS_DIR;
                else
                    f.type = FS_FILE;
                return f;
            }
            dir++;
        }
    }
    f.flags = FS_NULL;
    return f;
}

void fat_read(file *f, char *buf) {
    if(f) {
        device_t *dev = get_dev_by_id(f->dev);
        uint32_t phys_sector = 32 + (f->current_cluster - 1);
        unsigned char *sector = (unsigned char *) dev->read(phys_sector);
        memcpy(buf, sector, SECTOR_SIZE);
        
        if(dev->minfo.type == 0) {
            uint32_t fat_offset = f->current_cluster + (f->current_cluster / 2);
            uint32_t fat_sector = dev->minfo.fat_offset + (fat_offset / SECTOR_SIZE);
            uint32_t entry_offset = fat_offset % SECTOR_SIZE;
        
            sector = (unsigned char *) dev->read(fat_sector);
            memcpy(FAT, sector, SECTOR_SIZE);
        
            sector = (unsigned char *) dev->read(fat_sector + 1);
            memcpy(FAT + SECTOR_SIZE, sector, SECTOR_SIZE);
        
            uint16_t next_cluster = *(uint16_t *) &FAT[entry_offset];
            if(f->current_cluster & 0x0001)
                next_cluster >>= 4;
            else
                next_cluster &= 0x0FFF;
        
            if((next_cluster >= 0xFF8) || (next_cluster == 0)) {
                f->eof = 1;
                return;
            }
            
            f->current_cluster = next_cluster;
        } else if(dev->minfo.type == 1) {
            uint32_t fat_offset = f->current_cluster * 2;
            uint32_t fat_sector = dev->minfo.fat_offset + (fat_offset / SECTOR_SIZE);
            uint32_t entry_offset = fat_offset % SECTOR_SIZE;
            
            sector = (unsigned char *) dev->read(fat_sector);
            memcpy(FAT, sector, SECTOR_SIZE);
        
            sector = (unsigned char *) dev->read(fat_sector + 1);
            memcpy(FAT + SECTOR_SIZE, sector, SECTOR_SIZE);
            
            uint16_t next_cluster = *(uint16_t *) &FAT[entry_offset];
            
            if((next_cluster >= 0xFFF8) || (next_cluster == 0)) {
                f->eof = 1;
                return;
            }
            
            f->current_cluster = next_cluster;
        } else if(dev->minfo.type == 2) {
            uint32_t fat_offset = f->current_cluster * 4;
            uint32_t fat_sector = dev->minfo.fat_offset + (fat_offset / SECTOR_SIZE);
            uint32_t entry_offset = fat_offset % SECTOR_SIZE;
            
            sector = (unsigned char *) dev->read(fat_sector);
            memcpy(FAT, sector, SECTOR_SIZE);
        
            sector = (unsigned char *) dev->read(fat_sector + 1);
            memcpy(FAT + SECTOR_SIZE, sector, SECTOR_SIZE);
            
            uint32_t next_cluster = *(uint32_t *) &FAT[entry_offset] & 0x0FFFFFFF;
            
            if((next_cluster >= 0x0FFFFFF8) || (next_cluster == 0)) {
                f->eof = 1;
                return;
            }
            
            f->current_cluster = next_cluster;
        }
    }
}

void fat_write(file *f, char *str) {
    if(f) {
        char *file_name = kmalloc(sizeof(char) * 11);
        to_dos_file_name(f->name, file_name, 11);
        file_name[11] = 0;
        
        device_t *dev = get_dev_by_id(f->dev);
        uint32_t phys_sector = 32 + (f->current_cluster - 1);
        memset(dma_buffer, 0, SECTOR_SIZE);
        memcpy(dma_buffer, str, SECTOR_SIZE);
        dev->write(phys_sector);
        f->len++;
        
        uint8_t *buf;
        directory_t *dir;
        
        for(int i = 0; i < 4; i++) {
            buf = (unsigned char *) dev->read(dev->minfo.root_offset + i);
            dir = (directory_t *) buf;
            for(int j = 0; j < 16; j++) {
                char name[11];
                memcpy(name, dir->filename, 11);
                name[11] = 0;
                if(strncmp(file_name, name, 11) == 0) {
                    dir->file_size = f->len;
                    dev->write(dev->minfo.root_offset + i);
                    return;
                }
                dir++;
            }
        }
    }
}

void fat_close(file *f) {
    if(f)
        f->flags = FS_NULL;
}

file fat_open_subdir(file f, char *name) {
    file f2;
    
    char filename[11];
    to_dos_file_name(name, filename, 11);
    filename[11] = 0;
    
    while(!f.eof) {
        char buf[SECTOR_SIZE];
        fat_read(&f2, buf);
        
        directory_t *dir = (directory_t *) buf;
        
        for(int i = 0; i < 16; i++) {
            char cur_name[11];
            memcpy(cur_name, dir->filename, 11);
            cur_name[11] = 0;
            if(strcmp(cur_name, filename) == 0) {
                strcpy(f2.name, name);
                f2.current_cluster = dir->first_cluster;
                f2.len = dir->file_size;
                f2.eof = 0;
                if(dir->attrs == 0x10)
                    f2.flags = FS_DIR;
                else
                    f2.flags = FS_FILE;
                return f2;
            }
            dir++;
        }
    }
    f2.flags = FS_NULL;
    return f2;
}

file fat_open(char *name) {
    file cur_dir;
    char *p = 0;
    int root = 1;
    char *path = name;
    
    cur_dir.dev = get_dev_id_by_name(name);
    path = path + 4;
    p = strchr(path, '/');
    if(!p) {
        cur_dir = fat_directory(path, cur_dir.dev);
        if(cur_dir.flags == FS_FILE)
            return cur_dir;
        file ret;
        ret.flags = FS_NULL;
        return ret;
    }
    p++;
    while(p) {
        char pathname[16];
        int i;
        for(i = 0; i < 16; i++) {
            if((p[i] == '/') || (p[i] == '\0'))
                break;
            pathname[i] = p[i];
        }
        pathname[i] = 0;
        printk("xd:%s\n", pathname);
        if(root) {
            cur_dir = fat_directory(pathname, cur_dir.dev);
            root = 0;
        } else {
            cur_dir = fat_open_subdir(cur_dir, pathname);
        }
        
        if(cur_dir.flags == FS_NULL)
            break;
        else if(cur_dir.flags == FS_FILE)
            return cur_dir;
        
        p = strchr(p + 1, '/');
        if(p)
            p++;
    }
    file ret;
    ret.flags = FS_NULL;
    return ret;
}

void fat_ls(__attribute__((unused)) char *dir, __attribute__((unused)) char *str) {
/*
    directory_t *dir;
    //finire
    for(int i = 0; i < 14; i++) {
        buf = (unsigned char *) dev->read(dev->minfo.root_offset + i);
        dir = (directory_t *) buf;
        for(int j = 0; j < 16; j++) {
            char name[11];
            memcpy(name, dir->filename, 11);
            name[11] = 0;
            //if(j<2)printk("%shoho name:%shoho\n", file_name, name);
            //if(j < 2)print_dir(dir);
            if(strncmp(file_name, name, 11) == 0) {
                strcpy(f.name, dir_name);
                f.current_cluster = dir->first_cluster;
                f.len = dir->file_size;
                f.eof = 0;
                f.dev = devid;
                if(dir->attrs == 0x10)
                    f.type = FS_DIR;
                else
                    f.type = FS_FILE;
                return f;
            }
            dir++;
        }
    }*/
}

void fat_init(filesystem *fs_fat) {
    fs_fat->directory = &fat_directory;
    fs_fat->mount = &fat_mount;
    fs_fat->read = &fat_read;
    fs_fat->write = &fat_write;
    fs_fat->close = &fat_close;
    fs_fat->open = &fat_open;
    fs_fat->ls = &fat_ls;
}

