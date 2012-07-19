// ScreenEditView.h : header file
//

#include "FEManDoc.h"

#define DEFAULT_SCREEN_OFFSET_X			(20)
#define DEFAULT_SCREEN_OFFSET_Y			(20)

/////////////////////////////////////////////////////////////////////////////
// CScreenEditView view

class CScreenEditView : public CView
{
protected:
	CScreenEditView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CScreenEditView)

///////////////////////////////////////////////////////////////////////////////
// Alex added this stuff...

	CPoint lastRightClick;
	CPoint lastLeftClick;

	CRect selectionWindow;

	signed long lButtonStatus;
	signed long mousePosStatus;
		
// Attributes
public:
	CFEManDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScreenEditView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CScreenEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

///////////////////////////////////////////////////////////////////////////////
// Alex added this stuff...
private:
protected:
public:
	enum tagLButtonStatus
	{
		LBS_NORMAL,				// Nothing special.
		LBS_WINDOWCLICK,		// Clicked on no screen.
		LBS_SCREENOBJECTCLICK,	// Clicked on a screen object.

		LBS_IN_GIZMO_TOP_LEFT,		// Clicked on the top left selection gizmo.
		LBS_IN_GIZMO_TOP_MIDDLE,	// Clicked on the top middle selection gizmo.
		LBS_IN_GIZMO_TOP_RIGHT,		// Clicked on the top right selection gizmo.

		LBS_IN_GIZMO_MIDDLE_LEFT,	// Clicked on the middle left selection gizmo.
		LBS_IN_GIZMO_MIDDLE_RIGHT,	// Clicked on the middle right selection gizmo.

		LBS_IN_GIZMO_BOTTOM_LEFT,	// Clicked on the bottom left selection gizmo.
		LBS_IN_GIZMO_BOTTOM_MIDDLE,	// Clicked on the bottom middle selection gizmo.
		LBS_IN_GIZMO_BOTTOM_RIGHT,	// Clicked on the bottom right selection gizmo.

		LBS_LAST_LBS			// Range checking.
	};

	void DrawScreenObjects(CDC *pDC);
	void DrawSelectWindow(CDC *pDC);
	void DrawGrid(CDC *pDC);
	void DoScreenObjectRectPropertiesDlg(void);
	void DoScreenObjectBitmapPropertiesDlg(void);
	void DoScreenObjectStringPropertiesDlg(void);
	void DoGridOptionsDlg(void);
	void DoContextMenu(CPoint point);
	void ChooseMouseCursor(CPoint *point);

	// Generated message map functions
protected:
	//{{AFX_MSG(CScreenEditView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSedNewRectangle();
	afx_msg void OnSorDeleteselectedscreens();
	afx_msg void OnSorProperties();
	afx_msg void OnSorBringtofront();
	afx_msg void OnSorSendtoback();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSedNewBitmap();
	afx_msg void OnSobProperties();
	afx_msg void OnSedNewTextstring();
	afx_msg void OnSosProperties();
	afx_msg void OnSobDelete();
	afx_msg void OnSosDeleteselectedscreens();
	afx_msg void OnSosSizetocontent();
	afx_msg void OnSobSendtoback();
	afx_msg void OnSobBringtofront();
	afx_msg void OnSosBringtofront();
	afx_msg void OnSosSendtoback();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSorCopy();
	afx_msg void OnSorCut();
	afx_msg void OnSosCopy();
	afx_msg void OnSosCut();
	afx_msg void OnSobCopy();
	afx_msg void OnSobCut();
	afx_msg void OnSedPaste();
	afx_msg void OnEditUndo();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditCopy();
	afx_msg void OnSedGrid();
	afx_msg void OnSedGridoptions();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

#ifndef _DEBUG  // debug version in FlowTreeView.cpp
inline CFEManDoc* CFlowTreeView::GetDocument()
   { return (CFEManDoc*)m_pDocument; }
#endif
};

/////////////////////////////////////////////////////////////////////////////
