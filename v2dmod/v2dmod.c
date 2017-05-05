//
// Created by cpasjuste on 28/04/17.
//

#include <psp2/appmgr.h>
#include <psp2/gxm.h>
#include <taihen.h>
#include <libk/string.h>

#include "utils.h"
#include "vita2d.h"
#include "v2dmod.h"
#include "v2dmod_log.h"

#define HOOK_DISPLAY                    0
#define HOOK_GXM_BEGIN_SCENE            1
#define HOOK_GXM_END_SCENE              2
#define HOOK_GXM_SHADER_PATCHER_CREATE  3
#define HOOK_GXM_CREATE_CONTEXT         4
#define HOOK_GXM_CREATE_RENDER_TARGET   5
#define HOOK_GXM_COLOR_SURFACE_INIT     6
#define HOOK_CTRL_PEEK_1                7
#define HOOK_CTRL_PEEK_2                8
#define HOOK_CTRL_READ_1                9
#define HOOK_CTRL_READ_2                10

int _sceCtrlPeekBufferPositive(int port, SceCtrlData *ctrl, int count);

int _sceCtrlPeekBufferPositive2(int port, SceCtrlData *ctrl, int count);

int _sceCtrlReadBufferPositive(int port, SceCtrlData *ctrl, int count);

int _sceCtrlReadBufferPositive2(int port, SceCtrlData *ctrl, int count);

int _sceDisplaySetFrameBuf(const SceDisplayFrameBuf *pParam, int sync);

int _sceGxmCreateContext(const SceGxmContextParams *params, SceGxmContext **context);

int _sceGxmShaderPatcherCreate(const SceGxmShaderPatcherParams *params, SceGxmShaderPatcher **shaderPatcher);

int _sceGxmCreateRenderTarget(const SceGxmRenderTargetParams *params, SceGxmRenderTarget **renderTarget);

int _sceGxmColorSurfaceInit(SceGxmColorSurface *surface, SceGxmColorFormat colorFormat,
                            SceGxmColorSurfaceType surfaceType,
                            SceGxmColorSurfaceScaleMode scaleMode, SceGxmOutputRegisterSize outputRegisterSize,
                            unsigned int width, unsigned int height, unsigned int strideInPixels, void *data);

int _sceGxmBeginScene(SceGxmContext *context, unsigned int flags,
                      const SceGxmRenderTarget *renderTarget,
                      const SceGxmValidRegion *validRegion,
                      SceGxmSyncObject *vertexSyncObject,
                      SceGxmSyncObject *fragmentSyncObject,
                      const SceGxmColorSurface *colorSurface,
                      const SceGxmDepthStencilSurface *depthStencil);

int _sceGxmEndScene(SceGxmContext *context, const SceGxmNotification *vertexNotification,
                    const SceGxmNotification *fragmentNotification);

#define HOOK_COUNT 11
static Hook hooks[HOOK_COUNT] = {
        {-1, 0, 0x7A410B64, _sceDisplaySetFrameBuf},
        {-1, 0, 0x8734FF4E, _sceGxmBeginScene},
        {-1, 0, 0xFE300E2F, _sceGxmEndScene},
        {-1, 0, 0x5032658,  _sceGxmShaderPatcherCreate},
        {-1, 0, 0xE84CE5B4, _sceGxmCreateContext},
        {-1, 0, 0x207AF96B, _sceGxmCreateRenderTarget},
        {-1, 0, 0xED0F6E25, _sceGxmColorSurfaceInit},
        {-1, 0, 0xA9C3CED6, _sceCtrlPeekBufferPositive},
        {-1, 0, 0x15F81E8C, _sceCtrlPeekBufferPositive2},
        {-1, 0, 0x67E7AB83, _sceCtrlReadBufferPositive},
        {-1, 0, 0xC4226A3E, _sceCtrlReadBufferPositive2}
};

static initCallback initCb = NULL;
static drawCallback drawCb = NULL;
static setFbCallback setFbCb = NULL;
static ctrlCallback ctrlCb = NULL;

static SceGxmContext *gxmContext = NULL;
static SceGxmShaderPatcher *gxmShaderPatcher = NULL;
static SceGxmRenderTarget *gxmRenderTarget = NULL;
static SceGxmColorSurface *gxmColorSurface = NULL;
static int gxmScenesPerFrame = 0;
static int gxmSceneCurrent = 0;

static bool inited = false;
static bool can_draw = true;

vita2d_pgf *v2d_font = NULL;

static void init() {

    if (inited) {
        return;
    }

    if (gxmContext != NULL && gxmShaderPatcher != NULL) {
        inited = 1;
        vita2d_init_advanced(128 * 1024, gxmContext, gxmShaderPatcher);
        vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW);
        v2d_font = vita2d_load_custom_pgf("ux0:/data/default-20.pgf");

        initCb();
    }
}

static int _sceCtrlHooks(tai_hook_ref_t ref_hook, int port, SceCtrlData *ctrl, int count) {

    int ret = TAI_CONTINUE(int, ref_hook, port, ctrl, count);

    if (ctrlCb != NULL) {
        ctrlCb(port, ctrl, count);
    }

    return ret;
}

