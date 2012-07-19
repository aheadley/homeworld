// BTGView.cpp : implementation of the CBTGView class
//

#include "stdafx.h"
#include "BTG.h"

#include "BTGDoc.h"
#include "BTGView.h"

#include "btggl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBTGView

IMPLEMENT_DYNCREATE(CBTGView, CView)

BEGIN_MESSAGE_MAP(CBTGView, CView)
	//{{AFX_MSG_MAP(CBTGView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_BTG_DELETESELECTED, OnBtgDeleteselected)
	ON_COMMAND(ID_BTG_PREVPOLYEDGE, OnBtgPrevpolyedge)
	ON_COMMAND(ID_BTG_NEXTPOLYEDGE, OnBtgNextpolyedge)
	ON_COMMAND(ID_BTG_SUBPOLYEDGE, OnBtgSubpolyedge)
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_BTG_MERGESELECTED, OnBtgMergeselected)
	ON_COMMAND(ID_BTG_SCROLLRIGHT, OnBtgScrollright)
	ON_COMMAND(ID_BTG_SCROLLLEFT, OnBtgScrollleft)
	ON_COMMAND(ID_BTG_SCROLLUP, OnBtgScrollup)
	ON_COMMAND(ID_BTG_SCROLLDOWN, OnBtgScrolldown)
    ON_COMMAND(ID_BTG_RESETZOOM, OnBtgResetZoom)
	ON_COMMAND(ID_BTG_ZOOMIN, OnBtgZoomin)
	ON_COMMAND(ID_BTG_ZOOMOUT, OnBtgZoomout)
	ON_COMMAND(ID_BTG_BRIGHTNESS, OnBtgBrightness)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_BTG_CLEARSELECTED, OnBtgClearSelected)
	ON_COMMAND(ID_BTG_VERTEXMODE, OnBtgVertexmode)
	ON_COMMAND(ID_BTG_POLYMODE, OnBtgPolymode)
	ON_COMMAND(ID_BTG_STARMODE, OnBtgStarmode)
	ON_COMMAND(ID_BTG_TOGGLEDRAWMODE, OnBtgToggledrawmode)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_COMMAND(ID_EDIT_UNDO, OnBtgUndo)
	ON_COMMAND(ID_EDIT_REDO, OnBtgRedo)
    ON_COMMAND(ID_EDIT_COPY, OnBtgEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, OnBtgEditPaste)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBTGView construction/destruction

CBTGView::CBTGView()
{
}

CBTGView::~CBTGView()
{
}

