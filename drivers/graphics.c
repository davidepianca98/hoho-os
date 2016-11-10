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

#include <drivers/mouse.h>
#include <drivers/video.h>
#include <graphics.h>
#include <gui/window.h>

void paint_desktop() {
    draw_rect(0, 0, 1024, 768, 0xCE2C2C);
    paint_windows();
    paint_mouse();
}

void paint_mouse() {
    draw_rect(get_mouse_info()->x, get_mouse_info()->y, 10, 10, 0xFFFFFF);
}
