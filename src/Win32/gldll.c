/*=============================================================================
    Name    : gldll.c
    Purpose : alternative to linking against OpenGL32.dll

    Created 12/14/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <windows.h>
#include <stdio.h>
#include "gldll.h"
#include "glcaps.h"
#include "debug.h"
#include "devstats.h"
#include "main.h"

extern udword gDevcaps2;

#define WRAPPERS 0
#define BEGIN_WRAPPERS 0
#define TEXIMAGE_WRAPPERS 0
#define VERTEX_WRAPPERS 0
#define GENTEX_WRAPPERS 0
#define DELTEX_WRAPPERS 0

#define _ERROR_WRAP( func ) ((func) != 0) ? DynalinkFailed : (DynalinkFailed |= 1)
#define _STRINGIZE_NAME(name) #name
#define _STRINGIZE_NAME0(name) _STRINGIZE_NAME(name)
#define _WGL_STRINGIZE_NAME( name ) _STRINGIZE_NAME0(wgl##name)
#define _GL_STRINGIZE_NAME( name) _STRINGIZE_NAME0(gl##name)
#define _DGL_STRINGIZE_NAME( name ) _STRINGIZE_NAME0(_gl##name)
#define DYNALINK_GL_FUNCTION( name ) _ERROR_WRAP((*(int (__stdcall **)())&(gl##name)) = GetProcAddress(lib, _GL_STRINGIZE_NAME(name)))
#define DYNALINK_DGL_FUNCTION( name ) _ERROR_WRAP((*(int (__stdcall **)())&(_gl##name)) = GetProcAddress(lib, _GL_STRINGIZE_NAME(name)))
#define DYNALINK_WGL_FUNCTION( name) _ERROR_WRAP((*(int (__stdcall **)())&(rwgl##name)) = GetProcAddress(lib, _WGL_STRINGIZE_NAME(name)))

#if WRAPPERS
static GLuint lastBound = 0;
#endif

#if TEXIMAGE_WRAPPERS
#define NUM_WATCHES 2048
static int watchedIndex;
static GLuint watchedHandles[NUM_WATCHES];
#endif

HINSTANCE lib = NULL;

LOCKARRAYSEXTproc glLockArraysEXT;
UNLOCKARRAYSEXTproc glUnlockArraysEXT;

ALPHAFUNCproc glAlphaFunc;
BEGINproc glBegin;
BEGINproc _glBegin;
BINDTEXTUREproc glBindTexture;
BINDTEXTUREproc _glBindTexture;
BITMAPproc glBitmap;
BLENDFUNCproc glBlendFunc;
CLEARproc glClear;
CLEARCOLORproc glClearColor;
CLEARDEPTHproc glClearDepth;
CLEARINDEXproc glClearIndex;
COLOR3Fproc glColor3f;
COLOR3UBproc glColor3ub;
COLOR4Fproc glColor4f;
COLOR4UBproc glColor4ub;
COLORMASKproc glColorMask;
COLORTABLEproc glColorTable;
CULLFACEproc glCullFace;
DELETETEXTURESproc glDeleteTextures;
DELETETEXTURESproc _glDeleteTextures;
DEPTHFUNCproc glDepthFunc;
DEPTHMASKproc glDepthMask;
DEPTHRANGEproc glDepthRange;
DISABLEproc glDisable;
DISABLEproc _glDisable;
DRAWARRAYSproc glDrawArrays;
DRAWELEMENTSproc glDrawElements;
DRAWBUFFERproc glDrawBuffer;
DRAWPIXELSproc glDrawPixels;
ENABLEproc glEnable;
ENABLEproc _glEnable;
ENDproc glEnd;
ENDproc _glEnd;
EVALCOORD1Fproc glEvalCoord1f;
EVALCOORD2Fproc glEvalCoord2f;
EVALMESH1proc glEvalMesh1;
EVALMESH2proc glEvalMesh2;
EVALPOINT1proc glEvalPoint1;
EVALPOINT2proc glEvalPoint2;
FLUSHproc glFlush;
FOGFproc glFogf;
FOGFVproc glFogfv;
FOGIproc glFogi;
FRUSTUMproc glFrustum;
GENTEXTURESproc glGenTextures;
GENTEXTURESproc _glGenTextures;
GETDOUBLEVproc glGetDoublev;
GETERRORproc glGetError;
GETFLOATVproc glGetFloatv;
GETINTEGERVproc glGetIntegerv;
GETBOOLEANVproc glGetBooleanv;
GETSTRINGproc glGetString;
GETTEXLEVELPARAMETERIVproc glGetTexLevelParameteriv;
HINTproc glHint;
INTERLEAVEDARRAYSproc glInterleavedArrays;
ISENABLEDproc glIsEnabled;
LIGHTMODELFproc glLightModelf;
LIGHTMODELFVproc glLightModelfv;
LIGHTMODELIproc glLightModeli;
LIGHTFVproc glLightfv;
LINESTIPPLEproc glLineStipple;
LINEWIDTHproc glLineWidth;
LOADIDENTITYproc glLoadIdentity;
LOADMATRIXFproc glLoadMatrixf;
MAP1Fproc glMap1f;
MAP2Fproc glMap2f;
MAPGRID1Fproc glMapGrid1f;
MAPGRID2Dproc glMapGrid2d;
MATERIALFVproc glMaterialfv;
MATRIXMODEproc glMatrixMode;
MULTMATRIXDproc glMultMatrixd;
MULTMATRIXFproc glMultMatrixf;
NORMAL3Fproc glNormal3f;
NORMAL3FVproc glNormal3fv;
ORTHOproc glOrtho;
PIXELSTOREIproc glPixelStorei;
PIXELTRANSFERFproc glPixelTransferf;
POINTSIZEproc glPointSize;
POLYGONMODEproc glPolygonMode;
POPATTRIBproc glPopAttrib;
POPMATRIXproc glPopMatrix;
PUSHATTRIBproc glPushAttrib;
PUSHMATRIXproc glPushMatrix;
RASTERPOS2Fproc glRasterPos2f;
RASTERPOS2Iproc glRasterPos2i;
RASTERPOS4Fproc glRasterPos4f;
READBUFFERproc glReadBuffer;
READPIXELSproc glReadPixels;
ROTATEFproc glRotatef;
SCALEFproc glScalef;
SCISSORproc glScissor;
SHADEMODELproc glShadeModel;
TEXCOORD2Fproc glTexCoord2f;
TEXENVIproc glTexEnvi;
TEXIMAGE1Dproc glTexImage1D;
TEXIMAGE2Dproc glTexImage2D;
TEXIMAGE2Dproc _glTexImage2D;
TEXPARAMETERIproc glTexParameteri;
TRANSLATEDproc glTranslated;
TRANSLATEFproc glTranslatef;
VERTEX2Fproc glVertex2f;
VERTEX2Iproc glVertex2i;
VERTEX3Fproc glVertex3f;
VERTEX3FVproc glVertex3fv;
VERTEX3FVproc _glVertex3fv;
VERTEX4FVproc glVertex4fv;
VIEWPORTproc glViewport;
CLIPPLANEproc glClipPlane;
GETTEXIMAGEproc glGetTexImage;
VERTEXPOINTERproc glVertexPointer;
ENABLECLIENTSTATEproc glEnableClientState;
DISABLECLIENTSTATEproc glDisableClientState;
ARRAYELEMENTproc glArrayElement;
WCREATECONTEXTproc rwglCreateContext;
WDELETECONTEXTproc rwglDeleteContext;
WGETPROCADDRESSproc rwglGetProcAddress;
WMAKECURRENTproc rwglMakeCurrent;
WCHOOSEPIXELFORMATproc rwglChoosePixelFormat;
WSETPIXELFORMATproc rwglSetPixelFormat;
WGETPIXELFORMATproc rwglGetPixelFormat;
WDESCRIBEPIXELFORMATproc rwglDescribePixelFormat;
WSWAPBUFFERSproc rwglSwapBuffers;

#if VERTEX_WRAPPERS
FILE* vout;
void __stdcall myVertex3fv(GLfloat const* v)
{
    static GLfloat last[3];
    static GLfloat current[3];
    if (v[0] > 735916800.0f ||
        v[0] < -735916800.0f)
    {
        __asm int 3 ;
        fprintf(vout, "* %10.5f %10.5f %10.5f\n", v[0], v[1], v[2]);
        current[0] = last[0];
        current[1] = last[1];
        current[2] = last[2];
    }
    else
    {
        fprintf(vout, "  %10.5f %10.5f %10.5f\n", v[0], v[1], v[2]);
        last[0] = current[0] = v[0];
        last[1] = current[1] = v[1];
        last[2] = current[2] = v[2];
    }
    _glVertex3fv(current);
}
#endif

#if WRAPPERS
void __stdcall myBindTexture(GLenum target, GLuint textureName)
{
    if (textureName == lastBound)
    {
        return;
    }
    _glBindTexture(target, textureName);
    lastBound = textureName;
}
#endif

#if GENTEX_WRAPPERS
static FILE* genOut = NULL;
#if 1
void __stdcall myGenTextures(GLsizei n, GLuint* textureNames)
{
    _glGenTextures(n, textureNames);
    glFlush();
}
#else
void __stdcall myGenTextures(GLsizei n, GLuint* textureNames)
{
    int i;

    _glGenTextures(n, textureNames);
    if (genOut == NULL)
    {
        genOut = fopen("gentex.txt", "wt");
        if (genOut == NULL)
        {
            return;
        }
    }
    fprintf(genOut, "gen %d:", n);
    for (i = 0; i < n; i++)
    {
        fprintf(genOut, " %u", textureNames[i]);
    }
    fprintf(genOut, "\n");
}
#endif //1
#endif //GENTEX_WRAPPERS

#if DELTEX_WRAPPERS
void __stdcall myDeleteTextures(GLsizei n, GLuint const* textures)
{
    _glDeleteTextures(n, textures);
    glFlush();
}
#endif

#if BEGIN_WRAPPERS
static GLenum currentPrimitive = GL_NONE;
void __stdcall myBegin(GLenum primitive)
{
    if (primitive == GL_LINES ||
        primitive == GL_LINE_STRIP ||
        primitive == GL_LINE_LOOP)
    {
        currentPrimitive = GL_POINTS;
        __asm int 3 ;
    }
    else
    {
        currentPrimitive = primitive;
    }
    _glBegin(currentPrimitive);
}
#endif

static int int3 = -1;

void glDLLToggleWatch(unsigned int on)
{
    int3 = (int)on;
}

#if TEXIMAGE_WRAPPERS
void __stdcall myTexImage2D(GLenum target, GLint level, GLint internalFormat,
                            GLsizei width, GLsizei height, GLint border,
                            GLenum format, GLenum type,
                            GLvoid const* pixels)
{
#if WRAPPERS
    int index;

    for (index = 0; index < NUM_WATCHES; index++)
    {
        if (lastBound == watchedHandles[index])
        {
            //we're modifying a texture we're watching
            if (int3)
            {
                __asm int 3 ;
            }
        }
    }
#endif
    _glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
}
#endif

void __stdcall noDitheredAlphaEnable(GLenum cap)
{
    if (cap == GL_BLEND)
    {
        _glDisable(GL_DITHER);
        _glEnable(GL_BLEND);
    }
    else
    {
        _glEnable(cap);
    }
}

void __stdcall noDitheredAlphaDisable(GLenum cap)
{
    if (cap == GL_BLEND)
    {
        _glEnable(GL_DITHER);
        _glDisable(GL_BLEND);
    }
    else
    {
        _glDisable(cap);
    }
}

void glDLLReset(void)
{
#if TEXIMAGE_WRAPPERS
    int index;

    watchedIndex = 0;
    for (index = 0; index < NUM_WATCHES; index++)
    {
        watchedHandles[index] = 0;
    }
#endif

#if WRAPPERS
    myBindTexture(GL_TEXTURE_2D, 0);
#endif

#if GENTEX_WRAPPERS
    if (genOut != NULL)
    {
        fclose(genOut);
        genOut = NULL;
    }
#endif
}

GLboolean glDLL3Dfx = GL_FALSE;

void glDLLGetGLCompat(void)
{
    int DynalinkFailed = 0;

    if (lib == NULL)
    {
        return;
    }

    DYNALINK_GL_FUNCTION(DrawPixels);
    DYNALINK_GL_FUNCTION(RasterPos2f);
    DYNALINK_GL_FUNCTION(Clear);
    DYNALINK_GL_FUNCTION(Color3ub);
    DYNALINK_DGL_FUNCTION(Enable);
    DYNALINK_DGL_FUNCTION(Disable);
    if (bitTest(gDevcaps2, DEVSTAT2_NO_DALPHA))
    {
        glEnable  = noDitheredAlphaEnable;
        glDisable = noDitheredAlphaDisable;
    }
    else
    {
        glEnable  = _glEnable;
        glDisable = _glDisable;
    }

    dbgAssert(!DynalinkFailed);
}

void glDLLWatch(GLuint handle)
{
#if TEXIMAGE_WRAPPERS
    watchedHandles[watchedIndex++] = handle;
#endif
}

extern bool mainAllow3DNow;

#define glDLLEnvironment() \
    _putenv("FX_GLIDE_NO_SPLASH=1");\
    if (!mainAllow3DNow)\
    {\
        _putenv("__GL_FORCE_K3D=0");\
    }\
    if (!mainAllowKatmai)\
    {\
        _putenv("__GL_FORCE_KNI=0");\
    }

GLboolean glDLLGetProcs(char* dllName)
{
    int DynalinkFailed = 0;

#if TEXIMAGE_WRAPPERS
    int index;

    watchedIndex = 0;
    for (index = 0; index < NUM_WATCHES; index++)
    {
        watchedHandles[index] = 0;
    }
#endif

    if (strstr(dllName, "3dfx") != NULL ||
        strstr(dllName, "3Dfx") != NULL ||
        strstr(dllName, "dll\\") != NULL)
    {
        glDLL3Dfx = GL_TRUE;
    }
    else
    {
        glDLL3Dfx = GL_FALSE;
    }

    if (!glNT && glDLL3Dfx)
    {
        glDLLEnvironment();
        //try installed 3dfx GL first
        lib = LoadLibrary("3dfxvgl.dll");
        if (lib == NULL)
        {
            //not there, try our own
            glDLLEnvironment();
            lib = LoadLibrary(dllName);
            if (lib == NULL)
            {
                return GL_FALSE;
            }
        }
    }
    else
    {
        glDLLEnvironment();
        //try loading the .DLL
        lib = LoadLibrary(dllName);
        if (lib == NULL)
        {
            return GL_FALSE;
        }
    }

#if VERTEX_WRAPPERS
    vout = fopen("vout.dat", "wt");
#endif

    DYNALINK_GL_FUNCTION(AlphaFunc);
    DYNALINK_DGL_FUNCTION(Begin);
#if BEGIN_WRAPPERS
    glBegin = myBegin;
#else
    glBegin = _glBegin;
#endif
    DYNALINK_DGL_FUNCTION(BindTexture);
#if WRAPPERS
    glBindTexture = myBindTexture;
#else
    glBindTexture = _glBindTexture;
#endif
    DYNALINK_GL_FUNCTION(Bitmap);
    DYNALINK_GL_FUNCTION(BlendFunc);
    DYNALINK_GL_FUNCTION(Clear);
    DYNALINK_GL_FUNCTION(ClearColor);
    DYNALINK_GL_FUNCTION(ClearDepth);
    DYNALINK_GL_FUNCTION(ClearIndex);
    DYNALINK_GL_FUNCTION(Color3f);
    DYNALINK_GL_FUNCTION(Color3ub);
    DYNALINK_GL_FUNCTION(Color4f);
    DYNALINK_GL_FUNCTION(Color4ub);
    DYNALINK_GL_FUNCTION(ColorMask);
    glColorTable = (COLORTABLEproc)GetProcAddress(lib, "glColorTableEXT");
    DYNALINK_GL_FUNCTION(CullFace);
    DYNALINK_DGL_FUNCTION(DeleteTextures);
#if DELTEX_WRAPPERS
    glDeleteTextures = myDeleteTextures;
#else
    glDeleteTextures = _glDeleteTextures;
#endif
    DYNALINK_GL_FUNCTION(DepthFunc);
    DYNALINK_GL_FUNCTION(DepthMask);
    DYNALINK_GL_FUNCTION(DepthRange);
    DYNALINK_DGL_FUNCTION(Disable);
    DYNALINK_DGL_FUNCTION(Enable);
    if (bitTest(gDevcaps2, DEVSTAT2_NO_DALPHA))
    {
        glDisable = noDitheredAlphaDisable;
        glEnable  = noDitheredAlphaEnable;
    }
    else
    {
        glDisable = _glDisable;
        glEnable  = _glEnable;
    }
    DYNALINK_GL_FUNCTION(DrawArrays);
    DYNALINK_GL_FUNCTION(DrawElements);
    DYNALINK_GL_FUNCTION(DrawBuffer);
    DYNALINK_GL_FUNCTION(DrawPixels);
    DYNALINK_GL_FUNCTION(End);
    DYNALINK_GL_FUNCTION(EvalCoord1f);
    DYNALINK_GL_FUNCTION(EvalCoord2f);
    DYNALINK_GL_FUNCTION(EvalMesh1);
    DYNALINK_GL_FUNCTION(EvalMesh2);
    DYNALINK_GL_FUNCTION(EvalPoint1);
    DYNALINK_GL_FUNCTION(EvalPoint2);
    DYNALINK_GL_FUNCTION(Flush);
    DYNALINK_GL_FUNCTION(Fogf);
    DYNALINK_GL_FUNCTION(Fogfv);
    DYNALINK_GL_FUNCTION(Fogi);
    DYNALINK_GL_FUNCTION(Frustum);
    DYNALINK_DGL_FUNCTION(GenTextures);
#if GENTEX_WRAPPERS
    glGenTextures = myGenTextures;
#else
    glGenTextures = _glGenTextures;
#endif
    DYNALINK_GL_FUNCTION(GetDoublev);
    DYNALINK_GL_FUNCTION(GetError);
    DYNALINK_GL_FUNCTION(GetFloatv);
    DYNALINK_GL_FUNCTION(GetIntegerv);
    DYNALINK_GL_FUNCTION(GetBooleanv);
    DYNALINK_GL_FUNCTION(GetString);
    DYNALINK_GL_FUNCTION(GetTexLevelParameteriv);
    DYNALINK_GL_FUNCTION(Hint);
    DYNALINK_GL_FUNCTION(InterleavedArrays);
    DYNALINK_GL_FUNCTION(IsEnabled);
    DYNALINK_GL_FUNCTION(LightModelf);
    DYNALINK_GL_FUNCTION(LightModelfv);
    DYNALINK_GL_FUNCTION(LightModeli);
    DYNALINK_GL_FUNCTION(Lightfv);
    DYNALINK_GL_FUNCTION(LineStipple);
    DYNALINK_GL_FUNCTION(LineWidth);
    DYNALINK_GL_FUNCTION(LoadIdentity);
    DYNALINK_GL_FUNCTION(LoadMatrixf);
    DYNALINK_GL_FUNCTION(Map1f);
    DYNALINK_GL_FUNCTION(Map2f);
    DYNALINK_GL_FUNCTION(MapGrid1f);
    DYNALINK_GL_FUNCTION(MapGrid2d);
    DYNALINK_GL_FUNCTION(Materialfv);
    DYNALINK_GL_FUNCTION(MatrixMode);
    DYNALINK_GL_FUNCTION(MultMatrixd);
    DYNALINK_GL_FUNCTION(MultMatrixf);
    DYNALINK_GL_FUNCTION(Normal3f);
    DYNALINK_GL_FUNCTION(Normal3fv);
    DYNALINK_GL_FUNCTION(Ortho);
    DYNALINK_GL_FUNCTION(PixelStorei);
    DYNALINK_GL_FUNCTION(PixelTransferf);
    DYNALINK_GL_FUNCTION(PointSize);
    DYNALINK_GL_FUNCTION(PolygonMode);
    DYNALINK_GL_FUNCTION(PopAttrib);
    DYNALINK_GL_FUNCTION(PopMatrix);
    DYNALINK_GL_FUNCTION(PushAttrib);
    DYNALINK_GL_FUNCTION(PushMatrix);
    DYNALINK_GL_FUNCTION(RasterPos2f);
    DYNALINK_GL_FUNCTION(RasterPos2i);
    DYNALINK_GL_FUNCTION(RasterPos4f);
    DYNALINK_GL_FUNCTION(ReadBuffer);
    DYNALINK_GL_FUNCTION(ReadPixels);
    DYNALINK_GL_FUNCTION(Rotatef);
    DYNALINK_GL_FUNCTION(Scalef);
    DYNALINK_GL_FUNCTION(Scissor);
    DYNALINK_GL_FUNCTION(ShadeModel);
    DYNALINK_GL_FUNCTION(TexCoord2f);
    DYNALINK_GL_FUNCTION(TexEnvi);
    DYNALINK_GL_FUNCTION(TexImage1D);
#if TEXIMAGE_WRAPPERS
    glTexImage2D = myTexImage2D;
    DYNALINK_DGL_FUNCTION(TexImage2D);
#else
    DYNALINK_GL_FUNCTION(TexImage2D);
#endif
    DYNALINK_GL_FUNCTION(TexParameteri);
    DYNALINK_GL_FUNCTION(Translated);
    DYNALINK_GL_FUNCTION(Translatef);
    DYNALINK_GL_FUNCTION(Vertex2f);
    DYNALINK_GL_FUNCTION(Vertex2i);
    DYNALINK_GL_FUNCTION(Vertex3f);
#if VERTEX_WRAPPERS
    glVertex3fv = myVertex3fv;
    DYNALINK_DGL_FUNCTION(Vertex3fv);
#else
    DYNALINK_GL_FUNCTION(Vertex3fv);
#endif
    DYNALINK_GL_FUNCTION(Vertex4fv);
    DYNALINK_GL_FUNCTION(Viewport);
    DYNALINK_GL_FUNCTION(ClipPlane);
    DYNALINK_GL_FUNCTION(GetTexImage);
    DYNALINK_GL_FUNCTION(VertexPointer);
    DYNALINK_GL_FUNCTION(EnableClientState);
    DYNALINK_GL_FUNCTION(DisableClientState);
    DYNALINK_GL_FUNCTION(ArrayElement);

    if (DynalinkFailed)
    {
        return GL_FALSE;
    }

    DYNALINK_WGL_FUNCTION(CreateContext);
    DYNALINK_WGL_FUNCTION(DeleteContext);
    DYNALINK_WGL_FUNCTION(GetProcAddress);
    DYNALINK_WGL_FUNCTION(MakeCurrent);
    DYNALINK_WGL_FUNCTION(ChoosePixelFormat);
    DYNALINK_WGL_FUNCTION(SetPixelFormat);
    DYNALINK_WGL_FUNCTION(GetPixelFormat);
    DYNALINK_WGL_FUNCTION(DescribePixelFormat);
    DYNALINK_WGL_FUNCTION(SwapBuffers);

    if (rwglSwapBuffers != NULL && DynalinkFailed)
    {
        return GL_FALSE;
    }

    DYNALINK_GL_FUNCTION(LockArraysEXT);
    DYNALINK_GL_FUNCTION(UnlockArraysEXT);

    return GL_TRUE;
}
