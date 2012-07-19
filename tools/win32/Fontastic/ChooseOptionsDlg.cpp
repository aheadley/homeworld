// ChooseOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fontastic.h"
#include "ChooseOptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseOptionsDlg dialog


CChooseOptionsDlg::CChooseOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseOptionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChooseOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseOptionsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseOptionsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseOptionsDlg message handlers
