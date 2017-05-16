//
// Created by cpasjuste on 16/05/17.
//

#ifndef V2DMOD_MENU_MODULE_LIST_H
#define V2DMOD_MENU_MODULE_LIST_H

#include "../v2dmod_drawing.h"

void draw_module_list(Rect rectWindow, V2DModule *modules, int selection);

void menu_modules_draw(const char *title, V2DModule *modules, int selectionIndex);

int menu_modules_input(uint32_t buttons, V2DModule *modules, int selectionIndex);

#endif //V2DMOD_MENU_MODULE_LIST_H
