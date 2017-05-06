#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
#include <libk/string.h>
#include <libk/stdio.h>

#include "v2dmod.h"

V2DModule module;

static Color COLOR_FONT = {255, 255, 255, 200};
static Color COLOR_MENU = {102, 178, 255, 200};
static Color COLOR_MENU_BORDER = {255, 255, 255, 200};

uint64_t tick = 0;
int frames = 0;
int fps = 0;

void onDisplaySetFrameBuf(const SceDisplayFrameBuf *pParam, int sync) {

    uint64_t t_tick = sceKernelGetProcessTimeWide();
    if (tick == 0) {
        tick = t_tick;
    } else {
        if ((t_tick - tick) > 1000000) {
            fps = frames;
            frames = 0;
            tick = t_tick;
        }
    }
    frames++;
}

void onDraw() {

    Rect rectFps = {4, 4, 84, 34};
    int width = v2d_get_font_width("FPS: 159") + 8;
    rectFps.w = width;

    v2d_draw_rect_outline(rectFps, COLOR_MENU, COLOR_MENU_BORDER, 2);
    v2d_draw_font_advanced(rectFps, COLOR_FONT, true, true, "FPS: %d", fps);
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {

    memset(&module, 0, sizeof(module));
    strcpy(module.name, "v2d_fps");
    module.drawCb = onDraw;
    module.setFbCb = onDisplaySetFrameBuf;

    // important,
    if (!v2d_register(&module)) {
        return SCE_KERNEL_START_FAILED;
    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    v2d_unregister(&module);

    return SCE_KERNEL_STOP_SUCCESS;
}
