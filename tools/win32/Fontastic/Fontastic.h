// Fontastic.h : main header file for the FONTASTIC application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CFontasticApp:
// See Fontastic.cpp for the implementation of this class
//

class CFontasticApp : public CWinApp
{
public:
	CFontasticApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontasticApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CFontasticApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern LOGFONT	gFont;
extern LOGFONT  gBIGFont;
extern BOOL		enableAntialiasing;
extern BOOL		enableDropshadow;
extern CString	chooseTextTemplate;
extern float	gRatioX;
extern float	gRatioY;
extern float	*gConvFilterBuffer;
extern long		gAntialiasingLevel;
/////////////////////////////////////////////////////////////////////////////
