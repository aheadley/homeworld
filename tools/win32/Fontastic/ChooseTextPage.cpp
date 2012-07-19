// ChooseTextPage.cpp : implementation file
//

#include "stdafx.h"
#include "Fontastic.h"
#include "ChooseTextPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseTextPage property page

IMPLEMENT_DYNCREATE(CChooseTextPage, CPropertyPage)

CChooseTextPage::CChooseTextPage() : CPropertyPage(CChooseTextPage::IDD)
{
	//{{AFX_DATA_INIT(CChooseTextPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CChooseTextPage::~CChooseTextPage()
{
}

void CChooseTextPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseTextPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseTextPage, CPropertyPage)
	//{{AFX_MSG_MAP(CChooseTextPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseTextPage message handlers
