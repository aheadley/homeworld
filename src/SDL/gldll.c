/*=============================================================================
    Name    : gldll.c
    Purpose : alternative to linking against OpenGL32.dll

    Created 12/14/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _MACOSX
    #include <CoreServices/CoreServices.h>
#endif

#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gldll.h"
#include "glcaps.h"
#include "Debug.h"
#include "devstats.h"
#include "main.h"

#ifndef _MACOSX
    #include <dlfcn.h>
#endif

extern udword gDevcaps2;

#define WRAPPERS 0
#define BEGIN_WRAPPERS 0
#define TEXIMAGE_WRAPPERS 0
#define VERTEX_WRAPPERS 0
#define GENTEX_WRAPPERS 0
#define DELTEX_WRAPPERS 0

#ifdef _WIN32
#define GL_LIB_NAME( base )     (#base ".dll")
#else
#define GL_LIB_NAME( base )     ("lib" #base ".so")
#endif

#define _ERROR_WRAP( func ) ((func) != 0) ? DynalinkFailed : (DynalinkFailed |= 1)
#define _STRINGIZE_NAME(name) #name
#define _STRINGIZE_NAME0(name) _STRINGIZE_NAME(name)
#define _WGL_STRINGIZE_NAME( name ) _STRINGIZE_NAME0(wgl##name)
#define _GL_STRINGIZE_NAME( name) _STRINGIZE_NAME0(gl##name)
#define _DGL_STRINGIZE_NAME( name ) _STRINGIZE_NAME0(_gl##name)

#ifdef _MACOSX
    #define DYNALINK_GL_FUNCTION( name ) _ERROR_WRAP((*(int (APIENTRY **)())&(gl##name)) = osxGetProcAddress(_GL_STRINGIZE_NAME(name)))
    #define DYNALINK_DGL_FUNCTION( name ) _ERROR_WRAP((*(int (APIENTRY **)())&(_gl##name)) = osxGetProcAddress(_GL_STRINGIZE_NAME(name)))
    #define DYNALINK_WGL_FUNCTION( name ) _ERROR_WRAP((*(int (APIENTRY **)())&(rwgl##name)) = osxGetProcAddress(_WGL_STRINGIZE_NAME(name)))
#else
    #define DYNALINK_GL_FUNCTION( name ) _ERROR_WRAP((*(int (APIENTRY **)())&(gl##name)) = SDL_GL_GetProcAddress(_GL_STRINGIZE_NAME(name)))
    #define DYNALINK_DGL_FUNCTION( name ) _ERROR_WRAP((*(int (APIENTRY **)())&(_gl##name)) = SDL_GL_GetProcAddress(_GL_STRINGIZE_NAME(name)))
    #define DYNALINK_WGL_FUNCTION( name ) _ERROR_WRAP((*(int (APIENTRY **)())&(rwgl##name)) = SDL_GL_GetProcAddress(_WGL_STRINGIZE_NAME(name)))
#endif // _MACOSX

#if WRAPPERS
static GLuint lastBound = 0;
#endif

#if TEXIMAGE_WRAPPERS
#define NUM_WATCHES 2048
static int watchedIndex;
static GLuint watchedHandles[NUM_WATCHES];
#endif

/*
#ifdef _WIN32
HINSTANCE lib = NULL;
#else
int lib = NULL;
#endif
*/
bool bOpenGlLoaded = FALSE;

LOCKARRAYSEXTproc glLockArraysEXT;
UNLOCKARRAYSEXTproc glUnlockArraysEXT;

ALPHAFUNCproc glAlphaFunc;
BEGINproc glBegin;
BEGINproc _glBegin;
BINDTEXTUREproc glBindTexture;
BINDTEXTUREproc _glBindTexture;
BITMAPproc glBitmap;
BLENDFUNCproc glBlendFunc;
CLEARproc glClear = NULL;
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

WGETPROCADDRESSproc rwglGetProcAddress;
WSWAPBUFFERSproc rwglSwapBuffers;
#ifdef _WIN32
WCREATECONTEXTproc rwglCreateContext;
WDELETECONTEXTproc rwglDeleteContext;
WMAKECURRENTproc rwglMakeCurrent;
WCHOOSEPIXELFORMATproc rwglChoosePixelFormat;
WSETPIXELFORMATproc rwglSetPixelFormat;
WGETPIXELFORMATproc rwglGetPixelFormat;
WDESCRIBEPIXELFORMATproc rwglDescribePixelFormat;
#endif

#ifdef _MACOSX
char *gDllName = NULL;

void *osxGetProcAddress( char *proc )
{
	static Boolean loaded = false;

	// We may want to cache the bundleRef at some point
    static CFBundleRef bundle = NULL;
    CFURLRef bundleURL;
    CFStringRef functionName = CFStringCreateWithCString(kCFAllocatorDefault, proc, kCFStringEncodingASCII);
    void *function;

	bundleURL = CFURLCreateWithFileSystemPath (kCFAllocatorDefault, CFSTR("/System/Library/Frameworks/OpenGL.framework"), kCFURLPOSIXPathStyle, true);
//                                                        CFSTR("librgl.so.bundle"), kCFURLPOSIXPathStyle, true);

	if( !bundle )
	{
		bundle = CFBundleCreate (kCFAllocatorDefault, bundleURL);
		assert (bundle != NULL);
	}

	if( !loaded )
	{
		loaded = CFBundleLoadExecutable( bundle );
		assert( loaded );
	}

    function = CFBundleGetFunctionPointerForName (bundle, functionName);

    CFRelease ( bundleURL );
    CFRelease ( functionName );
//    CFRelease ( bundle );

    return function;
}
#endif // _MACOSX

