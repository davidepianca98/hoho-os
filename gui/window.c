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

#include <drivers/video.h>
#include <gui/window.h>
#include <graphics.h>
#include <lib/string.h>
#include <mm/kheap.h>

static window_list_t *list;

void windows_list_init() {
    list = (window_list_t *) kmalloc(sizeof(window_list_t));
    if(list) {
        list->next = NULL;
        list->window = NULL;
    }
}

window_t *window_create(char *title, int x, int y, int w, int h) {
    window_t *window = (window_t *) kmalloc(sizeof(window_t));
    if(!window)
        return NULL;
    strcpy(window->title, title);
    window->x = x;
    window->y = y;
    window->w = w;
    window->h = h;
    window_list_t *new = (window_list_t *) kmalloc(sizeof(window_list_t));
    new->window = window;
    new->next = list->next;
    list->next = new;
    return window;
}

void paint_windows() {
    window_list_t *app = list;
    while(app != NULL) {
        if(app->window != NULL) {
            paint_window(app->window);
        }
        app = app->next;
    }
}

void paint_window(window_t *window) {
    draw_rect(window->x, window->y, window->w, window->h, WINDOW_EDGE_COLOR);
    draw_rect(window->x + 10, window->y + 20, window->w - 20, window->h - 35, WINDOW_BACKGROUND_COLOR);
}