int _sceCtrlPeekBufferPositive(int port, SceCtrlData *ctrl, int count) {
    return _sceCtrlHooks(hooks[HOOK_CTRL_PEEK_1].ref, port, ctrl, count);
}

int _sceCtrlPeekBufferPositive2(int port, SceCtrlData *ctrl, int count) {
    return _sceCtrlHooks(hooks[HOOK_CTRL_PEEK_2].ref, port, ctrl, count);
}

int _sceCtrlReadBufferPositive(int port, SceCtrlData *ctrl, int count) {
    return _sceCtrlHooks(hooks[HOOK_CTRL_READ_1].ref, port, ctrl, count);
}

int _sceCtrlReadBufferPositive2(int port, SceCtrlData *ctrl, int count) {
    return _sceCtrlHooks(hooks[HOOK_CTRL_READ_2].ref, port, ctrl, count);
}

int _sceDisplaySetFrameBuf(const SceDisplayFrameBuf *pParam, int sync) {

    if (setFbCb != NULL) {
        setFbCb(pParam, sync);
    }

    return TAI_CONTINUE(int, hooks[HOOK_DISPLAY].ref, pParam, sync);
}

int _sceGxmCreateContext(const SceGxmContextParams *params, SceGxmContext **context) {

    int ret = TAI_CONTINUE(int, hooks[HOOK_GXM_CREATE_CONTEXT].ref, params, context);
    gxmContext = *context;

    V2D_LOG("sceGxmCreateContext: context = 0x%p\n", *context);
    init();

    return ret;
}

int _sceGxmShaderPatcherCreate(const SceGxmShaderPatcherParams *params, SceGxmShaderPatcher **shaderPatcher) {

    int ret = TAI_CONTINUE(int, hooks[HOOK_GXM_SHADER_PATCHER_CREATE].ref, params, shaderPatcher);
    gxmShaderPatcher = *shaderPatcher;

    V2D_LOG("sceGxmShaderPatcherCreate\n");
    init();

    return ret;
}

int _sceGxmCreateRenderTarget(const SceGxmRenderTargetParams *params, SceGxmRenderTarget **renderTarget) {

    int ret = TAI_CONTINUE(int, hooks[HOOK_GXM_CREATE_RENDER_TARGET].ref, params, renderTarget);

    V2D_LOG("sceGxmCreateRenderTarget: %ix%i (scenes=%i)\n", params->width, params->height, params->scenesPerFrame);

    //if (params->width == 960 && params->height == 544) {
    gxmRenderTarget = *renderTarget;
    gxmScenesPerFrame = params->scenesPerFrame;
    //}

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

int _sceGxmBeginScene(SceGxmContext *context, unsigned int flags,
                      const SceGxmRenderTarget *renderTarget,
                      const SceGxmValidRegion *validRegion,
                      SceGxmSyncObject *vertexSyncObject,
                      SceGxmSyncObject *fragmentSyncObject,
                      const SceGxmColorSurface *colorSurface,
                      const SceGxmDepthStencilSurface *depthStencil) {

    int ret = TAI_CONTINUE(int, hooks[HOOK_GXM_BEGIN_SCENE].ref, context, flags, renderTarget,
                           validRegion, vertexSyncObject, fragmentSyncObject, colorSurface, depthStencil);

    gxmSceneCurrent++;

    can_draw = true;//(gxmColorSurface == colorSurface);
    //can_draw = (gxmRenderTarget == renderTarget)
    //           && (gxmContext == context);

    if (validRegion != NULL) {
        V2D_LOG("sceGxmBeginScene: validRegion: %ix%i\n", validRegion->xMin, validRegion->xMax);
    }

    return ret;
}

int _sceGxmEndScene(SceGxmContext *context, const SceGxmNotification *vertexNotification,
                    const SceGxmNotification *fragmentNotification) {
    if (inited && gxmSceneCurrent == gxmScenesPerFrame) {
        gxmSceneCurrent = 0;
        vita2d_pool_reset();
        drawCb();
        //can_draw = false;
    }

    return TAI_CONTINUE(int, hooks[HOOK_GXM_END_SCENE].ref, context, vertexNotification, fragmentNotification);
}

void v2d_start(void (*iCb)(),
               void (*dCb)(),
               void (*sCb)(const SceDisplayFrameBuf *pParam, int sync),
               int (*cCb)(int port, SceCtrlData *ctrl, int count)) {

    // Getting title info
    char id[16], title[256];
    sceAppMgrAppParamGetString(0, 9, title , 256);
    sceAppMgrAppParamGetString(0, 12, id , 256);
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
}

void v2d_end() {

    for (int i = 0; i < HOOK_COUNT; i++) {
        if (hooks[i].uid >= 0) {
            taiHookRelease(hooks[i].uid, hooks[i].ref);
        }
    }

    vita2d_fini();

    if (v2d_font != NULL) {
        vita2d_free_pgf(v2d_font);
    }
}