BOOL CBTGView::PreCreateWindow(CREATESTRUCT& cs)
{
	// Add the vert and horiz scroll bar.
	cs.style |= WS_VSCROLL | WS_HSCROLL;

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CBTGView drawing

void CBTGView::OnDraw(CDC* pDC)
{
	CBTGDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Set up the GL stuff if necessary.
	if (pDoc->renderMode == CBTGDoc::btgRenderMode_Realistic)
	{
		btgGLInit(this);
	}
	else if (pDoc->renderMode == CBTGDoc::btgRenderMode_3D)
	{
		btgGLInit3D(this);
		btgGLDraw3D(pDoc, this);
	}

	if(pDoc->drawBackground)
	{
		btgGLDrawBackground(pDoc);
	}

	btgpRDraw(pDoc);
	
	if(pDoc->drawMode != CBTGDoc::btgDrawMode_Basic)
	{
		// Draw all BTGVertices.
		if (pDoc->renderMode == CBTGDoc::btgRenderMode_Preview)
		{
			btgvPDraw(pDC);
		}
		else if (pDoc->renderMode == CBTGDoc::btgRenderMode_Realistic)
		{
			btgvRDraw(pDoc);
		}
	}

	if(pDoc->bStars)
	{
		// Draw all the BTGStars.
		if (pDoc->renderMode == CBTGDoc::btgRenderMode_Preview)
		{
			btgsPDraw(pDC);
		}
		else if (pDoc->renderMode == CBTGDoc::btgRenderMode_Realistic)
		{
			btgsRDraw(pDoc);
		}
	}

	if(pDoc->bandBoxEnabled)
	{
		if (pDoc->renderMode == CBTGDoc::btgRenderMode_Preview)
		{
			bandBoxPDraw(pDC);
		}
		else if (pDoc->renderMode == CBTGDoc::btgRenderMode_Realistic)
		{
			bandBoxRDraw(pDoc);
		}
	}

	if (pDoc->renderMode == CBTGDoc::btgRenderMode_Preview)
	{
		pageSizePDraw(pDC);
	}
	else if (pDoc->renderMode == CBTGDoc::btgRenderMode_Realistic)
	{
		pageSizeRDraw(pDoc);
	}

	// Close down the GL stuff if necessary.
	if (pDoc->renderMode != CBTGDoc::btgRenderMode_Preview)
	{
		btgGLClose(this);
	}
}

void CBTGView::pageSizePDraw(CDC *pDC)
{
	CBTGDoc *pDoc = GetDocument();
	CPoint tempPoint(pDoc->pageWidth, pDoc->pageHeight);
	CPoint basePoint(0, 0);

	pDoc->CalculateScreenCoordinates(&tempPoint);
	pDoc->CalculateScreenCoordinates(&basePoint);

	pDC->SelectStockObject(WHITE_PEN);
	pDC->SelectStockObject(NULL_BRUSH);

	pDC->Rectangle(basePoint.x, basePoint.y, tempPoint.x, tempPoint.y);
}

void CBTGView::bandBoxPDraw(CDC *pDC)
{
	CBTGDoc *pDoc = GetDocument();
		
	CPen bandBoxPen(PS_DASH, 1, RGB(0xFF, 0xFF, 0xFF));
	
	pDC->SelectObject(&bandBoxPen);
	pDC->SelectStockObject(NULL_BRUSH); 

	pDC->Rectangle(pDoc->bandBoxAnchor.x, pDoc->bandBoxAnchor.y, pDoc->bandBoxFloat.x, pDoc->bandBoxFloat.y);
}

void CBTGView::btgvPDraw(CDC *pDC)
{
	CBTGDoc *pDocument;
	BTGVertex *pVertex;
	CPoint p;
	
	pDocument = GetDocument();
	pVertex = pDocument->GetFirstBTGVertex();

	while(pVertex)
	{
		if(pVertex->bVisible == FALSE)
		{
			pVertex = pVertex->GetNext();

			continue;
		}
		
		// Get this vertex's colors.
		CPen	standardPen(PS_SOLID, 1, RGB(0xFF, 0xFF, 0xFF));
		CPen	selectedPen(PS_SOLID, 2, RGB(0, 0xFF, 0xFF));
		CBrush	interiorBrush(RGB(pVertex->red, pVertex->green, pVertex->blue));

		pDC->SelectObject(&interiorBrush);
		
		// Get this vertex's screen coords.
		p.x = (long)pVertex->x; 
		p.y = (long)pVertex->y;
		pDocument->CalculateScreenCoordinates(&p);
		
		// Is this vertex selected?
		if(bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
		{
			pDC->SelectObject(&selectedPen);
		}
		else
		{
			pDC->SelectObject(&standardPen);
		}

		// Draw a rectangle around the vertex.
		pDC->Rectangle(p.x - BTGV_SIZE2, p.y - BTGV_SIZE2, p.x + BTGV_SIZE2, p.y + BTGV_SIZE2);

		pVertex = pVertex->GetNext();
	}
}

void CBTGView::btgsPDraw(CDC *pDC)
{
	CBTGDoc *pDocument;
	BTGStar *pStar;
	CPoint p;
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(pDC);
		
	pDocument = GetDocument();
	pStar = pDocument->GetFirstBTGStar();

	pDC->SelectStockObject(HOLLOW_BRUSH);
	
	while(pStar)
	{
		if(pStar->bVisible == FALSE)
		{
			pStar = pStar->GetNext();

			continue;
		}

		p.x = (long)pStar->x; 
		p.y = (long)pStar->y;
		pDocument->CalculateScreenCoordinates(&p);

		if(pStar->pMyStar)
		{
			// Get this star's colors.
			CPen	standardPen(PS_SOLID, 1, RGB(pStar->red, pStar->green, pStar->blue));
			CPen	selectedPen(PS_SOLID, 2, RGB(pStar->red, pStar->green, pStar->blue));	
			
			// Get this star's screen coords.
			p.x = (long)pStar->x; 
			p.y = (long)pStar->y;
			pDocument->CalculateScreenCoordinates(&p);
			
			// Is this star selected?
			if(bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
			{
				pDC->SelectObject(&selectedPen);
			}
			else
			{
				pDC->SelectObject(&standardPen);
			}

			// Draw a Ellipse around the Star.
			pDC->Ellipse(p.x - pStar->pMyStar->myFile.header.imageWidth / 2, 
				p.y - pStar->pMyStar->myFile.header.imageHeight / 2, 
				p.x + pStar->pMyStar->myFile.header.imageWidth / 2, 
				p.y + pStar->pMyStar->myFile.header.imageHeight / 2);
		}

		else
		{
			// Get this star's colors.
			CPen	standardPen(PS_SOLID, 1, RGB(pStar->red, pStar->green, pStar->blue));
			CPen	selectedPen(PS_SOLID, 2, RGB(pStar->red, pStar->green, pStar->blue));	
			
			// Get this star's screen coords.
			p.x = (long)pStar->x; 
			p.y = (long)pStar->y;
			pDocument->CalculateScreenCoordinates(&p);
			
			// Is this star selected?
			if(bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
			{
				pDC->SelectObject(&selectedPen);
			}
			else
			{
				pDC->SelectObject(&standardPen);
			}

			// Draw a ellipse around the Star.
			pDC->Ellipse(p.x - BTGS_SIZE2, p.y - BTGS_SIZE2, p.x + BTGS_SIZE2, p.y + BTGS_SIZE2);
			pDC->MoveTo(p.x - BTGS_SIZE2, p.y - BTGS_SIZE2);
			pDC->LineTo(p.x + BTGS_SIZE2, p.y + BTGS_SIZE2);

			pDC->MoveTo(p.x + BTGS_SIZE2, p.y - BTGS_SIZE2);
			pDC->LineTo(p.x - BTGS_SIZE2, p.y + BTGS_SIZE2);
		}

		pStar = pStar->GetNext();
	}
}

void CBTGView::btgpPDraw(CDC *pDC)
{
	CBTGDoc *pDocument;
	BTGPolygon *pPolygon;
	CPoint p0, p1, p2;
	CPen *regEdgePen;
		
	CPen	standardPen(PS_SOLID, 1, RGB(181, 182, 181));
	CPen	selectedPen(PS_SOLID, 2, RGB(181, 182, 181));	
	CPen	selEdgePen(PS_SOLID, 2, RGB(0, 255, 0));

	pDocument = GetDocument();
	pPolygon = pDocument->GetFirstBTGPolygon();

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

		pDocument->CalculateScreenCoordinates(&p0);
		pDocument->CalculateScreenCoordinates(&p1);
		pDocument->CalculateScreenCoordinates(&p2);

		// Is this polygon selected?
		if(bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected))
		{
			regEdgePen = &selectedPen;
		}
		else
		{
			regEdgePen = &standardPen;
		}

		if((pDocument->btgpSelectedEdge == 1) && (bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected)))
		{
			pDC->SelectObject(&selEdgePen);
		}
		else
		{
			pDC->SelectObject(regEdgePen);
		}

		// First Edge
		pDC->MoveTo(p0.x, p0.y);
		pDC->LineTo(p1.x, p1.y);

		if((pDocument->btgpSelectedEdge == 2) && (bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected)))
		{
			pDC->SelectObject(&selEdgePen);
		}
		else
		{
			pDC->SelectObject(regEdgePen);
		}

		// Second Edge
		pDC->LineTo(p2.x, p2.y);

		if((pDocument->btgpSelectedEdge == 3) && (bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected)))
		{
			pDC->SelectObject(&selEdgePen);
		}
		else
		{
			pDC->SelectObject(regEdgePen);
		}

		// Third Edge
		pDC->LineTo(p0.x, p0.y);

		pPolygon = pPolygon->GetNext();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBTGView printing

BOOL CBTGView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CBTGView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CBTGView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CBTGView diagnostics

#ifdef _DEBUG
void CBTGView::AssertValid() const
{
	CView::AssertValid();
}

void CBTGView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CBTGDoc* CBTGView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBTGDoc)));
	return (CBTGDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBTGView message handlers

void CBTGView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CBTGDoc *pDoc = GetDocument();
	unsigned char colArray[4];

	if(pDoc->bgPixelGrabEnabled == TRUE)
	{
		btgGLGrabPixel(pDoc, point, colArray);

		pDoc->GrabBGPixel(colArray);

		return;
	}

	if (pDoc->renderMode == CBTGDoc::btgRenderMode_3D)
	{
		pDoc->EnableDragMode(point);
		goto END_OF_LBUTTON_DOWN;
	}

	pDoc->btgSaveForUndo();

	if (GetKeyState(VK_CONTROL) & 0x8000)
	{
		pDoc->EnableDragMode(point);
		goto END_OF_LBUTTON_DOWN;
	}

	switch(pDoc->btgDocMode)
	{
	case CBTGDoc::btgDocMode_EditBTGVertices:
		// Did we click on an existing point?
		if(pDoc->btgvCheckExisting(point))
		{
			// Is it selected?
			if(!bitTest(pDoc->btgvCheckExisting(point)->flags, BTGVertex::btgvFlags_Selected))
			{
				// New click.  Clear selection.
				pDoc->btgvClearSelected();
			}
			
			// Yes.  Select it.
			pDoc->btgvSelect(point);

			// Available for dragging.
			pDoc->EnableDragMode(point);
		}
		else
		{
			// Clear all selected points.
			pDoc->btgvClearSelected();

			// Create a new point
			pDoc->btgvCreateNew(point);

			// Available for dragging.
			pDoc->EnableDragMode(point);
		}
		break;

	case CBTGDoc::btgDocMode_EditBTGStars:
		// Did we click on an existing star?
		if(pDoc->btgsCheckExisting(point))
		{
			// Is it selected?
			if(!bitTest(pDoc->btgsCheckExisting(point)->flags, BTGStar::btgsFlags_Selected))
			{
				// New click.  Clear selection.
				pDoc->btgsClearSelected();
			}
			
			// Yes.  Select it.
			pDoc->btgsSelect(point);

			// Available for dragging.
			pDoc->EnableDragMode(point);
		}
		else
		{
			// Clear all selected stars.
			pDoc->btgsClearSelected();

			// Create a new star
			pDoc->btgsCreateNew(point);

			// Available for dragging.
			pDoc->EnableDragMode(point);
		}
		break;

	case CBTGDoc::btgDocMode_EditBTGPolygons:
		// Did we click on an existing point?
		if(pDoc->btgvCheckExisting(point))
		{
			// Clear all selected polygons.
			pDoc->btgpClearSelected();

			// Add it to the list of vertices considered for this polygon.
			pDoc->btgpConsiderBTGVertex(point);
		}

		// Did we click on an existing polygon?
		else if(pDoc->btgpCheckExisting(point))
		{
			if(!bitTest(pDoc->btgpCheckExisting(point)->flags, BTGPolygon::btgpFlags_Selected))
			{
				pDoc->btgpClearSelected();
			}

			// Yes.  Select it.
			pDoc->btgpSelect(point);

			// Available for dragging.
			pDoc->EnableDragMode(point);
		}
		break;
	}
	
END_OF_LBUTTON_DOWN:
	// Reflect the changes.
	Invalidate();
	
	CView::OnLButtonDown(nFlags, point);
}

