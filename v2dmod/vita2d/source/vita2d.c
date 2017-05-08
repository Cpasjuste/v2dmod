#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/sysmodule.h>
#include <stdlib.h>
#include "vita2d.h"
#include "utils.h"

//#define USE_CLEAR_VERTEX

#ifdef DEBUG_BUILD
#  include <stdio.h>
#  define DEBUG(...) printf(__VA_ARGS__)
#else
#  define DEBUG_PRINT(...)
#endif

/* Defines */

#define DISPLAY_WIDTH           960
#define DISPLAY_HEIGHT          544
#define MSAA_MODE               SCE_GXM_MULTISAMPLE_NONE
#define DEFAULT_TEMP_POOL_SIZE  (1 * 1024 * 1024)

/* Extern */

#ifdef USE_CLEAR_VERTEX
extern const SceGxmProgram clear_v_gxp_start;
extern const SceGxmProgram clear_f_gxp_start;
#endif
extern const SceGxmProgram color_v_gxp_start;
extern const SceGxmProgram color_f_gxp_start;
extern const SceGxmProgram texture_v_gxp_start;
extern const SceGxmProgram texture_f_gxp_start;
extern const SceGxmProgram texture_tint_f_gxp_start;

/* Static variables */

//static int pgf_module_was_loaded = 0;

#ifdef USE_CLEAR_VERTEX
static const SceGxmProgram *const clearVertexProgramGxp = &clear_v_gxp_start;
static const SceGxmProgram *const clearFragmentProgramGxp = &clear_f_gxp_start;
#endif
static const SceGxmProgram *const colorVertexProgramGxp = &color_v_gxp_start;
static const SceGxmProgram *const colorFragmentProgramGxp = &color_f_gxp_start;
static const SceGxmProgram *const textureVertexProgramGxp = &texture_v_gxp_start;
static const SceGxmProgram *const textureFragmentProgramGxp = &texture_f_gxp_start;
static const SceGxmProgram *const textureTintFragmentProgramGxp = &texture_tint_f_gxp_start;

static int vita2d_initialized = 0;
#ifdef USE_CLEAR_VERTEX
static float clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
static unsigned int clear_color_u = 0xFF000000;
#endif
static int clip_rect_x_min = 0;
static int clip_rect_y_min = 0;
static int clip_rect_x_max = DISPLAY_WIDTH;
static int clip_rect_y_max = DISPLAY_HEIGHT;
static int drawing = 0;
static int clipping_enabled = 0;

static SceGxmShaderPatcher *shaderPatcher = NULL;
#ifdef USE_CLEAR_VERTEX
static SceGxmVertexProgram *clearVertexProgram = NULL;
static SceGxmFragmentProgram *clearFragmentProgram = NULL;

static SceGxmShaderPatcherId clearVertexProgramId;
static SceGxmShaderPatcherId clearFragmentProgramId;
#endif
static SceGxmShaderPatcherId colorVertexProgramId;
static SceGxmShaderPatcherId colorFragmentProgramId;
static SceGxmShaderPatcherId textureVertexProgramId;
static SceGxmShaderPatcherId textureFragmentProgramId;
static SceGxmShaderPatcherId textureTintFragmentProgramId;

#ifdef USE_CLEAR_VERTEX
static SceUID clearVerticesUid;
static SceUID clearIndicesUid;
static vita2d_clear_vertex *clearVertices = NULL;
static uint16_t *clearIndices = NULL;
#endif

/* Shared with other .c */
float _vita2d_ortho_matrix[4 * 4];
SceGxmContext *_vita2d_context = NULL;
SceGxmVertexProgram *_vita2d_colorVertexProgram = NULL;
SceGxmFragmentProgram *_vita2d_colorFragmentProgram = NULL;
SceGxmVertexProgram *_vita2d_textureVertexProgram = NULL;
SceGxmFragmentProgram *_vita2d_textureFragmentProgram = NULL;
SceGxmFragmentProgram *_vita2d_textureTintFragmentProgram = NULL;
const SceGxmProgramParameter *_vita2d_clearClearColorParam = NULL;
const SceGxmProgramParameter *_vita2d_colorWvpParam = NULL;
const SceGxmProgramParameter *_vita2d_textureWvpParam = NULL;
const SceGxmProgramParameter *_vita2d_textureTintColorParam = NULL;

