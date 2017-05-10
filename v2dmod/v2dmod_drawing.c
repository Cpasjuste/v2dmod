//
// Created by cpasjuste on 29/04/17.
//

#include <libk/string.h>
#include <libk/stdio.h>
#include <libk/stdarg.h>

#include "vita2d.h"
#include "v2dmod_drawing.h"

#define MAX_PATH 512

extern vita2d_bmf v2d_font;
static Color draw_color = {255, 255, 255, 255};
static float font_scaling = 1;

void v2d_set_draw_color(Color color) {
    draw_color = color;
}

Color v2d_get_draw_color() {
    return draw_color;
}

void v2d_set_font_scale(float scaling) {
    font_scaling = scaling;
}

float v2d_get_font_scale() {
    return font_scaling;
}

void v2d_scale_rect(Rect *rect, int pixels) {
    rect->x -= pixels;
    rect->y -= pixels;
    rect->w += pixels * 2;
    rect->h += pixels * 2;
}

void v2d_draw_line_advanced(int x1, int y1, int x2, int y2, int r, int g, int b, int a) {
    if (a > 0) {
        vita2d_draw_line(x1, y1, x2, y2, (unsigned int) RGBA8(r, g, b, a));
    }
}

void v2d_draw_line_color(const Line line, const Color color) {
    v2d_draw_line_advanced(line.p1.x, line.p1.y,
                           line.p2.x, line.p2.y,
                           color.r, color.g, color.b, color.a);
}

void v2d_draw_line(const Line line) {
    v2d_draw_line_color(line, draw_color);
}

void v2d_draw_rect_advanced(int _x, int _y, int _w, int _h, int r, int g, int b, int a, bool fill) {
    if (fill) {
        if (a > 0) {
            vita2d_draw_rectangle(_x, _y, _w, _h, (unsigned int) RGBA8(r, g, b, a));
        }
    } else {
        if (a > 0) {
            int x = _x + 1;
            int y = _y + 1;
            int w = _w - 1;
            int h = _h - 1;
            v2d_draw_line_advanced(x - 1, y, x + w, y, r, g, b, a);          // top
            v2d_draw_line_advanced(x, y, x, y + h, r, g, b, a);              // left
            v2d_draw_line_advanced(x, y + h, x + w - 1, y + h, r, g, b, a);  // bottom
            v2d_draw_line_advanced(x + w, y, x + w, y + h, r, g, b, a);      // right
        }
    }
}

void v2d_draw_rect_color(const Rect rect, const Color c, bool fill) {
    v2d_draw_rect_advanced(rect.x, rect.y, rect.w, rect.h, c.r, c.g, c.b, c.a, fill);
}

void v2d_draw_rect(const Rect rect, bool fill) {
    v2d_draw_rect_color(rect, draw_color, fill);
}

void v2d_draw_rect_outline(const Rect rect, Color color, Color outline, int outlineSize) {

    Rect r = rect;

    for (int i = 0; i < outlineSize; i++) {
        v2d_draw_rect_color(r, outline, false);
        v2d_scale_rect(&r, -1);
    }
    v2d_draw_rect_color(r, color, true);
}

// fonts
int v2d_get_font_width(const char *msg) {
    return vita2d_bmf_text_width(&v2d_font, font_scaling, msg);
}

int v2d_get_font_height(const char *msg) {
    return vita2d_bmf_text_height(&v2d_font, font_scaling, msg);
}

void v2d_draw_font_advanced(const Rect dst, const Color c,
                            bool centerX, bool centerY, const char *fmt, ...) {

    char msg[MAX_PATH];
    memset(msg, 0, MAX_PATH);
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, MAX_PATH, fmt, args);
    va_end(args);

    int width = v2d_get_font_width(msg);
    int height = v2d_get_font_height(msg);

    // cut message "properly" instead of clip
    while (v2d_get_font_width(msg) > dst.w) {
        int len = strlen(msg);
        if (len == 0) {
            break;
        }
        msg[strlen(msg) - 1] = '\0';
    }

    Rect rect = {dst.x, dst.y, dst.w, dst.h};

    if (centerY) {
        rect.y = (dst.y + (dst.h / 2) - height / 2);
    }

    if (centerX) {
        rect.x = dst.x + (dst.w / 2) - width / 2;
    }

    vita2d_bmf_draw_text(&v2d_font, rect.x, rect.y,
                         (unsigned int) RGBA8(c.r, c.g, c.b, c.a), font_scaling, msg);
}

void v2d_draw_font_color(const Rect dst, const Color c, const char *fmt, ...) {

    char msg[MAX_PATH];
    memset(msg, 0, MAX_PATH);
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, MAX_PATH, fmt, args);
    va_end(args);

    v2d_draw_font_advanced(dst, c, false, false, msg);
}

void v2d_draw_font(int x, int y, const char *fmt, ...) {

    char msg[MAX_PATH];
    memset(msg, 0, MAX_PATH);
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, MAX_PATH, fmt, args);
    va_end(args);

    Rect rect = {x, y, 960, 544};

    v2d_draw_font_color(rect, draw_color, msg);
}
