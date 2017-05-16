#include <psp2/kernel/modulemgr.h>
#include <libk/string.h>
#include "v2dmod.h"

#define OPT_ENABLED 0
#define OPT_R       1
#define OPT_G       2
#define OPT_B       3
#define OPT_A       4

V2DModule *module;

static Rect rect = {0, 0, 960, 544};

void onDraw() {

    // options[0] == "ENABLED" option
    if (module->options[OPT_ENABLED].index == 1) {
        Color color = {
                module->options[OPT_R].index * 25,
                module->options[OPT_G].index * 25,
                module->options[OPT_B].index * 25,
                module->options[OPT_A].index * 25
        };
        v2d_draw_rect_color(rect, color, true);
    }
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {

    // register module to main plugin (v2dmod)
    module = v2d_register("v2d_vflux"); // must be real module name
    if (module == NULL) {
        return SCE_KERNEL_START_FAILED;
    }

    strncpy(module->desc, "Screen filter for your eyes", 64);
    module->drawCb = onDraw;

    strncpy(module->options[OPT_ENABLED].name, "ENABLED", 27);
    module->options[OPT_ENABLED].index = 0; // OFF
    module->options[OPT_ENABLED].count = 2; // OFF / ON

    strncpy(module->options[OPT_R].name, "RED", 27);
    module->options[OPT_R].index = 3; // 75
    module->options[OPT_R].count = MAX_VALUES; // > 2 == multiple choices
    for (int i = 0; i < MAX_VALUES; i++) {
        module->options[OPT_R].values[i] = 25 * i;
    }

    strncpy(module->options[OPT_G].name, "GREEN", 27);
    module->options[OPT_G].index = 0;
    module->options[OPT_G].count = MAX_VALUES;
    for (int i = 0; i < MAX_VALUES; i++) {
        module->options[OPT_G].values[i] = 25 * i;
    }

    strncpy(module->options[OPT_B].name, "BLUE", 27);
    module->options[OPT_B].index = 0;
    module->options[OPT_B].count = MAX_VALUES;
    for (int i = 0; i < MAX_VALUES; i++) {
        module->options[OPT_B].values[i] = 25 * i;
    }

    strncpy(module->options[OPT_A].name, "ALPHA", 27);
    module->options[OPT_A].index = 2; // 50
    module->options[OPT_A].count = MAX_VALUES;
    for (int i = 0; i < MAX_VALUES; i++) {
        module->options[OPT_A].values[i] = 25 * i;
    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    v2d_unregister(module);

    return SCE_KERNEL_STOP_SUCCESS;
}
