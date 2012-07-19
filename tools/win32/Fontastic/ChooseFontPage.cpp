// ChooseFontPage.cpp : implementation file
//

#include "stdafx.h"
#include "Fontastic.h"
#include "ChooseFontPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseFontPage property page

IMPLEMENT_DYNCREATE(CChooseFontPage, CPropertyPage)

CChooseFontPage::CChooseFontPage() : CPropertyPage(CChooseFontPage::IDD)
{
	//{{AFX_DATA_INIT(CChooseFontPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CChooseFontPage::~CChooseFontPage()
{
}

void CChooseFontPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseFontPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseFontPage, CPropertyPage)
	//{{AFX_MSG_MAP(CChooseFontPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseFontPage message handlers
