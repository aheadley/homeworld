// BTGDoc.cpp : implementation of the CBTGDoc class
//

#include "stdafx.h"
#include "assert.h"
#include "afxdlgs.h"

#include "BTG.h"

#include "BTGDoc.h"
#include "mainfrm.h"

#include "BTGVertex.h"
#include "BTGPolygon.h"
#include "BTGStar.h"
#include "PageSizeDlg.h"
#include "tgacontainer.h"

#include "btggl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CBTGDoc

IMPLEMENT_DYNCREATE(CBTGDoc, CDocument)

BEGIN_MESSAGE_MAP(CBTGDoc, CDocument)
	//{{AFX_MSG_MAP(CBTGDoc)
	ON_COMMAND(ID_BTG_PAGE_SIZE, OnBtgPageSize)
	ON_COMMAND(ID_BTG_LOAD_BACKGROUND, OnBtgLoadBackground)
	ON_COMMAND(ID_BTG_TOGGLESOLID, OnBtgTogglesolid)
	ON_COMMAND(ID_BTG_TOGGLEBACKGROUND, OnBtgTogglebackground)
	ON_COMMAND(ID_BTG_REFERENCEBGPIXEL, OnBtgReferencebgpixel)
	ON_COMMAND(ID_BTG_HIDESELECTED, OnBtgHideselected)
	ON_COMMAND(ID_BTG_UNHIDEALL, OnBtgUnhideall)
	ON_COMMAND(ID_BTG_INVERTSEL, OnBtgInvertsel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBTGDoc construction/destruction

CBTGDoc::CBTGDoc()
{
    memset(&backgroundImage, 0x00, sizeof(TGAFile));
	
	undoBufferIndex = 0;
	for(long i=0;i<BTG_NUM_UNDOS;i++)
	{
		undoBuffers[i] = 0x00;
	}
    
	undoing = 0;
    Reset();
    undoSequence = 0;

    copyVertexList.PurgeList();
    copyPolygonList.PurgeList();

    gBTGDoc = this;
}

CBTGDoc::~CBTGDoc()
{
	KillTGAFile(&backgroundImage);

	for(long i=0;i<BTG_NUM_UNDOS;i++)
	{
		if(undoBuffers[i])
		{
			delete undoBuffers[i];
			undoBuffers[i] = 0;
		}
	}
}

void CBTGDoc::Reset(void)
{
	// Start in this mode by default.  Remember to set up menus.
	if (!undoing)
	{
		btgDocMode = btgDocMode_EditBTGVertices;

		undoBufferIndex = 0;

		for(long i=0;i<BTG_NUM_UNDOS;i++)
		{
			if(undoBuffers[i])
			{
				delete undoBuffers[i];
				undoBuffers[i] = 0;
			}
		}
	}

	xScrollVal = yScrollVal = 10;
	zoomVal = 0.5;

	EverythingClearSelected();

	DisableBandBox();

	DisableDragMode();

	mRed = mGreen = mBlue = mAlpha = mBrightness = 255;
	mBGRed = mBGGreen = mBGBlue = 0;

	pageWidth = 2*1280;
	pageHeight = 2*1024;

	BTGVertexList.PurgeList();
	BTGPolygonList.PurgeList();
	BTGStarList.PurgeList();
	//gBMPFileList.PurgeList();

	bStars = 1;
	renderMode = btgRenderMode_Realistic;

	if(undoing == 0)
	{
		drawMode = btgDrawMode_Basic;
	}

	bDisableModified = 0;

	UpdateAllViews(NULL);

	UpdateMainFrmDialogs();

	curStarFileName = 0;

	if(undoing == 0)
	{
		KillTGAFile(&backgroundImage);

		drawSolidPolys = TRUE;
		drawBackground = TRUE;
		bgPixelGrabEnabled = FALSE;
	}
}

BOOL CBTGDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	Reset();

	SetPathName("Untitled", FALSE);

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CBTGDoc serialization

void CBTGDoc::Serialize(CArchive& ar)
{
	undoing = 0;

	if (ar.IsStoring())
	{
		btgSaveNative(ar);
	}
	else
	{
		undoSequence = 0;
		btgLoadNative(ar);
		btgSaveForUndo();
	}
}

double btgpArea(BTGPolygon* pPolygon)
{
    double area;

	area  = (pPolygon->v0->x - pPolygon->v1->x) * (pPolygon->v0->y + pPolygon->v1->y);
	area += (pPolygon->v1->x - pPolygon->v2->x) * (pPolygon->v1->y + pPolygon->v2->y);
	area += (pPolygon->v2->x - pPolygon->v0->x) * (pPolygon->v2->y + pPolygon->v0->y);

    return area;
}

void CBTGDoc::btgFixupPolys()
{
    BTGPolygon* pPolygon;
    BTGPolygon* pDeadPolygon;

    pPolygon = GetFirstBTGPolygon();
    while (pPolygon)
    {
        if (btgpArea(pPolygon) == 0.0)
        {
			pDeadPolygon = pPolygon;
			pPolygon = pPolygon->GetNext();
			BTGPolygonList.PurgeNode(pDeadPolygon);

            continue;
        }

        pPolygon = pPolygon->GetNext();
    }
}

/*
 FILE VERSIONS (hex):
  500 - added version number in header.
		added brightness to vertices.
  600 - added background colour.
 */

void CBTGDoc::btgSaveNative(CArchive& ar)
{
	BTGVertex *pVertex;
	BTGPolygon *pPolygon;
	BTGStar *pStar;
	unsigned long btgFileVersion = 0x600;

    // purge zero area polygons
    btgFixupPolys();

	// Write out the header.
	ar << btgFileVersion;
	ar << BTGVertexList.GetNumElements();
	ar << BTGStarList.GetNumElements();
	ar << BTGPolygonList.GetNumElements();

	ar << xScrollVal;
	ar << yScrollVal;
	ar << zoomVal;
	ar << pageWidth;
	ar << pageHeight;
	ar << mRed;
	ar << mGreen;
	ar << mBlue;
	ar << mBGRed;
	ar << mBGGreen;
	ar << mBGBlue;
	ar << (BOOL)0; // Not used anymore, but pad out so we don't have to change file format.
	ar << (BOOL)0;
	ar << bStars;
	ar << (BOOL)0;
	ar << (BOOL)0;
	ar << renderMode;
		
	// Write out the vertices.
	pVertex = GetFirstBTGVertex();

	while(pVertex)
	{
		ar << pVertex->flags;
		ar << pVertex->x;
		ar << pVertex->y;
		ar << pVertex->red;
		ar << pVertex->green;
		ar << pVertex->blue;
		ar << pVertex->alpha;
		ar << pVertex->brightness;

		pVertex = pVertex->GetNext();
	}

	// Write out the stars.
	pStar = GetFirstBTGStar();

	while(pStar)
	{
		CString tempStr;

		ar << pStar->flags;
		ar << pStar->x;
		ar << pStar->y;
		ar << pStar->red;
		ar << pStar->green;
		ar << pStar->blue;
		ar << pStar->alpha;
		
		if (pStar->pMyStar)
		{
			tempStr = pStar->pMyStar->myFileName;
		}
		else
		{
			tempStr = "<untitled>";
		}
		ar << (unsigned int)tempStr.GetLength();
		for (int i = 0; i < tempStr.GetLength(); i++)
		{
			ar << (char)tempStr[i];
		}

		pStar = pStar->GetNext();
	}

	// Write out the polygons.
	pPolygon = GetFirstBTGPolygon();

	while(pPolygon)
	{
		ar << pPolygon->flags;
		ar << BTGVertexList.GetElementIndex(pPolygon->v0);
		ar << BTGVertexList.GetElementIndex(pPolygon->v1);
		ar << BTGVertexList.GetElementIndex(pPolygon->v2);
	
		pPolygon = pPolygon->GetNext();
	}
}

void CBTGDoc::btgLoadNative(CArchive& ar)
{
	BTGVertex *pVertex;
	BTGPolygon *pPolygon;
	BTGStar *pStar;
	unsigned long nVerts, nPolys, nStars, i, iV0, iV1, iV2;
	BOOL oStars;
	signed long oRenderMode;
	unsigned long btgFileVersion;
	BOOL temp;

	oRenderMode = renderMode;
	oStars = bStars;
	
	// Ensure document modified doesn't take place.
	bDisableModified = 1;
	
	// Clear out the existing data.
	Reset();

	// Read in the header.
	ar >> btgFileVersion;
	if (btgFileVersion != 0x500 &&
		btgFileVersion != 0x600)
	{
		nVerts = btgFileVersion;
		btgFileVersion = 0;
	}
	else
	{
		ar >> nVerts;
	}
	ar >> nStars;
	ar >> nPolys;

	ar >> xScrollVal;
	ar >> yScrollVal;
	ar >> zoomVal;
	ar >> pageWidth;
	ar >> pageHeight;
	ar >> mRed;
	ar >> mGreen;
	ar >> mBlue;
	if (btgFileVersion >= 0x600)
	{
		ar >> mBGRed;
		ar >> mBGGreen;
		ar >> mBGBlue;
	}
	else
	{
		mBGRed = mBGGreen = mBGBlue = 0;
	}
	ar >> temp; // Not used anymore, but pad out so we don't have to change file format.
	ar >> temp;
	ar >> bStars;
	ar >> temp;
	ar >> temp;
	ar >> renderMode;

	// Read in the verts.
	for( i=0 ; i<nVerts ; i++ )
	{
		pVertex = new BTGVertex;

		ar >> pVertex->flags;
		ar >> pVertex->x;
		ar >> pVertex->y;
		ar >> pVertex->red;
		ar >> pVertex->green;
		ar >> pVertex->blue;
		ar >> pVertex->alpha;
		if (btgFileVersion >= 0x500)
		{
			ar >> pVertex->brightness;
		}
		else
		{
			pVertex->brightness = 255;
		}

		BTGVertexList.AddNode(pVertex);
	}

	// Read in the stars.
	for( i=0 ; i<nStars ; i++ )
	{
		int length;
		char ch;

		pStar = new BTGStar("<untitled>");

		ar >> pStar->flags;
		ar >> pStar->x;
		ar >> pStar->y;
		ar >> pStar->red;
		ar >> pStar->green;
		ar >> pStar->blue;
		ar >> pStar->alpha;

		// Load in and resolve the name.
		CString tempStr;
		CTGAContainer *pContainer = (CTGAContainer *)gTGAFileList.GetHead();

		ar >> length;
		tempStr = "";
		for (int j = 0; j < length; j++)
		{
			ar >> ch;
			tempStr += ch;
		}

		while(pContainer)
		{
			if(!strcmp(tempStr, pContainer->myFileName))
			{
				pStar->pMyStar = pContainer;
				break;
			}

			pContainer = (CTGAContainer *)pContainer->GetNext();
		}
		
		BTGStarList.AddNode(pStar);
	}

	// Read in the polygons.
	for( i=0 ; i<nPolys ; i++ )
	{
		pPolygon = new BTGPolygon;

		ar >> pPolygon->flags;
		ar >> iV0;
		ar >> iV1;
		ar >> iV2;

		pPolygon->v0 = (BTGVertex *)BTGVertexList.GetNthElement(iV0);
		pPolygon->v1 = (BTGVertex *)BTGVertexList.GetNthElement(iV1);
		pPolygon->v2 = (BTGVertex *)BTGVertexList.GetNthElement(iV2);

		BTGPolygonList.AddNode(pPolygon);
	}

	// Allow document modification to take place now.
	bDisableModified = 0;

	if (undoing)
	{
		renderMode = oRenderMode;
		bStars = oStars;
		
		undoing = 0;
	}

	UpdateMainFrmDialogs();

	EverythingClearSelected();
}

/////////////////////////////////////////////////////////////////////////////
// CBTGDoc diagnostics

#ifdef _DEBUG
void CBTGDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBTGDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBTGDoc commands

// Vertex...
BTGVertex *CBTGDoc::GetFirstBTGVertex(void)
{
	return((BTGVertex *)BTGVertexList.GetHead());
}

// Star...
BTGStar *CBTGDoc::GetFirstBTGStar(void)
{
	return((BTGStar *)BTGStarList.GetHead());
}

// Polygon...
BTGPolygon *CBTGDoc::GetFirstBTGPolygon(void)
{
	return((BTGPolygon *)BTGPolygonList.GetHead());
}

// Vertex...
BTGVertex *CBTGDoc::btgvGetFirstSelected(void)
{
	BTGVertex *pVertex;
	
	pVertex = GetFirstBTGVertex();

	while(pVertex)
	{
		if(bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
		{
			return(pVertex);
		}

		pVertex = pVertex->GetNext();
	}

	return(NULL);
}

// Star...
BTGStar *CBTGDoc::btgsGetFirstSelected(void)
{
	BTGStar *pStar;
	
	pStar = GetFirstBTGStar();

	while(pStar)
	{
		if(bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
		{
			return(pStar);
		}

		pStar = pStar->GetNext();
	}

	return(NULL);
}

// Polygon...
BTGPolygon *CBTGDoc::btgpGetFirstSelected(void)
{
	BTGPolygon *pPolygon;
	
	pPolygon = GetFirstBTGPolygon();

	while(pPolygon)
	{
		if(bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected))
		{
			return(pPolygon);
		}

		pPolygon = pPolygon->GetNext();
	}

	return(NULL);
}

// Vertex...
unsigned long CBTGDoc::btgvNumSelected(void)
{
	BTGVertex *pVertex;
	unsigned long nSelected = 0;

	pVertex = GetFirstBTGVertex();

	while(pVertex)
	{
		if(bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
		{
			nSelected ++;
		}

		pVertex = pVertex->GetNext();
	}

	return(nSelected);
}

// Star...
unsigned long CBTGDoc::btgsNumSelected(void)
{
	BTGStar *pStar;
	unsigned long nSelected = 0;

	pStar = GetFirstBTGStar();

	while(pStar)
	{
		if(bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
		{
			nSelected ++;
		}

		pStar = pStar->GetNext();
	}

	return(nSelected);
}

// Polygon...
unsigned long CBTGDoc::btgpNumSelected(void)
{
	BTGPolygon *pPolygon;
	unsigned long nSelected = 0;

	pPolygon = GetFirstBTGPolygon();

	while(pPolygon)
	{
		if(bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected))
		{
			nSelected ++;
		}

		pPolygon = pPolygon->GetNext();
	}

	return(nSelected);
}

unsigned long CBTGDoc::btgpNumExisting(void)
{
	return(BTGPolygonList.GetNumElements());
}

unsigned long CBTGDoc::btgvNumExisting(void)
{
	return(BTGVertexList.GetNumElements());
}

// Vertex...
BTGVertex *CBTGDoc::btgvCheckExisting(CPoint point)
{
	BTGVertex *pVertex;
    double epsilon;

    epsilon = BTGV_SIZE + BTGV_SIZE2;
    epsilon /= zoomVal;

	// Handle zoom / scroll...
	CalculateActualCoordinates(&point);

	pVertex = GetFirstBTGVertex();

	while(pVertex)
	{
		if(PointInRect(&point, pVertex->x - epsilon, pVertex->y - epsilon,
			pVertex->x + epsilon, pVertex->y + epsilon))
		{
			return(pVertex);
		}

		pVertex = pVertex->GetNext();
	}

	return(NULL);
}

// Star...
BTGStar *CBTGDoc::btgsCheckExisting(CPoint point)
{
	BTGStar *pStar;

	// Handle zoom / scroll...
	CalculateActualCoordinates(&point);

	pStar = GetFirstBTGStar();

	while(pStar)
	{
		if(pStar->pMyStar)
		{
			if(PointInRect(&point, pStar->x - pStar->pMyStar->myFile.header.imageWidth, 
				pStar->y - pStar->pMyStar->myFile.header.imageHeight, 
				pStar->x + pStar->pMyStar->myFile.header.imageWidth, 
				pStar->y + pStar->pMyStar->myFile.header.imageHeight))
			{
				return(pStar);
			}
		}
		else
		{
			if(PointInRect(&point, pStar->x - BTGV_SIZE, pStar->y - BTGV_SIZE,
				pStar->x + BTGV_SIZE, pStar->y + BTGV_SIZE))
			{
				return(pStar);
			}
		}

		pStar = pStar->GetNext();
	}

	return(NULL);
}

// Polygon...
BTGPolygon *CBTGDoc::btgpCheckExisting(CPoint point)
{
	BTGPolygon *pPolygon;

	// Handle zoom / scroll...
	CalculateActualCoordinates(&point);

	pPolygon = GetFirstBTGPolygon();

	while(pPolygon)
	{
		if(PointInPolygon(&point, pPolygon))
		{
			return(pPolygon);
		}

		pPolygon = pPolygon->GetNext();
	}

	return(NULL);
}

BTGPolygon *CBTGDoc::btgpCheckExisting(BTGVertex *pv0, BTGVertex *pv1, BTGVertex *pv2)
{
	BTGPolygon *pPolygon;

	pPolygon = GetFirstBTGPolygon();

	while(pPolygon)
	{
		if(((pv0 == pPolygon->v0) || (pv0 == pPolygon->v1) || (pv0 == pPolygon->v2)) &&
			((pv1 == pPolygon->v0) || (pv1 == pPolygon->v1) || (pv1 == pPolygon->v2)) &&
			((pv2 == pPolygon->v0) || (pv2 == pPolygon->v1) || (pv2 == pPolygon->v2)))
		{
			return(pPolygon);
		}

		pPolygon = pPolygon->GetNext();
	}

	return(NULL);
}

// Polygon...
BTGPolygon *CBTGDoc::btgpCheckUsing(BTGVertex *pv)
{
	BTGPolygon *pPolygon;

	pPolygon = GetFirstBTGPolygon();

	while(pPolygon)
	{
		if((pv == pPolygon->v0) || (pv == pPolygon->v1) || (pv == pPolygon->v2))
		{
			return(pPolygon);
		}

		pPolygon = pPolygon->GetNext();
	}

	return(NULL);
}

// Vertex...
BTGVertex *CBTGDoc::btgvCreateNew(CPoint point)
{
	BTGVertex *pVertex;

//	btgSaveForUndo();

	// Handle zoom / scroll...
	CalculateActualCoordinates(&point);

	// Make sure it's in the page.
	if(!OnPage(&point))
	{
		if(point.x < 0) point.x = 0;
		if(point.x >= pageWidth) point.x = pageWidth - 1;
		if(point.y < 0) point.y = 0;
		if(point.y >= pageHeight) point.y = pageHeight - 1;
	}

	// Make a new vertex and add it to the list.
	pVertex = new BTGVertex(&point);
	BTGVertexList.AddNode(pVertex);

	// Set the colors.
	pVertex->red = mRed;
	pVertex->green = mGreen;
	pVertex->blue = mBlue;
	pVertex->alpha = mAlpha;
	pVertex->brightness = 255;
	
	// Select this new one.
	btgvSelect(pVertex);

	// Let the document know it's been modified.
	if(!bDisableModified)
	{
		SetModifiedFlag();
	}

	return(pVertex);
}

// Star...
void CBTGDoc::btgsCreateNew(CPoint point)
{
	BTGStar *pStar;

//	btgSaveForUndo();

	// Handle zoom / scroll...
	CalculateActualCoordinates(&point);

	// Make a new star and add it to the list.
	pStar = new BTGStar(&point, curStarFileName);
	BTGStarList.AddNode(pStar);

	// Set the colors.
	pStar->red = mRed;
	pStar->green = mGreen;
	pStar->blue = mBlue;
	
	// Select this new one.
	btgsSelect(pStar);

	// Let the document know it's been modified.
	if(!bDisableModified)
	{
		SetModifiedFlag();
	}
}

// Polygon...
void CBTGDoc::btgpCreateNew(BTGVertex *pv0, BTGVertex *pv1, BTGVertex *pv2)
{
	BTGPolygon *pPolygon;

//	btgSaveForUndo();

	// Make a new polygon and add it to the list.
	pPolygon = new BTGPolygon(pv0, pv1, pv2);
	BTGPolygonList.AddNode(pPolygon);

	// Make it counter clockwise.
	btgpForceCounterclockwise(pPolygon);

	// Select this new one.
	btgpSelect(pPolygon);

	// Let the document know it's been modified.
	if(!bDisableModified)
	{
		SetModifiedFlag();
	}
}

// Vertex...
void CBTGDoc::btgvDeleteSelected(void)
{
	BTGVertex *pVertex, *pDeadVertex;
	BTGPolygon *pDeadPolygon;

	btgSaveForUndo();

	// Loop through all the vertices and if they're selected, delete them.
	pVertex = GetFirstBTGVertex();

	while(pVertex)
	{
		if(bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
		{
			// Let the document know it's been modified.
			if(!bDisableModified)
			{
				SetModifiedFlag();
			}

			pDeadPolygon = btgpCheckUsing(pVertex);

			while(pDeadPolygon)
			{
				BTGPolygonList.PurgeNode(pDeadPolygon);

				pDeadPolygon = btgpCheckUsing(pVertex);
			}
		
			pDeadVertex = pVertex;

			pVertex = pVertex->GetNext();

			BTGVertexList.PurgeNode(pDeadVertex);

			continue;
		}

		pVertex = pVertex->GetNext();
	}
}

// Star...
void CBTGDoc::btgsDeleteSelected(void)
{
	BTGStar *pStar, *pDeadStar;

	btgSaveForUndo();

	// Loop through all the vertices and if they're selected, delete them.
	pStar = GetFirstBTGStar();

	while(pStar)
	{
		if(bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
		{
			// Let the document know it's been modified.
			if(!bDisableModified)
			{
				SetModifiedFlag();
			}
			
			pDeadStar = pStar;

			pStar = pStar->GetNext();

			BTGStarList.PurgeNode(pDeadStar);

			continue;
		}

		pStar = pStar->GetNext();
	}
}

// Polygon...
void CBTGDoc::btgpDeleteSelected(void)
{
	BTGPolygon *pPolygon, *pDeadPolygon;

	btgSaveForUndo();

	pPolygon = GetFirstBTGPolygon();

	while(pPolygon)
	{
		if(bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected))
		{
			// Let the document know it's been modified.
			if(!bDisableModified)
			{
				SetModifiedFlag();
			}
			
			pDeadPolygon = pPolygon;

			pPolygon = pPolygon->GetNext();

			BTGPolygonList.PurgeNode(pDeadPolygon);

			continue;
		}

		pPolygon = pPolygon->GetNext();
	}
}

// Everything...
void CBTGDoc::EverythingDeleteSelected(void)
{
	btgvDeleteSelected();
	btgpDeleteSelected();
	btgsDeleteSelected();

	btgpClearConsideredBTGVertices();
}

// Polygon...
void CBTGDoc::btgpConsiderBTGVertex(CPoint point)
{
	BTGVertex *pVertex;

	pVertex = btgvCheckExisting(point);

	// The view thinks there is definitely something to select here, this is probably a bug.
	assert(pVertex);

	// Have we already considered this vertex?
	if((pVertex == pTempV[0]) || (pVertex == pTempV[1]) || (pVertex == pTempV[2]))
	{
		// Yes, so ignore it.
		return;
	}

	// No, so add it.
	nTempV ++;
	btgvSelect(point);
	pTempV[nTempV - 1] = pVertex;

	// We shouldn't have more than 3 selected at once.
	assert(nTempV <= 3);

	// Is it time to make this a polygon?
	if(nTempV == 3)
	{
		if(!btgpCheckExisting(pTempV[0], pTempV[1], pTempV[2]))
		{
			// create a polygon with these vertices.
			btgpCreateNew(pTempV[0], pTempV[1], pTempV[2]);

			btgpClearConsideredBTGVertices();
		}
		else
		{
			btgpClearConsideredBTGVertices();
		}
	}
}

// Polygon...
void CBTGDoc::btgpClearConsideredBTGVertices(void)
{
	nTempV = 0;
	pTempV[0] = pTempV[1] = pTempV[2] = NULL;
	btgvClearSelected();
}

// Vertex...
void CBTGDoc::btgvSelect(CPoint point)
{
	BTGVertex *pVertex;

	pVertex = btgvCheckExisting(point);
	btgvSelect(pVertex);
}

// Vertex...
void CBTGDoc::btgvSelect(BTGVertex *pVertex)
{
	CSliderCtrl *sRed, *sGreen, *sBlue, *sAlpha, *sBrightness;

	// The view thinks there is definitely something to select here, this is probably a bug.
	assert(pVertex);

	bitSet(pVertex->flags, BTGVertex::btgvFlags_Selected);

	if (bandBoxEnabled || btgvNumSelected() > 1)
	{
		return;
	}

	//get the handles we need
	sRed = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_REDSLIDER);
	sGreen = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_GREENSLIDER);
	sBlue = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_BLUESLIDER);
	sAlpha = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_ALPHASLIDER);
	sBrightness = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_BRIGHTNESSSLIDER);

	if (GetKeyState(VK_MENU) & 0x8000)
	{
		pVertex->red = sRed->GetPos();
		pVertex->green = sGreen->GetPos();
		pVertex->blue = sBlue->GetPos();
		pVertex->alpha = sAlpha->GetPos();
		pVertex->brightness = sBrightness->GetPos();
	}
	else
	{
		//update the slider positions
		sRed->SetPos(pVertex->red);
		sGreen->SetPos(pVertex->green);
		sBlue->SetPos(pVertex->blue);
		sAlpha->SetPos(pVertex->alpha);
		sBrightness->SetPos(pVertex->brightness);
	}
}