// Temporary memory pool
static void *pool_addr = NULL;
static SceUID poolUid;
static unsigned int pool_index = 0;
static unsigned int pool_size = 0;

int vita2d_init(SceKernelMemBlockType memType, SceGxmContext *pCtx, SceGxmShaderPatcher *pPatcher)
{
    return vita2d_init_advanced(memType, DEFAULT_TEMP_POOL_SIZE, pCtx, pPatcher);
}

int vita2d_init_advanced(SceKernelMemBlockType memType, unsigned int temp_pool_size, SceGxmContext *pCtx, SceGxmShaderPatcher *pPatcher)
{
    int err;

    UNUSED(err);

    if (vita2d_initialized) {
        DEBUG_PRINT("libvita2d is already initialized!\n");
        return 1;
    }

    _vita2d_context = pCtx;
    shaderPatcher = pPatcher;

    // check the shaders
#ifdef USE_CLEAR_VERTEX
    err = sceGxmProgramCheck(clearVertexProgramGxp);
    DEBUG_PRINT("clear_v sceGxmProgramCheck(): 0x%08X\n", err);
    err = sceGxmProgramCheck(clearFragmentProgramGxp);
    DEBUG_PRINT("clear_f sceGxmProgramCheck(): 0x%08X\n", err);
#endif
    err = sceGxmProgramCheck(colorVertexProgramGxp);
    DEBUG_PRINT("color_v sceGxmProgramCheck(): 0x%08X\n", err);
    err = sceGxmProgramCheck(colorFragmentProgramGxp);
    DEBUG_PRINT("color_f sceGxmProgramCheck(): 0x%08X\n", err);
    err = sceGxmProgramCheck(textureVertexProgramGxp);
    DEBUG_PRINT("texture_v sceGxmProgramCheck(): 0x%08X\n", err);
    err = sceGxmProgramCheck(textureFragmentProgramGxp);
    DEBUG_PRINT("texture_f sceGxmProgramCheck(): 0x%08X\n", err);
    err = sceGxmProgramCheck(textureTintFragmentProgramGxp);
    DEBUG_PRINT("texture_tint_f sceGxmProgramCheck(): 0x%08X\n", err);

#ifdef USE_CLEAR_VERTEX
    // register programs with the patcher
    err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, clearVertexProgramGxp, &clearVertexProgramId);
    DEBUG_PRINT("clear_v sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

    err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, clearFragmentProgramGxp, &clearFragmentProgramId);
    DEBUG_PRINT("clear_f sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);
#endif
    err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, colorVertexProgramGxp, &colorVertexProgramId);
    DEBUG_PRINT("color_v sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

    err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, colorFragmentProgramGxp, &colorFragmentProgramId);
    DEBUG_PRINT("color_f sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

    err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, textureVertexProgramGxp, &textureVertexProgramId);
    DEBUG_PRINT("texture_v sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

    err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, textureFragmentProgramGxp, &textureFragmentProgramId);
    DEBUG_PRINT("texture_f sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

    err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, textureTintFragmentProgramGxp,
                                             &textureTintFragmentProgramId);
    DEBUG_PRINT("texture_tint_f sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

    // Fill SceGxmBlendInfo
    static const SceGxmBlendInfo blend_info = {
            .colorFunc = SCE_GXM_BLEND_FUNC_ADD,
            .alphaFunc = SCE_GXM_BLEND_FUNC_ADD,
            .colorSrc  = SCE_GXM_BLEND_FACTOR_SRC_ALPHA,
            .colorDst  = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .alphaSrc  = SCE_GXM_BLEND_FACTOR_ONE,
            .alphaDst  = SCE_GXM_BLEND_FACTOR_ZERO,
            .colorMask = SCE_GXM_COLOR_MASK_ALL
    };

#ifdef USE_CLEAR_VERTEX
    // get attributes by name to create vertex format bindings
    const SceGxmProgramParameter *paramClearPositionAttribute = sceGxmProgramFindParameterByName(clearVertexProgramGxp,
                                                                                                 "aPosition");

    // create clear vertex format
    SceGxmVertexAttribute clearVertexAttributes[1];
    SceGxmVertexStream clearVertexStreams[1];
    clearVertexAttributes[0].streamIndex = 0;
    clearVertexAttributes[0].offset = 0;
    clearVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
    clearVertexAttributes[0].componentCount = 2;
    clearVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramClearPositionAttribute);
    clearVertexStreams[0].stride = sizeof(vita2d_clear_vertex);
    clearVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

    // create clear programs
    err = sceGxmShaderPatcherCreateVertexProgram(
            shaderPatcher,
            clearVertexProgramId,
            clearVertexAttributes,
            1,
            clearVertexStreams,
            1,
            &clearVertexProgram);

    DEBUG_PRINT("clear sceGxmShaderPatcherCreateVertexProgram(): 0x%08X\n", err);

    err = sceGxmShaderPatcherCreateFragmentProgram(
            shaderPatcher,
            clearFragmentProgramId,
            SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
            MSAA_MODE,
            NULL,
            clearVertexProgramGxp,
            &clearFragmentProgram);

    DEBUG_PRINT("clear sceGxmShaderPatcherCreateFragmentProgram(): 0x%08X\n", err);

    // create the clear triangle vertex/index data
    clearVertices = (vita2d_clear_vertex *) gpu_alloc(
            SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
            3 * sizeof(vita2d_clear_vertex),
            4,
            SCE_GXM_MEMORY_ATTRIB_READ,
            &clearVerticesUid);

    clearIndices = (uint16_t *) gpu_alloc(
            SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
            3 * sizeof(uint16_t),
            2,
            SCE_GXM_MEMORY_ATTRIB_READ,
            &clearIndicesUid);

    clearVertices[0].x = -1.0f;
    clearVertices[0].y = -1.0f;
    clearVertices[1].x = 3.0f;
    clearVertices[1].y = -1.0f;
    clearVertices[2].x = -1.0f;
    clearVertices[2].y = 3.0f;

    clearIndices[0] = 0;
    clearIndices[1] = 1;
    clearIndices[2] = 2;
