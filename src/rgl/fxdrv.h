#ifndef __FXDRV_H
#define __FXDRV_H

#include <glide.h>

//#define LOG(s) _fxLog(s)
#define LOG(s)

void _fxLog(char* s);

#define MAXNUM_MIPMAPLEVELS 1
#define FX_NUM_TMU 2

#define FX_TMU0 GR_TMU0
#define FX_TMU1 GR_TMU1

typedef void (*setup_func)(GLcontext*, GLuint, GLuint);

typedef struct texmemnode_s
{
    struct texmemnode_s* next;
    FxU32 startadr, endadr;
    gl_texture_object* tex;
} texmemnode;

typedef struct fx_context_s
{
    GrColor_t color;        //current monocolor
    GrColor_t clearc;       //framebuffer clear color
    GrAlpha_t cleara;       //framebuffer clear alpha
    GLint alphaColor;

    //wbuffer/texture w coord scalar
    GLfloat wscale;

    //glide-style verts
    GrVertex gwin[VB_SIZE];

    //vertex setup function, altered by choose_setup_function
    setup_func setup;

    //incremented at each glBindTexture for LRU algo
    GLuint texbind;

    //palette stuff
    GLubyte*    gl_palette[FX_NUM_TMU]; //last palette from the gl's context

    //texmem management
    texmemnode* tmalloc[FX_NUM_TMU];
    texmemnode* tmfree[FX_NUM_TMU];

    GLint       numTMUs;

    //state flags
    GLint ScissorX, ScissorY;
    GLsizei ScissorWidth, ScissorHeight;

    GLboolean bias;

    GLfloat fogDensity;
    GLfloat fogColor[4];

    GLboolean fastbind;

    // ----
    // previous state settings
    // ----

    GLint               scissor[4];
    GrAlphaBlendFnc_t   grAlphaBlendFunction[4];
    FxBool              grColorMask[2];
    GrCmpFnc_t          grAlphaTestFunction;
    FxBool              valid_grAlphaTestReferenceValue;
    GrAlpha_t           grAlphaTestReferenceValue;
    GrDepthBufferMode_t grDepthBufferMode;
    GrCmpFnc_t          grDepthBufferFunction;
    FxBool              grDepthMask;
    GrFogMode_t         grFogMode;
    GrChromakeyMode_t   grChromakeyMode;
    FxBool              valid_grChromakeyValue;
    GrColor_t           grChromakeyValue;
    FxBool              valid_grConstantColorValue;
    GrColor_t           grConstantColorValue;
    GrTextureClampMode_t s_grTexClampMode[FX_NUM_TMU];
    GrTextureClampMode_t t_grTexClampMode[FX_NUM_TMU];
    GrTextureFilterMode_t min_grTexFilterMode[FX_NUM_TMU];
    GrTextureFilterMode_t mag_grTexFilterMode[FX_NUM_TMU];
    GrMipMapMode_t      grTexMipMapMode[FX_NUM_TMU];
    FxBool              blend_grTexMipMapMode[FX_NUM_TMU];
    //grColorCombine
    GrCombineFunction_t function_grColorCombine;
    GrCombineFactor_t   factor_grColorCombine;
    GrCombineLocal_t    local_grColorCombine;
    GrCombineOther_t    other_grColorCombine;
    FxBool              invert_grColorCombine;
    //grAlphaCombine
    GrCombineFunction_t function_grAlphaCombine;
    GrCombineFactor_t   factor_grAlphaCombine;
    GrCombineLocal_t    local_grAlphaCombine;
    GrCombineOther_t    other_grAlphaCombine;
    FxBool              invert_grAlphaCombine;
    //grTexCombine
    GrCombineFunction_t rgb_function_grTexCombine[FX_NUM_TMU];
    GrCombineFactor_t   rgb_factor_grTexCombine[FX_NUM_TMU];
    GrCombineFunction_t alpha_function_grTexCombine[FX_NUM_TMU];
    GrCombineFactor_t   alpha_factor_grTexCombine[FX_NUM_TMU];
    FxBool              rgb_invert_grTexCombine[FX_NUM_TMU];
    FxBool              alpha_invert_grTexCombine[FX_NUM_TMU];
} fx_context;

typedef struct texmeminfo_s
{
    GLboolean   inmemory;
    GLuint      lastused;

    FxU32       whichTMU;

    texmemnode* tm[FX_NUM_TMU];

    GLushort*   mipmaplevel[MAXNUM_MIPMAPLEVELS];
} texmeminfo;

typedef struct fx_texobj_s
{
    GLboolean       valid;
    GLsizei         width, height;

    GrTexInfo       info;

    GrTextureFilterMode_t   minfilt, maxfilt;
    GrTextureClampMode_t    sclamp, tclamp;
    GrMipMapMode_t          mmmode;

    GLfloat         sscale, tscale;
    GLint           levelsdefined;

    GuTexPalette    palette;

    texmeminfo      tmi;
} fx_texobj;

FxU32 FXALIGN(FxU32 adr);
void tmInit(GLcontext*, fx_context*, FxU32 where);
void tmInitTexobjs(fx_context*);
void tmLoadTexture(fx_context*, gl_texture_object*, FxU32);
void tmReloadMipmap(fx_context*, gl_texture_object*, GLint, FxU32);
void tmUnloadTexture(fx_context*, gl_texture_object*);
void tmFreeTexture(fx_context*, gl_texture_object*);
void tmFreeFreeNodes(texmemnode*);
void tmFreeAllocNodes(texmemnode*);
void tmClose(fx_context*, FxU32 where);
int tmTexInfo(int w, int h, GrLOD_t* lodlevel, GrAspectRatio_t* ar,
              float* sscale, float* tscale, int* wscale, int* hscale);

#endif
