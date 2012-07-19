/*
** BTGGL.CPP : Code to handle the GL implementation of the btg.
*/

#include "stdafx.h"

#include "math.h"

#include "BTGGL.H"

#include "btgview.h"
#include "btgdoc.h"
#include "mainfrm.h"
#include "btg.h"

HDC hGLDeviceContext;
HGLRC hGLRenderContext;
CRect drawRect, oldRect;

static double eye[3] = {0.0, 0.0, 0.0};
static double look[3] = {0.0, 1.0, 0.0};

static float camera_declination = 0.0f;
static float camera_angle = 0.0f;

#define DEPTH_SCALE 65535.0f
#define CLIP_RIGHT_BIT 1
#define CLIP_LEFT_BIT 2
#define CLIP_TOP_BIT 4
#define CLIP_BOTTOM_BIT 8
#define CLIP_NEAR_BIT 16
#define CLIP_FAR_BIT 32
#define CLIP_ALL_BITS 0x3f

typedef struct vector
{
	float x, y, z;
} vector;

typedef struct hvector
{
	float x, y, z, w;
} hvector;

void setupPixelFormat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),  /* size */
    1,              /* version */
    PFD_SUPPORT_OPENGL |
    PFD_DRAW_TO_WINDOW |
    PFD_DOUBLEBUFFER,       /* support double-buffering */
    PFD_TYPE_RGBA,          /* color type */
    16,             /* prefered color depth */
    0, 0, 0, 0, 0, 0,       /* color bits (ignored) */
    0,              /* no alpha buffer */
    0,              /* alpha bits (ignored) */
    0,              /* no accumulation buffer */
    0, 0, 0, 0,         /* accum bits (ignored) */
    16,             /* depth buffer */
    0,              /* no stencil buffer */
    0,              /* no auxiliary buffers */
    PFD_MAIN_PLANE,         /* main layer */
    0,              /* reserved */
    0, 0, 0,            /* no layer, visible, damage masks */
    };
    int pixelFormat;

    pixelFormat = ChoosePixelFormat(hDC, &pfd);

    if (pixelFormat == 0)
    {
        //assert(DBG_Loc, "ChoosePixelFormat failed.");
    }

    if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE)
    {
        //dbgFatal(DBG_Loc, "SetPixelFormat failed.");
    }
}

void setupPalette(HDC hDC)
{
    int pixelFormat = GetPixelFormat(hDC);
    PIXELFORMATDESCRIPTOR pfd;

    DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    if (pfd.dwFlags & PFD_NEED_PALETTE) 
	{
        //dbgFatal(DBG_Loc, "rndInit: needs paletted display");
    } 
	else 
	{
		return;
    }
}

