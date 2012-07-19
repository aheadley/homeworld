// StringObjectStringPropertiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FEMan.h"
#include "StringObjectStringPropertiesDlg.h"
#include "screen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStringObjectStringPropertiesDlg dialog


CStringObjectStringPropertiesDlg::CStringObjectStringPropertiesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStringObjectStringPropertiesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStringObjectStringPropertiesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

    memset(myString, 0, sizeof(myString));
	//myString = 0;

	userInfo = 0;
    justification = JUST_CENTRE;
}


void CStringObjectStringPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStringObjectStringPropertiesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStringObjectStringPropertiesDlg, CDialog)
	//{{AFX_MSG_MAP(CStringObjectStringPropertiesDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT4, OnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT5, OnChangeEdit5)
	ON_EN_CHANGE(IDC_EDIT6, OnChangeEdit6)
	ON_EN_CHANGE(IDC_EDIT7, OnChangeEdit7)
	ON_EN_CHANGE(IDC_EDIT8, OnChangeEdit8)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStringObjectStringPropertiesDlg message handlers

BOOL CStringObjectStringPropertiesDlg::OnInitDialog()
{
	CButton *tempButton;
	
	CDialog::OnInitDialog();
	
	// Set the first radio item.
	switch(contentVisible)
	{
	case ScreenObjectString::CT_NORMAL:
		tempButton = (CButton *)GetDlgItem(IDC_RADIO1);
		tempButton->SetCheck(1);

		tempButton = (CButton *)GetDlgItem(IDC_RADIO2);
		tempButton->SetCheck(0);

		tempButton = (CButton *)GetDlgItem(IDC_RADIO3);
		tempButton->SetCheck(0);
		break;

	case ScreenObjectString::CT_MASKED:
		tempButton = (CButton *)GetDlgItem(IDC_RADIO1);
		tempButton->SetCheck(0);

		tempButton = (CButton *)GetDlgItem(IDC_RADIO2);
		tempButton->SetCheck(1);

		tempButton = (CButton *)GetDlgItem(IDC_RADIO3);
		tempButton->SetCheck(0);
		break;

	case ScreenObjectString::CT_NONE:
		tempButton = (CButton *)GetDlgItem(IDC_RADIO1);
		tempButton->SetCheck(0);

		tempButton = (CButton *)GetDlgItem(IDC_RADIO2);
		tempButton->SetCheck(0);

		tempButton = (CButton *)GetDlgItem(IDC_RADIO3);
		tempButton->SetCheck(1);
		break;
	}

	tempButton = (CButton *)GetDlgItem(IDC_CHECK1);
	tempButton->SetCheck(autoSizeToContent);

	SetDlgItemText(IDC_EDIT13, myString[0]);
	SetDlgItemText(IDC_EDIT14, myString[1]);
	SetDlgItemText(IDC_EDIT15, myString[2]);
	SetDlgItemText(IDC_EDIT16, myString[3]);
	SetDlgItemText(IDC_EDIT17, myString[4]);
	SetDlgItemText(IDC_EDIT18, myString[5]);

	SetDlgItemInt(IDC_EDIT1, GetRValue(textColor), FALSE);
	SetDlgItemInt(IDC_EDIT4, GetGValue(textColor), FALSE);
	SetDlgItemInt(IDC_EDIT5, GetBValue(textColor), FALSE);

	SetDlgItemInt(IDC_EDIT6, GetRValue(textBackground), FALSE);
	SetDlgItemInt(IDC_EDIT7, GetGValue(textBackground), FALSE);
	SetDlgItemInt(IDC_EDIT8, GetBValue(textBackground), FALSE);

	SetDlgItemText(IDC_EDIT10, userInfo);

	// Limit the text.
	CEdit *pEdit;

	pEdit = (CEdit *)GetDlgItem(IDC_EDIT1);
	pEdit->SetLimitText(3);
	pEdit = (CEdit *)GetDlgItem(IDC_EDIT4);
	pEdit->SetLimitText(3);
	pEdit = (CEdit *)GetDlgItem(IDC_EDIT5);
	pEdit->SetLimitText(3);
	pEdit = (CEdit *)GetDlgItem(IDC_EDIT6);
	pEdit->SetLimitText(3);
	pEdit = (CEdit *)GetDlgItem(IDC_EDIT7);
	pEdit->SetLimitText(3);
	pEdit = (CEdit *)GetDlgItem(IDC_EDIT8);
	pEdit->SetLimitText(3);

    switch (justification)
    {
        case JUST_LEFT:
    		tempButton = (CButton *)GetDlgItem(IDC_JUST_LEFT);
    		tempButton->SetCheck(1);
    		tempButton = (CButton *)GetDlgItem(IDC_JUST_RIGHT);
    		tempButton->SetCheck(0);
    		tempButton = (CButton *)GetDlgItem(IDC_JUST_CENTRE);
    		tempButton->SetCheck(0);
            break;
        case JUST_RIGHT:
    		tempButton = (CButton *)GetDlgItem(IDC_JUST_LEFT);
    		tempButton->SetCheck(0);
    		tempButton = (CButton *)GetDlgItem(IDC_JUST_RIGHT);
    		tempButton->SetCheck(1);
    		tempButton = (CButton *)GetDlgItem(IDC_JUST_CENTRE);
    		tempButton->SetCheck(0);
            break;
        case JUST_CENTRE:
    		tempButton = (CButton *)GetDlgItem(IDC_JUST_LEFT);
    		tempButton->SetCheck(0);
    		tempButton = (CButton *)GetDlgItem(IDC_JUST_RIGHT);
    		tempButton->SetCheck(0);
    		tempButton = (CButton *)GetDlgItem(IDC_JUST_CENTRE);
    		tempButton->SetCheck(1);
            break;
    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStringObjectStringPropertiesDlg::OnButton1()
{
	CFontDialog dlg(&myFont);

	if(dlg.DoModal() == IDOK)
	{
		dlg.GetCurrentFont(&myFont);
	}
}

unsigned int editIndexTable[NUMBER_LANGUAGES] =
{
    IDC_EDIT13,
    IDC_EDIT14,
    IDC_EDIT15,
    IDC_EDIT16,
    IDC_EDIT17,
    IDC_EDIT18
};
void CStringObjectStringPropertiesDlg::OnOK()
{
    unsigned long index;
	CString tempString;
	CButton *tempButton;

	// Copy over the information in IDC_EDIT13..18 into the internal myString[] pointer array.
    for (index = 0; index < NUMBER_LANGUAGES; index++)
    {
        if (myString[index] != NULL)
        {
            delete [] myString[index];
            myString[index] = 0;
        }
    	GetDlgItemText(editIndexTable[index], tempString);

    	if(tempString.GetLength())
    	{
    		myString[index] = new char [tempString.GetLength() + 1];

    		strcpy(myString[index], tempString.GetBuffer(1));
    	}
    }
/*
	// Copy over the information in IDC_EDIT13 into the internal myString pointer.
	if(myString)
	{
		// Clean out the old one.
		delete [] myString;

		myString = 0;
	}

	CString tempString;

	GetDlgItemText(IDC_EDIT13, tempString);

	if(tempString.GetLength())
	{
		myString = new char [tempString.GetLength() + 1];

		strcpy(myString, tempString.GetBuffer(1));
	}
*/

	GetDlgItemText(IDC_EDIT10, tempString);

	if(userInfo)
			delete [] userInfo;

	userInfo = 0;

	if(tempString.GetLength())
	{
		userInfo = new char [tempString.GetLength() + 1];

		strcpy(userInfo, tempString.GetBuffer(1));
	}

    tempButton = (CButton *)GetDlgItem(IDC_JUST_LEFT);
    if (tempButton->GetCheck())
    {
        justification = JUST_LEFT;
    }
    tempButton = (CButton *)GetDlgItem(IDC_JUST_RIGHT);
    if (tempButton->GetCheck())
    {
        justification = JUST_RIGHT;
    }
    tempButton = (CButton *)GetDlgItem(IDC_JUST_CENTRE);
    if (tempButton->GetCheck())
    {
        justification = JUST_CENTRE;
    }


	CDialog::OnOK();
}

void CStringObjectStringPropertiesDlg::OnRadio1()
{
	contentVisible = ScreenObjectString::CT_NORMAL;
}

void CStringObjectStringPropertiesDlg::OnRadio2()
{
	contentVisible = ScreenObjectString::CT_MASKED;	
}

void CStringObjectStringPropertiesDlg::OnRadio3()
{
	contentVisible = ScreenObjectString::CT_NONE;	
}

void CStringObjectStringPropertiesDlg::OnChangeEdit1()
{
	BOOL temp;

	textColor = RGB(GetDlgItemInt(IDC_EDIT1, &temp, FALSE),
				    GetGValue(textColor), GetBValue(textColor));
}

void CStringObjectStringPropertiesDlg::OnChangeEdit4()
{
	BOOL temp;

	textColor = RGB(GetRValue(textColor),
				    GetDlgItemInt(IDC_EDIT4, &temp, FALSE), GetBValue(textColor));
}

void CStringObjectStringPropertiesDlg::OnChangeEdit5()
{
	BOOL temp;

	textColor = RGB(GetRValue(textColor),
				    GetGValue(textColor), GetDlgItemInt(IDC_EDIT5, &temp, FALSE));
}

void CStringObjectStringPropertiesDlg::OnChangeEdit6()
{
	BOOL temp;

	textBackground = RGB(GetDlgItemInt(IDC_EDIT6, &temp, FALSE),
				         GetGValue(textBackground), GetBValue(textBackground));
}

void CStringObjectStringPropertiesDlg::OnChangeEdit7()
{
	BOOL temp;

	textBackground = RGB(GetRValue(textBackground),
						 GetDlgItemInt(IDC_EDIT7, &temp, FALSE), GetBValue(textBackground));
}

void CStringObjectStringPropertiesDlg::OnChangeEdit8()
{
	BOOL temp;

	textBackground = RGB(GetRValue(textBackground),
						 GetGValue(textBackground), GetDlgItemInt(IDC_EDIT8, &temp, FALSE));
}

void CStringObjectStringPropertiesDlg::OnButton3()
{
	// Color picker for the text color.

	CHOOSECOLOR chooseColorInfo;
	COLORREF	defaultColors[16];

	memset(defaultColors, 0x00, sizeof(COLORREF) * 16);

	chooseColorInfo.lStructSize = sizeof(CHOOSECOLOR);
	chooseColorInfo.hwndOwner = NULL;
	chooseColorInfo.rgbResult = textColor;
	chooseColorInfo.lpCustColors = defaultColors;
	chooseColorInfo.Flags = CC_RGBINIT | CC_FULLOPEN;
	chooseColorInfo.lpTemplateName = NULL;

	ChooseColor(&chooseColorInfo);

	textColor = chooseColorInfo.rgbResult;

	SetDlgItemInt(IDC_EDIT1, GetRValue(textColor), FALSE);
	SetDlgItemInt(IDC_EDIT4, GetGValue(textColor), FALSE);
	SetDlgItemInt(IDC_EDIT5, GetBValue(textColor), FALSE);
}

void CStringObjectStringPropertiesDlg::OnButton2()
{
	// Color picker for the text background.

	CHOOSECOLOR chooseColorInfo;
	COLORREF	defaultColors[16];

	memset(defaultColors, 0x00, sizeof(COLORREF) * 16);

	chooseColorInfo.lStructSize = sizeof(CHOOSECOLOR);
	chooseColorInfo.hwndOwner = NULL;
	chooseColorInfo.rgbResult = textBackground;
	chooseColorInfo.lpCustColors = defaultColors;
	chooseColorInfo.Flags = CC_RGBINIT | CC_FULLOPEN;
	chooseColorInfo.lpTemplateName = NULL;

	ChooseColor(&chooseColorInfo);

	textBackground = chooseColorInfo.rgbResult;

	SetDlgItemInt(IDC_EDIT6, GetRValue(textBackground), FALSE);
	SetDlgItemInt(IDC_EDIT7, GetGValue(textBackground), FALSE);
	SetDlgItemInt(IDC_EDIT8, GetBValue(textBackground), FALSE);
}

void CStringObjectStringPropertiesDlg::OnCheck1()
{
	autoSizeToContent = 1 - autoSizeToContent;
}