void CBTGView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CBTGDoc *pDoc = GetDocument();

	if(pDoc->bgPixelGrabEnabled == TRUE)
	{
		pDoc->bgPixelGrabEnabled = FALSE;
	}

	pDoc->DisableDragMode();

	Invalidate();

	CView::OnLButtonUp(nFlags, point);
}

void CBTGView::OnBtgUndo()
{
	CBTGDoc *pDoc = GetDocument();

//	if (pDoc->IsModified())
	{
		pDoc->btgDocUndo();
	}
	
	Invalidate();
}

void CBTGView::OnBtgRedo()
{
	CBTGDoc *pDoc = GetDocument();

	pDoc->btgDocRedo();

	Invalidate();
}

void CBTGView::OnBtgEditCopy()
{
    CBTGDoc* pDoc = GetDocument();
    pDoc->Copy();
    Invalidate();
}

void CBTGView::OnBtgEditPaste()
{
    CBTGDoc* pDoc = GetDocument();
    pDoc->Paste();
    Invalidate();
}

void CBTGView::OnBtgDeleteselected() 
{
	CBTGDoc *pDoc = GetDocument();
	
	pDoc->EverythingDeleteSelected();

	Invalidate();
}

void CBTGView::OnBtgPrevpolyedge() 
{
	CBTGDoc *pDoc = GetDocument();

	switch(pDoc->btgDocMode)
	{
	case CBTGDoc::btgDocMode_EditBTGPolygons:
		pDoc->btgpPrevEdge();
		break;
	}

	Invalidate();
}

