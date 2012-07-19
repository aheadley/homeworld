/*=============================================================================
        Name    : glext.h
        Purpose : management of GL extensions

Created 06/07/1998 by khent
Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _GLEXT_H
#define _GLEXT_H

#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL 0x803A
#endif

#ifndef GL_SHARED_TEXTURE_PALETTE_EXT
#define GL_SHARED_TEXTURE_PALETTE_EXT 0x81FB
#endif

#ifndef GL_LIT_TEXTURE_PALETTE_EXT
#define GL_LIT_TEXTURE_PALETTE_EXT 0x82FB
#endif

#ifndef GL_CLIP_VOLUME_CLIPPING_HINT_EXT
#define GL_CLIP_VOLUME_CLIPPING_HINT_EXT 0x80F0
#endif

#ifndef RGL_ISA_BACKGROUND
#define RGL_ISA_BACKGROUND  0x1000
#define RGL_FULLSCREEN      0x1001
#define RGL_WINDOWED        0x1002
#define RGL_TRUECOLOR       0x1003
#define RGL_SLOWBLT         0x1004
#define RGL_NEXT_RENDERER   0x1005
#define RGL_ACCELERATED     0x1006
#define RGL_ACTIVATE        0x1007
#define RGL_DEACTIVATE      0x1008
#define RGL_LOCK            0x1009
#define RGL_UNLOCK          0x100A
#define RGL_FASTBLT         0x100B
#define RGL_HICOLOR         0x100C
#define RGL_REINIT_RENDERER 0x100D
#define RGL_SPEEDY          0x100E
#define RGL_SCREENSHOT      0x2000
#define RGL_MULTISHOT_START 0x2001
#define RGL_MULTISHOT_END   0x2002
#define RGL_EFFECTPOINT     0x3000
#define RGL_MAPPOINT        0x3001
#define RGL_SPECULAR_RENDER 0x4000
#define RGL_NORMAL_RENDER   0x4001
#define RGL_SPECULAR2_RENDER 0x4002
#define RGL_SPECULAR3_RENDER 0x4003
#define RGL_SANSDEPTH       0x5000
#define RGL_NORMDEPTH       0x5001
#define RGL_GAMMA_UP        0x6000
#define RGL_GAMMA_DN        0x6001
#define RGL_CHROMAKEY_ON    0x7000
#define RGL_CHROMAKEY_OFF   0x7001
#define RGL_FASTBIND_ON     0x8000
#define RGL_FASTBIND_OFF    0x8001
#define RGL_RESET_LINECOUNTER 0x9000
#define RGL_GET_LINECOUNTER   0x9001
#define RGL_RASTERIZE_ONLY  0x9002
#define RGL_BROKEN_MIXED_DEPTHTEST 0x9003
#define RGL_COLOROP_ADD     0x9004

#define RGL_FEATURE_ALPHA	0x3000
#define RGL_FEATURE_BLEND	0x3001
#define RGL_FEATURE_NOLINES 0x3010
#define RGL_FEATURE_LINES   0x3011

#define RGL_2D_QUADS        0x4500
#define RGL_RENDER_2D_QUAD  0x4501

#define RGL_SAVEBUFFER_ON   0x4600
#define RGL_SAVEBUFFER_OFF  0x4601

#define RGL_TEXTURE_LOG     0x4610

#define RGL_SHUTDOWN        0x4620

#define RGL_D3D_SHUFFLE     0x4630
#define RGL_D3D_FULLSCENE   0x4631

#define RGL_SKIP_RASTER     0x4640
#define RGL_NOSKIP_RASTER   0x4641
#endif //RGL_ISA_BACKGROUND

#ifndef RGL_VERTEX_LIST
#define RGL_VERTEX_LIST     1
#define RGL_NORMAL_LIST     2
#define RGL_POLY_LIST       3
#define RGL_SIZE            4
#define RGL_X               5
#define RGL_Y               6
#define RGL_Z               7
#define RGL_NORMAL          8
#define RGL_VERTICES        9
#define RGL_MATERIAL        10
#define RGL_TEXCOORDS       11
#endif //RGL_VERTEX_LIST

#define SWtype    0
#define GLIDEtype 1
#define D3Dtype   2
#define GLtype    3

typedef void* (*MemAllocFunc)(GLint len, char* name, GLuint flags);
typedef GLint (*MemFreeFunc)(void* pointer);

extern int (*rglFeature)(unsigned int feature);
extern void (*rglSpecExp)(GLint index, GLfloat exp);
extern void (*rglLightingAdjust)(GLfloat adj);
extern void (*rglSaveCursorUnder)(GLubyte* data, GLsizei width, GLsizei height, GLint x, GLint y);
extern void (*rglRestoreCursorUnder)(GLubyte* data, GLsizei width, GLsizei height, GLint x, GLint y);
extern unsigned char (*rglIsFast)(unsigned int feature);
extern unsigned char (*rglCreateWindow)(GLint ihwnd, GLint width, GLint depth);
extern void (*rglDeleteWindow)(GLint);
extern unsigned char (*rglIsClipped)(GLfloat*, GLfloat, GLfloat, GLfloat);
extern GLuint (*rglNumPolys)(void);
extern GLuint (*rglCulledPolys)(void);
extern void (*rglBackground)(GLubyte* pixels);
extern void (*rglSetAllocs)(MemAllocFunc allocFunc, MemFreeFunc freeFunc);
extern void (*rglSuperClear)(void);
extern void (*rglEnable)(GLint);
extern void (*rglDisable)(GLint);

extern void (*rglListSpec)(GLint pname, GLint param, GLint n, GLenum format);
extern void (*rglList)(GLint pname, GLvoid const* list);
extern void (*rglNormal)(GLint param);
extern void (*rglTriangle)(GLint iPoly);
extern void (*rglTexturedTriangle)(GLint iPoly);
extern void (*rglSmoothTriangle)(GLint iPoly);
extern void (*rglSmoothTexturedTriangle)(GLint iPoly);
extern void (*rglMeshRender)(GLint n, void (*callback)(GLint material), GLint* meshPolyMode);

extern void (*rglSelectDevice)(char* name, char* data);

extern void  (*rglD3DSetDevice)(char* dev);
extern char* (*rglD3DGetDevice)(void);

extern void* (*rglGetFramebuffer)(GLint* pitch);

extern void (*rglDrawPitchedPixels)(GLint x0, GLint y0,
                                    GLint x1, GLint y1,
                                    GLsizei width, GLsizei height, GLsizei pitch,
                                    GLvoid const* pixels);

#endif
