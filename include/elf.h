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

#ifndef ELF_H
#define ELF_H

#include <proc/proc.h>
#include <fs/vfs.h>
#include <types.h>

#define EXE_MAX_SEGMENTS    9

#define ELF_X86     0x3
#define ELF_X86_64  0x3E
#define ELF_ARM     0x28

#define ELF_LITTLE_ENDIAN   1
#define ELF_32BIT           1

// 52 byte
typedef struct elf_header {
    uint8_t magic[4];
    uint8_t arch;
    uint8_t machine;
    uint8_t elf_version;
    uint8_t abi;
    uint8_t unused[8];
    uint16_t type;
    uint16_t instruction_set;
    uint32_t elf_version2;
    uint32_t entry;
    uint32_t program_header;
    uint32_t section_header;
    uint32_t flags;
    uint16_t header_size;
    uint16_t entry_size_prog_header;
    uint16_t entry_number_prog_header;
    uint16_t entry_size_sect_header;
    uint16_t entry_number_sect_header;
    uint16_t index_sect_header_names;
} __attribute__((__packed__)) elf_header_t;

// 40 byte
typedef struct section_header {
    uint32_t s_name;
    uint32_t s_type;
    uint32_t s_flags;
    uint32_t s_addr;
    uint32_t s_offset;
    uint32_t s_size;
    uint32_t s_link;
    uint32_t s_info;
    uint32_t s_align;
    uint32_t s_entsize;
} __attribute__((__packed__)) section_header_t;

// 32 byte
typedef struct program_header {
    /*
     * 0 = NULL
     * 1 = load
     * 2 = dynamic
     * 3 = interp
     * 4 = notes
     */
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t undefined;
    uint32_t p_file_size;
    uint32_t p_mem_size;
    /*
     * 1 = executable
     * 2 = writable
     * 4 = readable
     */
    uint32_t p_flags;
    uint32_t align;
} __attribute__((__packed__)) program_header_t;

int elf_validate(elf_header_t *eh);
int load_elf(char *name, thread_t *thread, page_dir_t *pdir);
int load_elf_file(char *name);
int load_elf_relocate(thread_t *thread, page_dir_t *pdir, elf_header_t *eh);

#endif

