/*=============================================================================
    Name    : d3drv.h
    Purpose : Direct3D driver for rGL header file

    Created 9/24/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _D3DRV_H
#define _D3DRV_H

#define WIN32_LEAN_AND_MEAN
#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include <d3d.h>
#include <stdio.h>

extern "C" {
#include "kgl.h"
}

#define PAGE_FLIPPING           1
#define TEXTURE_TWIDDLING       0
#define TEXTURE_EVICTION        0
#define PREFER_CLAMPING         0
#define ONLY_GENERIC_BLITTERS   0
#define CONSERVE_MEMORY         1
#define LOG_ERRORS              0
#define LOG_DEVICE_CAPS         0
#define LOG_DISPLAY_MODES       0
#define LOG_SHUTDOWN            0
#define LOG_TEXTURE_FORMATS     0
#define IMPLICIT_BEGINS         1
#define INSCENE_CHECKING        0
#define WILL_ANTIALIAS          0
#define WILL_WBUFFER            0
#define ONLY_SHARED_PALETTES    0
#define EARLY_SHARED_ATTACHMENT 1
#define DEEP_DB                 0

#define CONSIDER_SQUARE_ASPECT  0 /* 0 means ignore wild square-only aspect discrepencies */
#define MAX_SQUARE_ASPECT       4 /* will this produce images too lo-res ? */

#define HAL_GUID IID_IDirect3DHALDevice
#define RGB_GUID IID_IDirect3DRGBDevice
#define REF_GUID IID_IDirect3DRefDevice

#define PRIMARY_GUID HAL_GUID

#if LOG_ERRORS
#define errLog errLogFn
#else
#define errLog errLogNull
#endif

#if LOG_SHUTDOWN
#define LOG logFn
#else
#define LOG logNull
#endif

#define RGBA_MAKE_ARRAY(C) RGBA_MAKE(C[0], C[1], C[2], C[3])
#define PAL_MAKE(r,g,b,a) (((a) << 24) | ((b) << 16) | ((g) << 8) | (r))

typedef struct d3d_context
{
    LPDIRECTDRAW             ddraw1;
    LPDIRECTDRAW4            ddraw4;
    LPDIRECTDRAWSURFACE4     PrimarySurface;
    LPDIRECTDRAWSURFACE4     BackSurface;
    LPDIRECTDRAWSURFACE4     DepthSurface;
    LPDIRECTDRAWGAMMACONTROL GammaControl;
    LPDIRECT3D3              d3dObject;
    LPDIRECT3DDEVICE3        d3dDevice;
    GUID                     d3dDeviceGUID;
    LPDIRECT3DVIEWPORT3      d3dViewport;
    RECT                     ScreenRect, ViewportRect;
    HWND                     hwnd;

    LPDIRECTDRAWPALETTE      sharedPalette;

    DDPIXELFORMAT* texCOLORINDEX;
    DDPIXELFORMAT* texRGBA16;
    DDPIXELFORMAT* texRGBA;
    DDPIXELFORMAT* texRGB;

    GLboolean   canRenderWindowed;
    GLboolean   Fullscreen;
    GLboolean   inScene;
	GLboolean   canGammaControl;
	DDGAMMARAMP awOldLUT;

    GLenum      Primitive;
    GLint       vertexMode;

    GLboolean   TexEnabled;
    GLboolean   DepthTest;
    GLenum      DepthFunc;
    GLboolean   DepthWrite;
    GLenum      ShadeModel;
    GLboolean   Blend;
    GLenum      BlendSrc, BlendDst;
    GLboolean   AlphaTest;
    GLenum      AlphaFunc;
    GLubyte     AlphaByteRef;
    GLboolean   LineSmooth;
    GLenum      CullFace;

    GLenum      texWrapS;
    GLenum      texWrapT;
    GLenum      texMinFilter;
    GLenum      texMagFilter;

    D3DTEXTUREOP colorOp;
    D3DTEXTUREOP alphaOp;

    D3DCOLOR    Monocolor;

    GLint       maxTexAspect;
    GLint       minTexWidth, minTexHeight;
    GLint       maxTexWidth, maxTexHeight;

    GLuint      canTexModulate;
    GLuint      canTexSelectArg1;
    GLuint      canTexAdd;

    GLuint      canAntialiasTriEdges;
    GLuint      canAntialiasTriDep;
    GLuint      canAntialiasTriIndep;
    GLuint      canAntialiasLineEdges;
    GLuint      canAntialiasLineDep;
    GLuint      canAntialiasLineIndep;
    GLuint      canDither;
    GLuint      canWBuffer;
    GLuint      canZBias;
    GLuint      canZCmpLess;
    GLuint      canZCmpLessEqual;
    GLboolean   canAlphaBlend;
    GLuint      canSrcBlendSrcAlpha;
    GLuint      canSrcBlendOne;
    GLuint      canSrcBlendZero;
    D3DBLEND    srcBlendFallback;
    GLuint      canDestBlendInvSrcAlpha;
    GLuint      canDestBlendOne;
    GLuint      canDestBlendZero;
    D3DBLEND    destBlendFallback;
    GLboolean   canAlphaTest;
    GLuint      canAlphaTestGreater;
    GLuint      canAlphaTestLess;
    D3DCMPFUNC  alphaTestFallback;
    GLuint      canPerspectiveCorrect;
    GLuint      squareOnly;
    GLuint      canFilterLinear;
    GLuint      canFilterNearest;
    GLuint      canClamp;
    GLuint      canWrap;
} d3d_context;

typedef struct d3d_texobj
{
    GLsizei              width, height;
    GLboolean            valid;
    GLboolean            paletted;
    LPDIRECTDRAWSURFACE4 texSurface;
    LPDIRECT3DTEXTURE2   texObj;

    GLenum      texWrapS;
    GLenum      texWrapT;
    GLenum      texMinFilter;
    GLenum      texMagFilter;
} d3d_texobj;

extern GLcontext* CTX;
extern d3d_context* D3D;

GLboolean d3d_begin_scene(d3d_context* d3d);
GLboolean d3d_end_scene(d3d_context* d3d);

void d3d_shutdown(GLcontext* ctx);

void texbind(gl_texture_object*);
void teximg(gl_texture_object*, GLint, GLint);
void texpalette(gl_texture_object*);
void texdel(gl_texture_object*);

void errLogFn(char* s, HRESULT hr);
void errLogNull(char* s, HRESULT hr);

void logFn(char* s);
void logNull(char* s);

WORD GetNumberOfBits(DWORD mask);
GLuint GetShiftBits(GLuint mask);

#endif
