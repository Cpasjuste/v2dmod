#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
#include <libk/string.h>

#include "v2dmod.h"

V2DModule module;

static Color BLACK = {0, 0, 0, 200};
static Color RED = {255, 0, 0, 200};
static Color GREEN = {0, 255, 0, 200};

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

    const Rect rectFps = {4, 4, 84, 34};

    v2d_draw_rect_outline(rectFps, GREEN, RED, 2);
    v2d_draw_font_advanced(rectFps, BLACK, true, true, "FPS: %d", fps);
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
