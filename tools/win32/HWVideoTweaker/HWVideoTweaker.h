// HWVideoTweaker.h : main header file for the HWVIDEOTWEAKER application
//

#if !defined(AFX_HWVIDEOTWEAKER_H__947925C7_6FA9_11D3_9146_00A0C982C829__INCLUDED_)
#define AFX_HWVIDEOTWEAKER_H__947925C7_6FA9_11D3_9146_00A0C982C829__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CHWVideoTweakerApp:
// See HWVideoTweaker.cpp for the implementation of this class
//

class CHWVideoTweakerApp : public CWinApp
{
public:
	CHWVideoTweakerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHWVideoTweakerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CHWVideoTweakerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HWVIDEOTWEAKER_H__947925C7_6FA9_11D3_9146_00A0C982C829__INCLUDED_)
