#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
#include <libk/string.h>
#include <taihen.h>

#include "v2dmod.h"

V2DModule module;

static Color BLACK = {0, 0, 0, 200};
static Color RED = {255, 0, 0, 200};
static Color GREEN = {0, 255, 0, 200};

static SceDisplayFrameBuf fbParams;

struct SurfaceInfo {
    SceGxmColorSurfaceScaleMode scaleMode;
    SceGxmColorFormat format;
    SceGxmColorSurfaceType type;
    unsigned int strideInPixels;
    SceGxmColorSurfaceGammaMode gammaMod;
    SceGxmColorSurfaceDitherMode ditherMode;
};

static struct SurfaceInfo surfaceInfo;

static SceUID sceGxmBeginScene_hook;
static tai_hook_ref_t sceGxmBeginScene_ref;

int sceGxmBeginScene_patched(SceGxmContext *context, unsigned int flags, const SceGxmRenderTarget *renderTarget,
                             const SceGxmValidRegion *validRegion, SceGxmSyncObject *vertexSyncObject,
                             SceGxmSyncObject *fragmentSyncObject, const SceGxmColorSurface *colorSurface,
                             const SceGxmDepthStencilSurface *depthStencil) {

    if (colorSurface != NULL) {
        surfaceInfo.scaleMode = sceGxmColorSurfaceGetScaleMode(colorSurface);
        surfaceInfo.format = sceGxmColorSurfaceGetFormat(colorSurface);
        surfaceInfo.type = sceGxmColorSurfaceGetType(colorSurface);
        surfaceInfo.strideInPixels = sceGxmColorSurfaceGetStrideInPixels(colorSurface);
        surfaceInfo.gammaMod = sceGxmColorSurfaceGetGammaMode(colorSurface);
        surfaceInfo.ditherMode = sceGxmColorSurfaceGetDitherMode(colorSurface);
    }

    return TAI_CONTINUE(int, sceGxmBeginScene_ref, context, flags, renderTarget, validRegion, vertexSyncObject,
            fragmentSyncObject, colorSurface, depthStencil);
}

void onDisplaySetFrameBuf(const SceDisplayFrameBuf *pParam, int sync) {

}

void onDraw() {

    /*
     if (fbParams.size > 0) {

         int font_h = 20;

         Rect rectFbTitle = {4, 64, 150, font_h + 8};
         v2d_draw_rect_outline(rectFbTitle, GREEN, RED, 2);
         v2d_draw_font_advanced(font, rectFbTitle, BLACK, true, true, "FRAMEBUFFER");

         Rect rectFb = {4, rectFbTitle.y + rectFbTitle.h, 220, 5 * font_h + 20};
         v2d_draw_rect_outline(rectFb, GREEN, RED, 2);
         rectFb.x += 8;
         rectFb.y += 10;
         v2d_draw_font(font, rectFb.x, rectFb.y, "RES: %ix%i", fbParams.width, fbParams.height);
         v2d_draw_font(font, rectFb.x, rectFb.y + 20, "PITCH: %i", fbParams.pitch);
         v2d_draw_font(font, rectFb.x, rectFb.y + 40, "FORMAT: 0x%08X", fbParams.pixelformat);
         v2d_draw_font(font, rectFb.x, rectFb.y + 60, "SIZE: 0x%X", fbParams.size);
         v2d_draw_font(font, rectFb.x, rectFb.y + 80, "ADDR: 0x%08X", fbParams.base);
     }

     if (surfaceInfo.strideInPixels > 0) {

         int font_h = 20;

         Rect rectSurfTitle = {4, 250, 150, font_h + 8};
         v2d_draw_rect_outline(rectSurfTitle, GREEN, RED, 2);
         v2d_draw_font_advanced(font, rectSurfTitle, BLACK, true, true, "SURFACE");

         Rect rectTitle = {4, rectSurfTitle.y + rectSurfTitle.h, 220, 6 * font_h + 20};
         v2d_draw_rect_outline(rectTitle, GREEN, RED, 2);
         rectTitle.x += 8;
         rectTitle.y += 10;

         if (surfaceInfo.scaleMode == SCE_GXM_COLOR_SURFACE_SCALE_NONE) {
             v2d_draw_font(font, rectTitle.x, rectTitle.y, "SCALING: SCALE_NONE");
         } else {
             v2d_draw_font(font, rectTitle.x, rectTitle.y, "SCALING: MSAA_DOWNSCALE");
         }

         v2d_draw_font(font, rectTitle.x, rectTitle.y + 20, "FORMAT: 0x%08X", surfaceInfo.format);

         if (surfaceInfo.type == SCE_GXM_COLOR_SURFACE_LINEAR) {
             v2d_draw_font(font, rectTitle.x, rectTitle.y + 40, "TYPE: LINEAR");
         } else if (surfaceInfo.type == SCE_GXM_COLOR_SURFACE_TILED) {
             v2d_draw_font(font, rectTitle.x, rectTitle.y + 40, "TYPE: TILED");
         } else {
             v2d_draw_font(font, rectTitle.x, rectTitle.y + 40, "TYPE: SWIZZLED");
         }

         v2d_draw_font(font, rectTitle.x, rectTitle.y + 60, "STRIDE: %i", surfaceInfo.strideInPixels);

         if (surfaceInfo.gammaMod == SCE_GXM_COLOR_SURFACE_GAMMA_NONE) {
             v2d_draw_font(font, rectTitle.x, rectTitle.y + 80, "GAMMA: NONE");
         } else if (surfaceInfo.gammaMod == SCE_GXM_COLOR_SURFACE_GAMMA_R) {
             v2d_draw_font(font, rectTitle.x, rectTitle.y + 80, "GAMMA: R");
         } else if (surfaceInfo.gammaMod == SCE_GXM_COLOR_SURFACE_GAMMA_GR) {
             v2d_draw_font(font, rectTitle.x, rectTitle.y + 80, "GAMMA: GR");
         } else {
             v2d_draw_font(font, rectTitle.x, rectTitle.y + 80, "GAMMA: BGR");
         }

         if (surfaceInfo.ditherMode == SCE_GXM_COLOR_SURFACE_DITHER_DISABLED) {
             v2d_draw_font(font, rectTitle.x, rectTitle.y + 100, "DITHER: DISABLED");
         } else {
             v2d_draw_font(font, rectTitle.x, rectTitle.y + 100, "DITHER: ENABLED");
         }
     }
     */
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {

    // Hooking sceGxmBeginScene
    sceGxmBeginScene_hook = taiHookFunctionImport(&sceGxmBeginScene_ref,
                                                  TAI_MAIN_MODULE,
                                                  TAI_ANY_LIBRARY,
                                                  0x8734FF4E,
                                                  sceGxmBeginScene_patched);

    memset(&module, 0, sizeof(module));
    module.drawCb = onDraw;
    module.setFbCb = onDisplaySetFrameBuf;
    v2d_register(&module);

    memset(&fbParams, 0, sizeof(fbParams));
    memset(&surfaceInfo, 0, sizeof(surfaceInfo));

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    taiHookRelease(sceGxmBeginScene_hook, sceGxmBeginScene_ref);

    return SCE_KERNEL_STOP_SUCCESS;
}
