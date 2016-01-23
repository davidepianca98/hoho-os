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

#ifndef SCHED_H
#define SCHED_H

#include <proc/proc.h>

process_t *get_cur_proc();
process_t *get_proc_by_id(int id);
uint32_t schedule(uint32_t esp);
void sched_add_proc(process_t *proc);
void sched_remove_proc(int id);
void sched_init();
int get_nproc();
void print_procs();

#endif

