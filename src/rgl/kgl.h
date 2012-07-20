#ifndef _kGL_H
#define _kGL_H

typedef unsigned int GLbitfield;
typedef unsigned char GLubyte;
typedef char GLbyte;
typedef unsigned char GLboolean;
typedef int GLint;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef unsigned short GLushort;
typedef short GLshort;

typedef int GLfixed;

#define LINES_DISABLEABLE 0

#define DEPTH_BITS 32

#if DEPTH_BITS == 16
typedef unsigned short GLdepth;
#elif DEPTH_BITS == 32
typedef unsigned int GLdepth;
#else
illegal number of depth bits
#endif

#if DEPTH_BITS == 16
#define MAX_DEPTH 0xffff
#define DEPTH_SCALE 65535.0f
#else
#define MAX_DEPTH 0x00ffffff
#define DEPTH_SCALE ((GLfloat)0x00ffffff)
#endif

#define MAX_ILLUM_LEVELS 16
#define ILLUM_SHIFT 4
#define ILLUM_SHIFT_DOWN (8-ILLUM_SHIFT)

extern GLint g_DepthMask;

#define Z_ADDRESS(CTX, X, Y) \
    (CTX->DepthBuffer + zMult[Y] + (X))

#define CTX_Z_ADDRESS(CTX, X, Y) \
    (CTX->DepthBuffer + CTX->zMult[Y] + (X))

#include "fixed.h"
#include "hash.h"

#define MATRIX_IDENTITY     1
#define MATRIX_2D_NO_ROT    2
#define MATRIX_2D           3
#define MATRIX_3D           4
#define MATRIX_GENERAL      5
#define MATRIX_ORTHO        6
#define MATRIX_PERSPECTIVE  7

#include "kgl_macros.h"

typedef unsigned int GLenum;

#include "gldefines.h"

#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL 0x803A
#endif

#ifndef GL_SHARED_TEXTURE_PALETTE_EXT
#define GL_SHARED_TEXTURE_PALETTE_EXT 0x81FB
#endif

#ifndef GL_LIT_TEXTURE_PALETTE_EXT
#define GL_LIT_TEXTURE_PALETTE_EXT 0x82FB
#endif

#define AUX_RGB     0
#define AUX_DEPTH   256
#define AUX_DOUBLE  2

#ifndef GL_COLOR_BUFFER_BIT
#define GL_COLOR_BUFFER_BIT	1
#define GL_DEPTH_BUFFER_BIT	2
#endif

#define CLIP_FCOLOR_BIT		0x1
#define CLIP_BCOLOR_BIT		0x2
#define CLIP_COLOR_BITS		0x3
#define CLIP_TEXTURE_BIT	0x4

#define FRONT_AMBIENT_BIT   0x1
#define BACK_AMBIENT_BIT    0x2
#define FRONT_DIFFUSE_BIT   0x4
#define BACK_DIFFUSE_BIT    0x8
#define FRONT_SPECULAR_BIT  0x10
#define BACK_SPECULAR_BIT   0x20
#define FRONT_SHININESS_BIT 0x40
#define BACK_SHININESS_BIT  0x80

#define FRONT_MATERIAL_BITS (FRONT_AMBIENT_BIT | FRONT_DIFFUSE_BIT | FRONT_SPECULAR_BIT | FRONT_SHININESS_BIT)
#define BACK_MATERIAL_BITS  (BACK_AMBIENT_BIT | BACK_DIFFUSE_BIT | BACK_SPECULAR_BIT | BACK_SHININESS_BIT)
#define ALL_MATERIAL_BITS   (FRONT_MATERIAL_BITS | BACK_MATERIAL_BITS)

#define MAX_LIGHTS		3

#define MAX_WIDTH		1600
#define MAX_HEIGHT		1200

#define MAX_MODELVIEW_STACK_DEPTH	16
#define MAX_PROJECTION_STACK_DEPTH	8
#define MAX_ATTRIB_STACK_DEPTH      16

#define NEW_LIGHTING        0x1
#define NEW_MODELVIEW       0x2
#define NEW_PROJECTION      0x4
#define NEW_MODELVIEWINV    0x8
#define NEW_RASTER          0x10
#define NEW_ALL             0x1f

