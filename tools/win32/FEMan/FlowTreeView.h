// FlowTreeView.h : header file
//

#include "FEManDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CFlowTreeView view

class CFlowTreeView : public CView
{
protected:
	CFlowTreeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFlowTreeView)

	// This is for the tab control.
	CTabCtrl myTabControl;

// Attributes
public:
	CFEManDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFlowTreeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint( CDC *pDC, CPrintInfo *pInfo );
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFlowTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Alex added this.
private:
	// Left button is pressed.  We have several choices.
	// 1: Pressed on nothing.  Start dragging a selection window.
	// 2: Pressed on a Screen.  Select the screen and drag it.
	// 3: Pressed on a Screen Link.  Select the link and drag it.
	// 4: Pressed on an already selected Screen.  Drag all selected items.
	// 5: Pressed on an already selected Screen Link.  Drag all selected items.
	
	enum tagLButtonStatus
	{
		LBS_NORMAL,				// Nothing special.
		LBS_WINDOWCLICK,		// Clicked on a window.
		LBS_SCREENCLICK,		// Clicked on a screen.
		LBS_LINKCLICK,			// Clicked on a screenlink.

		LBS_LAST_LBS			// Range checking.
	};

	CPoint lastRightClick;
	CPoint lastLeftClick;

	CRect selectionWindow;

	signed long lButtonStatus;

protected:
public:
	void RegionUpdate(CRect *pRect);
	void DrawScreenIcons(CDC *pDC);
	void DrawScreenLinks(CDC *pDC);
	void DrawSelectWindow(CDC *pDC);

	void DoScreenProperties(void);
	void DoScreenLinkPropertiesDlg(void);

	void ChooseMouseCursor(void);

	void CalculateScreenLinkName(ScreenLink *sl);

	// Generated message map functions
protected:
	//{{AFX_MSG(CFlowTreeView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnFtdNewScreen();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSiDeletescreen();
	afx_msg void OnSiProperties();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSiNewScreenlink();
	afx_msg void OnSlDeletescreenlink();
	afx_msg void OnSlProperties();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FlowTreeView.cpp
inline CFEManDoc* CFlowTreeView::GetDocument()
   { return (CFEManDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
