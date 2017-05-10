//
// Created by cpasjuste on 28/04/17.
//

#include <libk/string.h>
#include <psp2/appmgr.h>
#include <psp2/gxm.h>
#include <taihen.h>

#include "vita2d.h"
#include "v2dmod.h"
#include "v2dmod_log.h"
#include "v2dmod_internal.h"

#define HOOK_DISPLAY                    0
#define HOOK_GXM_BEGIN_SCENE            1
#define HOOK_GXM_END_SCENE              2
#define HOOK_GXM_SHADER_PATCHER_CREATE  3
#define HOOK_GXM_CREATE_CONTEXT         4
#define HOOK_GXM_CREATE_RENDER_TARGET   5
#define HOOK_GXM_DESTROY_RENDER_TARGET  6
#define HOOK_GXM_COLOR_SURFACE_INIT     7
#define HOOK_GXM_DISPLAY_QUEUE          8
#define HOOK_GXM_FINISH                 9
#define HOOK_CTRL_PEEK_NEG              10
#define HOOK_CTRL_PEEK_1                11
#define HOOK_CTRL_PEEK_2                12
#define HOOK_CTRL_READ_NEG              13
#define HOOK_CTRL_READ_1                14
#define HOOK_CTRL_READ_2                15
#define HOOK_ALLOC                      16

#define HOOK_COUNT 17

static Hook hooks[HOOK_COUNT] = {
        {-1, 0, 0x7A410B64, _sceDisplaySetFrameBuf},
        {-1, 0, 0x8734FF4E, _sceGxmBeginScene},
        {-1, 0, 0xFE300E2F, _sceGxmEndScene},
        {-1, 0, 0x5032658,  _sceGxmShaderPatcherCreate},
        {-1, 0, 0xE84CE5B4, _sceGxmCreateContext},
        {-1, 0, 0x207AF96B, _sceGxmCreateRenderTarget},
        {-1, 0, 0xB94C50A,  _sceGxmDestroyRenderTarget},
        {-1, 0, 0xED0F6E25, _sceGxmColorSurfaceInit},
        {-1, 0, 0xEC5C26B5, _sceGxmDisplayQueueAddEntry},
        {-1, 0, 0x733D8AE,  _sceGxmFinish},
        {-1, 0, 0x104ED1A7, _sceCtrlPeekBufferNegative},
//      {-1, 0, 0x27A0C5FB, _sceCtrlPeekBufferNegative2},
        {-1, 0, 0xA9C3CED6, _sceCtrlPeekBufferPositive},
        {-1, 0, 0x15F81E8C, _sceCtrlPeekBufferPositive2},
        {-1, 0, 0x15F96FB0, _sceCtrlReadBufferNegative},
//      {-1, 0, 0x27A0C5FB, _sceCtrlReadBufferNegative2},
        {-1, 0, 0x67E7AB83, _sceCtrlReadBufferPositive},
        {-1, 0, 0xC4226A3E, _sceCtrlReadBufferPositive2},
        {-1, 0, 0xB9D5EBDE, _sceKernelAllocMemBlock}
};

static initCallback initCb = NULL;
static drawCallback drawCb = NULL;
static setFbCallback setFbCb = NULL;
static ctrlCallback ctrlCb = NULL;

static SceGxmContext *gxmContext = NULL;
static SceGxmShaderPatcher *gxmShaderPatcher = NULL;
//static SceGxmRenderTarget *gxmRenderTarget = NULL;

#define MAX_TARGET 16
int gxmRenderTargetCurrent = 0;
int gxmRenderTargetCount = 0;
static SceGxmRenderTarget *gxmRenderTarget[MAX_TARGET];

//static SceGxmColorSurface *gxmColorSurface = NULL;
//static int gxmScenesPerFrame = 0;
//static int gxmSceneCurrent = 0;

static bool inited = false;
static bool can_draw = true;
static bool new_input_loop = false;
//static int scenes_count = 0;

vita2d_bmf *v2d_font = NULL;

static void init() {

    if (inited) {
        return;
    }

    if (gxmContext != NULL && gxmShaderPatcher != NULL) {

        vita2d_init_advanced(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, 32 * 1024, gxmContext, gxmShaderPatcher);
        vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW);
        v2d_font = vita2d_load_bmf("ux0:/tai/v2dmod/data/impact-25.bmp",
                                   "ux0:/tai/v2dmod/data/impact-25.fnt");
        initCb();

        inited = 1;
    }
}

