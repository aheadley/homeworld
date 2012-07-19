// PageSizeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTG.h"
#include "PageSizeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPageSizeDlg dialog


CPageSizeDlg::CPageSizeDlg(CWnd* pParent /*=NULL*/, signed long pw /*= 0*/, signed long ph /*= 0*/)
	: CDialog(CPageSizeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPageSizeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	pageWidth = pw;
	pageHeight = ph;
}


void CPageSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageSizeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageSizeDlg, CDialog)
	//{{AFX_MSG_MAP(CPageSizeDlg)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT2, OnChangeEdit2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageSizeDlg message handlers

BOOL CPageSizeDlg::OnInitDialog() 
{
	char buffer[20];
	CDialog::OnInitDialog();
	
	CEdit *pWidth, *pHeight;

	pWidth = (CEdit *)GetDlgItem(IDC_EDIT1);
	pHeight = (CEdit *)GetDlgItem(IDC_EDIT2);

	pWidth->LimitText(4);
	pHeight->LimitText(4);

	pWidth->SetWindowText(itoa(pageWidth, buffer, 10));
	pHeight->SetWindowText(itoa(pageHeight, buffer, 10));
	
	pWidth->SetFocus();
	pWidth->SetSel(0, -1);

	return FALSE;
}

void CPageSizeDlg::OnChangeEdit1() 
{
	pageWidth = GetDlgItemInt(IDC_EDIT1);
}

void CPageSizeDlg::OnChangeEdit2() 
{
	pageHeight = GetDlgItemInt(IDC_EDIT2);
}
