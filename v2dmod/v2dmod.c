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

static initCallback initCb = NULL;
static drawCallback drawCb = NULL;
static setFbCallback setFbCb = NULL;

static SceUID display_hook;
static tai_hook_ref_t display_ref;

static SceUID gxmEndScene_hook, gxmShaderPatcherCreate_hook, sceGxmCreateContext_hook;
static tai_hook_ref_t gxmEndScene_ref, gxmShaderPatcherCreate_ref, sceGxmCreateContext_ref;

static SceGxmContext *gxmContext = NULL;
static SceGxmShaderPatcher *gxmShaderPatcher = NULL;

V2DModule *module = NULL;

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

int v2d_register(V2DModule *m) {
    module = m;
    return 0;
}

void v2d_start(void (*iCb)(), void (*dCb)(), void (*sCb)(const SceDisplayFrameBuf *pParam, int sync)) {

    initCb = iCb;
    drawCb = dCb;
    setFbCb = sCb;

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
}

void v2d_end() {

    taiHookRelease(display_hook, display_ref);
    taiHookRelease(sceGxmCreateContext_hook, sceGxmCreateContext_ref);
    taiHookRelease(gxmEndScene_hook, gxmEndScene_ref);
    taiHookRelease(gxmShaderPatcherCreate_hook, gxmShaderPatcherCreate_ref);

    vita2d_fini();

    if (v2d_font != NULL) {
        vita2d_free_pgf(v2d_font);
    }
}
