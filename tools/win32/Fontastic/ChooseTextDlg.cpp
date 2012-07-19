// ChooseTextDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fontastic.h"
#include "ChooseTextDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseTextDlg dialog


CChooseTextDlg::CChooseTextDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseTextDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseTextDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChooseTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseTextDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseTextDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseTextDlg)
	ON_BN_CLICKED(IDC_FROMFILE, OnFromfile)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseTextDlg message handlers

void CChooseTextDlg::OnFromfile() 
{
	static char BASED_CODE szFilter[] = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*||";
	  
	CFileDialog dlgTemp(TRUE, ".TXT", NULL, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilter, this);

	int nResult = dlgTemp.DoModal();

	if(nResult == IDOK)
	{
		CString strTemp = dlgTemp.GetFileName();

		// Load the file here and set textTemplate to it's contents.
		CFile fileTemp(strTemp, CFile::modeRead );

		if(!fileTemp)
			return;

		char *bufTemp = new char [fileTemp.GetLength()];

		fileTemp.Read(bufTemp, fileTemp.GetLength());

		textTemplate = bufTemp;

		SetDlgItemText(IDC_EDIT1, textTemplate);

		Invalidate();

		delete [] bufTemp;
	}
}

BOOL CChooseTextDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetDlgItemText(IDC_EDIT1, textTemplate);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseTextDlg::OnChangeEdit1() 
{
	GetDlgItemText(IDC_EDIT1, textTemplate);	
}
