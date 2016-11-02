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
#include <fs/mbr.h>
#include <drivers/video.h>

#define SECTOR_SIZE 512

extern uint32_t *dma_buffer;

uint8_t FAT[SECTOR_SIZE * 2];

void fat_mount(device_t *dev) {
    // Trying with bootsector
    bootsector_t *bs = (bootsector_t *) dev->read(0);
    if((bs->ignore[0] != 0xEB) || (bs->ignore[2] != 0x90)) // Not a FAT fs
        return;
    
    // Scan for partitions
    mbr_t *mbr = (mbr_t *) bs;
    for(int i = 0; i < 4; i++) {
        if(mbr->partition_table[i].sys_id != FAT32_SYSTEM_ID) {
            continue;
        } else {
            //uint32_t lba = mbr->partition_table[i].lba_start;
            //uint32_t totsec = mbr->partition_table[i].total_sectors;
            //printk("lba: %d sects: %d\n", lba, totsec);
            //bs = (bootsector_t *) dev->read(lba);
            break;
        }
        return;
    }
        
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
    
    //printk("FAT type: %d\n", dev->minfo.type);
    //printk("%x %x %x\n", bs->ignore[0], bs->ignore[1], bs->ignore[2]);
    //printk("sect bytes: %d clus sect: %d res sect: %d n fats: %d\n n dir entr: %d n sect: %d media: %d fat sect: %d\n", bs->bpb.sector_bytes, bs->bpb.cluster_sectors, bs->bpb.reserved_sectors, bs->bpb.n_fats, bs->bpb.n_dir_entries, bs->bpb.n_sectors, bs->bpb.media, bs->bpb.fat_sectors);
    //printk("n secs:%d fat offs:%d fat size:%d fat en size:%d\nn root en:%d root offs:%d root size:%d first data: %d data secs: %d\n", dev->minfo.n_sectors, dev->minfo.fat_offset, dev->minfo.fat_size, dev->minfo.fat_entry_size, dev->minfo.n_root_entries, dev->minfo.root_offset, dev->minfo.root_size, dev->minfo.first_data_sector, dev->minfo.data_sectors);
}

void to_dos_file_name(char *name, char *str, int len) {
    int i;
    
    if((len > NAME_LEN) || (!name) || (!str))
        return;
    
    memset(str, ' ', len);
    for(i = 0; i < strlen(name) && i < len; i++) {
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
    str[NAME_LEN] = 0;
}

void to_normal_file_name(char *name, char *str, int len) {
    int i, j = 0, flag = 1;
    
    if((len > NAME_LEN) || (!name) || (!str))
        return;
    
    memset(str, ' ', len);
    for(i = 0; i < strlen(name) && i < len; i++) {
        if(name[i] != ' ') {
            str[j] = tolower(name[i]);
            j++;
        } else if((flag == 1) && (name[9] != ' ')) {
            flag = 0;
            str[j] = '.';
            j++;
        }
    }
    str[j] = 0;
}

void print_dir(directory_t *dir) {
    printk("%s %s %d %d %d\n", dir->filename, dir->extension, dir->attrs, dir->first_cluster, dir->file_size);
}

int fat_touch(char *name) {
    file f = fat_search(name);
    if(f.type != FS_NULL) {
        return 0;
    }
    
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
        char *file_name = kmalloc(NAME_LEN);
        to_dos_file_name(f->name, file_name, NAME_LEN);
        
        device_t *dev = get_dev_by_id(f->dev);
        uint32_t phys_sector = 32 + (f->current_cluster - 1);
        memset(dma_buffer, 0, SECTOR_SIZE);
        memcpy(dma_buffer, str, strlen(str));
        dev->write(phys_sector);
        f->len++;
        
        uint8_t *buf;
        directory_t *dir;
        
        for(int i = 0; i < 4; i++) {
            buf = (unsigned char *) dev->read(dev->minfo.root_offset + i);
            dir = (directory_t *) buf;
            for(int j = 0; j < 16; j++) {
                char name[NAME_LEN];
                memcpy(name, dir->filename, NAME_LEN);
                name[NAME_LEN] = 0;
                if(strncmp(file_name, name, NAME_LEN) == 0) {
                    dir->file_size = f->len;
                    dev->write(dev->minfo.root_offset + i);
                    kfree(file_name);
                    return;
                }
                dir++;
            }
        }
        kfree(file_name);
    }
}

void fat_close(file *f) {
    if(f)
        f->type = FS_NULL;
}

