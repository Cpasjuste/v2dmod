#include <libk/stdlib.h>
#include <libk/stdio.h>
#include <libk/stdarg.h>
#include "vita2d.h"
#include "utils.h"

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int xoffset;
    int yoffset;
    int xadvance;
} BMFont_Char;

typedef struct {
    int size;
    int outline; // 1 for using outline, 0 otherwise
    char pagefile[256]; // where the font image sheet is
    BMFont_Char chr[95]; // there are 95 chars between ASCII 32 and 126
    int charset; // one of BMFont_CharsetEnum
    int unicode; // 1 for unicode characters, 0 otherwise
    int pages; // num of pages the characters are spread across
    int alphaChnl; // one of BMFont_ChnlHoldsEnum
    int redChnl; // one of BMFont_ChnlHoldsEnum
    int greenChnl; // one of BMFont_ChnlHoldsEnum
    int blueChnl; // one of BMFont_ChnlHoldsEnum
    int packed; // 1 if packed, 0 otherwise
} BMFont;

extern BMFont v2d_bmf;
static vita2d_bmf v2d_font;

typedef struct vita2d_bmf {

    vita2d_texture *texture;
    BMFont *bmf;

} vita2d_bmf;

vita2d_bmf *vita2d_load_bmf(const char *img_path, const char *fnt_paht) {

    /*
    vita2d_bmf *font = sce_malloc(sizeof(vita2d_bmf));
    if (font == NULL) {
        printf("couldn't allocate font\n");
        return NULL;
    }
    */

    v2d_font.bmf = &v2d_bmf;

    v2d_font.texture = vita2d_load_BMP_file(img_path);
    if (v2d_font.texture == NULL) {
        printf("couldn't load texture: %s\n", img_path);
        //sce_free(font);
        return NULL;
    }

    return &v2d_font;
}

void vita2d_free_bmf(vita2d_bmf *font) {
    if (font) {
        if (font->texture) {
            sce_free(font->texture);
        }
        //sce_free(font);
    }
}

int generic_bmf_draw_text(vita2d_bmf *font, int draw, int *height,
                          int x, int y, unsigned int color, float scale,
                          const char *text) {
    unsigned int c;
    int srcx, srcy, srcw, srch;

    vita2d_texture *tex = font->texture;
    int start_x = x;
    int max_x = 0;
    int pen_x = x;
    int pen_y = y;

    const char *tmp;
    for (tmp = text; *tmp != '\0'; tmp++) {

        c = (unsigned int) (*tmp - ' ');

        if (c == '\n') {
            if (pen_x > max_x)
                max_x = pen_x;
            pen_x = start_x;
            pen_y += font->bmf->size * scale;
            continue;
        }

        if (c < 0 || c >= 95)
            continue;

        srcx = font->bmf->chr[c].x;
        srcy = font->bmf->chr[c].y;
        srcw = font->bmf->chr[c].width;
        srch = font->bmf->chr[c].height;

        if (draw) {
            vita2d_draw_texture_tint_part_scale(tex,
                                                pen_x + font->bmf->chr[c].xoffset * scale,
                                                pen_y + font->bmf->chr[c].yoffset * scale,
                                                srcx, srcy, srcw, srch,
                                                scale,
                                                scale,
                                                color);
        }

        pen_x += font->bmf->chr[c].xadvance * scale;
    }

    if (pen_x > max_x)
        max_x = pen_x;

    if (height)
        *height = (int) (pen_y + font->bmf->size * scale - y);

    return max_x - x;
}

int vita2d_bmf_draw_text(vita2d_bmf *font, int x, int y,
                         unsigned int color, float scale,
                         const char *text) {
    return generic_bmf_draw_text(font, 1, NULL, x, y, color, scale, text);
}

int vita2d_bmf_draw_textf(vita2d_bmf *font, int x, int y,
                          unsigned int color, float scale,
                          const char *text, ...) {
    char buf[1024];
    va_list argptr;
    va_start(argptr, text);
    vsnprintf(buf, sizeof(buf), text, argptr);
    va_end(argptr);
    return vita2d_bmf_draw_text(font, x, y, color, scale, buf);
}

void vita2d_bmf_text_dimensions(vita2d_bmf *font, float scale,
                                const char *text, int *width, int *height) {
    int w;
    w = generic_bmf_draw_text(font, 0, height, 0, 0, 0, scale, text);

    if (width)
        *width = w;
}

int vita2d_bmf_text_width(vita2d_bmf *font, float scale, const char *text) {
    int width;
    vita2d_bmf_text_dimensions(font, scale, text, &width, NULL);
    return width;
}

int vita2d_bmf_text_height(vita2d_bmf *font, float scale, const char *text) {
    int height;
    vita2d_bmf_text_dimensions(font, scale, text, NULL, &height);
    return height;
}
