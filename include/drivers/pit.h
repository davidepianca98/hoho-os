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

#ifndef PIT_H
#define PIT_H

#include <types.h>

#define PIT_BINCOUNT_MASK       0x1
#define PIT_MODE_MASK           0xE
#define PIT_RL_MASK             0x30
#define PIT_COUNTER_MASK        0xC0

#define	PIT_BINCOUNT_BINARY     0
#define PIT_BINCOUNT_BCD        1
#define	PIT_MODE_TERMINALCOUNT  0
#define	PIT_MODE_ONESHOT        0x2
#define	PIT_MODE_RATEGEN        0x4
#define	PIT_MODE_SQUAREWAVEGEN  0x6
#define	PIT_MODE_SOFTWARETRIG   0x8
#define	PIT_MODE_HARDWARETRIG   0xA
#define PIT_RL_LATCH            0
#define	PIT_RL_LSBONLY          0x10
#define	PIT_RL_MSBONLY          0x20
#define	PIT_RL_DATA             0x30
#define PIT_COUNTER_0           0
#define	PIT_COUNTER_1           0x40
#define	PIT_COUNTER_2           0x80

#define PIT_REG_COUNTER0        0x40
#define PIT_REG_COUNTER1        0x41
#define PIT_REG_COUNTER2        0x42
#define PIT_REG_COMMAND         0x43

void sched_state(int on);
int get_sched_state();
void pit_send_command(uint8_t cmd);
void pit_send_data(uint16_t data, uint8_t counter);
uint8_t pit_read_data(uint8_t counter);
void pit_init();
void pit_start_counter(uint32_t frequency, uint8_t counter, uint8_t mode);
int get_tick_count();

#endif

