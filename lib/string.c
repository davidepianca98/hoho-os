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

#include <lib/string.h>

void strcpy(char *str, char *format) {
    int i;
    for(i = 0; i < strlen(format); i++) {
        str[i] = format[i];
    }
}

void strncpy(char *str, char *format, size_t len) {
    while(len--) {
        if(*format)
            *str++ = *format++;
        else
            break;
    }
    *str = 0;
}

int strcmp(char *str1, char *str2) {
    while(*str1 != '\0') {
        if((*str2 == '\0') || (*str1 > *str2))
            return 1;
        else if(*str2 > *str1)
            return -1;
        str1++;
        str2++;
    }
    if(*str2 != '\0')
        return -1;
    return 0;
}

int strncmp(char *str1, char *str2, size_t len) {
    while(len--)
        if(*str1++ != *str2++)
            return *(uint8_t *) (str1 - 1) - *(uint8_t *) (str2 - 1);
    return 0;
}

void memset(void *start, uint32_t val, size_t len) {
    uint8_t *p = start;
    while(len--) {
        *p++ = (uint8_t) val;
    }
}

void memcpy(void *dest, void *src, int size) {
    int i = 0;
    char *dest8 = (char *) dest;
    char *src8 = (char *) src;
    for(i = 0; i < size; i++) {
        if(&dest8[i] == src8)
            return;
        dest8[i] = src8[i];
    }
}

void itoa(int val, char *str, int base) {
    static const char *tokens = "0123456789ABCDEF";
    int flag = 0;
    
    if(val < 0) {
        val = -val;
        flag = 1;
    }
    
    int i = 0, n;
    do {
        str[i++] = tokens[val % base];
        val /= base;
    } while(val);
    
    if(flag)
        str[i++] = '-';
    str[i--] = '\0';
    
    char c;
    for(n = 0; n < i; n++, i--) {
        c = str[n];
        str[n] = str[i];
        str[i] = c;
    }
}

int atoi(char *str) {
    int n = 0;
    while(*str) {
        n = (n << 3) + (n << 1) + (*str) - '0';
        str++;
    }
    return n;
}

int strlen(char *str) {
    int i = 0;
    while(str[i] != '\0')
        i++;
    return i;
}

char char_at(char *str, int pos) {
    if(pos < strlen(str)) {
        return str[pos];
    }
    return NULL;
}

void to_uppercase(char *str, char *format) {
    int i;
    
    for(i = 0; i < strlen(format); i++) {
        str[i] = format[i] - 32;
    }
    str[i + 1] = '\0';
}

void to_lowercase(char *str, char *format) {
    int i;
    
    for(i = 0; i < strlen(format); i++) {
        str[i] = format[i] + 32;
    }
    str[i + 1] = '\0';
}

char toupper(char c) {
    return c - 32;
}

char tolower(char c) {
    return c + 32;
}

int vsprintf(char *str, char *format, va_list args) {
    int i, j = 0, k;
    char *buf;
    
    for(i = 0; i < strlen(format); i++) {
        switch(format[i]) {
            case '%':
                i++;
                switch(format[i]) {
                    case 'c':
                        str[j] = va_arg(args, char);
                        j++;
                        break;
                    case 'd':
                    case 'i':
                        itoa(va_arg(args, int), buf, 10);
                        for(k = 0; k < strlen(buf); k++) {
                            str[j] = buf[k];
                            j++;
                        }
                        break;
                    case 'f': //TODO
                        break;
                    case 's':
                        buf = va_arg(args, char *);
                        for(k = 0; k < strlen(buf); k++) {
                            str[j] = buf[k];
                            j++;
                        }
                        break;
                    case 'x':
                        itoa(va_arg(args, int), buf, 16);
                        for(k = 0; k < strlen(buf); k++) {
                            str[j] = buf[k];
                            j++;
                        }
                        break;
                    case 'b':
                        itoa(va_arg(args, int), buf, 2);
                        for(k = 0; k < strlen(buf); k++) {
                            str[j] = buf[k];
                            j++;
                        }
                        break;
                }
                break;
            default:
                str[j] = format[i];
                j++;
                break;
        }
    }
    str[j] = '\0';
    return 0;
}

char *strchr(char *str, int c) {
    char *p = str;
    while(*p != '\0') {
        if(*p == c)
            return p;
        p++;
    }
    return NULL;
}

char *strcat(char *dest, char *src) {
    char *ret = dest;
    
    while(*dest)
        dest++;
    while(*src)
        *dest++ = *src++;
    return ret;
}