// Vertex...
void CBTGDoc::btgvUnSelect(BTGVertex *pVertex)
{
	// The view thinks there is definitely something to select here, this is probably a bug.
	assert(pVertex);

	bitClear(pVertex->flags, BTGVertex::btgvFlags_Selected);
}

// Star...
void CBTGDoc::btgsSelect(CPoint point)
{
	BTGStar *pStar;

	pStar = btgsCheckExisting(point);
	btgsSelect(pStar);
}

// Star...
void CBTGDoc::btgsSelect(BTGStar *pStar)
{
	CSliderCtrl *sRed, *sGreen, *sBlue, *sAlpha;

	// The view thinks there is definitely something to select here, this is probably a bug.
	assert(pStar);

	bitSet(pStar->flags, BTGStar::btgsFlags_Selected);

	if (bandBoxEnabled || btgsNumSelected() > 1)
	{
		return;
	}

	//get the handles we need
	sRed = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_REDSLIDER);
	sGreen = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_GREENSLIDER);
	sBlue = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_BLUESLIDER);
	sAlpha = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_ALPHASLIDER);
	
	if (GetKeyState(VK_MENU) & 0x8000)
	{
		pStar->red = sRed->GetPos();
		pStar->green = sGreen->GetPos();
		pStar->blue = sBlue->GetPos();
		pStar->alpha = sAlpha->GetPos();
	}
	else
	{
		//update the slider positions
		sRed->SetPos(pStar->red);
		sGreen->SetPos(pStar->green);
		sBlue->SetPos(pStar->blue);
		sAlpha->SetPos(pStar->alpha);
	}
}

