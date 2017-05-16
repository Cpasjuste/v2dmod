//
// Created by cpasjuste on 16/05/17.
//

#ifndef V2DMOD_MENU_MAIN_H
#define V2DMOD_MENU_MAIN_H

#include "../v2dmod.h"
#include "../v2dmod_drawing.h"
#include "../v2dmod_utility.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

#define MENU_MAIN       0
#define MENU_MODULES    1
#define MENU_SETTINGS   2
#define MENU_COUNT      3
// sub menu
#define MENU_MODULE_OPTION 4

#define menuRect v2d_rect(256, 128, SCREEN_WIDTH - 4 * 128, SCREEN_HEIGHT - 256)

void menu_main_draw(const char *title, ItemList *items, int selectionIndex);

int menu_main_input(uint32_t buttons, ItemList *items, int selectionIndex);

#endif //V2DMOD_MENU_MAIN_H