#if VERTEX_WRAPPERS
FILE* vout;
void APIENTRY myVertex3fv(GLfloat const* v)
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
void APIENTRY myBindTexture(GLenum target, GLuint textureName)
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
void APIENTRY myGenTextures(GLsizei n, GLuint* textureNames)
{
    _glGenTextures(n, textureNames);
    glFlush();
}
#else
void APIENTRY myGenTextures(GLsizei n, GLuint* textureNames)
{
    char *genOutFileName;
    int i;

    _glGenTextures(n, textureNames);
    if (genOut == NULL)
    {
        genOutFileName = filePathPrepend("gentex.txt", FF_UserSettingsPath);

        if (!fileMakeDestinationDirectory(genOutFileName))
            return;

        genOut = fopen(genOutFileName, "wt");
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
void APIENTRY myDeleteTextures(GLsizei n, GLuint const* textures)
{
    _glDeleteTextures(n, textures);
    glFlush();
}
#endif

#if BEGIN_WRAPPERS
static GLenum currentPrimitive = GL_NONE;
void APIENTRY myBegin(GLenum primitive)
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
void APIENTRY myTexImage2D(GLenum target, GLint level, GLint internalFormat,
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

void APIENTRY noDitheredAlphaEnable(GLenum cap)
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

void APIENTRY noDitheredAlphaDisable(GLenum cap)
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

    if (!bOpenGlLoaded)
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
    putenv("FX_GLIDE_NO_SPLASH=1");\
    if (!mainAllow3DNow)\
    {\
        putenv("__GL_FORCE_K3D=0");\
    }\
    if (!mainAllowKatmai)\
    {\
        putenv("__GL_FORCE_KNI=0");\
    }

/* TC 2003-10-01:
 * I've made attempts to provide dummy functions in rgl to make SDL believe it
 * is a valid OpenGL implementation.  Only SDL_GL_LoadLibrary() and
 * SDL_GL_GetProcAddress() are expected to work properly; SDL_GL_SwapBuffers()
 * should only be called if you know the DLL is actually an OpenGL DLL and not
 * an rgl DLL (check the value of the "RGL" global variable).
 *
 * This currently only works with WGL (Windows) and glX.
 */
GLboolean glDLLGetProcs(char* dllName)
{
    int DynalinkFailed = 0;
    Uint32 sdl_flags;
#if VERTEX_WRAPPERS
    char *voutFileName;
#endif

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
#ifdef _WIN32
        strstr(dllName, "dll\\") != NULL)
#else
        strstr(dllName, "dll/") != NULL)
#endif
    {
        glDLL3Dfx = GL_TRUE;
    }
    else
    {
        glDLL3Dfx = GL_FALSE;
    }

    /* Make sure SDL video is initialized. */
    sdl_flags = SDL_WasInit(SDL_INIT_EVERYTHING);
    if (!sdl_flags)
    {
        if (SDL_Init(SDL_INIT_VIDEO) == -1)
            return GL_FALSE;
    }
    else if (!(sdl_flags & SDL_INIT_VIDEO))
    {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
            return GL_FALSE;
    }

    if (!glNT && glDLL3Dfx)
    {
        glDLLEnvironment();
        //try installed 3dfx GL first
        if (SDL_GL_LoadLibrary(GL_LIB_NAME(3dfxvgl)) == -1)
        {
            //not there, try our own
            glDLLEnvironment();
            if (SDL_GL_LoadLibrary(dllName) == -1)
            {
                return GL_FALSE;
            }
        }
    }
    else
    {
        glDLLEnvironment();

        #if 0 // #ifdef _MACOSX_FIX_ME
        // On OS X SDL_GL_LoadLibrary does nothing, so we must check if the given dll exists.
        {
		    FILE *dll_test = fopen( "librgl.so.bundle", "rb" );
		    if( dll_test )
			    fclose( dll_test );
		    else
    		{
	    		fprintf( stderr, "Cannot load %s library\n", dllName );
		    	return GL_FALSE;
	    	}
	    }
        #endif

        //try loading the .DLL
        if (SDL_GL_LoadLibrary(dllName) == -1)
        {
            fprintf(stderr, "SDL_GL_LoadLibrary(%s): %s\n",
                dllName, SDL_GetError());
            return GL_FALSE;
        }
    }

    bOpenGlLoaded = TRUE;

#if VERTEX_WRAPPERS
    voutFileName = filePathPrepend("vout.dat", FF_UserSettingsPath);

    if (fileMakeDestinationDirectory(voutFileName))
        vout = fopen(voutFileName, "wt");
#endif

#ifdef _MACOSX_FIX_ME
	gDllName = dllName;
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
#ifdef _MACOSX
	glColorTable = (COLORTABLEproc)osxGetProcAddress("glColorTableEXT");
#else
    glColorTable = (COLORTABLEproc)SDL_GL_GetProcAddress("glColorTableEXT");
#endif // _MACOSX
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

#if 0
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
#endif

#ifdef _MACOSX
	rwglGetProcAddress = (WGETPROCADDRESSproc)osxGetProcAddress;
#else
    rwglGetProcAddress = (WGETPROCADDRESSproc)SDL_GL_GetProcAddress("rglGetProcAddress");
    if (!rwglGetProcAddress)
    {
        /* Not an RGL lib, must be regular OpenGL. */
        rwglGetProcAddress = (WGETPROCADDRESSproc)SDL_GL_GetProcAddress;
    }
#endif // _MACOSX

    DYNALINK_GL_FUNCTION(LockArraysEXT);
    DYNALINK_GL_FUNCTION(UnlockArraysEXT);

    return GL_TRUE;
}