typedef struct device_s
{
    GLboolean fastAlpha;
    GLboolean fastBlend;
    GLboolean litPalette;
    char* name;
    char* aliases;
} device_t;

extern GLint activeDevice;
extern GLint nDevices;
extern device_t* devices;

typedef struct gl_texture_object_s
{
    GLuint  Name;
    GLfloat Priority;
    GLenum  WrapS;      //GL_CLAMP or GL_REPEAT
    GLenum  WrapT;
    GLenum  Min, Mag;
    GLboolean created;

    GLenum  Format;     //GL_RGB
    GLuint  Width;      //Width
    GLuint  Height;     //Height
    GLuint  WidthLog2;  //log2(Width2)
    GLuint  HeightLog2; //log2(Height2)
    GLuint  WidthMask;  //Width-1 iff Width is power of 2
    GLuint  HeightMask; //Height-1 iff Height is power of 2
    GLuint  MaxLog2;    //MAX(WidthLog2, HeightLog2)
    GLubyte *Data;
    GLubyte *Data2;

    GLvoid  *DriverData;    //optional Driver-specific data

    GLubyte* Palette;
} gl_texture_object;

/* texture object hashtable */
extern hashtable* _texobjs;

typedef enum
{
    GL_RGB888,
    GL_BGR888,
    GL_RGB555,
    GL_RGB565,
    GL_RGB32,
    GL_BGR32,
    GL_RGBUNKNOWN
} gl_pixel_type;

typedef struct gl_framebuffer_s
{
    GLuint Width, Height;
    GLuint Pitch;           //or use scrMultByte
    GLuint Depth;           //bits per pixel: 16, 24, or 32.  15 == 16
    GLuint ByteMult;        //Depth / 8
    GLfloat rscale, gscale, bscale, ascale;
    GLubyte maxr, maxg, maxb, maxa;
    gl_pixel_type PixelType;
} gl_framebuffer;

typedef struct gl_material_s
{
    GLfloat Ambient[4];
    GLfloat Diffuse[4];
    GLfloat Specular[4];
    GLfloat Shininess;
} gl_material;

typedef struct gl_light_s
{
    GLfloat Ambient[4];
    GLfloat Diffuse[4];
    GLfloat Specular[4];
    GLfloat Position[4];
    GLfloat ConstantAttenuation;
    GLfloat LinearAttenuation;
    GLfloat QuadraticAttenuation;
    GLboolean Enabled;

    GLfloat OPos[4];

    GLfloat VP_inf_norm[3];
    GLfloat MatAmbient[2][3];
    GLfloat MatDiffuse[2][3];
    GLfloat MatSpecular[2][3];

    struct gl_light_s* next;
} gl_light;

typedef struct gl_viewport_s
{
    GLint X, Y;
    GLsizei Width, Height;
    GLfloat Near, Far;
    GLfloat Sx, Sy, Sz;
    GLfloat Tx, Ty, Tz;
} gl_viewport;

typedef struct gl_current_s
{
    GLubyte Color[4];
    GLfloat RasterColor[4];
    GLfloat RasterPos[4];
    GLfloat Normal[3];
    GLfloat TexCoord[2];
    GLubyte* Bitmap;
} gl_current;

typedef struct gl_attrib_s
{
    GLenum  type;
    union
    {
        GLfloat fvalue;
        GLint   ivalue;
    } val;
} gl_attrib;