#endif

    const SceGxmProgramParameter *paramColorPositionAttribute = sceGxmProgramFindParameterByName(colorVertexProgramGxp,
                                                                                                 "aPosition");
    DEBUG_PRINT("aPosition sceGxmProgramFindParameterByName(): %p\n", paramColorPositionAttribute);

    const SceGxmProgramParameter *paramColorColorAttribute = sceGxmProgramFindParameterByName(colorVertexProgramGxp,
                                                                                              "aColor");
    DEBUG_PRINT("aColor sceGxmProgramFindParameterByName(): %p\n", paramColorColorAttribute);

    // create color vertex format
    SceGxmVertexAttribute colorVertexAttributes[2];
    SceGxmVertexStream colorVertexStreams[1];
    /* x,y,z: 3 float 32 bits */
    colorVertexAttributes[0].streamIndex = 0;
    colorVertexAttributes[0].offset = 0;
    colorVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
    colorVertexAttributes[0].componentCount = 3; // (x, y, z)
    colorVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramColorPositionAttribute);
    /* color: 4 unsigned char  = 32 bits */
    colorVertexAttributes[1].streamIndex = 0;
    colorVertexAttributes[1].offset = 12; // (x, y, z) * 4 = 12 bytes
    colorVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_U8N;
    colorVertexAttributes[1].componentCount = 4; // (color)
    colorVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramColorColorAttribute);
    // 16 bit (short) indices
    colorVertexStreams[0].stride = sizeof(vita2d_color_vertex);
    colorVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

    // create color shaders
    err = sceGxmShaderPatcherCreateVertexProgram(
            shaderPatcher,
            colorVertexProgramId,
            colorVertexAttributes,
            2,
            colorVertexStreams,
            1,
            &_vita2d_colorVertexProgram);

    DEBUG_PRINT("color sceGxmShaderPatcherCreateVertexProgram(): 0x%08X\n", err);

    err = sceGxmShaderPatcherCreateFragmentProgram(
            shaderPatcher,
            colorFragmentProgramId,
            SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
            MSAA_MODE,
            &blend_info,
            colorVertexProgramGxp,
            &_vita2d_colorFragmentProgram);

    DEBUG_PRINT("color sceGxmShaderPatcherCreateFragmentProgram(): 0x%08X\n", err);


    const SceGxmProgramParameter *paramTexturePositionAttribute = sceGxmProgramFindParameterByName(
            textureVertexProgramGxp, "aPosition");
    DEBUG_PRINT("aPosition sceGxmProgramFindParameterByName(): %p\n", paramTexturePositionAttribute);

    const SceGxmProgramParameter *paramTextureTexcoordAttribute = sceGxmProgramFindParameterByName(
            textureVertexProgramGxp, "aTexcoord");
    DEBUG_PRINT("aTexcoord sceGxmProgramFindParameterByName(): %p\n", paramTextureTexcoordAttribute);

    // create texture vertex format
    SceGxmVertexAttribute textureVertexAttributes[2];
    SceGxmVertexStream textureVertexStreams[1];
    /* x,y,z: 3 float 32 bits */
    textureVertexAttributes[0].streamIndex = 0;
    textureVertexAttributes[0].offset = 0;
    textureVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
    textureVertexAttributes[0].componentCount = 3; // (x, y, z)
    textureVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramTexturePositionAttribute);
    /* u,v: 2 floats 32 bits */
    textureVertexAttributes[1].streamIndex = 0;
    textureVertexAttributes[1].offset = 12; // (x, y, z) * 4 = 12 bytes
    textureVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
    textureVertexAttributes[1].componentCount = 2; // (u, v)
    textureVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramTextureTexcoordAttribute);
    // 16 bit (short) indices
    textureVertexStreams[0].stride = sizeof(vita2d_texture_vertex);
    textureVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

    // create texture shaders
    err = sceGxmShaderPatcherCreateVertexProgram(
            shaderPatcher,
            textureVertexProgramId,
            textureVertexAttributes,
            2,
            textureVertexStreams,
            1,
            &_vita2d_textureVertexProgram);

    DEBUG_PRINT("texture sceGxmShaderPatcherCreateVertexProgram(): 0x%08X\n", err);

    err = sceGxmShaderPatcherCreateFragmentProgram(
            shaderPatcher,
            textureFragmentProgramId,
            SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
            MSAA_MODE,
            &blend_info,
            textureVertexProgramGxp,
            &_vita2d_textureFragmentProgram);

    DEBUG_PRINT("texture sceGxmShaderPatcherCreateFragmentProgram(): 0x%08X\n", err);

    err = sceGxmShaderPatcherCreateFragmentProgram(
            shaderPatcher,
            textureTintFragmentProgramId,
            SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
            MSAA_MODE,
            &blend_info,
            textureVertexProgramGxp,
            &_vita2d_textureTintFragmentProgram);

    DEBUG_PRINT("texture_tint sceGxmShaderPatcherCreateFragmentProgram(): 0x%08X\n", err);

