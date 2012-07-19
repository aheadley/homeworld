// ChooseOptionsPage.cpp : implementation file
//

#include "stdafx.h"
#include "Fontastic.h"
#include "ChooseOptionsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseOptionsPage property page

IMPLEMENT_DYNCREATE(CChooseOptionsPage, CPropertyPage)

CChooseOptionsPage::CChooseOptionsPage() : CPropertyPage(CChooseOptionsPage::IDD)
{
	//{{AFX_DATA_INIT(CChooseOptionsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CChooseOptionsPage::~CChooseOptionsPage()
{
}

void CChooseOptionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseOptionsPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseOptionsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CChooseOptionsPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseOptionsPage message handlers
