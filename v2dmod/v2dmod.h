//
// Created by cpasjuste on 28/04/17.
//

#ifndef V2DMOD_H
#define V2DMOD_H

#include <psp2/display.h>
#include "v2dmod_drawing.h"

typedef void (*initCallback) (void);
typedef void (*drawCallback) (void);
typedef void (*setFbCallback) (const SceDisplayFrameBuf *pParam, int sync);

typedef struct V2DModule {
    int id;
    char path[512];
    initCallback initCb;
    drawCallback drawCb;
    setFbCallback setFbCb;
} V2DModule;

int v2d_register(V2DModule *module);
int v2d_unregister(V2DModule *m);

#endif //V2DMOD_H
