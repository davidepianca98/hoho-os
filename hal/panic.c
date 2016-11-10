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

#include <drivers/audio.h>
#include <drivers/io.h>
#include <drivers/video.h>
#include <panic.h>

void panic() {
    disable_int();
    beep(1);
    printk("\nPANIC");
    draw_rect(400, 300, 200, 200, 0x000000);
    while(1);
}

