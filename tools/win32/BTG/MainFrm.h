// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__9EDD67CC_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_)
#define AFX_MAINFRM_H__9EDD67CC_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "btgdialogbar.h"
#include "btgdoc.h"

extern CBTGVisibleBar *gpm_wndVisibleBar;
extern CBTGVDialogBar *gpm_wndDialogBar;
extern CBTGVBackgroundBar *gpm_wndBackgroundBar;
extern ccList gTGAFileList;					// Holds Bitmaps.
extern CBTGDoc* gBTGDoc;

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CStatusBar		m_wndStatusBar;
	CToolBar		m_wndToolBar;
	CBTGVDialogBar	m_wndDialogBar;
	CBTGVisibleBar	m_wndVisibleBar;
	CBTGBitmapBar	m_wndBitmapBar;
	CBTGVBackgroundBar m_wndBackgroundBar;

	unsigned long btgDocMode;			// What are we doing right now?
	BOOL bPolyCount;

protected:  // control bar embedded members

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewColorControl();
	afx_msg void OnUpdateBtgPolyCount(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtgVertexmode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtgPolymode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtgStarmode(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__9EDD67CC_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_)
