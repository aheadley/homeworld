// ChooseFontDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fontastic.h"
#include "ChooseFontDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseFontDlg dialog


CChooseFontDlg::CChooseFontDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseFontDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseFontDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChooseFontDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseFontDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseFontDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseFontDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseFontDlg message handlers