void CBTGView::OnBtgNextpolyedge() 
{
	CBTGDoc *pDoc = GetDocument();

	switch(pDoc->btgDocMode)
	{
	case CBTGDoc::btgDocMode_EditBTGPolygons:
		pDoc->btgpNextEdge();
		break;
	}

	Invalidate();
}

void CBTGView::OnBtgSubpolyedge() 
{
	CBTGDoc *pDoc = GetDocument();
	
	switch(pDoc->btgDocMode)
	{
	case CBTGDoc::btgDocMode_EditBTGPolygons:
		pDoc->btgpSubEdge();
		break;
	}

	Invalidate();
}

void CBTGView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CBTGDoc *pDoc = GetDocument();

	if (pDoc->renderMode != CBTGDoc::btgRenderMode_3D)
	{
		pDoc->EnableBandBox(point);
		Invalidate();
	}

	CView::OnRButtonDown(nFlags, point);
}

void CBTGView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CBTGDoc *pDoc = GetDocument();

	if (pDoc->renderMode != CBTGDoc::btgRenderMode_3D)
	{
		pDoc->UpdateBandBox(point);
		pDoc->DisableBandBox();
		Invalidate();
	}
	
	CView::OnRButtonUp(nFlags, point);
}

void CBTGView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBTGDoc *pDoc = GetDocument();

	if (pDoc->bandBoxEnabled)
	{
		pDoc->CalculateBandBox(point);
		Invalidate();
	}

	if (pDoc->dragEnabled)
	{
		pDoc->UpdateDragMode(point);
		Invalidate();
	}
	
	CView::OnMouseMove(nFlags, point);
}

