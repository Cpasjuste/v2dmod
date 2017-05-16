//
// Created by cpasjuste on 16/05/17.
//

#ifndef V2DMOD_MENU_DRAWING_HELPER_H
#define V2DMOD_MENU_DRAWING_HELPER_H

#include "../v2dmod_drawing.h"
#include "../v2dmod_utility.h"

void draw_border(const Rect rect);

void draw_title(const Rect rect, const char *title);

void draw_list(Rect rectWindow, ItemList *items, int selection);

#endif //V2DMOD_MENU_DRAWING_HELPER_H