void btgGLInit(CView *pView)
{
	oldRect = drawRect;

	pView->GetClientRect(&drawRect);

	if(oldRect != drawRect)
	{
		if (hGLDeviceContext != NULL)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hGLRenderContext);
			hGLRenderContext = 0;
		}
	}

	hGLDeviceContext = wglGetCurrentDC();
	if (hGLDeviceContext == NULL || hGLRenderContext == 0)
	{
		hGLDeviceContext = ::GetDC(pView->GetSafeHwnd());

		setupPixelFormat(hGLDeviceContext);
		setupPalette(hGLDeviceContext);
		hGLRenderContext = wglCreateContext(hGLDeviceContext);  //create GL render context and select into window
		wglMakeCurrent(hGLDeviceContext, hGLRenderContext);

		//create the viewport for rendering
		auxInitPosition(0, 0, drawRect.right - drawRect.left, drawRect.bottom - drawRect.top);
		//set mode (direct color)
		auxInitDisplayMode(AUX_RGB);
	}
	else
	{
		//wglMakeCurrent(hGLDeviceContext, hGLRenderContext);
	}

	CBTGDoc* pDoc = (CBTGDoc*)pView->GetDocument();
	glClearColor((GLfloat)pDoc->mBGRed / 255.0f,
				 (GLfloat)pDoc->mBGGreen / 255.0f,
				 (GLfloat)pDoc->mBGBlue / 255.0f,
				 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, drawRect.right - drawRect.left, 0, drawRect.bottom - drawRect.top);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void btgGLInit3D(CView *pView)
{
	oldRect = drawRect;

	pView->GetClientRect(&drawRect);

	if (oldRect != drawRect)
	{
		if (hGLDeviceContext != NULL)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hGLRenderContext);
			hGLRenderContext = 0;
		}
	}

	hGLDeviceContext = wglGetCurrentDC();
	if (hGLDeviceContext == NULL || hGLRenderContext == 0)
	{
		hGLDeviceContext = ::GetDC(pView->GetSafeHwnd());

		setupPixelFormat(hGLDeviceContext);
		setupPalette(hGLDeviceContext);
		hGLRenderContext = wglCreateContext(hGLDeviceContext);
		wglMakeCurrent(hGLDeviceContext, hGLRenderContext);

		auxInitPosition(0, 0, drawRect.right - drawRect.left, drawRect.bottom - drawRect.top);
		auxInitDisplayMode(AUX_RGB);
	}
	else
	{
		//wglMakeCurrent(hGLDeviceContext, hGLRenderContext);
	}

	CBTGDoc* pDoc = (CBTGDoc*)pView->GetDocument();
	glClearColor((GLfloat)pDoc->mBGRed / 255.0f,
				 (GLfloat)pDoc->mBGGreen / 255.0f,
				 (GLfloat)pDoc->mBGBlue / 255.0f,
				 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (float)(drawRect.right - drawRect.left) / (float)(drawRect.bottom - drawRect.top), 1.0, 2000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	btgGLRecalc3DCamera();
	gluLookAt(eye[0], eye[1], eye[2],
			  look[0], look[1], look[2],
			  0.0, 0.0, 1.0);

	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void btgGLClose(CView *pView)
{
	CDC *pDC = pView->GetDC();

	glFlush();
	SwapBuffers(pDC->m_hDC);
#if 0
	if (hGLRenderContext)
    {
        wglMakeCurrent(NULL, NULL);                         //release GL render context
        wglDeleteContext(hGLRenderContext);
		hGLRenderContext = 0;
    }
#endif
}

void btgGLBlend(CBTGDoc *pDoc)
{
	if (pDoc->bStars)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

void btgColor(CBTGDoc *pDoc, BTGVertex *pVertex)
{
	if (1 || pDoc->bStars)
	{
		//faux blend for efficiency
		int red = pVertex->red;
		int green = pVertex->green;
		int blue = pVertex->blue;
		int alpha = pVertex->alpha;
		int brightness = pVertex->brightness;

		alpha = (alpha * brightness) >> 8;

		red = (red * alpha) >> 8;
		green = (green * alpha) >> 8;
		blue = (blue * alpha) >> 8;
//		alpha = 255;

        if (alpha < 252)
        {
            if (red < pDoc->mBGRed) red = pDoc->mBGRed;
		    if (green < pDoc->mBGGreen) green = pDoc->mBGGreen;
		    if (blue < pDoc->mBGBlue) blue = pDoc->mBGBlue;
        }

		glColor4ub((unsigned char)red, (unsigned char)green, (unsigned char)blue, (unsigned char)255/*alpha*/);
	}
	else
	{
		glColor4ub((unsigned char)pVertex->red, (unsigned char)pVertex->green,
				   (unsigned char)pVertex->blue, (unsigned char)pVertex->alpha);
	}
}

void bandBoxRDraw(CBTGDoc *pDoc)
{
	glColor3ub(0xFF, 0xFF, 0xFF);
	glLineStipple(8, 0xAAAA);
	glEnable(GL_LINE_STIPPLE);
	glRectangle(pDoc->bandBoxAnchor.x, pDoc->bandBoxAnchor.y, pDoc->bandBoxFloat.x, pDoc->bandBoxFloat.y);
	glDisable(GL_LINE_STIPPLE);
}

void pageSizeRDraw(CBTGDoc *pDoc)
{
	CPoint tempPoint(pDoc->pageWidth, pDoc->pageHeight);
	CPoint basePoint(0, 0);

	glColor3ub(0xFF, 0xFF, 0xFF);

	pDoc->CalculateScreenCoordinates(&tempPoint);
	pDoc->CalculateScreenCoordinates(&basePoint);

	glRectangle(basePoint.x, basePoint.y, tempPoint.x, tempPoint.y);
}

void btgvRDraw(CBTGDoc *pDoc)
{
	BTGVertex *pVertex;
	CPoint p;

	pVertex = pDoc->GetFirstBTGVertex();

	while(pVertex)
	{
		if(pVertex->bVisible == FALSE)
		{
			pVertex = pVertex->GetNext();

			continue;
		}

		// Get this vertex's screen coords.
		p.x = (long)pVertex->x; 
		p.y = (long)pVertex->y;
		pDoc->CalculateScreenCoordinates(&p);
		
		// Get this vertex's colors.
		glColor4ub((unsigned char)pVertex->red, (unsigned char)pVertex->green,
				   (unsigned char)pVertex->blue, (unsigned char)pVertex->alpha);
		glSolidRectangle(p.x - BTGV_SIZE2, p.y - BTGV_SIZE2, p.x + BTGV_SIZE2, p.y + BTGV_SIZE2);

		// Is this vertex selected?
		if(bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
		{
			glLineWidth(2.0f);
			glColor3ub(255,0,0);
			glRectangle(p.x - BTGV_SIZE2, p.y - BTGV_SIZE2, p.x + BTGV_SIZE2, p.y + BTGV_SIZE2);
			glLineWidth(1.0f);
		}
		else
		{
			glColor3ub(0xFF, 0xFF, 0xFF);
			glRectangle(p.x - BTGV_SIZE2, p.y - BTGV_SIZE2, p.x + BTGV_SIZE2, p.y + BTGV_SIZE2);
		}

		pVertex = pVertex->GetNext();
	}
}

unsigned int glCreateStar(BTGStar* pStar)
{
    unsigned int thandle;

    if (pStar->pMyStar == NULL)
    {
        return 0;
    }

    glGenTextures(1, &thandle);
    glBindTexture(GL_TEXTURE_2D, thandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 pStar->pMyStar->myFile.header.imageWidth, pStar->pMyStar->myFile.header.imageHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, pStar->pMyStar->myFile.bitmapBits);

    return thandle;
}

void glRecreateStar(BTGStar* pStar)
{
    if (pStar->pMyStar == NULL)
    {
        return;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 pStar->pMyStar->myFile.header.imageWidth, pStar->pMyStar->myFile.header.imageHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, pStar->pMyStar->myFile.bitmapBits);
}

void glDeleteStar(unsigned int thandle)
{
    if (thandle != 0)
    {
        glDeleteTextures(1, &thandle);
    }
}

void btglRDraw(CBTGDoc *pDoc)
{
    BTGLight* pLight;
    CPoint p;
    int x2, y2;

    glDisable(GL_BLEND);

    pLight = pDoc->GetFirstBTGLight();

    while (pLight)
    {
        if (pLight->bVisible == FALSE)
        {
            pLight = pLight->GetNext();
            continue;
        }

        // Get this light's screen coords.
        p.x = (long)pLight->x;
        p.y = (long)pLight->y;
        pDoc->CalculateScreenCoordinates(&p);

        glDisable(GL_TEXTURE_2D);

        x2 = 3*BTGV_SIZE;
        y2 = 3*BTGV_SIZE;

        x2 = (int)((float)x2 * pDoc->zoomVal);
        y2 = (int)((float)y2 * pDoc->zoomVal);

        p.y = drawRect.bottom - p.y;

        if (bitTest(pLight->flags, BTGLight::btglFlags_Selected))
        {
            glColor3ub(0,0,0);
        }
        else
        {
            glColor3ub(140,140,140);
        }
        glBegin(GL_QUADS);
		glVertex2i(p.x - x2 - 2, p.y - y2 - 2);
		glVertex2i(p.x - x2 - 2, p.y + y2 + 2);
		glVertex2i(p.x + x2 + 2, p.y + y2 + 2);
		glVertex2i(p.x + x2 + 2, p.y - y2 - 2);
        glEnd();

        glColor3ub((GLubyte)pLight->dred, (GLubyte)pLight->dgreen, (GLubyte)pLight->dblue);
        glBegin(GL_QUADS);
		glVertex2i(p.x - x2, p.y - y2);
		glVertex2i(p.x - x2, p.y + y2);
		glVertex2i(p.x + x2, p.y + y2);
		glVertex2i(p.x + x2, p.y - y2);
        glEnd();

        pLight = pLight->GetNext();
    }
}

void btgsRDraw(CBTGDoc *pDoc)
{
	BTGStar *pStar;
	CPoint p;
	unsigned int thandle;
	int x2, y2;

	btgGLBlend(pDoc);

	pStar = pDoc->GetFirstBTGStar();
    thandle = pStar ? glCreateStar(pStar) : 0;

	while(pStar)
	{
		if (pStar->bVisible == FALSE || pStar->pMyStar == NULL)
		{
			pStar = pStar->GetNext();
			continue;
		}

		// Get this Star's screen coords.
		p.x = (long)pStar->x; 
		p.y = (long)pStar->y;
		pDoc->CalculateScreenCoordinates(&p);

        glEnable(GL_TEXTURE_2D);
        glRecreateStar(pStar);

        x2 = pStar->pMyStar->myFile.header.imageWidth / 2;
		y2 = pStar->pMyStar->myFile.header.imageHeight / 2;

		x2 = (int)((float)x2 * pDoc->zoomVal);
		y2 = (int)((float)y2 * pDoc->zoomVal);

		p.y = drawRect.bottom - p.y;

		glColor4ub((unsigned char)pStar->red, (unsigned char)pStar->green,
				   (unsigned char)pStar->blue, (unsigned char)pStar->alpha);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(p.x - x2, p.y - y2);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(p.x - x2, p.y + y2);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i(p.x + x2, p.y + y2);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(p.x + x2, p.y - y2);
		glEnd();

		glDisable(GL_TEXTURE_2D);
		if (bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
		{
			glBegin(GL_LINE_LOOP);
			glVertex2i(p.x - x2, p.y - y2);
			glVertex2i(p.x - x2, p.y + y2);
			glVertex2i(p.x + x2, p.y + y2);
			glVertex2i(p.x + x2, p.y - y2);
			glEnd();
		}

		pStar = pStar->GetNext();
	}

    if (thandle)
    {
        glDeleteStar(thandle);
    }

    glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void btgpRDraw(CBTGDoc *pDoc)
{
	BTGPolygon *pPolygon;
	CPoint p0, p1, p2;
	unsigned long lineWidth = 1;
	unsigned char red, green, blue;

	if(pDoc->drawSolidPolys)
	{
		btgpRDrawSolid(pDoc);
	}
	
	if(pDoc->drawMode != CBTGDoc::btgDrawMode_VertsPolys)
	{
		return;
	}
		
	glShadeModel(GL_FLAT);

	pPolygon = pDoc->GetFirstBTGPolygon();

	while(pPolygon)
	{
		if(pPolygon->bVisible == FALSE)
		{
			pPolygon = pPolygon->GetNext();

			continue;
		}

		// Calculate the proper coordinates.
		p0.x = (int)pPolygon->v0->x;
		p0.y = (int)pPolygon->v0->y;
		p1.x = (int)pPolygon->v1->x;
		p1.y = (int)pPolygon->v1->y;
		p2.x = (int)pPolygon->v2->x;
		p2.y = (int)pPolygon->v2->y;

		pDoc->CalculateScreenCoordinates(&p0);
		pDoc->CalculateScreenCoordinates(&p1);
		pDoc->CalculateScreenCoordinates(&p2);

		p0.y = drawRect.bottom - p0.y;
		p1.y = drawRect.bottom - p1.y;
		p2.y = drawRect.bottom - p2.y;

		// Is this polygon selected?
		if(bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected))
		{
			glLineWidth(2);
			red = 0xFF;
			green = 0x10;
			blue = 0x20;
		}
		else
		{
			glLineWidth(1);
			red = 181;
			green = 182;
			blue = 181;
		}

		if((pDoc->btgpSelectedEdge == 1) && (bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected)))
		{
			glColor3ub(0, 0xFF, 0);
		}
		else
		{
			glColor3ub(red, green, blue);
		}

		// First Edge
		glBegin(GL_LINES);
		glVertex2i(p0.x, p0.y);
		glVertex2i(p1.x, p1.y);
		glEnd();
		
		if((pDoc->btgpSelectedEdge == 2) && (bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected)))
		{
			glColor3ub(0, 0xFF, 0);
		}
		else
		{
			glColor3ub(red, green, blue);
		}

		// Second Edge
		glBegin(GL_LINES);
		glVertex2i(p1.x, p1.y);
		glVertex2i(p2.x, p2.y);
		glEnd();

		if((pDoc->btgpSelectedEdge == 3) && (bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected)))
		{
			glColor3ub(0, 0xFF, 0);
		}
		else
		{
			glColor3ub(red, green, blue);
		}

		// Third Edge
		glBegin(GL_LINES);
		glVertex2i(p2.x, p2.y);
		glVertex2i(p0.x, p0.y);
		glEnd();

		glEnd();

		glLineWidth(1);

		pPolygon = pPolygon->GetNext();
	}
}

void btgpRDrawSolid(CBTGDoc *pDoc)
{
	BTGPolygon *pPolygon;
	CPoint p0, p1, p2;
	unsigned long lineWidth = 1;

//	btgGLBlend(pDoc);
    glDisable(GL_BLEND);

	glShadeModel(GL_SMOOTH);

	pPolygon = pDoc->GetFirstBTGPolygon();

	while(pPolygon)
	{
		if(pPolygon->bVisible == FALSE)
		{
			pPolygon = pPolygon->GetNext();

			continue;
		}

		// Calculate the proper coordinates.
		p0.x = (int)pPolygon->v0->x;
		p0.y = (int)pPolygon->v0->y;
		p1.x = (int)pPolygon->v1->x;
		p1.y = (int)pPolygon->v1->y;
		p2.x = (int)pPolygon->v2->x;
		p2.y = (int)pPolygon->v2->y;

		pDoc->CalculateScreenCoordinates(&p0);
		pDoc->CalculateScreenCoordinates(&p1);
		pDoc->CalculateScreenCoordinates(&p2);

		p0.y = drawRect.bottom - p0.y;
		p1.y = drawRect.bottom - p1.y;
		p2.y = drawRect.bottom - p2.y;

		// First Edge
		glBegin(GL_TRIANGLES);

		btgColor(pDoc, pPolygon->v0);
		glVertex2i(p0.x, p0.y);
		
		// Second Edge
		btgColor(pDoc, pPolygon->v1);
		glVertex2i(p1.x, p1.y);

		// Third Edge
		btgColor(pDoc, pPolygon->v2);
		glVertex2i(p2.x, p2.y);
		
		glEnd();

		pPolygon = pPolygon->GetNext();
	}
}

void glRectangle(signed long l, signed long t, signed long r, signed long b)
{
	float nt, nb, nl, nr;

	glBegin(GL_LINE_LOOP);

	nt = (float)drawRect.bottom - t;
	nb = (float)drawRect.bottom - b;
	nl = (float)l;
	nr = (float)r;
	
	glVertex2f(nl, nt);
	glVertex2f(nr, nt);
	glVertex2f(nr, nb);
	glVertex2f(nl, nb);

	glEnd();
}

void glSolidRectangle(signed long l, signed long t, signed long r, signed long b)
{
	float nt, nb, nl, nr;

	glBegin(GL_QUADS);

	nt = (float)drawRect.bottom - t;
	nb = (float)drawRect.bottom - b;
	nl = (float)l;
	nr = (float)r;
	
	glVertex2f(nl, nt);
	glVertex2f(nr, nt);
	glVertex2f(nr, nb);
	glVertex2f(nl, nb);

	glEnd();
}

#define PI		(3.14159f)
#define TWOPI	(PI * 2)
void glCircle(signed long xp, signed long yp, signed long r)
{
    signed long segment, endSegment;
    float angle, angleInc;
    float x, y, nyp;

    nyp = (float)drawRect.bottom - yp;
	
	segment = 0;
    endSegment = (signed long)(10 - 0.01f);//get ending segment

    glBegin(GL_LINE_STRIP);

    x = xp + (float)sin((double)0) * r;    //first vertex
    y = nyp + (float)cos((double)0) * r;
    glVertex2f(x, y);
    segment++;
    angle = (float)segment * (2.0f * PI / (float)10);
    angleInc = (2.0f * PI / (float)10);

    for (; segment <= endSegment; segment++)
    {                                                       //for start and all complete segments
        x = xp + (float)sin((double)angle) * r;
        y = nyp + (float)cos((double)angle) * r;
        glVertex2f(x, y);
        angle += angleInc;                                  //update angle
    }
    x = xp + (float)sin((double)TWOPI) * r;
    y = nyp + (float)cos((double)TWOPI) * r;
    glVertex2f(x, y);                                       //draw last vertex

    glEnd();
}

#define fsin(x) (float)sin((double)x)
#define fcos(x) (float)cos((double)x)

void btgConvertAVert(CBTGDoc* pDoc, float* outX, float* outY, float* outZ, float x, float y)
{
    float xFrac, yFrac;
    float theta, phi;
    float sinTheta, cosTheta;
    float sinPhi, cosPhi;
    float pageWidth, pageHeight;
    float radius;

    pageWidth  = (float)pDoc->pageWidth;
    pageHeight = (float)pDoc->pageHeight;

	x = (pageWidth - 1.0f) - x;

    xFrac = x / pageWidth;
    yFrac = y / pageHeight;

    theta = 2.0f * 3.14159265f * xFrac;
    phi   = 1.0f * 3.14159265f * yFrac;

    sinTheta = fsin(theta);
    cosTheta = fcos(theta);
    sinPhi = fsin(phi);
    cosPhi = fcos(phi);

    radius = 100.0;
    *outX = radius * cosTheta * sinPhi;
    *outY = radius * sinTheta * sinPhi;
    *outZ = radius * cosPhi;
}

void btg3DVert(CBTGDoc* pDoc, double x, double y)
{
	float ox, oy, oz;

	btgConvertAVert(pDoc, &ox, &oy, &oz, (float)x, (float)y);
	glVertex3f(ox, oy, oz);
}

void matMakeRotAboutY(float* matrix, float costheta, float sintheta)
{
	matrix[0] = costheta;
	matrix[1] = 0.0f;
	matrix[2] = -sintheta;

	matrix[3] = 0.0f;
	matrix[4] = 1.0f;
	matrix[5] = 0.0f;

	matrix[6] = sintheta;
	matrix[7] = 0.0f;
	matrix[8] = costheta;
}

void matMakeRotAboutZ(float* matrix, float costheta, float sintheta)
{
	matrix[0] = costheta;
	matrix[1] = sintheta;
	matrix[2] = 0.0f;

	matrix[3] = -sintheta;
	matrix[4] = costheta;
	matrix[5] = 0.0f;

	matrix[6] = 0.0f;
	matrix[7] = 0.0f;
	matrix[8] = 1.0f;
}

#define SOURCE  esi
#define DEST    ebx
#define MATRIX  edi
#define FPTR    dword ptr
#define FSIZE   4

void matMultiplyMatByVec(float* result, float* matrix, float* vector)
{
	float* dest = result;
	float* source = vector;
	float* mat = matrix;

    _asm
    {
        push    esi
        push    edi
        push    ebx

        mov     esi,[source]
        mov     ebx,[dest]
        mov     edi,[mat]

        fld     FPTR [SOURCE+0*FSIZE]               //s0
        fmul    FPTR [MATRIX+(0+0*3)*FSIZE]         //a0
        fld     FPTR [SOURCE+1*FSIZE]               //s1 a0
        fmul    FPTR [MATRIX+(0+1*3)*FSIZE]         //a1 a0
        fld     FPTR [SOURCE+2*FSIZE]               //s2 a1 a0
        fmul    FPTR [MATRIX+(0+2*3)*FSIZE]         //a2 a1 a0

        fxch    st(1)                               //a1 a2 a0
        faddp   st(2),st                            //a2 a1+a0

        fld     FPTR [SOURCE+0*FSIZE]               //s0 a2 a1+a0
        fmul    FPTR [MATRIX+(1+0*3)*FSIZE]         //b0 a2 a1+a0

        fxch    st(1)                               //a2 b0 a1+a0
        faddp   st(2),st                            //b0 d0

        fld     FPTR [SOURCE+1*FSIZE]               //s1 b0 d0
        fmul    FPTR [MATRIX+(1+1*3)*FSIZE]         //b1 b0 d0
        fld     FPTR [SOURCE+2*FSIZE]               //s2 b1 b0 d0
        fmul    FPTR [MATRIX+(1+2*3)*FSIZE]         //b2 b1 b0 d0

        fxch    st(1)                               //b1 b2 b0 d0
        faddp   st(1),st                            //b1+b2 b0 d0

        fld     FPTR [SOURCE+0*FSIZE]               //s0 b1+b2 b0 d0
        fmul    FPTR [MATRIX+(2+0*3)*FSIZE]         //c0 b1+b2 b0 d0

        fxch    st(1)                               //b1+b2 c0 b0 d0
        faddp   st(2),st                            //c0 d1 d0

        fld     FPTR [SOURCE+1*FSIZE]               //s1 c0 d1 d0
        fmul    FPTR [MATRIX+(2+1*3)*FSIZE]         //c1 c0 d1 d0
        fld     FPTR [SOURCE+2*FSIZE]               //s2 c1 c0 d1 d0
        fmul    FPTR [MATRIX+(2+2*3)*FSIZE]         //c2 c1 c0 d1 d0

        fxch    st(1)                               //c1 c2 c0 d1 d0
        faddp   st(1),st                            //c1+c2 c0 d1 d0

        fxch    st(2)                               //d1 c0 c1+c2 d0
        fstp    [DEST+1*FSIZE]                      //c0 c1+c2 d0
        fxch    st(1)                               //c1+c2 c0 d0
        faddp   st(1),st                            //d2 d0
        fxch    st(1)                               //d0 d2
        fstp    [DEST+0*FSIZE]                      //d2
        fstp    [DEST+2*FSIZE]

        pop     ebx
        pop     edi
        pop     esi
    }
}

#undef FSIZE
#undef FPTR
#undef MATRIX
#undef DEST
#undef SOURCE

void btgGLRecalc3DCamera()
{
	float dirvec[3];
	float rotymatrix[9], rotzmatrix[9];
	float rot1vec[3], rot2vec[3];

    dirvec[0] = 100.0f;
	dirvec[1] = dirvec[2] = 0.0f;

    matMakeRotAboutY(rotymatrix, fcos(camera_declination), fsin(camera_declination));
    matMakeRotAboutZ(rotzmatrix, fcos(camera_angle), fsin(camera_angle));

    matMultiplyMatByVec(rot1vec, rotymatrix, dirvec);
    matMultiplyMatByVec(rot2vec, rotzmatrix, rot1vec);

	look[0] = rot2vec[0] + eye[0];
	look[1] = rot2vec[1] + eye[1];
	look[2] = rot2vec[2] + eye[2];
}

#define RADIAN_PER_DEG (2.0f*3.14159f/360.0f)
#define DEG_TO_RAD(x) ((x) * RADIAN_PER_DEG)

void btgRotDeclination(float n)
{
	camera_declination += n;
	if (camera_declination > DEG_TO_RAD(86.9f))
	{
		camera_declination = DEG_TO_RAD(86.9f);
	}
	else if (camera_declination < -DEG_TO_RAD(86.9f))
	{
		camera_declination = -DEG_TO_RAD(86.9f);
	}
}

#ifndef TWOPI
#define TWOPI 2.0f * 3.14159f
#endif

void btgRotAngle(float n)
{
	camera_angle += n;
	if (camera_angle < 0.0f)
	{
		camera_angle += TWOPI;
	}
	else if (camera_angle > TWOPI)
	{
		camera_angle -= TWOPI;
	}
}

void btgGLUpdate3DMouse(float x, float y)
{
	float width, height;
	float mouseX, mouseY;

	width  = (float)(drawRect.right - drawRect.left);
	height = (float)(drawRect.bottom - drawRect.top);

	mouseX = -x;
	mouseY = y;

	btgRotDeclination(DEG_TO_RAD(0.02f) * mouseY);
	btgRotAngle(DEG_TO_RAD(0.02f) * mouseX);
}

void btgGLDraw3D(CBTGDoc* pDoc, CView* pView)
{
	BTGPolygon* pPolygon;

	glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

	pPolygon = pDoc->GetFirstBTGPolygon();
	glBegin(GL_TRIANGLES);
	while (pPolygon)
	{
		btgColor(pDoc, pPolygon->v0);
		btg3DVert(pDoc, pPolygon->v0->x, pPolygon->v0->y);
		btgColor(pDoc, pPolygon->v1);
		btg3DVert(pDoc, pPolygon->v1->x, pPolygon->v1->y);
		btgColor(pDoc, pPolygon->v2);
		btg3DVert(pDoc, pPolygon->v2->x, pPolygon->v2->y);

		pPolygon = pPolygon->GetNext();
	}
	glEnd();
}

void btgGLDrawBackground(CBTGDoc *pDoc)
{
	CPoint p0, p1;
	TGAFile *pBackground = pDoc->GetBackground();
	float zx, zy;
	float rx, ry;
	unsigned char bitmap[16];
	
	if(pBackground->bitmapBits == NULL)
		return;
	
	// Get this Star's screen coords.
	p0.x = 0; 
	p0.y = 0;
	pDoc->CalculateScreenCoordinates(&p0);

	p1.x = pBackground->header.imageWidth;
	p1.y = pDoc->pageHeight;
	pDoc->CalculateScreenCoordinates(&p1);

	zx = (float)pDoc->pageWidth / (float)pBackground->header.imageWidth;
	zy = (float)pDoc->pageHeight / (float)pBackground->header.imageHeight;

	rx = (float)p0.x;
	ry = (float)drawRect.bottom-p1.y;

	//glRasterPos2f(rx, ry);
	glRasterPos2f(0, 0);
	glBitmap(0, 0, 0, 0, rx, ry, bitmap); //<-- Keith is insane

	glPixelZoom(pDoc->zoomVal * zx, pDoc->zoomVal * zy);

	glDrawPixels(pBackground->header.imageWidth, pBackground->header.imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, pBackground->bitmapBits);
	
	glPixelZoom(1.0, 1.0);
}

void btgGLGrabPixel(CBTGDoc *pDoc, CPoint point, unsigned char *colArray)
{
	glReadPixels(point.x, drawRect.bottom - point.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, colArray);
}