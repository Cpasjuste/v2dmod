#include <psp2/kernel/modulemgr.h>
#include <psp2/display.h>
#include <psp2/io/dirent.h>
#include <libk/string.h>
#include <libk/stdio.h>

#include "v2dmod.h"
#include "utils.h"
#include "v2dmod_utility.h"

//static Color BLACK = {0, 0, 0, 255};
static Color WHITE = {255, 255, 255, 255};

void v2d_start(void (*iCb)(), void (*dCb)(), void (*sCb)(const SceDisplayFrameBuf *pParam, int sync));

void v2d_end();

int v2d_register(V2DModule *m);

int v2d_unregister(V2DModule *m);

int get_modules_count();

V2DModule *modules = NULL;

FileList *fileList = NULL;

void menu() {

    for (int i = 0; i < fileList->count; i++) {
        if (strlen(fileList->files[i].path) > 0) {
            v2d_draw_font(128, (i + 1) * 20, "FILE: %s", fileList->files[i].path);
        }
    }
}

void onDraw() {

    v2d_set_draw_color(WHITE);

    for (int i = 0; i < MAX_MODULES; i++) {
        if (modules[i].id > -1) {
            if (modules[i].drawCb != NULL) {
                modules[i].drawCb();
                v2d_set_draw_color(WHITE);
            }
        }
    }

    menu();
}

void onInit() {

    modules = (V2DModule *) sce_malloc(MAX_MODULES * sizeof(V2DModule));
    memset(modules, 0, MAX_MODULES * sizeof(V2DModule));
    for (int i = 0; i < MAX_MODULES; i++) {
        modules[i].id = -1;
    }

    fileList = v2d_get_file_list("ux0:/tai/");
    //sceKernelLoadStartModule("ux0:/tai/v2d_fps.suprx", 0, NULL, 0, NULL, NULL);
}

void onDisplaySetFrameBuf(const SceDisplayFrameBuf *pParam, int sync) {

    for (int i = 0; i < MAX_MODULES; i++) {
        if (modules[i].id > -1) {
            if (modules[i].setFbCb != NULL) {
                modules[i].setFbCb(pParam, sync);
            }
        }
    }
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {

    v2d_start(onInit, onDraw, onDisplaySetFrameBuf);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    v2d_end();
    sce_free(modules);

    return SCE_KERNEL_STOP_SUCCESS;
}

void module_exit(void) {

}

int get_modules_count() {

    int count = 0;

    for (int i = 0; i < MAX_MODULES; i++) {
        if (modules[i].id >= 0) {
            count++;
        }
    }

    return count;
}

int v2d_register(V2DModule *m) {

    for (int i = 0; i < MAX_MODULES; i++) {
        if (modules[i].id < 0) {
            modules[i] = *m;
            modules[i].id = i;
            return 1;
        }
    }

    return -1;
}

int v2d_unregister(V2DModule *m) {

    if (m != NULL) {
        memset(&modules[m->id], 0, sizeof(V2DModule));
        modules[m->id].id = -1;
        return 1;
    }

    return -1;
}