typedef struct gl_driver_funcs_s
{
    /* some of these may be NULL, so check before using */

    //also allocates DriverCtx
    //called during gl_init_context, always.
    //MUST NOT DEPEND on any values in the context, but is expected
    //to determine whether the underlying hardware has appropriate
    //capabilities and return TRUE or FALSE depending
    //init_driver(ctx)
    GLboolean (*init_driver)();
    //post_init_driver(ctx)
    //MAY DEPEND on values in the context
    GLboolean (*post_init_driver)();
    //shutdown_driver(ctx)
    void (*shutdown_driver)();

    //GLboolean create_window(GLint, GLint)
    GLboolean (*create_window)();
    //delete_window(GLint)
    void (*delete_window)();
    //set_save_state(ctx, GLint)
    void (*set_save_state)();
    //GLubyte* get_scratch(ctx)
    GLubyte* (*get_scratch)();

    //driver_caps(ctx)
    //NOTE: this is a required fn, and is expected to fill in
    //Buffer.Pitch, Depth, and PixelType
    void (*driver_caps)();

    //allocate_depthbuffer(ctx)
    void (*allocate_depthbuffer)();
    //allocate_colorbuffer(ctx)
    void (*allocate_colorbuffer)();

    //clear_depthbuffer(ctx)
    void (*clear_depthbuffer)();
    //clear_colorbuffer(ctx)
    void (*clear_colorbuffer)();
    //clear_both_buffers(ctx)
    void (*clear_both_buffers)();

    //lock_buffer(ctx)
    void (*lock_buffer)();
    //unlock_buffer(ctx)
    void (*unlock_buffer)();
    //GLubyte* get_framebuffer(ctx)
    GLubyte* (*get_framebuffer)();

    /* a setup fn should at least save the given ctx */

    //setup_triangle(ctx)
    void (*setup_triangle)();
    //setup_line(ctx)
    void (*setup_line)();
    //setup_point(ctx)
    void (*setup_point)();
    //setup_raster(ctx)
    void (*setup_raster)();

    //set_monocolor(GLcontext* ctx, GLint r, g, b, a);
    void (*set_monocolor)();
    //flush()
    void (*flush)();
    //clear_color(GLubyte r, g, b, a)
    void (*clear_color)();
    //scissor(GLint x, y, GLsizei width, height)
    void (*scissor)();

    /* notice that draw_* fns aren't passed a context */

    //flush_batch(void)
    void (*flush_batch)();

    //draw_triangle(vl[], pv);
    void (*draw_triangle)();
    //draw_triangle_array(n, vl[], pv);
    void (*draw_triangle_array)();
    //draw_quad(vl[], pv);
    void (*draw_quad)();
    //draw_triangle_fan(n, vl[], pv);
    void (*draw_triangle_fan)();
    //draw_triangle_strip(n, vl[], pv);
    void (*draw_triangle_strip)();
    //draw_polygon(n, vl[], pv);
    void (*draw_polygon)();
    //draw_line(v0, v1, pv);
    void (*draw_line)();
    //framebuffer-level pixel routine w/ bounds checking
    //size==1 only; the GL will call multiple times as necessary
    //draw_pixel(GLint x, GLint y, GLdepth z);
    void (*draw_pixel)();
    //expected to set the raster color itself
    //draw_bitmap(
    //  GLcontext* ctx,
    //  GLsizei width, height, GLfloat xb0, yb0, xb1, yb1,
    //  GLubyte const* bitmap)
    void (*draw_bitmap)();
    //draw_pixels(
    //  GLcontext* ctx,
    //  GLsizei width, height,
    //  GLenum format, type
    void (*draw_pixels)();
    //read_pixels(
    //  GLcontext* ctx,
    //  GLint x, y,
    //  GLsizei width, height,
    //  GLenum format, type
    void (*read_pixels)();
    //expects every point to have a different color.
    //should handle size >= 1
    //draw_point(first, last)
    void (*draw_point)();

    //draw_triangle_elements(GLsizei count, GLsizei numVerts, GLvoid const* indices)
    void (*draw_triangle_elements)(GLsizei, GLsizei, GLvoid const*);

    //draw_clipped_triangle(vl[], pv)
    void (*draw_clipped_triangle)();
    //draw_clipped_polygon(n, vl[], pv)
    void (*draw_clipped_polygon)();

    //bind_texture()
    void (*bind_texture)();
    //tex_param(GLenum pname, GLfloat const* params)
    void (*tex_param)();
    //tex_del(*tex)
    void (*tex_del)();
    //tex_palette(gl_texture_object* tex)
    void (*tex_palette)();
    //tex_img(*tex, level, internalFormat)
    void (*tex_img)();
    //tex_env(GLenum param)
    void (*tex_env)();

    //deactivate()
    void (*deactivate)();
    //activate()
    void (*activate)();

    //screenshot(GLubyte*)
    void (*screenshot)();

    //gamma_up()
    void (*gamma_up)();
    //gamma_dn()
    void (*gamma_dn)();

    //chromakey(GLubyte r, GLubyte g, GLubyte b, GLboolean on)
    void (*chromakey)();
    //super_clear()
    void (*super_clear)();

    //fastbind_set(GLboolean state)
    void (*fastbind_set)();

    //draw_background(GLubyte* pixels)
    void (*draw_background)();

    //fog_vertices(GLcontext* ctx)
    void (*fog_vertices)();

    /* for drivers that perform transformation */

    //begin(GLcontext* ctx, GLenum primitive)
    void (*begin)();
    //end(GLcontext* ctx)
    void (*end)();

    //vertex(GLfloat x, y, z)
    void (*vertex)(GLfloat, GLfloat, GLfloat);

    void (*update_modelview)();
    void (*update_projection)();

    void (*draw_c4ub_v3f)(GLenum, GLint, GLsizei);

    //int feature_exists(GLint)
    int (*feature_exists)(GLint);

    void (*fullscene)(GLboolean);

    //GLubyte* get_animaticbuffer(GLint* pitch)
    GLubyte* (*get_animaticbuffer)(GLint*);

    //rglDrawPitchedPixels
    void (*draw_pitched_pixels)(GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLvoid const*);
} gl_driver_funcs;