SceUID _sceKernelAllocMemBlock(
        const char *name, SceKernelMemBlockType type,
        int size, SceKernelAllocMemBlockOpt *optp) {

    SceUID ret = TAI_CONTINUE(SceUID, hooks[HOOK_ALLOC].ref, name, type, size, optp);

    if (ret >= 0) {

        if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_RW)
            V2D_LOG("sceKernelAllocMemBlock(%s, %i, USER_RW)\n", name, size);
        else if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE)
            V2D_LOG("sceKernelAllocMemBlock(%s, %i, USER_RW_UNCACHE)\n", name, size);
        else if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW)
            V2D_LOG("sceKernelAllocMemBlock(%s, %i, USER_CDRAM_RW)\n", name, size);
        else if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_RW)
            V2D_LOG("sceKernelAllocMemBlock(%s, %i, SER_MAIN_PHYCONT_RW)\n", name, size);
        else if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW)
            V2D_LOG("sceKernelAllocMemBlock(%s, %i, USER_MAIN_PHYCONT_NC_RW\n", name, size);
    } else {
        V2D_LOG("sceKernelAllocMemBlock(%s, %i, FAILED)\n", name, size);
    }

    return ret;
}

static int _sceCtrlHooks(tai_hook_ref_t ref_hook, int port, SceCtrlData *ctrl, int count) {

    if (ref_hook == 0) {
        return 1;
    }

    int ret = TAI_CONTINUE(int, ref_hook, port, ctrl, count);

    if (inited && new_input_loop && ctrl != NULL && ctrlCb != NULL) {
        new_input_loop = false;
        if (ctrlCb(port, ctrl, count)) {
            ctrl->buttons = 0;
        }
    }

    return ret;
}

int _sceCtrlPeekBufferNegative(int port, SceCtrlData *pad_data, int count) {
    return _sceCtrlHooks(hooks[HOOK_CTRL_PEEK_NEG].ref, port, pad_data, count);
}

int _sceCtrlPeekBufferPositive(int port, SceCtrlData *pad_data, int count) {
    return _sceCtrlHooks(hooks[HOOK_CTRL_PEEK_1].ref, port, pad_data, count);
}

int _sceCtrlPeekBufferPositive2(int port, SceCtrlData *pad_data, int count) {
    return _sceCtrlHooks(hooks[HOOK_CTRL_PEEK_2].ref, port, pad_data, count);
}

int _sceCtrlReadBufferNegative(int port, SceCtrlData *pad_data, int count) {
    return _sceCtrlHooks(hooks[HOOK_CTRL_READ_NEG].ref, port, pad_data, count);
}

int _sceCtrlReadBufferPositive(int port, SceCtrlData *pad_data, int count) {
    return _sceCtrlHooks(hooks[HOOK_CTRL_READ_1].ref, port, pad_data, count);
}

int _sceCtrlReadBufferPositive2(int port, SceCtrlData *pad_data, int count) {
    return _sceCtrlHooks(hooks[HOOK_CTRL_READ_2].ref, port, pad_data, count);
}

int _sceDisplaySetFrameBuf(const SceDisplayFrameBuf *pParam, int sync) {

    if (inited && setFbCb != NULL) {
        setFbCb(pParam, sync);
    }

    return TAI_CONTINUE(int, hooks[HOOK_DISPLAY].ref, pParam, sync);
}

int _sceGxmCreateContext(const SceGxmContextParams *params, SceGxmContext **context) {

    int ret = TAI_CONTINUE(int, hooks[HOOK_GXM_CREATE_CONTEXT].ref, params, context);
    gxmContext = *context;

    V2D_LOG("sceGxmCreateContext: %p\n", *context);
    init();

    return ret;
}

int _sceGxmShaderPatcherCreate(const SceGxmShaderPatcherParams *params, SceGxmShaderPatcher **shaderPatcher) {

    int ret = TAI_CONTINUE(int, hooks[HOOK_GXM_SHADER_PATCHER_CREATE].ref, params, shaderPatcher);
    gxmShaderPatcher = *shaderPatcher;

    V2D_LOG("sceGxmShaderPatcherCreate: %p\n", *shaderPatcher);
    init();

    return ret;
}

int _sceGxmColorSurfaceInit(SceGxmColorSurface *surface, SceGxmColorFormat colorFormat,
                            SceGxmColorSurfaceType surfaceType,
                            SceGxmColorSurfaceScaleMode scaleMode, SceGxmOutputRegisterSize outputRegisterSize,
                            unsigned int width, unsigned int height, unsigned int strideInPixels, void *data) {

    int ret = TAI_CONTINUE(int, hooks[HOOK_GXM_COLOR_SURFACE_INIT].ref, surface, colorFormat, surfaceType, scaleMode,
                           outputRegisterSize, width, height, strideInPixels, data);

    V2D_LOG("sceGxmColorSurfaceInit: %ix%i, fmt=0x%08X, type=0x%08X, scale=0x%08X, stride=%i\n",
            width, height, colorFormat, surfaceType, scaleMode, strideInPixels);

    return ret;
}

