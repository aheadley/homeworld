// ScreenPropertiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FEMan.h"
#include "ScreenPropertiesDlg.h"

#include "Screen.h"
#include "FEManDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScreenPropertiesDlg dialog


CScreenPropertiesDlg::CScreenPropertiesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScreenPropertiesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScreenPropertiesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	userInfo = 0;
}


void CScreenPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScreenPropertiesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScreenPropertiesDlg, CDialog)
	//{{AFX_MSG_MAP(CScreenPropertiesDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScreenPropertiesDlg message handlers

BOOL CScreenPropertiesDlg::OnInitDialog() 
{
	// Set up the name field.	
	CEdit *pEdit;
	
	pEdit = (CEdit *)GetDlgItem(IDC_EDIT1);
	
	SetDlgItemText(IDC_EDIT1, screenName);

	pEdit->LimitText(255);

	pEdit->SetFocus();
	pEdit->SetSel(0, -1);

	// Set up the width.
	pEdit = (CEdit *)GetDlgItem(IDC_EDIT2);

	SetDlgItemInt(IDC_EDIT2, screenWidth, FALSE);
	
	pEdit->LimitText(4);

	// Set up the height.
	pEdit = (CEdit *)GetDlgItem(IDC_EDIT3);
	
	SetDlgItemInt(IDC_EDIT3, screenHeight, FALSE);
	
	pEdit->LimitText(4);

	SetDlgItemText(IDC_EDIT10, userInfo);
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScreenPropertiesDlg::OnOK() 
{
	char tempBuffer[255];

	GetDlgItemText(IDC_EDIT1, tempBuffer, 255);

	strcpy(screenName, tempBuffer);

	screenWidth = GetDlgItemInt(IDC_EDIT2, NULL, FALSE);
	screenHeight = GetDlgItemInt(IDC_EDIT3, NULL, FALSE);

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
