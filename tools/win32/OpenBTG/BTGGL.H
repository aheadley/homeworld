/*
** BTGGL.H : Header file for BTGGL.CPP.
*/

#ifndef __BTGGL_H
#define __BTGLG_H

#include <GL/gl.h>
#include <GL/glaux.h>
#include <GL/glu.h>

#include "btgdoc.h"

	void setupPixelFormat(HDC hDC);
	void setupPalette(HDC hDC);
	void btgGLInit(CView *pView);
	void btgGLInit3D(CView *pView);
	void btgGLClose(CView *pView);

	void btgGLDraw3D(CBTGDoc *pDoc, CView *pView);
	void btgGLDrawBackground(CBTGDoc *pDoc);

	void btgGLBlend(CBTGDoc *pDoc);

	void bandBoxRDraw(CBTGDoc *pDoc);
	void pageSizeRDraw(CBTGDoc *pDoc);
	void btgvRDraw(CBTGDoc *pDoc);
	void btgsRDraw(CBTGDoc *pDoc);
	void btgpRDraw(CBTGDoc *pDoc);
	void btgpRDrawSolid(CBTGDoc *pDoc);
    void btglRDraw(CBTGDoc *pDoc);

	void glRectangle(signed long l, signed long t, signed long r, signed long b);
	void glSolidRectangle(signed long l, signed long t, signed long r, signed long b);
	void glCircle(signed long xp, signed long yp, signed long r);

	void btgGLRecalc3DCamera();
	void btgGLUpdate3DMouse(float x, float y);

	void btgGLGrabPixel(CBTGDoc *pDoc, CPoint point, unsigned char *colArray);

#endif