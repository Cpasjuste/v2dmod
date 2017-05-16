#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
#include <libk/string.h>

#include "v2dmod.h"

V2DModule *module;

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

    module = v2d_register("v2d_fps"); // must be real module name
    if (module == NULL) {
        return SCE_KERNEL_START_FAILED;
    }

    strncpy(module->desc, "Frames per second counter", 64);
    module->drawCb = onDraw;
    module->setFbCb = onDisplaySetFrameBuf;

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    v2d_unregister(module);

    return SCE_KERNEL_STOP_SUCCESS;
}
