//
// Created by cpasjuste on 16/05/17.
//

#include <libk/string.h>
#include "../v2dmod.h"
#include "menu_main.h"
#include "menu_module_option.h"
#include "menu_drawing_helper.h"
#include "../v2dmod_config.h"

extern int menu;
extern int draw_module_option_index;

void draw_option_list(Rect rectWindow, V2DModule *module, int selection) {

    draw_border(rectWindow);
    v2d_scale_rect(&rectWindow, -10);

    int font_height = v2d_get_font_height("Hp") + 2;
    int max_lines = rectWindow.h / font_height;
    int page = selection / max_lines;

    rectWindow.h = font_height;

    V2DOption *options = module->options;

    for (int i = page * max_lines; i < page * max_lines + max_lines; i++) {

        if (i >= MAX_OPTIONS || strlen(options[i].name) <= 0)
            break;

        Rect r = rectWindow;
        r.x += 4;
        r.w -= 8;

        // set highlight
        if (i == selection) {
            v2d_draw_rect_outline(rectWindow, COLOR_SELECTION, COLOR_SELECTION_BORDER, 1);
            v2d_draw_font_advanced(r, COLOR_SELECTION_BORDER, false, true, options[i].name);
        } else {
            v2d_draw_font_advanced(r, COLOR_FONT, false, true, options[i].name);
        }

        Rect rect = {rectWindow.x + rectWindow.w - 64, rectWindow.y, 48, rectWindow.h};
        int count = options[i].count;
        int index = options[i].index;
        int value = options[i].values[index];
        if (count == 2) { // ON / OFF
            v2d_draw_font_advanced(rect, COLOR_FONT, false, true, "%s", index == 1 ? "ON" : "OFF");
        } else if (count > 2) { // multiple choices
            v2d_draw_font_advanced(rect, COLOR_FONT, false, true, "%i", value);
        }

        rectWindow.y += font_height;
    }
}


void menu_module_option_draw(V2DModule *module, int selectionIndex) {

    draw_title(menuRect, module->name);
    draw_option_list(menuRect, module, selectionIndex);
}

int menu_module_option_input(uint32_t buttons, V2DModule *module, int selectionIndex) {

    int selection = selectionIndex;

    V2DOption *options = module->options;
    V2DOption *option = &options[selection];

    int count = 0;
    for (int i = 0; i < MAX_OPTIONS; i++) {
        if (strlen(options[i].name) <= 0) {
            break;
        }
        count++;
    }

    if (buttons & SCE_CTRL_DOWN) {
        selection++;
        if (selection >= count)
            selection = 0;
    } else if (buttons & SCE_CTRL_UP) {
        selection--;
        if (selection < 0)
            selection = count - 1;
    } else if (buttons & SCE_CTRL_LEFT) {
        option->index--;
        if (option->index < 0)
            option->index = option->count - 1;
    } else if (buttons & SCE_CTRL_RIGHT) {
        option->index++;
        if (option->index >= option->count)
            option->index = 0;
    } else if (buttons & SCE_CTRL_CROSS) {
    } else if (buttons & SCE_CTRL_CIRCLE) {
        v2d_cfg_write(module);
        selection = 0;
        draw_module_option_index = -1;
        menu = MENU_MODULES;
    }

    return selection;
}
