//
// Created by cpasjuste on 16/05/17.
//

#include <libk/string.h>
#include "menu_drawing_helper.h"

void draw_border(const Rect rect) {
    Rect r = rect;
    v2d_draw_rect_outline(r, COLOR_MENU, COLOR_BLUE, 1);
    v2d_scale_rect(&r, -1);
    v2d_draw_rect_outline(r, COLOR_MENU, COLOR_MENU_BORDER, 2);
}

void draw_title(const Rect rect, const char *title) {

    Rect r = {rect.x, rect.y - 48, rect.w, 48};
    draw_border(r);
    r.x += 10;
    v2d_set_font_scale(1.2f);
    v2d_draw_font_advanced(r, COLOR_FONT, false, true, title);
    v2d_set_font_scale(1.0f);
}

void draw_list(Rect rectWindow, ItemList *items, int selection) {

    draw_border(rectWindow);
    v2d_scale_rect(&rectWindow, -10);

    if (items == NULL) {
        return;
    }

    int font_height = v2d_get_font_height("Hp") + 2;
    int max_lines = rectWindow.h / font_height;
    int page = selection / max_lines;

    rectWindow.h = font_height;

    for (int i = page * max_lines; i < page * max_lines + max_lines; i++) {

        if (i >= items->count)
            break;

        Rect r = rectWindow;
        r.x += 4;
        r.w -= 8;

        // set highlight
        if (i == selection) {
            v2d_draw_rect_outline(rectWindow, COLOR_SELECTION, COLOR_SELECTION_BORDER, 1);
            v2d_draw_font_advanced(r, COLOR_SELECTION_BORDER, false, true, items->name[i]);
        } else {
            v2d_draw_font_advanced(r, COLOR_FONT, false, true, items->name[i]);
        }

        rectWindow.y += font_height;
    }
}
