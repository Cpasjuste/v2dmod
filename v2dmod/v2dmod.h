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

#define CFG_PATH "ux0:/data/v2dmod/config/"
#define MODULES_PATH "ux0:/data/v2dmod/modules/"

typedef void (*initCallback)(void);

typedef void (*drawCallback)(void);

typedef void (*setFbCallback)(const SceDisplayFrameBuf *pParam, int sync);

typedef int (*ctrlCallback)(int port, SceCtrlData *ctrl, int count);

typedef struct V2DOption {
    char name[27];
    int values[MAX_VALUES];
    int index;
    int count;
} V2DOption;

typedef struct V2DModule {
    char name[27];
    char desc[64];
    char path[MAX_PATH];
    SceUID modid;
    drawCallback drawCb;
    setFbCallback setFbCb;
    V2DOption options[MAX_OPTIONS];
} V2DModule;

V2DModule *v2d_register(const char *name);

int v2d_unregister(V2DModule *m);

//void v2d_draw_message(const char *fmt, ...);

extern char v2d_game_name[256];
extern char v2d_game_id[16];

#endif //V2DMOD_H