#include "kvb.h"
#include "rglext.h"

typedef struct gl_context_s
{
    gl_framebuffer Buffer;

    vertex_buffer* VB;

    GLenum Primitive;

    GLubyte	ClipMask;

    GLubyte NewMask;

    gl_current	Current;		/* current Color, Normal for vertex processing */

    GLfloat ModelViewInv[16];
    GLfloat ModelViewMatrix[16];
    GLfloat ModelViewStack[MAX_MODELVIEW_STACK_DEPTH][16];
    GLuint  ModelViewStackDepth;
    GLint   ModelViewMatrixType;

    GLfloat ProjectionMatrix[16];
    GLfloat ProjectionStack[MAX_PROJECTION_STACK_DEPTH][16];
    GLuint  ProjectionStackDepth;
    GLint   ProjectionMatrixType;

    GLenum  MatrixMode;			/* GL_MODELVIEW, GL_PROJECTION */

    GLboolean   ScaleDepthValues;
    GLboolean   RasterizeOnly;

    GLboolean   UserClip;
    GLboolean   ClipEnabled[MAX_CLIP_PLANES];
    GLfloat     ClipEquation[MAX_CLIP_PLANES][4];

    GLboolean	CullFace;
    GLboolean	Normalize;
    GLboolean   RescaleNormal;
    GLboolean	Lighting;
    GLboolean	Blend;
    GLboolean	DepthTest;
    GLboolean   LineStipple;
    GLboolean   LineSmooth;
    GLboolean   PointSmooth;
    GLboolean   ScissorTest;
    GLboolean   Bias;
    GLboolean   AlphaTest;
    GLboolean   Fog;
    GLboolean   TwoSide;
    GLboolean   PolygonStipple;
    GLboolean   PerspectiveCorrect;

    GLint       FogMode;
    GLfloat     FogDensity;
    GLfloat     FogColor[4];

    GLboolean   SpecularRender;
    GLfloat     SpecularDefault[3];
    GLfloat     SpecularExponent[3];

    GLfloat     FloatBias[3];
    GLubyte     ByteBias[3];

    GLint       ScissorX, ScissorY;
    GLsizei     ScissorWidth, ScissorHeight;

    GLuint      CpuType;
    GLboolean   CpuMMX;
    GLboolean   CpuKatmai;

    /* lighting */
    GLenum	    ShadeModel;		/* GL_FLAT, GL_SMOOTH */
    GLfloat	    Ambient[4];
    gl_light	Light[MAX_LIGHTS];
    gl_light*   ActiveLight;
    gl_material Material[2];    /* 0=front, 1=back */
    GLboolean   FastLighting;
    GLfloat     BaseColor[2][4];

    /* textures */
    GLboolean   TexEnabled;
    GLenum      TexEnvMode;     /* GL_MODULATE, GL_REPLACE */
    gl_texture_object* TexBoundObject;
    GLboolean   UsingSharedPalette;
    GLboolean   UsingLitPalette;
    GLubyte*    SharedPalette;
    GLushort    SharedPalette16[256];
    GLushort    SharedPalette444[256];
    GLushort*   SharedIllumPalettes;

    /* depthbuffer */
    GLdepth*	DepthBuffer;
    GLenum	    DepthFunc;		/* GL_LESS only */
    GLboolean   DepthWrite;     /* default GL_TRUE */

    /* rgb framebuffer */
    GLubyte*	FrontFrameBuffer;
    GLubyte*	BackFrameBuffer;
    GLubyte*	FrameBuffer;		/* {Front,Back}FrameBuffer */
    GLenum	    DrawBuffer, ReadBuffer;
    GLboolean   ColorWrite;     /* default GL_TRUE */

    /* viewport */
    gl_viewport	Viewport;

    /* polygon */
    GLenum	CullFaceMode;
    GLenum  PolygonMode;

    /* color */
    GLenum	BlendSrc;
    GLenum	BlendDst;
    GLfloat	ClearColor[4];
    GLubyte ClearColorByte[4];
    GLfloat DepthClear;
    GLenum  AlphaFunc;
    GLubyte AlphaByteRef;

    /* points */
    GLfloat PointSize;

    /* lines */
    GLfloat  LineWidth;
    GLint    StippleFactor;
    GLushort StipplePattern;
    GLuint   StippleCounter;

    /* attrib stack */
    gl_attrib AttribStack[MAX_ATTRIB_STACK_DEPTH];
    GLuint  AttribStackDepth;

    GLfloat PlaneEq[4];

    GLvoid* VertexArray;
    GLsizei VertexSize;
    GLenum  VertexFormat;

    /* driver thingz */
    GLvoid* DriverCtx;              //device driver context, if needed
    gl_driver_funcs DriverFuncs;    //device driver jumptable
    GLint* scrMult;
    GLint* zMult;
    GLint* scrMultByte;
    GLboolean RequireLocking;
    GLboolean DriverTransforms;

    GLboolean ExclusiveLock;        //don't ever un/lock while set

    GLboolean AmRendering;

    GLboolean EffectPoint;
    GLboolean SansDepth;
    GLboolean PointHack;

    GLfloat GammaAdjust;

    GLfloat LightingAdjust;

    /* use these */
    void* (*AllocFunc)();   //AllocFunc(GLint size)
    void (*FreeFunc)();     //FreeFunc(void*)

    GLint LineCount;
    GLboolean LinesEnabled;

    /* "speedy" feature */
    GLboolean Speedy;

    GLint D3DReference;
} gl_context;

