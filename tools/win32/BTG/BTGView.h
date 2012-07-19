// BTGView.h : interface of the CBTGView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BTGVIEW_H__9EDD67D0_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_)
#define AFX_BTGVIEW_H__9EDD67D0_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CBTGView : public CView
{
protected: // create from serialization only
	CBTGView();
	DECLARE_DYNCREATE(CBTGView)

// Attributes
public:
	CBTGDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBTGView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBTGView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	
protected:
	void bandBoxPDraw(CDC *pDC);
	void pageSizePDraw(CDC *pDC);
	void btgvPDraw(CDC *pDC);
	void btgsPDraw(CDC *pDC);
	void btgpPDraw(CDC *pDC);

// Generated message map functions
protected:
	//{{AFX_MSG(CBTGView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBtgDeleteselected();
	afx_msg void OnBtgPrevpolyedge();
	afx_msg void OnBtgNextpolyedge();
	afx_msg void OnBtgSubpolyedge();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBtgMergeselected();
	afx_msg void OnBtgScrollright();
	afx_msg void OnBtgScrollleft();
	afx_msg void OnBtgScrollup();
	afx_msg void OnBtgScrolldown();
	afx_msg void OnBtgResetZoom();
    afx_msg void OnBtgZoomin();
	afx_msg void OnBtgZoomout();
	afx_msg void OnBtgBrightness();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBtgUndo();
	afx_msg void OnBtgRedo();
	afx_msg void OnBtgClearSelected();
	afx_msg void OnBtgVertexmode();
	afx_msg void OnBtgPolymode();
	afx_msg void OnBtgStarmode();
	afx_msg void OnBtgToggledrawmode();
    afx_msg void OnBtgEditCopy();
    afx_msg void OnBtgEditPaste();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in BTGView.cpp
inline CBTGDoc* CBTGView::GetDocument()
   { return (CBTGDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BTGVIEW_H__9EDD67D0_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_)
