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

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <proc/proc.h>
#include <types.h>

void default_ir_handler();

extern void invop_handle();
extern void gpf_handle();
extern void pf_handle();

void ex_divide_by_zero();                                   // 0
void ex_single_step();                                      // 1
void ex_nmi();                                              // 2
void ex_breakpoint();                                       // 3
void ex_overflow();                                         // 4
void ex_bounds_check();                                     // 5
void ex_invalid_opcode(struct regs_error *re);              // 6
void ex_device_not_available();                             // 7
void ex_double_fault();                                     // 8
// 9 reserved
void ex_invalid_tss();                                      // 10
void ex_segment_not_present();                              // 11
void ex_stack_fault();                                      // 12
void ex_gpf(struct regs_error *re);                         // 13
void ex_page_fault();                                       // 14
// 15 reserved
void ex_fpu_error();                                        // 16
void ex_alignment_check();                                  // 17
void ex_machine_check();                                    // 18
void ex_simd_fpu();                                         // 19
// 20 - 31 reserved

#endif

