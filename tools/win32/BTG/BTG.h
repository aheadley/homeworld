// BTG.h : main header file for the BTG application
//

#if !defined(AFX_BTG_H__9EDD67C8_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_)
#define AFX_BTG_H__9EDD67C8_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "btgdoc.h"

// Shared program defines.
#define BTGV_SIZE			(4)
#define BTGV_SIZE2			(BTGV_SIZE / 2)

#define BTGS_SIZE			(10)
#define BTGS_SIZE2			(BTGS_SIZE / 2)

//macros to set/test/clear bits
#define bitSet(dest, bitFlags)      ((dest) |= (bitFlags))
#define bitClear(dest, bitFlags)    ((dest) &= ~(bitFlags))
#define bitTest(dest, bitFlags)     ((dest) & (bitFlags))

/////////////////////////////////////////////////////////////////////////////
// CBTGApp:
// See BTG.cpp for the implementation of this class
//

class CBTGApp : public CWinApp
{
public:
	CBTGApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBTGApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CBTGApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BTG_H__9EDD67C8_9A9A_11D1_BEB6_00A0C960350A__INCLUDED_)
