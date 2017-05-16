//
// Created by cpasjuste on 16/05/17.
//

#include <libk/stdio.h>
#include "../v2dmod.h"
#include "../v2dmod_internal.h"
#include "menu_main.h"
#include "menu_module_list.h"
#include "menu_drawing_helper.h"

extern int menu;
extern int modules_count;
extern int draw_module_option_index;

extern void load_start_module(char *path);

extern char message[MAX_PATH];

void draw_module_list(Rect rectWindow, V2DModule *modules, int selection) {

    draw_border(rectWindow);
    v2d_scale_rect(&rectWindow, -10);

    int font_height = (v2d_get_font_height("Hp") + 2) * 2;
    int max_lines = rectWindow.h / font_height;
    int page = selection / max_lines;

    rectWindow.h = font_height;

    for (int i = page * max_lines; i < page * max_lines + max_lines; i++) {

        if (i >= modules_count)
            break;

        Rect r = rectWindow;
        r.x += 4;
        r.w -= 8;

        // set highlight
        if (i == selection) {
            v2d_draw_rect_outline(rectWindow, COLOR_SELECTION, COLOR_SELECTION_BORDER, 1);
            v2d_draw_font_color(r, COLOR_SELECTION_BORDER, modules[i].name);
            r.y += rectWindow.h / 2;
            v2d_draw_font_color(r, COLOR_SELECTION_BORDER, modules[i].desc);
        } else {
            v2d_draw_font_color(r, COLOR_FONT, modules[i].name);
            r.y += rectWindow.h / 2;
            v2d_draw_font_color(r, COLOR_FONT, modules[i].desc);
        }

        modules[i].modid = v2d_get_module_id(modules[i].name);
        Color color = modules[i].modid > 0 ? COLOR_OK : COLOR_NOK;
        Rect circle = {rectWindow.x + rectWindow.w - 32, rectWindow.y + rectWindow.h / 2, 10, 10};
        v2d_draw_circle(circle, color);

        rectWindow.y += font_height;
    }
}

void menu_modules_draw(const char *title, V2DModule *modules, int selectionIndex) {

    draw_title(menuRect, title);
    draw_module_list(menuRect, modules, selectionIndex);
}

int menu_modules_input(uint32_t buttons, V2DModule *modules, int selectionIndex) {

    int selection = selectionIndex;

    if (buttons & SCE_CTRL_DOWN) {
        selection++;
        if (selection >= v2d_get_module_count())
            selection = 0;
    } else if (buttons & SCE_CTRL_UP) {
        selection--;
        if (selection < 0)
            selection = v2d_get_module_count() - 1;
    } else if (buttons & SCE_CTRL_CROSS) {
        if (modules[selection].modid <= 0) {
            load_start_module(modules[selection].path);
        } else {
            V2DModule *module = &modules[selection];
            if (module->options->count > 0) {
                draw_module_option_index = selection;
                selection = 0;
                menu = MENU_MODULE_OPTION;
            } else {
                v2d_draw_message("No options");
            }
        }
    } else if (buttons & SCE_CTRL_CIRCLE) {
        selection = 0;
        menu = MENU_MAIN;
    }

    return selection;
}
