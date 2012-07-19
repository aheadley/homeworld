// BTGDoc.h : interface of the CBTGDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BTGDOC_H__9EDD67CE_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_)
#define AFX_BTGDOC_H__9EDD67CE_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "cclist.h"

#include "btgVertex.h"
#include "btgStar.h"
#include "btgPolygon.h"

#define BTG_NUM_UNDOS 50

class CBTGDoc : public CDocument
{
protected: // create from serialization only
	CBTGDoc();
	DECLARE_DYNCREATE(CBTGDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBTGDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBTGDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void Reset(void);

	void btgSaveNative(CArchive& ar);
	void btgLoadNative(CArchive& ar);

	void btgSaveForUndo(void);
	void btgDocUndo(void);
	void btgDocRedo(void);

    void btgFixupPolys(void);

	BTGVertex *GetFirstBTGVertex(void);
	BTGStar *GetFirstBTGStar(void);
	BTGPolygon *GetFirstBTGPolygon(void);

	BTGVertex *btgvGetFirstSelected(void);
	BTGPolygon *btgpGetFirstSelected(void);
	BTGStar *btgsGetFirstSelected(void);

	inline TGAFile *GetBackground(void) {return(&backgroundImage);}

	unsigned long btgvNumSelected(void);
	unsigned long btgvNumExisting(void);
	unsigned long btgsNumSelected(void);
	unsigned long btgpNumSelected(void);
	unsigned long btgpNumExisting(void);
	
	BTGVertex *btgvCheckExisting(CPoint point);
	BTGStar *btgsCheckExisting(CPoint point);
	BTGPolygon *btgpCheckExisting(CPoint point);
	BTGPolygon *btgpCheckExisting(BTGVertex *pv0, BTGVertex *pv1, BTGVertex *pv2);

	BTGPolygon *btgpCheckUsing(BTGVertex *pv);

	BTGVertex *btgvCreateNew(CPoint point);
	void btgsCreateNew(CPoint point);
	void btgpCreateNew(BTGVertex *pv0, BTGVertex *pv1, BTGVertex *pv2);

	void btgvDeleteSelected(void);
	void btgsDeleteSelected(void);
	void btgpDeleteSelected(void);
	void EverythingDeleteSelected(void);

	void btgpConsiderBTGVertex(CPoint point);
	void btgpClearConsideredBTGVertices(void);

	void btgvSelect(CPoint point);
	void btgvSelect(BTGVertex *pVertex);
	void btgvUnSelect(BTGVertex *pVertex);
	void btgsSelect(CPoint point);
	void btgsSelect(BTGStar *pStar);
	void btgsUnSelect(BTGStar *pStar);
	void btgpSelect(CPoint point);
	void btgpSelect(BTGPolygon *pPolygon);
	void btgpUnSelect(BTGPolygon *pPolygon);

	void btgvClearSelected(void);
	void btgsClearSelected(void);
	void btgpClearSelected(void);
	void EverythingClearSelected(void);

	void CalculateActualCoordinates(CPoint *pPoint);
	void CalculateScreenCoordinates(CPoint *pPoint);

	BOOL PointInRect(CPoint *pPoint, double l, double t, double r, double b);
	BOOL PointInPolygon(CPoint *pPoint, BTGPolygon *pPolygon);

	void btgpForceCounterclockwise(BTGPolygon *pPolygon);

	void btgpPrevEdge(void);
	void btgpNextEdge(void);
	void btgpSubEdge(void);

	void btgvMergeSelected(void);

	void EnableBandBox(CPoint point);
	void DisableBandBox(void);
	void CalculateBandBox(CPoint point);
	void UpdateBandBox(CPoint point);

	void EnableDragMode(CPoint point);
	void DisableDragMode(void);
	void UpdateDragMode(CPoint point);

	void btgvUpdateMasterColor(signed long red, signed long green, signed long blue, signed long alpha, signed long brightness);
	void btgvUpdateBackgroundColor(signed long red, signed long green, signed long blue);
	void btgvUpdateBrightness(signed long brightness);
	void btgUpdateVisible(BOOL bs, signed long mode);
	void btgUpdateBitmap(char *starFileName);

	void GrabBGPixel(unsigned char *colArray);

	BOOL OnPage(CPoint *pPoint);

	void UpdateMainFrmDialogs(void);

    void Copy(void);
    void Paste(void);

	enum btgDocModes
	{
		btgDocMode_FirstBTGMode = 0,	// Range checking.  Don't use.
		
		btgDocMode_EditBTGVertices = 0,	// Edit the vertex list.
		btgDocMode_EditBTGPolygons,		// Edit the polygon list.
		btgDocMode_EditBTGStars,		// Edit the star list.

		btgDocMode_LastBTGMode			// Range checking.  Don't use.
	};

	enum btges
	{
		btgRenderMode_FirstRenderMode = 0,	// Range checking.  Don't use.

		btgRenderMode_Preview = 0,
		btgRenderMode_Realistic,
		btgRenderMode_3D,

		btgRenderMode_LastRenderMode		// Range checking.  Don't use.
	};

	enum btgDrawModes
	{
		btgDrawMode_FirstDrawMode = 0,	// Range checking.  Don't use.

		btgDrawMode_Basic = 0,
		btgDrawMode_Verts1,
		btgDrawMode_VertsPolys,
		
		btgDrawMode_LastDrawMode		// Range checking.  Don't use.
	};

	BOOL drawSolidPolys;
	BOOL drawBackground;
	BOOL bgPixelGrabEnabled;
	
	unsigned long btgDocMode;			// What are we doing right now?
	signed long btgpSelectedEdge;		// Currently selected edge.
	unsigned long bandBoxEnabled;		// Status of band box.
	CPoint bandBoxAnchor;				// Anchor point of band box.
	CPoint bandBoxFloat;				// Band box point on mouse.
	unsigned long dragEnabled;			// Status of drag mode.
	CPoint oldMousePos;					// Last mouse pos sampled.
	signed long	xScrollVal;				// X Scroll offset.
	signed long yScrollVal;				// Y Scroll offset.
	float zoomVal;						// Amount to zoom the view by.
	signed long pageWidth, pageHeight;	// Width and height of the virtual page.
	BOOL bStars;	// Visibility status of the draw items.
	signed long renderMode;				// preview or realistic?
	signed long drawMode;				// what the hell are we drawing?

	signed long mBGRed, mBGGreen, mBGBlue;

protected:
	ccList BTGVertexList;				// Holds polygon vertices.
	ccList BTGPolygonList;				// Holds polygons.
	ccList BTGStarList;					// Holds stars.

    ccList copyVertexList;
    ccList copyPolygonList;

	TGAFile backgroundImage;			// .TGA background image.

	BOOL undoing;

	unsigned int undoSequence;

	unsigned long nTempV;				// Number of verts selected for poly construction.
	BTGVertex *pTempV[3];				// Array of verts for above.
	signed long mRed, mGreen, mBlue;	// Master colors from the color picker.
	signed long mAlpha;
	signed long mBrightness;
	BOOL bDisableModified;				// Allow / disallow document modified flag to be set.

	char *curStarFileName;

	unsigned long undoBufferIndex;
	CMemFile *undoBuffers[BTG_NUM_UNDOS];

// Generated message map functions
protected:
	//{{AFX_MSG(CBTGDoc)
	afx_msg void OnBtgPageSize();
	afx_msg void OnBtgLoadBackground();
	afx_msg void OnBtgTogglesolid();
	afx_msg void OnBtgTogglebackground();
	afx_msg void OnBtgReferencebgpixel();
	afx_msg void OnBtgHideselected();
	afx_msg void OnBtgUnhideall();
	afx_msg void OnBtgInvertsel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BTGDOC_H__9EDD67CE_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_)