#ifdef USE_CLEAR_VERTEX
    // find vertex uniforms by name and cache parameter information
    _vita2d_clearClearColorParam = sceGxmProgramFindParameterByName(clearFragmentProgramGxp, "uClearColor");
    DEBUG_PRINT("_vita2d_clearClearColorParam sceGxmProgramFindParameterByName(): %p\n", _vita2d_clearClearColorParam);
#endif
    _vita2d_colorWvpParam = sceGxmProgramFindParameterByName(colorVertexProgramGxp, "wvp");
    DEBUG_PRINT("color wvp sceGxmProgramFindParameterByName(): %p\n", _vita2d_colorWvpParam);

    _vita2d_textureWvpParam = sceGxmProgramFindParameterByName(textureVertexProgramGxp, "wvp");
    DEBUG_PRINT("texture wvp sceGxmProgramFindParameterByName(): %p\n", _vita2d_textureWvpParam);

    _vita2d_textureTintColorParam = sceGxmProgramFindParameterByName(textureTintFragmentProgramGxp, "uTintColor");
    DEBUG_PRINT("texture wvp sceGxmProgramFindParameterByName(): %p\n", _vita2d_textureWvpParam);

    // Allocate memory for the memory pool
    pool_size = temp_pool_size;
    pool_addr = gpu_alloc(
            memType,
            pool_size,
            sizeof(void *),
            SCE_GXM_MEMORY_ATTRIB_READ,
            &poolUid);

    matrix_init_orthographic(_vita2d_ortho_matrix, 0.0f, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0.0f, 0.0f, 1.0f);

    vita2d_initialized = 1;
    return 1;
}

