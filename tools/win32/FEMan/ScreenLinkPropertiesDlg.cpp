// ScreenLinkPropertiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FEMan.h"
#include "ScreenLinkPropertiesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScreenLinkPropertiesDlg dialog


CScreenLinkPropertiesDlg::CScreenLinkPropertiesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScreenLinkPropertiesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScreenLinkPropertiesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CScreenLinkPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScreenLinkPropertiesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScreenLinkPropertiesDlg, CDialog)
	//{{AFX_MSG_MAP(CScreenLinkPropertiesDlg)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScreenLinkPropertiesDlg message handlers

BOOL CScreenLinkPropertiesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetDlgItemText(IDC_EDIT1, screenLinkName);

	CEdit *pEdit = (CEdit *)GetDlgItem(IDC_EDIT1);

	pEdit->LimitText(255);

	pEdit->SetFocus();
	pEdit->SetSel(0, -1);
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScreenLinkPropertiesDlg::OnOK() 
{
	GetDlgItemText(IDC_EDIT1, screenLinkName, 255);

	CDialog::OnOK();
}

void CScreenLinkPropertiesDlg::OnChangeEdit1() 
{	
}
