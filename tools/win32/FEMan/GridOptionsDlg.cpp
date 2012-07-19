// GridOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FEMan.h"
#include "GridOptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridOptionsDlg dialog


CGridOptionsDlg::CGridOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGridOptionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGridOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGridOptionsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGridOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CGridOptionsDlg)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_BN_CLICKED(IDC_CHECK2, OnCheck2)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridOptionsDlg message handlers

void CGridOptionsDlg::OnCheck1() 
{
	// TODO: Add your control notification handler code here
	
	gridVisible = 1 - gridVisible;
}

void CGridOptionsDlg::OnCheck2() 
{
	// TODO: Add your control notification handler code here
	gridActive = 1 - gridActive;
}

BOOL CGridOptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the active check.
	CButton *tempButton = (CButton *)GetDlgItem(IDC_CHECK1);
	tempButton->SetCheck(gridVisible);

	// Set the visible check.
	tempButton = (CButton *)GetDlgItem(IDC_CHECK2);
	tempButton->SetCheck(gridActive);

	// Limit the spacing dialog.
	// Set the spacing dialog.
	CEdit *tempEdit = (CEdit *)GetDlgItem(IDC_EDIT1);
	tempEdit->SetLimitText(3);
	SetDlgItemInt(IDC_EDIT1, gridSpacing, FALSE);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGridOptionsDlg::OnChangeEdit1() 
{
	BOOL temp;

	gridSpacing = GetDlgItemInt(IDC_EDIT1, &temp, FALSE);
}
