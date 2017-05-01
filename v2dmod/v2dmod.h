//
// Created by cpasjuste on 28/04/17.
//

#ifndef V2DMOD_H
#define V2DMOD_H

#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <taihen.h>
#include "v2dmod_drawing.h"

typedef void (*initCallback) (void);
typedef void (*drawCallback) (void);
typedef void (*setFbCallback) (const SceDisplayFrameBuf *pParam, int sync);
typedef int (*ctrlCallback) (int port, SceCtrlData *ctrl, int count);

typedef struct V2DModule {
    char name[27];
    char path[256];
    tai_module_info_t modinfo;
    initCallback initCb;
    drawCallback drawCb;
    setFbCallback setFbCb;
} V2DModule;

int v2d_register(V2DModule *module);
int v2d_unregister(V2DModule *m);
int v2d_get_module_info(tai_module_info_t *modinfo);

#endif //V2DMOD_H