file fat_directory(char *dir_name, int devid) {
    file f;
    directory_t *dir;
    char *file_name = kmalloc(NAME_LEN);
    to_dos_file_name(dir_name, file_name, NAME_LEN);
    
    device_t *dev = get_dev_by_id(devid);
    
    for(int i = 0; i < 14; i++) {
        dir = (directory_t *) dev->read(dev->minfo.root_offset + i);
        f = fat_fill_file(file_name, dir, dir_name, devid);
        if(f.type != FS_NULL) {
            kfree(file_name);
            return f;
        }
    }
    kfree(file_name);
    return f;
}

file fat_open_subdir(file directory, char *name) {
    file f;
    directory_t *dir;
    char *file_name = kmalloc(NAME_LEN);
    to_dos_file_name(name, file_name, NAME_LEN);
    char *buf = kmalloc(SECTOR_SIZE);
    
    while(!directory.eof) {
        fat_read(&directory, buf);
        dir = (directory_t *) buf;
        f = fat_fill_file(file_name, dir, name, directory.dev);
        if(f.type != FS_NULL) {
            kfree(buf);
            kfree(file_name);
            return f;
        }
    }
    kfree(buf);
    kfree(file_name);
    return f;
}

file fat_fill_file(char *file_name, directory_t *dir, char* dir_name, int devid) {
    file f;
    for(int i = 0; i < 16; i++) {
        if(strncmp(file_name, (char *) dir->filename, NAME_LEN) == 0) {
            strcpy(f.name, dir_name);
            f.current_cluster = dir->first_cluster;
            f.len = dir->file_size;
            f.eof = 0;
            f.dev = devid;
            if(dir->attrs & 0x10)
                f.type = FS_DIR;
            else
                f.type = FS_FILE;
            return f;
        }
        dir++;
    }
    f.type = FS_NULL;
    return f;
}

file fat_open(char *name) {
    file f = fat_search(name);
    if(f.type == FS_FILE || f.type == FS_NULL) {
        return f;
    } else {
        f.type = FS_NULL;
        return f;
    }
}

file fat_cd(char *dir) {
    file f = fat_search(dir);
    if(f.type == FS_DIR || f.type == FS_NULL) {
        return f;
    } else {
        f.type = FS_NULL;
        return f;
    }
}

file fat_search(char *name) {
    file cur_dir;
    int root = 1;
    char *path = name;
    
    cur_dir.dev = get_dev_id_by_name(name);
    path = path + 4;
    if(!strchr(path, '/')) {
        cur_dir = fat_directory(path, cur_dir.dev);
        return cur_dir;
    }
    while(path) {
        char pathname[16];
        int i;
        for(i = 0; i < 16; i++) {
            if((path[i] == '/') || (path[i] == '\0'))
                break;
            pathname[i] = path[i];
        }
        pathname[i] = 0;
        if(root) {
            cur_dir = fat_directory(pathname, cur_dir.dev);
            root = 0;
        } else {
            cur_dir = fat_open_subdir(cur_dir, pathname);
        }
        
        path = strchr(path, '/');
        if(path)
            path++;
    }
    return cur_dir;
}

void fat_ls(char *dir) {
    directory_t *direc;
    uint8_t *buf;
    
    char *file_name = kmalloc(NAME_LEN);
    to_dos_file_name(dir, file_name, NAME_LEN);
    
    char *normal_name = kmalloc(NAME_LEN);
    
    device_t *dev = get_dev_by_name(dir);
    for(int i = 0; i < 14; i++) {
        buf = (unsigned char *) dev->read(dev->minfo.root_offset + i);
        direc = (directory_t *) buf;
        for(int j = 0; j < 16; j++) {
            char name[NAME_LEN];
            memcpy(name, direc->filename, NAME_LEN);
            direc++;
            name[NAME_LEN] = 0;
            if(name[0] == 0)
                continue;
            to_normal_file_name(name, normal_name, NAME_LEN);
            printk("%s  ", normal_name);
        }
    }
    printk("\n");
    kfree(file_name);
    kfree(normal_name);
}

void fat_init(filesystem *fs_fat) {
    fs_fat->directory = &fat_directory;
    fs_fat->mount = &fat_mount;
    fs_fat->read = &fat_read;
    fs_fat->write = &fat_write;
    fs_fat->close = &fat_close;
    fs_fat->open = &fat_open;
    fs_fat->ls = &fat_ls;
    fs_fat->cd = &fat_cd;
}