int _sceGxmDestroyRenderTarget(SceGxmRenderTarget *renderTarget) {

    V2D_LOG("sceGxmDestroyRenderTarget: %p\n", renderTarget);

    if (renderTarget == gxmRenderTarget[gxmRenderTargetCurrent]) {
        // TODO: deinit/reinit vita2d ?
    }

    int ret = TAI_CONTINUE(int, hooks[HOOK_GXM_DESTROY_RENDER_TARGET].ref, renderTarget);


    return ret;
}

int _sceGxmCreateRenderTarget(const SceGxmRenderTargetParams *params, SceGxmRenderTarget **renderTarget) {

    int ret = TAI_CONTINUE(int, hooks[HOOK_GXM_CREATE_RENDER_TARGET].ref, params, renderTarget);

    V2D_LOG("sceGxmCreateRenderTarget: %p - %ix%i (scenes=%i)\n",
            *renderTarget, params->width, params->height, params->scenesPerFrame);

    if ((params->width == 960 && params->height == 544) && gxmRenderTargetCount < MAX_TARGET) {
        gxmRenderTarget[gxmRenderTargetCount] = *renderTarget;
        gxmRenderTargetCount++;
    }
    /*
    if (gxmRenderTarget == NULL && gxmShaderPatcher != NULL
        && params->width == 960 && params->height == 544) {
        gxmRenderTarget = *renderTarget;
    }
    */

    return ret;
}

int _sceGxmBeginScene(SceGxmContext *context, unsigned int flags,
                      const SceGxmRenderTarget *renderTarget,
                      const SceGxmValidRegion *validRegion,
                      SceGxmSyncObject *vertexSyncObject,
                      SceGxmSyncObject *fragmentSyncObject,
                      const SceGxmColorSurface *colorSurface,
                      const SceGxmDepthStencilSurface *depthStencil) {

    int ret = TAI_CONTINUE(int, hooks[HOOK_GXM_BEGIN_SCENE].ref, context, flags, renderTarget,
                           validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, depthStencil);

    can_draw = (context == gxmContext)
               //&& (gxmRenderTarget[gxmRenderTargetCurrent] == renderTarget)
               && (colorSurface != NULL)
               && (sceGxmTextureGetWidth(&colorSurface->backgroundTex) == 960);

    V2D_LOG("sceGxmBeginScene: t=%p, d=%i\n", renderTarget, can_draw);

    return ret;
}

int _sceGxmEndScene(SceGxmContext *context, const SceGxmNotification *vertexNotification,
                    const SceGxmNotification *fragmentNotification) {

    V2D_LOG("sceGxmEndScene: can_draw=%i\n", can_draw);
    if (inited && can_draw) {
        drawCb();
    }

    return TAI_CONTINUE(int, hooks[HOOK_GXM_END_SCENE].ref, context, vertexNotification, fragmentNotification);
}

void _sceGxmFinish(SceGxmContext *context) {

    TAI_CONTINUE(void, hooks[HOOK_GXM_FINISH].ref, context);
    vita2d_pool_reset();
}

int _sceGxmDisplayQueueAddEntry(SceGxmSyncObject *oldBuffer, SceGxmSyncObject *newBuffer, const void *callbackData) {

    int ret = TAI_CONTINUE(int, hooks[HOOK_GXM_DISPLAY_QUEUE].ref, oldBuffer, newBuffer, callbackData);

    new_input_loop = true;
    V2D_LOG("LOOP\n");

    return ret;
}

int v2d_start(void (*iCb)(),
              void (*dCb)(),
              void (*sCb)(const SceDisplayFrameBuf *pParam, int sync),
              int (*cCb)(int port, SceCtrlData *ctrl, int count)) {

    // Getting title info
    char id[16], title[256];
    sceAppMgrAppParamGetString(0, 9, title, 256);
    sceAppMgrAppParamGetString(0, 12, id, 256);

    V2D_LOG("====================\n");
    V2D_LOG(" %s: %s\n", id, title);
    V2D_LOG("====================\n");

    initCb = iCb;
    drawCb = dCb;
    setFbCb = sCb;
    ctrlCb = cCb;

    for (int i = 0; i < HOOK_COUNT; i++) {
        hooks[i].uid = taiHookFunctionImport(&hooks[i].ref,
                                             TAI_MAIN_MODULE,
                                             TAI_ANY_LIBRARY,
                                             hooks[i].nid,
                                             hooks[i].func);
    }

    return 1;
}

void v2d_end() {

    for (int i = 0; i < HOOK_COUNT; i++) {
        if (hooks[i].uid >= 0) {
            taiHookRelease(hooks[i].uid, hooks[i].ref);
        }
    }

    vita2d_fini();

    if (v2d_font != NULL) {
        vita2d_free_bmf(v2d_font);
    }
}
