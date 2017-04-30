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
    char path[512];
    initCallback initCb;
    drawCallback drawCb;
    setFbCallback setFbCb;
} V2DModule;

int v2d_register(V2DModule *module);

#endif //V2DMOD_H