void CBTGDoc::btgsUnSelect(BTGStar *pStar)
{
	// The view thinks there is definitely something to select here, this is probably a bug.
	assert(pStar);

	bitClear(pStar->flags, BTGStar::btgsFlags_Selected);
}

// Polygon...
void CBTGDoc::btgpSelect(CPoint point)
{
	BTGPolygon *pPolygon;

	pPolygon = btgpCheckExisting(point);

	// The view thinks there is definitely something to select here, this is probably a bug.
	assert(pPolygon);

	bitSet(pPolygon->flags, BTGPolygon::btgpFlags_Selected);
}

// Polygon...
void CBTGDoc::btgpSelect(BTGPolygon *pPolygon)
{
	// The view thinks there is definitely something to select here, this is probably a bug.
	assert(pPolygon);

	bitSet(pPolygon->flags, BTGPolygon::btgpFlags_Selected);
}

void CBTGDoc::btgpUnSelect(BTGPolygon *pPolygon)
{
	// The view thinks there is definitely something to select here, this is probably a bug.
	assert(pPolygon);

	bitClear(pPolygon->flags, BTGPolygon::btgpFlags_Selected);
}

// Vertex...
void CBTGDoc::btgvClearSelected(void)
{
	BTGVertex *pVertex;

	pVertex = GetFirstBTGVertex();

	while(pVertex)
	{
		bitClear(pVertex->flags, BTGVertex::btgvFlags_Selected);

		pVertex = pVertex->GetNext();
	}
}