void CBTGView::OnBtgMergeselected() 
{
	CBTGDoc *pDoc = GetDocument();
	
	pDoc->btgvMergeSelected();

	Invalidate();
}

void CBTGView::OnBtgScrollright() 
{
	CBTGDoc *pDoc = GetDocument();

	if (pDoc->renderMode == CBTGDoc::btgRenderMode_3D)
	{
		return;
	}

	pDoc->xScrollVal += 16;

	ScrollWindow(16, 0);
}

void CBTGView::OnBtgScrollleft() 
{
	CBTGDoc *pDoc = GetDocument();

	if (pDoc->renderMode == CBTGDoc::btgRenderMode_3D)
	{
		return;
	}

	pDoc->xScrollVal -= 16;

	ScrollWindow(-16, 0);
}

void CBTGView::OnBtgScrollup() 
{
	CBTGDoc *pDoc = GetDocument();

	if (pDoc->renderMode == CBTGDoc::btgRenderMode_3D)
	{
		return;
	}

	pDoc->yScrollVal -= 16;

	ScrollWindow(0, -16);
}

void CBTGView::OnBtgScrolldown() 
{
	CBTGDoc *pDoc = GetDocument();

	if (pDoc->renderMode == CBTGDoc::btgRenderMode_3D)
	{
		return;
	}

	pDoc->yScrollVal += 16;

	ScrollWindow(0, 16);
}

