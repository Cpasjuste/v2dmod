//
// Created by cpasjuste on 06/05/17.
//

#ifndef V2DMOD_V2DMOD_FB_H
#define V2DMOD_V2DMOD_FB_H

#include <psp2/display.h>

#define COLOR_CYAN    0x00ffff00
#define COLOR_MAGENDA 0x00ff00ff
#define COLOR_YELLOW  0x0000ffff

#define RGB(R,G,B)    (((B)<<16)|((G)<<8)|(R))
#define RGBT(R,G,B,T) (((T)<<24)|((B)<<16)|((G)<<8)|(R))

#define CENTER(num) ((960/2)-(num*(16/2)))

int blit_setup(void);
void blit_set_color(int fg_col,int bg_col);
int blit_string(int sx,int sy,const char *msg);
int blit_string_ctr(int sy,const char *msg);
int blit_stringf(int sx, int sy, const char *msg, ...);
int blit_set_frame_buf(const SceDisplayFrameBuf *param);
void blit_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);

#endif //V2DMOD_V2DMOD_FB_H
