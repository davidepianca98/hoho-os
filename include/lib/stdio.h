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

#ifndef STDIO_H
#define STDIO_H

#include "../types.h"

typedef struct {
    char name[32];
    uint32_t flags;
    uint32_t len;
    uint32_t eof;
    uint32_t dev;
    uint32_t current_cluster;
    uint32_t type;
} FILE;

void printf(char *buffer, ...);
void scanf(char *format, ...);
FILE *fopen(char *filename, char *mode);
void fclose(FILE *f);

#endif

