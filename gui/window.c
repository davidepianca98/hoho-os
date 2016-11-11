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
#include <graphics.h>
#include <gui/window.h>
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
    
    window->components = (components_list_t *) kmalloc(sizeof(components_list_t));
    window->components->component = NULL;
    window->components->next = NULL;
    
    return window;
}

void add_component(window_t *window, void *component) {
    components_list_t *app = window->components;
    window->components = (components_list_t *) kmalloc(sizeof(components_list_t));
    window->components->component = (uint32_t *) component;
    window->components->next = app;
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
    draw_string(window->x + 5, window->y + 5, window->title);
    
    // Draw components
    components_list_t *app = window->components;
    while(app != NULL) {
        if(app->component != NULL) {
            switch(app->component[0]) {
                case TYPE_TEXT_AREA:
                    draw_text_area(window, (text_area_t *) app->component);
                    break;
            }
        }
        app = app->next;
    }
}

text_area_t *create_text_area(int x, int y, int w, int h) {
    text_area_t *text_area = (text_area_t *) kmalloc(sizeof(text_area_t));
    text_area->type = TYPE_TEXT_AREA;
    text_area->x = x;
    text_area->y = y;
    text_area->w = w;
    text_area->h = h;
    text_area->cursorx = 0;
    text_area->cursory = 0;
    return text_area;
}

void text_area_set_text(text_area_t *text_area, char *text) {
    strcpy(text_area->content, text);
}

void text_area_append(text_area_t *text_area, char *text) {
    strcat(text_area->content, text);
}

void draw_text_area(window_t *window, text_area_t *text_area) {
    draw_rect(window->x + 10 + text_area->x, window->y + 20 + text_area->y, text_area->w, text_area->h, TEXT_AREA_COLOR);
    draw_string(window->x + 10 + text_area->x + text_area->cursorx, window->y + 20 + text_area->y + text_area->cursory, text_area->content);
}