void vita2d_wait_rendering_done() {
    sceGxmFinish(_vita2d_context);
}

int vita2d_fini() {

    if (!vita2d_initialized) {
        DEBUG_PRINT("libvita2d is not initialized!\n");
        return 1;
    }

    // wait until rendering is done
    sceGxmFinish(_vita2d_context);

    // clean up allocations
#ifdef USE_CLEAR_VERTEX
    sceGxmShaderPatcherReleaseFragmentProgram(shaderPatcher, clearFragmentProgram);
    sceGxmShaderPatcherReleaseVertexProgram(shaderPatcher, clearVertexProgram);
#endif
    sceGxmShaderPatcherReleaseFragmentProgram(shaderPatcher, _vita2d_colorFragmentProgram);
    sceGxmShaderPatcherReleaseVertexProgram(shaderPatcher, _vita2d_colorVertexProgram);
    sceGxmShaderPatcherReleaseFragmentProgram(shaderPatcher, _vita2d_textureFragmentProgram);
    sceGxmShaderPatcherReleaseFragmentProgram(shaderPatcher, _vita2d_textureTintFragmentProgram);
    sceGxmShaderPatcherReleaseVertexProgram(shaderPatcher, _vita2d_textureVertexProgram);
#ifdef USE_CLEAR_VERTEX
    gpu_free(clearIndicesUid);
    gpu_free(clearVerticesUid);
#endif
    // wait until display queue is finished before deallocating display buffers
    sceGxmDisplayQueueFinish();

    // unregister programs and destroy shader patcher
#ifdef USE_CLEAR_VERTEX
    sceGxmShaderPatcherUnregisterProgram(shaderPatcher, clearFragmentProgramId);
    sceGxmShaderPatcherUnregisterProgram(shaderPatcher, clearVertexProgramId);
#endif
    sceGxmShaderPatcherUnregisterProgram(shaderPatcher, colorFragmentProgramId);
    sceGxmShaderPatcherUnregisterProgram(shaderPatcher, colorVertexProgramId);
    sceGxmShaderPatcherUnregisterProgram(shaderPatcher, textureFragmentProgramId);
    sceGxmShaderPatcherUnregisterProgram(shaderPatcher, textureTintFragmentProgramId);
    sceGxmShaderPatcherUnregisterProgram(shaderPatcher, textureVertexProgramId);

    gpu_free(poolUid);

    // terminate libgxm
    //sceGxmTerminate();

    /* if (pgf_module_was_loaded != SCE_SYSMODULE_LOADED)
        sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF); */

    vita2d_initialized = 0;

    return 1;
}

#ifdef USE_CLEAR_VERTEX
void vita2d_clear_screen() {
    // set clear shaders
    sceGxmSetVertexProgram(_vita2d_context, clearVertexProgram);
    sceGxmSetFragmentProgram(_vita2d_context, clearFragmentProgram);

    // set the clear color
    void *color_buffer;
    sceGxmReserveFragmentDefaultUniformBuffer(_vita2d_context, &color_buffer);
    sceGxmSetUniformDataF(color_buffer, _vita2d_clearClearColorParam, 0, 4, clear_color);

    // draw the clear triangle
    sceGxmSetVertexStream(_vita2d_context, 0, clearVertices);
    sceGxmDraw(_vita2d_context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, clearIndices, 3);
}
#endif

void vita2d_enable_clipping() {
    clipping_enabled = 1;
    vita2d_set_clip_rectangle(clip_rect_x_min, clip_rect_y_min, clip_rect_x_max, clip_rect_y_max);
}

void vita2d_disable_clipping() {
    clipping_enabled = 0;
    sceGxmSetFrontStencilFunc(
            _vita2d_context,
            SCE_GXM_STENCIL_FUNC_ALWAYS,
            SCE_GXM_STENCIL_OP_KEEP,
            SCE_GXM_STENCIL_OP_KEEP,
            SCE_GXM_STENCIL_OP_KEEP,
            0xFF,
            0xFF);
}

int vita2d_get_clipping_enabled() {
    return clipping_enabled;
}

