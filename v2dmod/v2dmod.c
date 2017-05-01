//
// Created by cpasjuste on 28/04/17.
//

#include <psp2/kernel/modulemgr.h>
#include <psp2/gxm.h>
#include <taihen.h>
#include <string.h>

#include "utils.h"
#include "vita2d.h"
#include "v2dmod.h"

vita2d_pgf *v2d_font = NULL;

typedef void (*v2d_ctrlCallback)(SceCtrlData *ctrl, int count);

static initCallback initCb = NULL;
static drawCallback drawCb = NULL;
static setFbCallback setFbCb = NULL;
static ctrlCallback ctrlCb = NULL;

static SceUID display_hook;
static tai_hook_ref_t display_ref;

static SceUID gxmEndScene_hook, gxmShaderPatcherCreate_hook, sceGxmCreateContext_hook;
static tai_hook_ref_t gxmEndScene_ref, gxmShaderPatcherCreate_ref, sceGxmCreateContext_ref;

static tai_hook_ref_t sceCtrlPeekBufferPositive_ref;
static SceUID sceCtrlPeekBufferPositive_hook = -1;
static tai_hook_ref_t sceCtrlPeekBufferPositive2_ref;
static SceUID sceCtrlPeekBufferPositive2_hook = -1;
static tai_hook_ref_t sceCtrlReadBufferPositive_ref;
static SceUID sceCtrlReadBufferPositive_hook = -1;
static tai_hook_ref_t sceCtrlReadBufferPositive2_ref;
static SceUID sceCtrlReadBufferPositive2_hook = -1;

static SceGxmContext *gxmContext = NULL;
static SceGxmShaderPatcher *gxmShaderPatcher = NULL;

static int inited = 0;

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

static int controls(tai_hook_ref_t ref_hook, int port, SceCtrlData *ctrl, int count) {

    if (ctrlCb != NULL) {
        if (ctrlCb(port, ctrl, count) > 0) {
            TAI_CONTINUE(int, ref_hook, port, ctrl, count);
            memset(ctrl, 0, sizeof(SceCtrlData));
            return -1;
        }
    }

    return TAI_CONTINUE(int, ref_hook, port, ctrl, count);
}

int sceCtrlPeekBufferPositive_hook_func(int port, SceCtrlData *ctrl, int count) {
    return controls(sceCtrlPeekBufferPositive_ref, port, ctrl, count);
}

int sceCtrlPeekBufferPositive2_hook_func(int port, SceCtrlData *ctrl, int count) {
    return controls(sceCtrlPeekBufferPositive2_ref, port, ctrl, count);
}

int sceCtrlReadBufferPositive_hook_func(int port, SceCtrlData *ctrl, int count) {
    return controls(sceCtrlReadBufferPositive_ref, port, ctrl, count);
}

int sceCtrlReadBufferPositive2_hook_func(int port, SceCtrlData *ctrl, int count) {
    return controls(sceCtrlReadBufferPositive2_ref, port, ctrl, count);
}

int sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf *pParam, int sync) {

    if (setFbCb != NULL) {
        setFbCb(pParam, sync);
    }

    return TAI_CONTINUE(int, display_ref, pParam, sync);
}

int sceGxmCreateContext_patched(const SceGxmContextParams *params, SceGxmContext **context) {

    int ret = TAI_CONTINUE(int, sceGxmCreateContext_ref, params, context);
    gxmContext = *context;

    init();

    return ret;
}

int sceGxmShaderPatcherCreate_patched(const SceGxmShaderPatcherParams *params, SceGxmShaderPatcher **shaderPatcher) {

    int ret = TAI_CONTINUE(int, gxmShaderPatcherCreate_ref, params, shaderPatcher);
    gxmShaderPatcher = *shaderPatcher;

    init();

    return ret;
}

int sceGxmEndScene_patched(SceGxmContext *context, const SceGxmNotification *vertexNotification,
                           const SceGxmNotification *fragmentNotification) {

    if (inited) {
        vita2d_pool_reset();
        drawCb();
    }

    return TAI_CONTINUE(int, gxmEndScene_ref, context, vertexNotification, fragmentNotification);
}