// Star...
void CBTGDoc::btgsClearSelected(void)
{
	BTGStar *pStar;

	pStar = GetFirstBTGStar();

	while(pStar)
	{
		bitClear(pStar->flags, BTGStar::btgsFlags_Selected);

		pStar = pStar->GetNext();
	}
}

// Polygon...
void CBTGDoc::btgpClearSelected(void)
{
	BTGPolygon *pPolygon;

	pPolygon = GetFirstBTGPolygon();

	while(pPolygon)
	{
		bitClear(pPolygon->flags, BTGPolygon::btgpFlags_Selected);

		pPolygon = pPolygon->GetNext();
	}

	btgpSelectedEdge = 0;
}

// Everything...
void CBTGDoc::EverythingClearSelected(void)
{
	btgvClearSelected();
	btgpClearSelected();
	btgsClearSelected();
	btgpClearConsideredBTGVertices();
}

// Screen coords to Actual coords...
void CBTGDoc::CalculateActualCoordinates(CPoint *pPoint)
{
	pPoint->x -= (long)((float)xScrollVal * zoomVal);
	pPoint->y -= (long)((float)yScrollVal * zoomVal);

	pPoint->x = (long)((float)pPoint->x / zoomVal);
	pPoint->y = (long)((float)pPoint->y / zoomVal);
}

