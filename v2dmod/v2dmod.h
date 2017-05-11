//
// Created by cpasjuste on 28/04/17.
//

#ifndef V2DMOD_H
#define V2DMOD_H

#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <taihen.h>
#include "v2dmod_drawing.h"
#include "v2dmod_utility.h"

typedef void (*initCallback)(void);

typedef void (*drawCallback)(void);

typedef void (*setFbCallback)(const SceDisplayFrameBuf *pParam, int sync);

typedef int (*ctrlCallback)(int port, SceCtrlData *ctrl, int count);

typedef struct Hook {
    SceUID uid;
    tai_hook_ref_t ref;
    uint32_t nid;
    const void *func;
} Hook;

typedef struct V2DModule {
    char name[27];
    char desc[64];
    char path[MAX_PATH];
    SceUID modid;
    initCallback initCb;
    drawCallback drawCb;
    setFbCallback setFbCb;
} V2DModule;

int v2d_register(V2DModule *module);

int v2d_unregister(V2DModule *m);

V2DModule *v2d_get_module(SceUID mid);

SceUID v2d_get_module_id(const char *name);

void v2d_draw_message(const char *fmt, ...);

extern char v2d_game_name[256];
extern char v2d_game_id[16];

#endif //V2DMOD_H
