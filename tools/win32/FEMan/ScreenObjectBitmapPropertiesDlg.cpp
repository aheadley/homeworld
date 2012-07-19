// ScreenObjectBitmapPropertiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FEMan.h"
#include "ScreenObjectBitmapPropertiesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScreenObjectBitmapPropertiesDlg dialog


CScreenObjectBitmapPropertiesDlg::CScreenObjectBitmapPropertiesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScreenObjectBitmapPropertiesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScreenObjectBitmapPropertiesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	userInfo = 0;
}


void CScreenObjectBitmapPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScreenObjectBitmapPropertiesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScreenObjectBitmapPropertiesDlg, CDialog)
	//{{AFX_MSG_MAP(CScreenObjectBitmapPropertiesDlg)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScreenObjectBitmapPropertiesDlg message handlers

void CScreenObjectBitmapPropertiesDlg::OnCheck1() 
{
	// Checked = borderVisible. 
	// Unchecked = borderHidden.

	// Already set so just flip.
	contentVisible = 1 - contentVisible;
}

BOOL CScreenObjectBitmapPropertiesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CButton *tempButton = (CButton *)GetDlgItem(IDC_CHECK1);
	
	tempButton->SetCheck(contentVisible);

	SetDlgItemText(IDC_EDIT10, userInfo);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScreenObjectBitmapPropertiesDlg::OnOK() 
{
		CString tempString;

	GetDlgItemText(IDC_EDIT10, tempString);

	if(userInfo)
			delete [] userInfo;

	userInfo = 0;

	if(tempString.GetLength())
	{
		userInfo = new char [tempString.GetLength() + 1];

		strcpy(userInfo, tempString.GetBuffer(1));
	}

	CDialog::OnOK();
}
