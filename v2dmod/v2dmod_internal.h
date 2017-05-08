//
// Created by cpasjuste on 08/05/17.
//

#ifndef V2DMOD_INTERNAL_H
#define V2DMOD_INTERNAL_H

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

int _sceGxmDestroyRenderTarget(SceGxmRenderTarget *renderTarget);

#endif // V2DMOD_INTERNAL_H
