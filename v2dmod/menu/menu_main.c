//
// Created by cpasjuste on 16/05/17.
//

#include <psp2/ctrl.h>
#include "menu_main.h"
#include "menu_drawing_helper.h"

extern bool draw_menu;
extern int menu;

void menu_main_draw(const char *title, ItemList *items, int selectionIndex) {

    draw_title(menuRect, title);
    draw_list(menuRect, items, selectionIndex);
}

int menu_main_input(uint32_t buttons, ItemList *items, int selectionIndex) {

    int selection = selectionIndex;

    if (buttons & SCE_CTRL_DOWN) {
        selection++;
        if (selection >= items->count)
            selection = 0;
    } else if (buttons & SCE_CTRL_UP) {
        selection--;
        if (selection < 0)
            selection = items->count - 1;
    } else if (buttons & SCE_CTRL_CROSS) {
        menu = selection + 1;
        selection = 0;
    } else if (buttons & SCE_CTRL_CIRCLE) {
        selection = 0;
        draw_menu = false;
    }

    return selection;
}
