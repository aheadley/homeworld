// ScreenObjectRectPropertiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FEMan.h"
#include "ScreenObjectRectPropertiesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScreenObjectRectPropertiesDlg dialog


CScreenObjectRectPropertiesDlg::CScreenObjectRectPropertiesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScreenObjectRectPropertiesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScreenObjectRectPropertiesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	userInfo = 0;
}


void CScreenObjectRectPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScreenObjectRectPropertiesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScreenObjectRectPropertiesDlg, CDialog)
	//{{AFX_MSG_MAP(CScreenObjectRectPropertiesDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_BN_CLICKED(IDC_CHECK2, OnCheck2)
	ON_EN_CHANGE(IDC_EDIT6, OnChangeEdit6)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT4, OnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT5, OnChangeEdit5)
	ON_EN_CHANGE(IDC_EDIT11, OnChangeEdit11)
	ON_EN_CHANGE(IDC_EDIT12, OnChangeEdit12)
	ON_EN_CHANGE(IDC_EDIT7, OnChangeEdit7)
	ON_EN_CHANGE(IDC_EDIT8, OnChangeEdit8)
	ON_EN_CHANGE(IDC_EDIT9, OnChangeEdit9)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScreenObjectRectPropertiesDlg message handlers

void CScreenObjectRectPropertiesDlg::OnButton1() 
{
	// Color picker for the border color.

	CHOOSECOLOR chooseColorInfo;
	COLORREF	defaultColors[16];

	memset(defaultColors, 0x00, sizeof(COLORREF) * 16);

	chooseColorInfo.lStructSize = sizeof(CHOOSECOLOR);
	chooseColorInfo.hwndOwner = NULL;
	chooseColorInfo.rgbResult = borderColor;
	chooseColorInfo.lpCustColors = defaultColors;
	chooseColorInfo.Flags = CC_RGBINIT | CC_FULLOPEN;
	chooseColorInfo.lpTemplateName = NULL;

	ChooseColor(&chooseColorInfo);

	borderColor = chooseColorInfo.rgbResult;

	SetDlgItemInt(IDC_EDIT1, GetRValue(borderColor), FALSE);
	SetDlgItemInt(IDC_EDIT4, GetGValue(borderColor), FALSE);
	SetDlgItemInt(IDC_EDIT5, GetBValue(borderColor), FALSE);
}

void CScreenObjectRectPropertiesDlg::OnButton2() 
{
	// Color picker for the content color.

	CHOOSECOLOR chooseColorInfo;
	COLORREF	defaultColors[16];

	memset(defaultColors, 0x00, sizeof(COLORREF) * 16);

	chooseColorInfo.lStructSize = sizeof(CHOOSECOLOR);
	chooseColorInfo.hwndOwner = NULL;
	chooseColorInfo.rgbResult = contentColor;
	chooseColorInfo.lpCustColors = defaultColors;
	chooseColorInfo.Flags = CC_RGBINIT | CC_FULLOPEN;
	chooseColorInfo.lpTemplateName = NULL;

	ChooseColor(&chooseColorInfo);

	contentColor = chooseColorInfo.rgbResult;

	SetDlgItemInt(IDC_EDIT7, GetRValue(contentColor), FALSE);
	SetDlgItemInt(IDC_EDIT8, GetGValue(contentColor), FALSE);
	SetDlgItemInt(IDC_EDIT9, GetBValue(contentColor), FALSE);
}

void CScreenObjectRectPropertiesDlg::OnCheck1() 
{
	// Checked = borderVisible. 
	// Unchecked = borderHidden.

	// Already set so just flip.
	borderVisible = 1 - borderVisible;
}

BOOL CScreenObjectRectPropertiesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CButton *tempButton = (CButton *)GetDlgItem(IDC_CHECK1);
	
	tempButton->SetCheck(borderVisible);

	tempButton = (CButton *)GetDlgItem(IDC_CHECK2);
	
	tempButton->SetCheck(contentVisible);

	SetDlgItemInt(IDC_EDIT6, borderWidth, FALSE);

	SetDlgItemInt(IDC_EDIT1, GetRValue(borderColor), FALSE);
	SetDlgItemInt(IDC_EDIT4, GetGValue(borderColor), FALSE);
	SetDlgItemInt(IDC_EDIT5, GetBValue(borderColor), FALSE);

	SetDlgItemInt(IDC_EDIT7, GetRValue(contentColor), FALSE);
	SetDlgItemInt(IDC_EDIT8, GetGValue(contentColor), FALSE);
	SetDlgItemInt(IDC_EDIT9, GetBValue(contentColor), FALSE);

	SetDlgItemInt(IDC_EDIT11, contentWidth, FALSE);
	SetDlgItemInt(IDC_EDIT12, contentHeight, FALSE);

	SetDlgItemText(IDC_EDIT10, userInfo);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScreenObjectRectPropertiesDlg::OnCheck2() 
{
	// Checked = borderVisible. 
	// Unchecked = borderHidden.

	// Already set so just flip.
	contentVisible = 1 - contentVisible;
}

void CScreenObjectRectPropertiesDlg::OnChangeEdit6() 
{
	// the border width changed.
	borderWidth = GetDlgItemInt(IDC_EDIT6);
}

void CScreenObjectRectPropertiesDlg::OnChangeEdit1() 
{
	// the border red changed.
	borderColor = RGB(GetDlgItemInt(IDC_EDIT1), GetGValue(borderColor), GetBValue(borderColor));
}

void CScreenObjectRectPropertiesDlg::OnChangeEdit4() 
{
	// the border green changed.
	borderColor = RGB(GetRValue(borderColor), GetDlgItemInt(IDC_EDIT4), GetBValue(borderColor));
}

void CScreenObjectRectPropertiesDlg::OnChangeEdit5() 
{
	// the border blue changed.
	borderColor = RGB(GetRValue(borderColor), GetGValue(borderColor), GetDlgItemInt(IDC_EDIT5));
}

void CScreenObjectRectPropertiesDlg::OnChangeEdit11() 
{
	// the content width changed.
	contentWidth = GetDlgItemInt(IDC_EDIT11);	
}

void CScreenObjectRectPropertiesDlg::OnChangeEdit12() 
{
	// the content height changed.
	contentHeight = GetDlgItemInt(IDC_EDIT12);	
}

void CScreenObjectRectPropertiesDlg::OnChangeEdit7() 
{
	// the content red changed.
	contentColor = RGB(GetDlgItemInt(IDC_EDIT7), GetGValue(contentColor), GetBValue(contentColor));	
}

void CScreenObjectRectPropertiesDlg::OnChangeEdit8() 
{
	// the content green changed.
	contentColor = RGB(GetRValue(contentColor), GetDlgItemInt(IDC_EDIT8), GetBValue(contentColor));
}

void CScreenObjectRectPropertiesDlg::OnChangeEdit9() 
{
	// the content blue changed.
	contentColor = RGB(GetRValue(contentColor), GetGValue(contentColor), GetDlgItemInt(IDC_EDIT9));
}

void CScreenObjectRectPropertiesDlg::OnOK() 
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
