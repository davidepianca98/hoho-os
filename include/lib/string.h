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

#ifndef STRING_H
#define STRING_H

#include <types.h>

void strcpy(char *str, char *format);
void strncpy(char *str, char *format, size_t len);
int strcmp(char *str1, char *str2);
int strncmp(char *str1, char *str2, size_t len);
void memset(void *start, uint32_t val, size_t len);
void memcpy(void *dest, void *src, int size);
void itoa(int val, char *str, int base);
int atoi(char *str);
int strlen(char *str);
char char_at(char *str, int pos);
void to_uppercase(char *str, char *format);
void to_lowercase(char *str, char *format);
char toupper(char c);
char tolower(char c);
int vsprintf(char *str, char *format, va_list args);
char *strchr(char *str, int c);
char *strcat(char *dest, char *src);

#endif
