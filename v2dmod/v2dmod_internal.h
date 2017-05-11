//
// Created by cpasjuste on 08/05/17.
//

#ifndef V2DMOD_INTERNAL_H
#define V2DMOD_INTERNAL_H

#include <psp2/kernel/sysmem.h>
#include <psp2/gxm.h>

int v2d_get_module_count();

V2DModule *v2d_get_module_by_path(const char *path);

int v2d_start(void (*iCb)(), void (*dCb)(),
              void (*sCb)(const SceDisplayFrameBuf *pParam, int sync),
              int (*cCb)(int port, SceCtrlData *ctrl, int count));

void v2d_end();

int v2d_register(V2DModule *m);

int v2d_unregister(V2DModule *m);

int _sceCtrlPeekBufferNegative(int port, SceCtrlData *pad_data, int count);

int _sceCtrlPeekBufferNegative2(int port, SceCtrlData *pad_data, int count);

int _sceCtrlPeekBufferPositive(int port, SceCtrlData *pad_data, int count);

int _sceCtrlPeekBufferPositive2(int port, SceCtrlData *pad_data, int count);

int _sceCtrlPeekBufferPositiveExt(int port, SceCtrlData *pad_data, int count);

int _sceCtrlPeekBufferPositiveExt2(int port, SceCtrlData *pad_data, int count);

int _sceCtrlReadBufferNegative(int port, SceCtrlData *pad_data, int count);

int _sceCtrlReadBufferNegative2(int port, SceCtrlData *pad_data, int count);

int _sceCtrlReadBufferPositive(int port, SceCtrlData *pad_data, int count);

int _sceCtrlReadBufferPositive2(int port, SceCtrlData *pad_data, int count);

int _sceCtrlReadBufferPositiveExt(int port, SceCtrlData *pad_data, int count);

int _sceCtrlReadBufferPositiveExt2(int port, SceCtrlData *pad_data, int count);

int _sceDisplaySetFrameBuf(const SceDisplayFrameBuf *pParam, int sync);

int _sceGxmCreateContext(const SceGxmContextParams *params, SceGxmContext **context);

int _sceGxmShaderPatcherCreate(const SceGxmShaderPatcherParams *params, SceGxmShaderPatcher **shaderPatcher);

int _sceGxmShaderPatcherCreateVertexProgram(SceGxmShaderPatcher *shaderPatcher, SceGxmShaderPatcherId programId,
                                            const SceGxmVertexAttribute *attributes, unsigned int attributeCount,
                                            const SceGxmVertexStream *streams, unsigned int streamCount,
                                            SceGxmVertexProgram **vertexProgram);

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

int _sceGxmDisplayQueueAddEntry(SceGxmSyncObject *oldBuffer, SceGxmSyncObject *newBuffer, const void *callbackData);

int _sceGxmDestroyRenderTarget(SceGxmRenderTarget *renderTarget);

void _sceGxmFinish(SceGxmContext *context);

SceUID _sceKernelAllocMemBlock(const char *name, SceKernelMemBlockType type, int size, SceKernelAllocMemBlockOpt *optp);

#endif // V2DMOD_INTERNAL_H