void vita2d_set_clip_rectangle(int x_min, int y_min, int x_max, int y_max) {
    clip_rect_x_min = x_min;
    clip_rect_y_min = y_min;
    clip_rect_x_max = x_max;
    clip_rect_y_max = y_max;
    // we can only draw during a scene, but we can cache the values since they're not going to have any visible effect till the scene starts anyways
    if (drawing) {
        // clear the stencil buffer to 0
        sceGxmSetFrontStencilFunc(
                _vita2d_context,
                SCE_GXM_STENCIL_FUNC_NEVER,
                SCE_GXM_STENCIL_OP_ZERO,
                SCE_GXM_STENCIL_OP_ZERO,
                SCE_GXM_STENCIL_OP_ZERO,
                0xFF,
                0xFF);
        vita2d_draw_rectangle(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);
        // set the stencil to 1 in the desired region
        sceGxmSetFrontStencilFunc(
                _vita2d_context,
                SCE_GXM_STENCIL_FUNC_NEVER,
                SCE_GXM_STENCIL_OP_REPLACE,
                SCE_GXM_STENCIL_OP_REPLACE,
                SCE_GXM_STENCIL_OP_REPLACE,
                0xFF,
                0xFF);
        vita2d_draw_rectangle(x_min, y_min, x_max - x_min, y_max - y_min, 0);
        if (clipping_enabled) {
            // set the stencil function to only accept pixels where the stencil is 1
            sceGxmSetFrontStencilFunc(
                    _vita2d_context,
                    SCE_GXM_STENCIL_FUNC_EQUAL,
                    SCE_GXM_STENCIL_OP_KEEP,
                    SCE_GXM_STENCIL_OP_KEEP,
                    SCE_GXM_STENCIL_OP_KEEP,
                    0xFF,
                    0xFF);
        } else {
            sceGxmSetFrontStencilFunc(
                    _vita2d_context,
                    SCE_GXM_STENCIL_FUNC_ALWAYS,
                    SCE_GXM_STENCIL_OP_KEEP,
                    SCE_GXM_STENCIL_OP_KEEP,
                    SCE_GXM_STENCIL_OP_KEEP,
                    0xFF,
                    0xFF);
        }
    }
}

void vita2d_get_clip_rectangle(int *x_min, int *y_min, int *x_max, int *y_max) {
    *x_min = clip_rect_x_min;
    *y_min = clip_rect_y_min;
    *x_max = clip_rect_x_max;
    *y_max = clip_rect_y_max;
}

#ifdef USE_CLEAR_VERTEX
void vita2d_set_clear_color(unsigned int color) {
    clear_color[0] = ((color >> 8 * 0) & 0xFF) / 255.0f;
    clear_color[1] = ((color >> 8 * 1) & 0xFF) / 255.0f;
    clear_color[2] = ((color >> 8 * 2) & 0xFF) / 255.0f;
    clear_color[3] = ((color >> 8 * 3) & 0xFF) / 255.0f;
    clear_color_u = color;
}

unsigned int vita2d_get_clear_color() {
    return clear_color_u;
}
#endif

void vita2d_set_region_clip(SceGxmRegionClipMode mode, unsigned int x_min, unsigned int y_min, unsigned int x_max,
                            unsigned int y_max) {
    sceGxmSetRegionClip(_vita2d_context, mode, x_min, y_min, x_max, y_max);
}

void *vita2d_pool_malloc(unsigned int size) {
    if ((pool_index + size) < pool_size) {
        void *addr = (void *) ((unsigned int) pool_addr + pool_index);
        pool_index += size;
        return addr;
    }
    return NULL;
}

void *vita2d_pool_memalign(unsigned int size, unsigned int alignment) {
    unsigned int new_index = (pool_index + alignment - 1) & ~(alignment - 1);
    if ((new_index + size) < pool_size) {
        void *addr = (void *) ((unsigned int) pool_addr + new_index);
        pool_index = new_index + size;
        return addr;
    } else {
        // crap !
        vita2d_pool_reset();
        return vita2d_pool_memalign(size, alignment);
    }
    return NULL;
}

unsigned int vita2d_pool_free_space() {
    return pool_size - pool_index;
}

void vita2d_pool_reset() {
    pool_index = 0;
}
