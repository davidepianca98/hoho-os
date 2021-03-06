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

void beep() {
    outportb(0xB6, 0x43);
    outportb(30, 0x42);
    outportb(5, 0x42);
    
    outportb(inportb(0x61) | 3, 0x61);
    //outportb(inportb(0x61) & ~3, 0x61);
}
