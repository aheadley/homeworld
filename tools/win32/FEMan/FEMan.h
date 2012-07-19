// FEMan.h : main header file for the FEMAN application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CFEManApp:
// See FEMan.cpp for the implementation of this class
//

class CFEManApp : public CWinApp
{
public:
	CFEManApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFEManApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CFEManApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CFEManApp	theApp;
extern LOGFONT		gDefaultFont;
extern CRect		gAppRect;
extern int			gSplitWidth;

/////////////////////////////////////////////////////////////////////////////