void CBTGView::OnBtgBrightness()
{
	CBTGDoc *pDoc = GetDocument();

	if (pDoc->renderMode == CBTGDoc::btgRenderMode_3D)
	{
		return;
	}

	pDoc->btgvUpdateBrightness(-1);
}

void CBTGView::OnBtgResetZoom()
{
    CBTGDoc *pDoc = GetDocument();

    pDoc->zoomVal = 0.5;
    pDoc->xScrollVal = 10;
    pDoc->yScrollVal = 10;

    Invalidate();
}

void CBTGView::OnBtgZoomin() 
{
	CBTGDoc *pDoc = GetDocument();
	CRect r;
	extern CRect oldRect;

	if (pDoc->renderMode == CBTGDoc::btgRenderMode_3D)
	{
		return;
	}

    pDoc->zoomVal *= 1.35f;
	
	r = oldRect;
	pDoc->xScrollVal -= (long)((double)(r.right - r.left) / 6.0 / pDoc->zoomVal);
	pDoc->yScrollVal -= (long)((double)(r.bottom - r.top) / 6.0 / pDoc->zoomVal);
	
	Invalidate();
}

void CBTGView::OnBtgZoomout() 
{
	CBTGDoc *pDoc = GetDocument();
	CRect r;
	extern CRect oldRect;

	if (pDoc->renderMode == CBTGDoc::btgRenderMode_3D)
	{
		return;
	}

	r = oldRect;
	pDoc->xScrollVal += (long)((double)(r.right - r.left) / 6.0 / pDoc->zoomVal);
	pDoc->yScrollVal += (long)((double)(r.bottom - r.top) / 6.0 / pDoc->zoomVal);

    pDoc->zoomVal /= 1.35f;

	Invalidate();
}

void CBTGView::OnBtgClearSelected()
{
	CBTGDoc *pDoc = GetDocument();

	pDoc->EverythingClearSelected();

	Invalidate();
}

BOOL CBTGView::OnEraseBkgnd(CDC* pDC) 
{
	CBTGDoc *pDoc = GetDocument();
	CRect tempRect;

	if (pDoc->renderMode != CBTGDoc::btgRenderMode_Preview)
	{
		return(1);
	}

	GetClientRect(&tempRect);

	pDC->FillSolidRect(&tempRect, RGB(0, 0, 0));

	return(1);
}

void CBTGView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

void CBTGView::OnBtgVertexmode() 
{
	CBTGDoc *pDoc = GetDocument();
	
	pDoc->btgDocMode = CBTGDoc::btgDocMode_EditBTGVertices;

	// Ensure we don't carry any baggage to new mode.
	pDoc->EverythingClearSelected();

	// Update the screen.
	Invalidate();
}

void CBTGView::OnBtgPolymode() 
{
	CBTGDoc *pDoc = GetDocument();
	
	pDoc->btgDocMode = CBTGDoc::btgDocMode_EditBTGPolygons;

	// Ensure we don't carry any baggage to new mode.
	pDoc->EverythingClearSelected();

	// Update the screen.
	Invalidate();
}

void CBTGView::OnBtgStarmode() 
{
	CBTGDoc *pDoc = GetDocument();
	
	pDoc->btgDocMode = CBTGDoc::btgDocMode_EditBTGStars;

	// Ensure we don't carry any baggage to new mode.
	pDoc->EverythingClearSelected();

	// Update the screen.
	Invalidate();
}

void CBTGView::OnBtgToggledrawmode() 
{
	CBTGDoc *pDoc = GetDocument();
	
	pDoc->drawMode ++;

	if(pDoc->drawMode >= CBTGDoc::btgDrawMode_LastDrawMode)
		pDoc->drawMode = CBTGDoc::btgDrawMode_Basic;

	Invalidate();
}
