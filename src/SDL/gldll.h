/*=============================================================================
    Name    : gldll.h
    Purpose : alternative to linking against OpenGL32.dll

    Created 12/14/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _GLDLL_H
#define _GLDLL_H

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;

typedef void (*WGLPROC)();

#include "gldefines.h"

typedef void (APIENTRY * LOCKARRAYSEXTproc)(GLint, GLint);
typedef void (APIENTRY * UNLOCKARRAYSEXTproc)(void);

typedef void (APIENTRY * ALPHAFUNCproc)(GLenum, GLclampf);
typedef void (APIENTRY * BEGINproc)(GLenum);
typedef void (APIENTRY * BINDTEXTUREproc)(GLenum, GLuint);
typedef void (APIENTRY * BITMAPproc)(GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, GLubyte const*);
typedef void (APIENTRY * BLENDFUNCproc)(GLenum, GLenum);
typedef void (APIENTRY * CLEARproc)(GLbitfield);
typedef void (APIENTRY * CLEARCOLORproc)(GLclampf, GLclampf, GLclampf, GLclampf);
typedef void (APIENTRY * CLEARDEPTHproc)(GLclampd);
typedef void (APIENTRY * CLEARINDEXproc)(GLfloat);
typedef void (APIENTRY * COLOR3Fproc)(GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY * COLOR3UBproc)(GLubyte, GLubyte, GLubyte);
typedef void (APIENTRY * COLOR4Fproc)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY * COLOR4UBproc)(GLubyte, GLubyte, GLubyte, GLubyte);
typedef void (APIENTRY * COLORMASKproc)(GLboolean, GLboolean, GLboolean, GLboolean);
typedef void (APIENTRY * COLORTABLEproc)(GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid const*);
typedef void (APIENTRY * CULLFACEproc)(GLenum);
typedef void (APIENTRY * DELETETEXTURESproc)(GLsizei, GLuint const*);
typedef void (APIENTRY * DEPTHFUNCproc)(GLenum);
typedef void (APIENTRY * DEPTHMASKproc)(GLboolean);
typedef void (APIENTRY * DEPTHRANGEproc)(GLclampd, GLclampd);
typedef void (APIENTRY * DISABLEproc)(GLenum);
typedef void (APIENTRY * DRAWARRAYSproc)(GLenum, GLint, GLsizei);
typedef void (APIENTRY * DRAWELEMENTSproc)(GLenum, GLsizei, GLenum, GLvoid const*);
typedef void (APIENTRY * DRAWBUFFERproc)(GLenum);
typedef void (APIENTRY * DRAWPIXELSproc)(GLsizei, GLsizei, GLenum, GLenum, GLvoid const*);
typedef void (APIENTRY * ENABLEproc)(GLenum);
typedef void (APIENTRY * ENDproc)(void);
typedef void (APIENTRY * EVALCOORD1Fproc)(GLfloat);
typedef void (APIENTRY * EVALCOORD2Fproc)(GLfloat, GLfloat);
typedef void (APIENTRY * EVALMESH1proc)(GLenum, GLint, GLint);
typedef void (APIENTRY * EVALMESH2proc)(GLenum, GLint, GLint, GLint, GLint);
typedef void (APIENTRY * EVALPOINT1proc)(GLint);
typedef void (APIENTRY * EVALPOINT2proc)(GLint, GLint);
typedef void (APIENTRY * FLUSHproc)(void);
typedef void (APIENTRY * FOGFproc)(GLenum, GLfloat);
typedef void (APIENTRY * FOGFVproc)(GLenum, GLfloat const*);
typedef void (APIENTRY * FOGIproc)(GLenum, GLint);
typedef void (APIENTRY * FRUSTUMproc)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
typedef void (APIENTRY * GENTEXTURESproc)(GLsizei, GLuint*);
typedef void (APIENTRY * GETDOUBLEVproc)(GLenum, GLdouble*);
typedef GLenum (APIENTRY * GETERRORproc)(void);
typedef void (APIENTRY * GETFLOATVproc)(GLenum, GLfloat*);
typedef void (APIENTRY * GETINTEGERVproc)(GLenum, GLint*);
typedef void (APIENTRY * GETBOOLEANVproc)(GLenum, GLboolean*);
typedef GLubyte const* (APIENTRY * GETSTRINGproc)(GLenum);
typedef void (APIENTRY * GETTEXLEVELPARAMETERIVproc)(GLenum, GLint, GLenum, GLint*);
typedef void (APIENTRY * HINTproc)(GLenum, GLenum);
typedef void (APIENTRY * INTERLEAVEDARRAYSproc)(GLenum, GLsizei, GLvoid const*);
typedef GLboolean (APIENTRY * ISENABLEDproc)(GLenum);
typedef void (APIENTRY * LIGHTMODELFproc)(GLenum, GLfloat);
typedef void (APIENTRY * LIGHTMODELFVproc)(GLenum, GLfloat const*);
typedef void (APIENTRY * LIGHTMODELIproc)(GLenum, GLint);
typedef void (APIENTRY * LIGHTFVproc)(GLenum, GLenum, GLfloat const*);
typedef void (APIENTRY * LINESTIPPLEproc)(GLint, GLushort);
typedef void (APIENTRY * LINEWIDTHproc)(GLfloat);
typedef void (APIENTRY * LOADIDENTITYproc)(void);
typedef void (APIENTRY * LOADMATRIXFproc)(GLfloat const*);
typedef void (APIENTRY * MAP1Fproc)(GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat const*);
typedef void (APIENTRY * MAP2Fproc)(GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, GLfloat const*);
typedef void (APIENTRY * MAPGRID1Fproc)(GLint, GLfloat, GLfloat);
typedef void (APIENTRY * MAPGRID2Dproc)(GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble);
typedef void (APIENTRY * MATERIALFVproc)(GLenum, GLenum, GLfloat const*);
typedef void (APIENTRY * MATRIXMODEproc)(GLenum);
typedef void (APIENTRY * MULTMATRIXDproc)(GLdouble const*);
typedef void (APIENTRY * MULTMATRIXFproc)(GLfloat const*);
typedef void (APIENTRY * NORMAL3Fproc)(GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY * NORMAL3FVproc)(GLfloat*);
typedef void (APIENTRY * ORTHOproc)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
typedef void (APIENTRY * PIXELSTOREIproc)(GLenum, GLint);
typedef void (APIENTRY * PIXELTRANSFERFproc)(GLenum, GLfloat);
typedef void (APIENTRY * POINTSIZEproc)(GLfloat);
typedef void (APIENTRY * POLYGONMODEproc)(GLenum, GLenum);
typedef void (APIENTRY * POPATTRIBproc)(void);
typedef void (APIENTRY * POPMATRIXproc)(void);
typedef void (APIENTRY * PUSHATTRIBproc)(GLbitfield);
typedef void (APIENTRY * PUSHMATRIXproc)(void);
typedef void (APIENTRY * RASTERPOS2Fproc)(GLfloat, GLfloat);
typedef void (APIENTRY * RASTERPOS2Iproc)(GLint, GLint);
typedef void (APIENTRY * RASTERPOS4Fproc)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY * READBUFFERproc)(GLenum);
typedef void (APIENTRY * READPIXELSproc)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*);
typedef void (APIENTRY * ROTATEFproc)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY * SCALEFproc)(GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY * SCISSORproc)(GLint, GLint, GLsizei, GLsizei);
typedef void (APIENTRY * SHADEMODELproc)(GLenum);
typedef void (APIENTRY * TEXCOORD2Fproc)(GLfloat, GLfloat);
typedef void (APIENTRY * TEXENVIproc)(GLenum, GLenum, GLint);
typedef void (APIENTRY * TEXIMAGE1Dproc)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, GLvoid const*);
typedef void (APIENTRY * TEXIMAGE2Dproc)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const*);
typedef void (APIENTRY * TEXPARAMETERIproc)(GLenum, GLenum, GLint);
typedef void (APIENTRY * TRANSLATEDproc)(GLdouble, GLdouble, GLdouble);
typedef void (APIENTRY * TRANSLATEFproc)(GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY * VERTEX2Fproc)(GLfloat, GLfloat);
typedef void (APIENTRY * VERTEX2Iproc)(GLint, GLint);
typedef void (APIENTRY * VERTEX3Fproc)(GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY * VERTEX3FVproc)(GLfloat const*);
typedef void (APIENTRY * VERTEX4FVproc)(GLfloat const*);
typedef void (APIENTRY * VIEWPORTproc)(GLint, GLint, GLsizei, GLsizei);
typedef void (APIENTRY * CLIPPLANEproc)(GLenum, GLdouble const*);
typedef void (APIENTRY * GETTEXIMAGEproc)(GLenum, GLint, GLenum, GLenum, GLvoid*);
typedef void (APIENTRY * VERTEXPOINTERproc)(GLint, GLenum, GLsizei, GLvoid const*);
typedef void (APIENTRY * ENABLECLIENTSTATEproc)(GLenum);
typedef void (APIENTRY * DISABLECLIENTSTATEproc)(GLenum);
typedef void (APIENTRY * ARRAYELEMENTproc)(GLint);

typedef WGLPROC (APIENTRY * WGETPROCADDRESSproc)(char*);
typedef void (APIENTRY * WSWAPBUFFERSproc)(void);
#ifdef _WIN32
typedef unsigned int (APIENTRY * WCREATECONTEXTproc)(int);
typedef unsigned int (APIENTRY * WDELETECONTEXTproc)(int);
typedef unsigned int (APIENTRY * WMAKECURRENTproc)(int, int);
typedef int (APIENTRY * WCHOOSEPIXELFORMATproc)(HDC, PIXELFORMATDESCRIPTOR const*);
typedef BOOL (APIENTRY * WSETPIXELFORMATproc)(HDC, int, PIXELFORMATDESCRIPTOR const*);
typedef int (APIENTRY * WGETPIXELFORMATproc)(HDC);
typedef int (APIENTRY * WDESCRIBEPIXELFORMATproc)(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
typedef BOOL (APIENTRY * WSWAPBUFFERSproc)(HDC);
#endif

extern LOCKARRAYSEXTproc glLockArraysEXT;
extern UNLOCKARRAYSEXTproc glUnlockArraysEXT;

extern ALPHAFUNCproc glAlphaFunc;
extern BEGINproc glBegin;
extern BINDTEXTUREproc glBindTexture;
extern BITMAPproc glBitmap;
extern BLENDFUNCproc glBlendFunc;
extern CLEARproc glClear;
extern CLEARCOLORproc glClearColor;
extern CLEARDEPTHproc glClearDepth;
extern CLEARINDEXproc glClearIndex;
extern COLOR3Fproc glColor3f;
extern COLOR3UBproc glColor3ub;
extern COLOR4Fproc glColor4f;
extern COLOR4UBproc glColor4ub;
extern COLORMASKproc glColorMask;
extern COLORTABLEproc glColorTable;
extern CULLFACEproc glCullFace;
extern DELETETEXTURESproc glDeleteTextures;
extern DEPTHFUNCproc glDepthFunc;
extern DEPTHMASKproc glDepthMask;
extern DEPTHRANGEproc glDepthRange;
extern DISABLEproc glDisable;
extern DRAWARRAYSproc glDrawArrays;
extern DRAWELEMENTSproc glDrawElements;
extern DRAWBUFFERproc glDrawBuffer;
extern DRAWPIXELSproc glDrawPixels;
extern ENABLEproc glEnable;
extern ENDproc glEnd;
extern EVALCOORD1Fproc glEvalCoord1f;
extern EVALCOORD2Fproc glEvalCoord2f;
extern EVALMESH1proc glEvalMesh1;
extern EVALMESH2proc glEvalMesh2;
extern EVALPOINT1proc glEvalPoint1;
extern EVALPOINT2proc glEvalPoint2;
extern FLUSHproc glFlush;
extern FOGFproc glFogf;
extern FOGFVproc glFogfv;
extern FOGIproc glFogi;
extern FRUSTUMproc glFrustum;
extern GENTEXTURESproc glGenTextures;
extern GETDOUBLEVproc glGetDoublev;
extern GETERRORproc glGetError;
extern GETFLOATVproc glGetFloatv;
extern GETINTEGERVproc glGetIntegerv;
extern GETBOOLEANVproc glGetBooleanv;
extern GETSTRINGproc glGetString;
extern GETTEXLEVELPARAMETERIVproc glGetTexLevelParameteriv;
extern HINTproc glHint;
extern INTERLEAVEDARRAYSproc glInterleavedArrays;
extern ISENABLEDproc glIsEnabled;
extern LIGHTMODELFproc glLightModelf;
extern LIGHTMODELFVproc glLightModelfv;
extern LIGHTMODELIproc glLightModeli;
extern LIGHTFVproc glLightfv;
extern LINESTIPPLEproc glLineStipple;
extern LINEWIDTHproc glLineWidth;
extern LOADIDENTITYproc glLoadIdentity;
extern LOADMATRIXFproc glLoadMatrixf;
extern MAP1Fproc glMap1f;
extern MAP2Fproc glMap2f;
extern MAPGRID1Fproc glMapGrid1f;
extern MAPGRID2Dproc glMapGrid2d;
extern MATERIALFVproc glMaterialfv;
extern MATRIXMODEproc glMatrixMode;
extern MULTMATRIXDproc glMultMatrixd;
extern MULTMATRIXFproc glMultMatrixf;
extern NORMAL3Fproc glNormal3f;
extern NORMAL3FVproc glNormal3fv;
extern ORTHOproc glOrtho;
extern PIXELSTOREIproc glPixelStorei;
extern PIXELTRANSFERFproc glPixelTransferf;
extern POINTSIZEproc glPointSize;
extern POLYGONMODEproc glPolygonMode;
extern POPATTRIBproc glPopAttrib;
extern POPMATRIXproc glPopMatrix;
extern PUSHATTRIBproc glPushAttrib;
extern PUSHMATRIXproc glPushMatrix;
extern RASTERPOS2Fproc glRasterPos2f;
extern RASTERPOS2Iproc glRasterPos2i;
extern RASTERPOS4Fproc glRasterPos4f;
extern READBUFFERproc glReadBuffer;
extern READPIXELSproc glReadPixels;
extern ROTATEFproc glRotatef;
extern SCALEFproc glScalef;
extern SCISSORproc glScissor;
extern SHADEMODELproc glShadeModel;
extern TEXCOORD2Fproc glTexCoord2f;
extern TEXENVIproc glTexEnvi;
extern TEXIMAGE1Dproc glTexImage1D;
extern TEXIMAGE2Dproc glTexImage2D;
extern TEXPARAMETERIproc glTexParameteri;
extern TRANSLATEDproc glTranslated;
extern TRANSLATEFproc glTranslatef;
extern VERTEX2Fproc glVertex2f;
extern VERTEX2Iproc glVertex2i;
extern VERTEX3Fproc glVertex3f;
extern VERTEX3FVproc glVertex3fv;
extern VERTEX4FVproc glVertex4fv;
extern VIEWPORTproc glViewport;
extern CLIPPLANEproc glClipPlane;
extern GETTEXIMAGEproc glGetTexImage;
extern VERTEXPOINTERproc glVertexPointer;
extern ENABLECLIENTSTATEproc glEnableClientState;
extern DISABLECLIENTSTATEproc glDisableClientState;
extern ARRAYELEMENTproc glArrayElement;

extern WGETPROCADDRESSproc rwglGetProcAddress;
#if 0	/* Trying to phase these out... */
extern WCREATECONTEXTproc rwglCreateContext;
extern WDELETECONTEXTproc rwglDeleteContext;
extern WMAKECURRENTproc rwglMakeCurrent;
extern WCHOOSEPIXELFORMATproc rwglChoosePixelFormat;
extern WSETPIXELFORMATproc rwglSetPixelFormat;
extern WGETPIXELFORMATproc rwglGetPixelFormat;
extern WDESCRIBEPIXELFORMATproc rwglDescribePixelFormat;
extern WSWAPBUFFERSproc rwglSwapBuffers;
#endif

extern GLboolean glDLL3Dfx;

GLboolean glDLLGetProcs(char* dllName);
void glDLLReset(void);
void glDLLGetGLCompat(void);

#endif