typedef gl_context GLcontext;

void gl_update_modelview();
void gl_invert_modelview();
void gl_update_projection();
void gl_update_lighting(GLcontext*);
void gl_update_raster(GLcontext*);

DLL void API glFlush();

DLL void rauxInitDisplayMode(GLuint flags);
DLL int  rauxInitPosition(GLuint x, GLuint y, GLuint width, GLuint height, GLuint depth);

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

DLL void rglDDrawActivate(unsigned char);

typedef void* (*MemAllocFunc)(GLint len, char* name, GLuint flags);
typedef GLint (*MemFreeFunc)(void* pointer);
DLL gl_texture_object* rglGetTexobj(GLuint name);
DLL GLuint rglGetMaxTexobj(void);
DLL hashtable* rglGetTexobjs(void);
DLL void rglAnotherPoly(void);

DLL GLubyte* API glGetString(GLenum cap);

DLL void API glLoadMatrixf(GLfloat const* m);
DLL void API glScalef(GLfloat x, GLfloat y, GLfloat z);

DLL void API glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
DLL void API glMatrixMode(GLenum mode);
DLL void API glLoadIdentity();
DLL void API glEnable(GLenum cap);
DLL void API glDisable(GLenum cap);
DLL GLboolean API glIsEnabled(GLenum cap);
DLL void API glCullFace(GLenum mode);
DLL void API glShadeModel(GLenum mode);
DLL void API glDepthFunc(GLenum func);
DLL void API glBlendFunc(GLenum sfactor, GLenum dfactor);
DLL void API glAlphaFunc(GLenum func, GLclampf ref);
DLL void API glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
DLL void API glClearDepth(GLdouble d);
DLL void API glClear(GLbitfield mask);
DLL void API glPushMatrix();
DLL void API glPopMatrix();
DLL void API glMultMatrixf(GLfloat const* m);
DLL void API glMultMatrixd(GLdouble const* m);
DLL void API glTranslatef(GLfloat x, GLfloat y, GLfloat z);
DLL void API glTranslated(GLdouble x, GLdouble y, GLdouble z);
DLL void API glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
DLL void API glLightfv(GLenum light, GLenum pname, GLfloat const* params);
DLL void API glInterleavedArrays(GLenum format, GLsizei stride, GLvoid const* pointer);
DLL void API glVertexPointer(GLint size, GLenum type, GLsizei stride, GLvoid const* pointer);
DLL void API glArrayElement(GLint i);
DLL void API glEnableClientState(GLenum array);
DLL void API glDisableClientState(GLenum array);
DLL void API glDrawElements(GLenum mode, GLsizei count, GLenum type, GLvoid const* indices);
DLL void API glFrustum(
           GLdouble left, GLdouble right,
	       GLdouble bottom, GLdouble top,
	       GLdouble zNear, GLdouble zFar);
