// ChooseOutputPage.cpp : implementation file
//

#include "stdafx.h"
#include "Fontastic.h"
#include "ChooseOutputPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseOutputPage property page

IMPLEMENT_DYNCREATE(CChooseOutputPage, CPropertyPage)

CChooseOutputPage::CChooseOutputPage() : CPropertyPage(CChooseOutputPage::IDD)
{
	//{{AFX_DATA_INIT(CChooseOutputPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CChooseOutputPage::~CChooseOutputPage()
{
}

void CChooseOutputPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseOutputPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseOutputPage, CPropertyPage)
	//{{AFX_MSG_MAP(CChooseOutputPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseOutputPage message handlers
