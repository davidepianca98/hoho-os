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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <fs/vfs.h>

void console_init(char *usr);
void console_init_gui(char *usr);
void console_run();
void console_run_gui();
void console_gui_print(char *str);

char *get_dir();
int character_check(char c);
void console_print(char *buffer, ...);
void console_exec(char *buf);
void console_cd(char *dir, char *command);
void console_start(char *dir, char *command);
void console_read(char *dir, char *command);
void console_write(char *dir, char *command);
void console_touch(char *dir, char *command);
void console_delete(char *dir, char *command);
char *console_pwd();
char *console_pwd_user();
void print_file(file *f);
void print_meminfo();
char *get_argument(char *command, int n);

#endif

