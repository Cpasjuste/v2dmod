#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
#include <libk/string.h>
#include <libk/stdio.h>

#include "v2dmod.h"

V2DModule module;

static Color COLOR = {255, 130, 0, 60};

static Rect rect = {0, 0, 960, 544};

void onDraw() {

    v2d_draw_rect_color(rect, COLOR, true);
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {

    memset(&module, 0, sizeof(module));
    strncpy(module.name, "v2d_vflux", 27);
    strncpy(module.desc, "Screen filter", 64);
    module.drawCb = onDraw;

    if (!v2d_register(&module)) {
        return SCE_KERNEL_START_FAILED;
    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    v2d_unregister(&module);

    return SCE_KERNEL_STOP_SUCCESS;
}