DLL void API glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                 GLdouble nearval, GLdouble farval);
DLL void API glDepthRange(GLclampd nearval, GLclampd farval);
DLL void API glDepthMask(GLboolean flag);
DLL void API glColorMask(GLboolean red, GLboolean green, GLboolean blue);
DLL void API glMaterialfv(GLenum face, GLenum pname, GLfloat const* params);

DLL void API glBegin(GLenum p);
DLL void API glEnd();
DLL void API glVertex3f(GLfloat x, GLfloat y, GLfloat z);
DLL void API glVertex3fv(GLfloat const* v);
DLL void API glVertex2f(GLfloat x, GLfloat y);
DLL void API glVertex2i(GLint x, GLint y);
DLL void API glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
DLL void API glNormal3fv(GLfloat* n);
DLL void API glColor3f(GLfloat r, GLfloat g, GLfloat b);
DLL void API glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
DLL void API glColor3ub(GLubyte r, GLubyte g, GLubyte b);
DLL void API glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a);

DLL void API glTexCoord2f(GLfloat s, GLfloat t);
DLL void API glGenTextures(GLsizei n, GLuint* textureNames);
DLL void API glBindTexture(GLenum target, GLuint textureName);
DLL void API glTexParameteri(GLenum target, GLenum pname, GLenum param);
DLL void API glTexImage2D(GLenum target, GLint level, GLint internalFormat,
                      GLsizei width, GLsizei height, GLint border,
                      GLenum format, GLenum type,
                      GLvoid const* pixels);
DLL void API glTexEnvi(GLenum target, GLenum pname, GLenum param);

DLL void API glGetFloatv(GLenum pname, GLfloat* param);
DLL void API glGetBooleanv(GLenum pname, GLboolean* param);
DLL void API glGetDoublev(GLenum pname, GLdouble* param);
DLL void API glGetIntegerv(GLenum pname, GLint* param);
DLL GLenum API glGetError();

DLL void API glRasterPos2f(GLfloat x, GLfloat y);
DLL void API glRasterPos2i(GLint x, GLint y);
DLL void API glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
DLL void API glBitmap(GLsizei width, GLsizei height,
                  GLfloat xb0, GLfloat yb0,
                  GLfloat xb1, GLfloat yb1,
                  GLubyte const* bitmap);

DLL void API glPushAttrib(GLenum attrib);
DLL void API glPopAttrib();
DLL void API glLineWidth(GLfloat width);
DLL void API glPointSize(GLfloat size);

