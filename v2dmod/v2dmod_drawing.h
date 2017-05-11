//
// Created by cpasjuste on 29/04/17.
//

#ifndef V2DMOD_DRAWING_H
#define V2DMOD_DRAWING_H

#include <libk/stdbool.h>

#define COLOR_RED v2d_color(255, 0, 0, 255)
#define COLOR_GREEN v2d_color(0, 255, 0, 255)
#define COLOR_BLUE v2d_color(0, 0, 255, 255)
#define COLOR_BLACK v2d_color(0, 0, 0, 255)
#define COLOR_WHITE v2d_color(255, 255, 255, 255)

#define COLOR_FONT v2d_color(255, 255, 255, 200)
#define COLOR_MENU v2d_color(102, 178, 255, 200)
#define COLOR_MENU_BORDER v2d_color(255, 255, 255, 200)
#define COLOR_SELECTION v2d_color(51, 153, 255, 200)
#define COLOR_SELECTION_BORDER v2d_color(102, 255, 102, 200)
#define COLOR_OK v2d_color(102, 255, 102, 200)
#define COLOR_NOK v2d_color(255, 51, 51, 200)

typedef struct Point {
    int x;
    int y;
} Point;

typedef struct Line {
    Point p1;
    Point p2;
} Line;

typedef struct Rect {
    int x;
    int y;
    int w;
    int h;
} Rect;

typedef struct Color {
    int r;
    int g;
    int b;
    int a;
} Color;

Color v2d_color(int r, int g, int b, int a);

void v2d_set_draw_color(Color color);

Color v2d_get_draw_color();

void v2d_set_font_scale(float scaling);

float v2d_get_font_scale();

void v2d_scale_rect(Rect *rect, int pixels);

// line
void v2d_draw_line(const Line line);

void v2d_draw_line_color(const Line line, const Color color);

void v2d_draw_line_advanced(int x1, int y1, int x2, int y2, int r, int g, int b, int a);

// rect
void v2d_draw_rect(const Rect rect, bool fill);

void v2d_draw_rect_color(const Rect rect, const Color c, bool fill);

void v2d_draw_rect_advanced(int x, int y, int w, int h, int r, int g, int b, int a, bool fill);

void v2d_draw_rect_outline(const Rect rect, Color color, Color outline, int outlineSize);

void v2d_draw_circle(const Rect rect, Color color);

// font
int v2d_get_font_width(const char *msg);

int v2d_get_font_height(const char *msg);

void v2d_draw_font(int x, int y, const char *fmt, ...);

void v2d_draw_font_color(const Rect dst, const Color c, const char *fmt, ...);

void v2d_draw_font_advanced(const Rect dst, const Color c,
                            bool centerX, bool centerY, const char *fmt, ...);

#endif //V2DMOD_DRAWING_H
