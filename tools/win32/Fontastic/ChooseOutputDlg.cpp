// ChooseOutputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fontastic.h"
#include "ChooseOutputDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseOutputDlg dialog


CChooseOutputDlg::CChooseOutputDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseOutputDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseOutputDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChooseOutputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseOutputDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseOutputDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseOutputDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseOutputDlg message handlers