// Actual coords to Screen coords...
void CBTGDoc::CalculateScreenCoordinates(CPoint *pPoint)
{
	pPoint->x = (long)((float)pPoint->x * zoomVal);
	pPoint->y = (long)((float)pPoint->y * zoomVal);

	pPoint->x += (long)((float)xScrollVal * zoomVal);
	pPoint->y += (long)((float)yScrollVal * zoomVal);
}

BOOL CBTGDoc::PointInRect(CPoint *pPoint, double l, double t, double r, double b)
{
	CRect rect;

	rect.left = (long)l;
	rect.top = (long)t;
	rect.right = (long)r;
	rect.bottom = (long)b;

	return(rect.PtInRect(*pPoint));
}

BOOL CBTGDoc::PointInPolygon(CPoint *pPoint, BTGPolygon *pPolygon)
{
	double a, b, c, r0, r1, r2;

	a = pPolygon->v0->y - pPolygon->v1->y;
	b = pPolygon->v1->x - pPolygon->v0->x;
	c = pPolygon->v0->x * pPolygon->v1->y - pPolygon->v0->y * pPolygon->v1->x;

	r0 = a * pPoint->x + b * pPoint->y + c;

	a = pPolygon->v1->y - pPolygon->v2->y;
	b = pPolygon->v2->x - pPolygon->v1->x;
	c = pPolygon->v1->x * pPolygon->v2->y - pPolygon->v1->y * pPolygon->v2->x;

	r1 = a * pPoint->x + b * pPoint->y + c;

	a = pPolygon->v2->y - pPolygon->v0->y;
	b = pPolygon->v0->x - pPolygon->v2->x;
	c = pPolygon->v2->x * pPolygon->v0->y - pPolygon->v2->y * pPolygon->v0->x;

	r2 = a * pPoint->x + b * pPoint->y + c;

	if((r0 < 0) && (r1 < 0) && (r2 < 0))
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

void CBTGDoc::btgpForceCounterclockwise(BTGPolygon *pPolygon)
{
	double area;
	BTGVertex *pVertex;
    
	// Calculate the area.
    area = btgpArea(pPolygon);
    
	// Zero area polygon = bad polygon.  BAD.  BAD POLYGON!
	if(area == 0.0)
	{
		return;
	}
	
	// Hacky check for clockwise polygon.
	if(area > 0)
	{
		pVertex = pPolygon->v2;
		pPolygon->v2 = pPolygon->v1;
		pPolygon->v1 = pVertex;
	}
}

// Polygon...
void CBTGDoc::btgpPrevEdge(void)
{
	// Only do this if there's just one poly selected.
	if(btgpNumSelected() != 1)
	{
		return;
	}

	btgpSelectedEdge --;

	// btgpSelectedEdge is 1 based so 0 can disable it.
	if(btgpSelectedEdge < 1)
	{
		btgpSelectedEdge = 3;
	}
}

// Polygon...
void CBTGDoc::btgpNextEdge(void)
{
	// Only do this if there's just one poly selected.
	if(btgpNumSelected() != 1)
	{
		return;
	}

	btgpSelectedEdge ++;

	// btgpSelectedEdge is 1 based so 0 can disable it.
	if(btgpSelectedEdge > 3)
	{
		btgpSelectedEdge = 1;
	}
}

// Polygon...
void CBTGDoc::btgpSubEdge(void)
{
	BTGPolygon *pPolygon;
	BTGVertex *v0, *v1, *v2, *pNewVertex;
	CPoint newVertexCoord;
	
	// Only do this if there's just one poly selected.
	if(btgpNumSelected() != 1)
	{
		return;
	}

	// Only do this if there's an edge selected.
	if(btgpSelectedEdge == 0)
	{
		return;
	}

	// Get the selected polygon.  We ensured there's only one above.
	pPolygon = btgpGetFirstSelected();

	// Should be one at least.
	assert(pPolygon);

	// Find the appropriate edge.
	switch(btgpSelectedEdge)
	{
	case 1:
		v0 = pPolygon->v0;
		v1 = pPolygon->v1;
		v2 = pPolygon->v2;
		break;

	case 2:
		v0 = pPolygon->v1;
		v1 = pPolygon->v2;
		v2 = pPolygon->v0;
		break;

	case 3:
		v0 = pPolygon->v2;
		v1 = pPolygon->v0;
		v2 = pPolygon->v1;
		break;
	}

	// Find the point at the midpoint of the line.
	newVertexCoord.x = (long)((v1->x + v0->x) / 2);
	newVertexCoord.y = (long)((v1->y + v0->y) / 2);

	// btgvCreateNew expects screen coordinates.
	CalculateScreenCoordinates(&newVertexCoord);	
	
	// Create a new vertex at this point.
	pNewVertex = btgvCreateNew(newVertexCoord);

	// Make sure it's in the page.
	assert(pNewVertex);

	// Add the two new polygons.
	btgpCreateNew(pNewVertex, v1, v2);
	btgpCreateNew(pNewVertex, v0, v2);

	// Delete the old polygon.
	BTGPolygonList.PurgeNode(pPolygon);

	// Make sure nothing is selected.
	EverythingClearSelected();
}

void CBTGDoc::btgvMergeSelected(void)
{
	BTGVertex *pSave, *pWork, *pDead;
	BTGPolygon *pPolygon;
	CPoint pt;
	
	// Only do this if there is more than one vertex selected.
	if(btgvNumSelected() <= 1)
	{
		return;
	}

	btgSaveForUndo();

	// Get the first vertex and save it.
	pSave = btgvGetFirstSelected();

	while(pSave)
	{
		// Loop through all the other vertices to check them now.
		pWork = GetFirstBTGVertex();

		while(pWork)
		{
			// Don't check the save vertex against itself.
			if(pWork == pSave)
			{
				pWork = pWork->GetNext();
				continue;
			}

			// Don't check non-selected vertices.
			if(!bitTest(pWork->flags, BTGVertex::btgvFlags_Selected))
			{
				pWork = pWork->GetNext();
				continue;
			}

			pt.x = (long)pWork->x;
			pt.y = (long)pWork->y;

			if(PointInRect(&pt, pSave->x - 2*BTGV_SIZE2, pSave->y - 2*BTGV_SIZE2,
				pSave->x + 2*BTGV_SIZE2, pSave->y + 2*BTGV_SIZE2))
			{
				// The work point is a valid one to eliminate.
				// Find any polygons that use this vertex and replace it with pSave.
				pPolygon = GetFirstBTGPolygon();

				// Let the document know it's been modified.
				if(!bDisableModified)
				{
					SetModifiedFlag();
				}

				while(pPolygon)
				{
					if(pPolygon->v0 == pWork)
					{
						pPolygon->v0 = pSave;
					}

					if(pPolygon->v1 == pWork)
					{
						pPolygon->v1 = pSave;
					}

					if(pPolygon->v2 == pWork)
					{
						pPolygon->v2 = pSave;
					}

					pPolygon = pPolygon->GetNext();
				}

				pDead = pWork;

				pWork = pWork->GetNext();

				BTGVertexList.PurgeNode(pDead);

				continue;
			}

			pWork = pWork->GetNext();
		}

		// Deselect this vertex so we can move on.
		bitClear(pSave->flags, BTGVertex::btgvFlags_Selected);

		pSave = btgvGetFirstSelected();
	}
}

void CBTGDoc::EnableBandBox(CPoint point)
{
	bandBoxEnabled = 1;
	bandBoxAnchor = point;
	bandBoxFloat = point;
}

void CBTGDoc::DisableBandBox(void)
{
	bandBoxEnabled = 0;
	bandBoxAnchor.x = bandBoxAnchor.y = 0;
}

void CBTGDoc::CalculateBandBox(CPoint point)
{
	bandBoxFloat = point;
}

void CBTGDoc::Copy(void)
{
    BTGPolygon* poly;

    //ensure polygon edit mode
    if (btgDocMode != btgDocMode_EditBTGPolygons)
    {
        return;
    }

    //purge copy buffers
    copyVertexList.PurgeList();
    copyPolygonList.PurgeList();

    //now actually copy the stuff
    /*
     copy polygons, so flag vertices from selected polys & put in a list or something
     */
    ccList* vertexList = new ccList();
    poly = GetFirstBTGPolygon();
    while (poly)
    {
        if (bitTest(poly->flags, BTGPolygon::btgpFlags_Selected))
        {
            //a selected poly
        }
    }
    vertexList->PurgeList();
    delete vertexList;
}

void CBTGDoc::Paste(void)
{
    //ensure polygon edit mode
    if (btgDocMode != btgDocMode_EditBTGPolygons)
    {
        return;
    }

    //anything to paste?
    if (copyPolygonList.GetNumElements() == 0)
    {
        return;
    }

    //paste the copy buffers (append)
}

void CBTGDoc::UpdateBandBox(CPoint point)
{
	bandBoxFloat = point;
	BTGVertex *pVertex;
	BTGStar *pStar;
	BTGPolygon *pPolygon;
	unsigned long l, t, r, b;
	CPoint bbAnchor, bbFloat;
	BOOL select;

	// Move the anchor and float into actual coordinates.
	bbAnchor = bandBoxAnchor;
	bbFloat = bandBoxFloat;

	CalculateActualCoordinates(&bbAnchor);
	CalculateActualCoordinates(&bbFloat);

	// Clear anything that's currently selected.
	if (GetKeyState(VK_SHIFT) & 0x8000)
	{
		select = TRUE;
		if (btgvNumSelected() == 0)
		{
			EverythingClearSelected();
		}
	}
	else
	{
		if (btgvNumSelected() == 0)
		{
			select = TRUE;
		}
		else
		{
			select = FALSE;
		}
	}

	// Make sure the rectangle is positive.
	l = min(bbAnchor.x, bbFloat.x);
	t = min(bbAnchor.y, bbFloat.y);
	r = max(bbAnchor.x, bbFloat.x);
	b = max(bbAnchor.y, bbFloat.y);

	// Check to see what's interior to the band box.
	switch(btgDocMode)
	{
	case btgDocMode_EditBTGVertices:
		pVertex = GetFirstBTGVertex();

		while(pVertex)
		{
			CPoint pt((int)pVertex->x, (int)pVertex->y);

			if(PointInRect(&pt, l, t, r, b))
			{
				if (select)
				{
					btgvSelect(pVertex);
				}
				else
				{
					btgvUnSelect(pVertex);
				}
			}

			pVertex = pVertex->GetNext();
		}
		break;

	case btgDocMode_EditBTGPolygons:
		pPolygon = GetFirstBTGPolygon();

		while(pPolygon)
		{
			CPoint pt0((int)pPolygon->v0->x, (int)pPolygon->v0->y);
			CPoint pt1((int)pPolygon->v1->x, (int)pPolygon->v1->y);
			CPoint pt2((int)pPolygon->v2->x, (int)pPolygon->v2->y);

			if((PointInRect(&pt0, l, t, r, b)) && 
				(PointInRect(&pt1, l, t, r, b)) && 
				(PointInRect(&pt2, l, t, r, b)))
			{
				if (select)
				{
					btgpSelect(pPolygon);
				}
				else
				{
					btgpUnSelect(pPolygon);
				}
			}
			
			pPolygon = pPolygon->GetNext();
		}
		break;

	case btgDocMode_EditBTGStars:
		pStar = GetFirstBTGStar();

		while(pStar)
		{
			CPoint pt((int)pStar->x, (int)pStar->y);

			if(PointInRect(&pt, l, t, r, b))
			{
				if (select)
				{
					btgsSelect(pStar);
				}
				else
				{
					btgsUnSelect(pStar);
				}
			}

			pStar = pStar->GetNext();
		}
		break;
	}
}


void CBTGDoc::EnableDragMode(CPoint point)
{
	dragEnabled = 1;
	oldMousePos = point;
}

void CBTGDoc::DisableDragMode(void)
{
	dragEnabled = 0;
}

void CBTGDoc::UpdateDragMode(CPoint point)
{
	double delta_x, delta_y;
	BTGVertex *pVertex;
	BTGPolygon *pPolygon;
	BTGStar *pStar;

	delta_x = (double)(point.x - oldMousePos.x);
	delta_y = (double)(point.y - oldMousePos.y);

	if (renderMode == btgRenderMode_3D)
	{
		btgGLUpdate3DMouse((float)delta_x, (float)delta_y);
		return;
	}

	delta_x = (double)((float)delta_x / zoomVal);
	delta_y = (double)((float)delta_y / zoomVal);

	oldMousePos = point;

	if (GetKeyState(VK_CONTROL) & 0x8000)
	{
		xScrollVal += (long)delta_x;
		yScrollVal += (long)delta_y;
		return;
	}

	switch(btgDocMode)
	{
	case btgDocMode_EditBTGVertices:
		pVertex = GetFirstBTGVertex();

		while(pVertex)
		{
			if(bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
			{
				// Let the document know it's been modified.
				if(!bDisableModified)
				{
					SetModifiedFlag();
				}
				
				pVertex->x += delta_x;
				pVertex->y += delta_y;

				if(pVertex->x < 0) pVertex->x = 0;
				if(pVertex->x > pageWidth) pVertex->x = pageWidth;

				if(pVertex->y < 0) pVertex->y = 0;
				if(pVertex->y > pageHeight) pVertex->y = pageHeight;
			}

			pVertex = pVertex->GetNext();
		}
		break;

	case btgDocMode_EditBTGPolygons:
		// Make sure no vertices are selected.
		btgvClearSelected();

		pPolygon = GetFirstBTGPolygon();

		while(pPolygon)
		{
			if(bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected))
			{
				// Let the document know it's been modified.
				if(!bDisableModified)
				{
					SetModifiedFlag();
				}

				
				// Select these verts.
				bitSet(pPolygon->v0->flags, BTGVertex::btgvFlags_Selected);
				bitSet(pPolygon->v1->flags, BTGVertex::btgvFlags_Selected);
				bitSet(pPolygon->v2->flags, BTGVertex::btgvFlags_Selected);
			}

			pPolygon = pPolygon->GetNext();
		}

		// Now loop through the selected verts and move them.
		pVertex = btgvGetFirstSelected();

		while(pVertex)
		{
			if(bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
			{
				pVertex->x += delta_x;
				pVertex->y += delta_y;

				if(pVertex->x < 0) pVertex->x = 0;
				if(pVertex->x > pageWidth) pVertex->x = pageWidth;

				if(pVertex->y < 0) pVertex->y = 0;
				if(pVertex->y > pageHeight) pVertex->y = pageHeight;
			}

			pVertex = pVertex->GetNext();
		}

		// Make sure no vertices are selected.
		btgvClearSelected();
		break;

	case btgDocMode_EditBTGStars:
		pStar = GetFirstBTGStar();

		while(pStar)
		{
			if(bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
			{
				// Let the document know it's been modified.
				if(!bDisableModified)
				{	
					SetModifiedFlag();
				}
				
				pStar->x += delta_x;
				pStar->y += delta_y;

				if(pStar->x < 0) pStar->x = 0;
				if(pStar->x > pageWidth) pStar->x = pageWidth;

				if(pStar->y < 0) pStar->y = 0;
				if(pStar->y > pageHeight) pStar->y = pageHeight;
			}

			pStar = pStar->GetNext();
		}
		break;
	}
}

void CBTGDoc::btgvUpdateBrightness(signed long brightness)
{
	BTGVertex* pVertex;

	if (brightness == mBrightness)
	{
		return;
	}

	btgSaveForUndo();

	if (brightness >= 0)
	{
		mBrightness = brightness;
	}
	else
	{
		brightness = mBrightness;
	}

	switch (btgDocMode)
	{
	case btgDocMode_EditBTGVertices:
		pVertex = btgvGetFirstSelected();

		while (pVertex)
		{
			if (bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
			{
				if (!bDisableModified)
				{
					SetModifiedFlag();
				}

				pVertex->brightness = brightness;
			}

			pVertex = pVertex->GetNext();
		}
		break;
	}

	UpdateAllViews(NULL);
}

void CBTGDoc::btgvUpdateBackgroundColor(signed long red, signed long green, signed long blue)
{
	if ((red == mBGRed) && (green == mBGGreen) && (blue == mBGBlue))
	{
		return;
	}

	SetModifiedFlag();
	
	mBGRed = red;
	mBGGreen = green;
	mBGBlue = blue;

	UpdateAllViews(NULL);
}

void CBTGDoc::btgvUpdateMasterColor(signed long red, signed long green, signed long blue, signed long alpha, signed long brightness)
{
	BTGVertex *pVertex;
	BTGStar *pStar;

	if (brightness != mBrightness)
	{
		mBrightness = brightness;
	}

	if((red == mRed) && (green == mGreen) && (blue == mBlue) && (alpha == mAlpha))
	{
		return;
	}

	mRed = red;
	mGreen = green;
	mBlue = blue;
	mAlpha = alpha;

	switch(btgDocMode)
	{
	case btgDocMode_EditBTGVertices:
		pVertex = btgvGetFirstSelected();

		while(pVertex)
		{
			if(bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
			{
				// Let the document know it's been modified.
				if(!bDisableModified)
				{
					SetModifiedFlag();
				}
				
				pVertex->red = mRed;
				pVertex->green = mGreen;
				pVertex->blue = mBlue;
				pVertex->alpha = mAlpha;
			}

			pVertex = pVertex->GetNext();
		}
		break;

	case btgDocMode_EditBTGStars:
		pStar = btgsGetFirstSelected();

		while(pStar)
		{
			if(bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
			{
				// Let the document know it's been modified.
				if(!bDisableModified)
				{
					SetModifiedFlag();
				}

				pStar->red = mRed;
				pStar->green = mGreen;
				pStar->blue = mBlue;
			}

			pStar = pStar->GetNext();
		}
		break;
	}

	UpdateAllViews(NULL);
}

void CBTGDoc::btgUpdateVisible(BOOL bs, signed long mode)
{
	if((bs == bStars) && (mode == renderMode))
	{
		return;
	}

	// Let the document know it's been modified.
	if(!bDisableModified)
	{
		SetModifiedFlag();
	}

	bStars = bs;
	renderMode = mode;

	UpdateAllViews(NULL);
}

void CBTGDoc::btgUpdateBitmap(char *starFileName)
{
	BTGStar *pStar;

//	btgSaveForUndo();

	// selection change?
	if(curStarFileName != starFileName)
	{
		pStar = btgsGetFirstSelected();

		while(pStar)
		{
			if(bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
			{
				// Let the document know it's been modified.
				if(!bDisableModified)
				{
					SetModifiedFlag();
				}

				// Find the new container.
				CTGAContainer *pContainer = (CTGAContainer *)gTGAFileList.GetHead();

				while(pContainer)
				{
					if(!strcmp(starFileName, pContainer->myFileName))
						break;

					pContainer = (CTGAContainer *)pContainer->GetNext();
				}

				pStar->pMyStar = pContainer;
			}

			pStar = pStar->GetNext();
		}

		curStarFileName = starFileName;

		UpdateAllViews(NULL);
	}
}

void CBTGDoc::OnBtgPageSize() 
{
	CPageSizeDlg sizeDialog(NULL, pageWidth, pageHeight);

	btgSaveForUndo();

	if(sizeDialog.DoModal() == IDOK)
	{
		// Let the document know it's been modified.
		if(!bDisableModified)
		{
			SetModifiedFlag();
		}

		pageWidth = sizeDialog.pageWidth;
		pageHeight = sizeDialog.pageHeight;

		UpdateAllViews(NULL);
	}
}

BOOL CBTGDoc::OnPage(CPoint *pPoint)
{
	return(PointInRect(pPoint, 0, 0, pageWidth, pageHeight));
}

void CBTGDoc::UpdateMainFrmDialogs(void)
{
	CButton *check2, *radio1, *radio2, *radio3;

	if (!gpm_wndVisibleBar)
	{
		return;
	}

	check2 = (CButton *)gpm_wndVisibleBar->GetDlgItem(IDC_CHECK2);
	radio1 = (CButton *)gpm_wndVisibleBar->GetDlgItem(IDC_RADIO1);
	radio2 = (CButton *)gpm_wndVisibleBar->GetDlgItem(IDC_RADIO2);	
	radio3 = (CButton *)gpm_wndVisibleBar->GetDlgItem(IDC_RADIO3);

	check2->SetCheck(bStars);
	
	if (renderMode == CBTGDoc::btgRenderMode_Preview)
	{
		if (radio1 != NULL) radio1->SetCheck(1);
		radio2->SetCheck(0);
		radio3->SetCheck(0);
	}
	else if (renderMode == CBTGDoc::btgRenderMode_Realistic)
	{
		if (radio1 != NULL) radio1->SetCheck(0);
		radio2->SetCheck(1);
		radio3->SetCheck(0);
	}
	else if (renderMode == CBTGDoc::btgRenderMode_3D)
	{
		if (radio1 != NULL) radio1->SetCheck(0);
		radio2->SetCheck(0);
		radio3->SetCheck(1);
	}

	if (!gpm_wndBackgroundBar)
	{
		return;
	}

	CSliderCtrl *sRed, *sGreen, *sBlue;

	sRed = (CSliderCtrl *)gpm_wndBackgroundBar->GetDlgItem(ID_BGREDSLIDER);
	sGreen = (CSliderCtrl *)gpm_wndBackgroundBar->GetDlgItem(ID_BGGREENSLIDER);
	sBlue = (CSliderCtrl *)gpm_wndBackgroundBar->GetDlgItem(ID_BGBLUESLIDER);

	sRed->SetPos(mBGRed);
	sGreen->SetPos(mBGGreen);
	sBlue->SetPos(mBGBlue);
}

void CBTGDoc::btgSaveForUndo(void)
{
	/*
	char filename[128];

	sprintf(filename, "\\homeworld\\data\\btg\\undo\\undo%u.btg", undoSequence % BTG_NUM_UNDOS);
	undoSequence++;
		
	CFile fi(filename, CFile::modeWrite | CFile::modeCreate);
	CArchive ar(&fi, CArchive::store);
	btgSaveNative(ar);
	*/

	if(!undoing)
	{
		undoSequence = undoBufferIndex;
	}
	
	if(undoBufferIndex + 1 >= BTG_NUM_UNDOS)
	{
		// Reset the undo queue.
		for(long i=0;i<BTG_NUM_UNDOS;i++)
		{
			if(undoBuffers[i])
			{
				delete undoBuffers[i];
			}

			undoBuffers[i] = 0x00;
		}

		undoBufferIndex = 0;
		undoSequence = 0;
	}

	undoBuffers[undoBufferIndex] = new CMemFile;
	CArchive ar(undoBuffers[undoBufferIndex], CArchive::store);
	btgSaveNative(ar);
	
	undoBufferIndex ++;
	undoSequence++;
}

void CBTGDoc::btgDocUndo(void)
{
	/*
	char filename[128];

	btgSaveForUndo();
	undoSequence--;
	if (undoSequence > 0)
	{
		undoSequence--;
	}
	sprintf(filename, "\\homeworld\\data\\btg\\undo\\undo%u.btg", undoSequence % BTG_NUM_UNDOS);

	CFile fi(filename, CFile::modeRead);
	CArchive ar(&fi, CArchive::load);
	undoing = 1;
	btgLoadNative(ar);

	UpdateAllViews(NULL);
	*/

	undoing = 1;
		
	btgSaveForUndo();

	if(undoSequence > 0)
	{
		undoSequence--;
	}

	if (undoSequence > 0)
	{
		undoSequence--;
	}

	assert(undoBuffers[undoSequence]);

	undoBuffers[undoSequence]->SeekToBegin();
	CArchive ar(undoBuffers[undoSequence], CArchive::load);
	btgLoadNative(ar);

	UpdateAllViews(NULL);
}

void CBTGDoc::btgDocRedo(void)
{
	/*
	char filename[128];
	CFile fi;

	undoSequence++;
	sprintf(filename, "\\homeworld\\data\\btg\\undo\\undo%u.btg", undoSequence % BTG_NUM_UNDOS);

	if (!fi.Open(filename, CFile::modeRead))
	{
		undoSequence--;
		return;
	}
	CArchive ar(&fi, CArchive::load);
	undoing = 1;
	btgLoadNative(ar);

	UpdateAllViews(NULL);
	*/

	if(undoSequence < (undoBufferIndex - 1))
	{
		undoSequence++;
	}
	else
	{
		return;
	}

	assert(undoBuffers[undoSequence]);
	
	undoBuffers[undoSequence]->SeekToBegin();
	CArchive ar(undoBuffers[undoSequence], CArchive::load);
	undoing = 1;
	btgLoadNative(ar);

	UpdateAllViews(NULL);
}

void CBTGDoc::OnBtgLoadBackground() 
{
	static char BASED_CODE szFilter[] = "Targa Bitmap Files (*.tga)|*.tga|All Files (*.*)|*.*||";
	CFileDialog fileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL);
	CString fileName;
	LPCTSTR pFileName;
	BOOL retVal;
	TGAFile tempImage;
	
	if(fileDialog.DoModal() == IDCANCEL)
		return;

	fileName = fileDialog.GetPathName();

	pFileName = fileName;

	retVal = LoadTGAFile((char *)pFileName, &tempImage);

	if(!retVal)
	{
		AfxMessageBox("Error Opening .TGA file.");
		return;
	}
	
	KillTGAFile(&backgroundImage);

	memcpy(&backgroundImage, &tempImage, sizeof(TGAFile));

	UpdateAllViews(NULL);
}

void CBTGDoc::OnBtgTogglesolid() 
{
	drawSolidPolys = 1 - drawSolidPolys;

	UpdateAllViews(NULL);
}

void CBTGDoc::OnBtgTogglebackground() 
{
	drawBackground = 1 - drawBackground;
	
	UpdateAllViews(NULL);
}

void CBTGDoc::OnBtgReferencebgpixel() 
{
	bgPixelGrabEnabled = TRUE;	
}

void CBTGDoc::GrabBGPixel(unsigned char *colArray)
{
	CSliderCtrl *sRed, *sGreen, *sBlue, *sAlpha, *sBrightness;

	//get the handles we need
	sRed = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_REDSLIDER);
	sGreen = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_GREENSLIDER);
	sBlue = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_BLUESLIDER);
	sAlpha = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_ALPHASLIDER);
	sBrightness = (CSliderCtrl*)gpm_wndDialogBar->GetDlgItem(ID_BRIGHTNESSSLIDER);

	//update the slider positions
	sRed->SetPos(colArray[0]);
	sGreen->SetPos(colArray[1]);
	sBlue->SetPos(colArray[2]);
	sAlpha->SetPos(colArray[3]);
}

void CBTGDoc::OnBtgHideselected() 
{
	BTGPolygon *pPolygon;
	BTGStar *pStar;
	BTGVertex *pVertex;

	pPolygon = GetFirstBTGPolygon();
	pStar = GetFirstBTGStar();
	pVertex = GetFirstBTGVertex();

	while (pPolygon)
    {
        if(bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected))
		{
			pPolygon->bVisible = FALSE;

			btgpUnSelect(pPolygon);
		}

        pPolygon = pPolygon->GetNext();
    }

	while (pStar)
    {
        if(bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
		{
			pStar->bVisible = FALSE;

			btgsUnSelect(pStar);
		}

        pStar = pStar->GetNext();
    }

	while (pVertex)
    {
        if(bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
		{
			pVertex->bVisible = FALSE;

			btgvUnSelect(pVertex);
		}

        pVertex = pVertex->GetNext();
    }

	UpdateAllViews(NULL);
}

void CBTGDoc::OnBtgUnhideall() 
{
	BTGPolygon *pPolygon;
	BTGStar *pStar;
	BTGVertex *pVertex;

	pPolygon = GetFirstBTGPolygon();
	pStar = GetFirstBTGStar();
	pVertex = GetFirstBTGVertex();

	while (pPolygon)
    {
        pPolygon->bVisible = TRUE;
		
		pPolygon = pPolygon->GetNext();
    }

	while (pStar)
    {
        pStar->bVisible = TRUE;
		
        pStar = pStar->GetNext();
    }

	while (pVertex)
    {
        pVertex->bVisible = TRUE;
		
        pVertex = pVertex->GetNext();
    }

	UpdateAllViews(NULL);
}

void CBTGDoc::OnBtgInvertsel() 
{
	BTGPolygon *pPolygon;
	BTGStar *pStar;
	BTGVertex *pVertex;

	pPolygon = GetFirstBTGPolygon();
	pStar = GetFirstBTGStar();
	pVertex = GetFirstBTGVertex();

	if(btgDocMode == btgDocMode_EditBTGPolygons)
	{
		while (pPolygon)
		{
			if(bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected))
			{
				btgpUnSelect(pPolygon);

				pPolygon = pPolygon->GetNext();
				continue;
			}
			
			if(!bitTest(pPolygon->flags, BTGPolygon::btgpFlags_Selected))
			{
				bitSet(pPolygon->flags, BTGPolygon::btgpFlags_Selected);

				pPolygon = pPolygon->GetNext();
				continue;
			}

			pPolygon = pPolygon->GetNext();
		}
	}

	if(btgDocMode == btgDocMode_EditBTGStars)
	{
		while (pStar)
		{
			if(bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
			{
				btgsUnSelect(pStar);

				pStar = pStar->GetNext();
				continue;
			}
			
			if(!bitTest(pStar->flags, BTGStar::btgsFlags_Selected))
			{
				bitSet(pStar->flags, BTGStar::btgsFlags_Selected);

				pStar = pStar->GetNext();
				continue;
			}

			pStar = pStar->GetNext();
		}
	}

	if(btgDocMode == btgDocMode_EditBTGVertices)
	{
		while (pVertex)
		{
			if(bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
			{
				btgvUnSelect(pVertex);

				pVertex = pVertex->GetNext();
				continue;
			}
			
			if(!bitTest(pVertex->flags, BTGVertex::btgvFlags_Selected))
			{
				bitSet(pVertex->flags, BTGVertex::btgvFlags_Selected);

				pVertex = pVertex->GetNext();
				continue;
			}

			pVertex = pVertex->GetNext();
		}
	}

	UpdateAllViews(NULL);
}
