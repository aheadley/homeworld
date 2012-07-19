#if !defined(AFX_PAGESIZEDLG_H__2D0C4945_A27F_11D1_BEB6_00A0C960350A__INCLUDED_)
#define AFX_PAGESIZEDLG_H__2D0C4945_A27F_11D1_BEB6_00A0C960350A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PageSizeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPageSizeDlg dialog

class CPageSizeDlg : public CDialog
{
// Construction
public:
	CPageSizeDlg(CWnd* pParent = NULL, signed long pw = 0, signed long ph = 0);   // standard constructor

	signed long pageWidth;
	signed long pageHeight;

// Dialog Data
	//{{AFX_DATA(CPageSizeDlg)
	enum { IDD = IDD_PAGE_SIZE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPageSizeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPageSizeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEdit1();
	afx_msg void OnChangeEdit2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGESIZEDLG_H__2D0C4945_A27F_11D1_BEB6_00A0C960350A__INCLUDED_)