DLL void API glLightModelf(GLenum pname, GLfloat param);
DLL void API glLightModelfv(GLenum pname, GLfloat const* params);
DLL void API glLightModeli(GLenum pname, GLint param);
DLL void API glLineStipple(GLint factor, GLushort pattern);
DLL void API glPixelStorei(GLenum pname, GLint param);
DLL void API glDeleteTextures(GLsizei n, GLuint const* textures);
DLL void API glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
DLL void API glDrawPixels(
    GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid const* pixels);

DLL void API glDrawArrays(GLenum mode, GLint first, GLsizei count);

DLL void API glColorTableEXT(
    GLenum target, GLenum internalformat, GLsizei length,
    GLenum format, GLenum type, GLvoid const* palette);
DLL void API glLitColorTableEXT(
    GLenum target, GLenum internalformat, GLsizei length,
    GLenum format, GLenum type, GLvoid const* palette);
DLL void API glColorTable(
    GLenum target, GLenum internalformat, GLsizei length,
    GLenum format, GLenum type, GLvoid const* palette);
DLL void API glLitColorTable(
    GLenum target, GLenum internalformat, GLsizei length,
    GLenum format, GLenum type, GLvoid const* palette);

DLL void API glPixelTransferf(GLenum pname, GLfloat param);

DLL void API glFogi(GLenum pname, GLint param);
DLL void API glFogf(GLenum pname, GLfloat param);
DLL void API glFogfv(GLenum pname, GLfloat const* params);

DLL void API glHint(GLenum target, GLenum mode);

DLL void API glPolygonMode(GLenum face, GLenum mode);

DLL void API glClipPlane(GLenum plane, GLdouble const* equation);
DLL void API glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels);

DLL GLuint glGetFrameCountEXT(void);
DLL GLboolean rglGetSkip(GLint y);

DLL void glSuperClear();

DLL char* glGluErrorString(GLenum err);
DLL char* gluErrorString(GLenum err);

DLL void gluPerspective(GLdouble fovy, GLdouble aspect,
		    GLdouble zNear, GLdouble zFar);
DLL void gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez,
               GLdouble centerx, GLdouble centery, GLdouble centerz,
               GLdouble upx, GLdouble upy, GLdouble upz);
DLL GLint gluProject(GLdouble objx, GLdouble objy, GLdouble objz,
                 GLdouble const* model, GLdouble const* proj,
                 GLint const* viewport,
                 GLdouble* winx, GLdouble* winy, GLdouble* winz);
DLL GLint gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
                   GLdouble const* model, GLdouble const* proj,
                   GLint const* viewport,
                   GLdouble* objx, GLdouble* objy, GLdouble* objz);

DLL void gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top);

DLL void glutInit(int* argc, char** argv);
DLL void glutInitDisplayString(char* s);
DLL void glutInitWindowSize(GLint width, GLint height);
DLL GLboolean glutCreateWindow(char* name);
DLL void glutDisplayFunc(void (*func)(void));
DLL void glutIdleFunc(void (*func)(void));
DLL void glutPostRedisplay();
DLL void glutMainLoop();
DLL void glutSwapBuffers();

void gl_init_context(GLcontext* ctx,
		     GLdepth* depthbuffer,
		     GLubyte* frontframebuffer,
		     GLubyte* backframebuffer);

DLL void gl_error(GLcontext* ctx, GLenum err, char* s);
DLL void gl_problem(GLcontext* ctx, char* s);

DLL int rglFeature(unsigned int feature);
DLL void rglSelectDevice(char* name, char* data);

DLL void rglSetRendererString(char*);
DLL void rglSetExtensionString(char*);
DLL GLboolean rglGetFullscreen(void);
DLL GLboolean rglGetTruecolor(void);
DLL GLboolean rglGetSlow(void);
DLL GLint rglCreate0(void);
DLL GLint rglCreate1(void);
DLL GLint rglCreate2(void);

DLL void  rglD3DSetDevice(char* dev);
DLL char* rglD3DGetDevice(void);

void gl_classify_modelview();
void gl_classify_projection();

extern GLint g_DepthMask;

DLL GLcontext* gl_get_context_ext();

DLL void API glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params);

#endif