void v2d_start(void (*iCb)(),
               void (*dCb)(),
               void (*sCb)(const SceDisplayFrameBuf *pParam, int sync),
               int (*cCb)(int port, SceCtrlData *ctrl, int count)) {

    initCb = iCb;
    drawCb = dCb;
    setFbCb = sCb;
    ctrlCb = cCb;

    // Hooking sceDisplaySetFrameBuf
    display_hook = taiHookFunctionImport(&display_ref,
                                         TAI_MAIN_MODULE,
                                         TAI_ANY_LIBRARY,
                                         0x7A410B64,
                                         sceDisplaySetFrameBuf_patched);

    // sceGxmCreateContext
    sceGxmCreateContext_hook = taiHookFunctionImport(&sceGxmCreateContext_ref,
                                                     TAI_MAIN_MODULE,
                                                     TAI_ANY_LIBRARY,
                                                     0xE84CE5B4,
                                                     sceGxmCreateContext_patched);

    // sceGxmEndScene
    gxmEndScene_hook = taiHookFunctionImport(&gxmEndScene_ref,
                                             TAI_MAIN_MODULE,
                                             TAI_ANY_LIBRARY,
                                             0xFE300E2F,
                                             sceGxmEndScene_patched);

    // sceGxmShaderPatcherCreate
    gxmShaderPatcherCreate_hook = taiHookFunctionImport(&gxmShaderPatcherCreate_ref,
                                                        TAI_MAIN_MODULE,
                                                        TAI_ANY_LIBRARY,
                                                        0x5032658,
                                                        sceGxmShaderPatcherCreate_patched);

    // sceCtrlPeekBufferPositive
    sceCtrlPeekBufferPositive_hook = taiHookFunctionImport(&sceCtrlPeekBufferPositive_ref,
                                                           TAI_MAIN_MODULE,
                                                           TAI_ANY_LIBRARY,
                                                           0xA9C3CED6,
                                                           sceCtrlPeekBufferPositive_hook_func);

    // sceCtrlPeekBufferPositive2
    sceCtrlPeekBufferPositive2_hook = taiHookFunctionImport(&sceCtrlPeekBufferPositive2_ref,
                                                            TAI_MAIN_MODULE,
                                                            TAI_ANY_LIBRARY,
                                                            0x15F81E8C,
                                                            sceCtrlPeekBufferPositive2_hook_func);

    // sceCtrlReadBufferPositive
    sceCtrlReadBufferPositive_hook = taiHookFunctionImport(&sceCtrlReadBufferPositive_ref,
                                                           TAI_MAIN_MODULE,
                                                           TAI_ANY_LIBRARY,
                                                           0x67E7AB83,
                                                           sceCtrlReadBufferPositive_hook_func);

    // sceCtrlReadBufferPositive2
    sceCtrlReadBufferPositive2_hook = taiHookFunctionImport(&sceCtrlReadBufferPositive2_ref,
                                                            TAI_MAIN_MODULE,
                                                            TAI_ANY_LIBRARY,
                                                            0xC4226A3E,
                                                            sceCtrlReadBufferPositive2_hook_func);
}

void v2d_end() {

    if (display_hook >= 0)
        taiHookRelease(display_hook, display_ref);

    if (sceGxmCreateContext_hook >= 0)
        taiHookRelease(sceGxmCreateContext_hook, sceGxmCreateContext_ref);

    if (gxmEndScene_hook >= 0)
        taiHookRelease(gxmEndScene_hook, gxmEndScene_ref);

    if (gxmShaderPatcherCreate_hook >= 0)
        taiHookRelease(gxmShaderPatcherCreate_hook, gxmShaderPatcherCreate_ref);

    if (sceCtrlPeekBufferPositive_hook >= 0)
        taiHookRelease(sceCtrlPeekBufferPositive_hook, sceCtrlPeekBufferPositive_ref);

    if (sceCtrlPeekBufferPositive2_hook >= 0)
        taiHookRelease(sceCtrlPeekBufferPositive2_hook, sceCtrlPeekBufferPositive2_ref);

    if (sceCtrlReadBufferPositive_hook >= 0)
        taiHookRelease(sceCtrlReadBufferPositive_hook, sceCtrlReadBufferPositive_ref);

    if (sceCtrlReadBufferPositive2_hook >= 0)
        taiHookRelease(sceCtrlReadBufferPositive2_hook, sceCtrlReadBufferPositive2_ref);

    vita2d_fini();

    if (v2d_font != NULL) {
        vita2d_free_pgf(v2d_font);
    }
}
