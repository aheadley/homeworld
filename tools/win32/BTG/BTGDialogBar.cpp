/*
** CBTGVDialogBar.cpp : Code to handle special BTG dialog bars.
*/

#include "stdafx.h"
#include "BTG.h"

#include "MainFrm.h"

#include "btgdialogbar.h"
#include "tgaContainer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBTGVBackgroundBar

BEGIN_MESSAGE_MAP(CBTGVBackgroundBar, CDialogBar)
	//{{AFX_MSG_MAP(CBTGVBackgroundBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBTGVBackgroundBar construction/destruction

CBTGVBackgroundBar::CBTGVBackgroundBar()
{
	red = green = blue = 0;
}

CBTGVBackgroundBar::~CBTGVBackgroundBar()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBTGVBackgroundBar diagnostics

#ifdef _DEBUG
void CBTGVBackgroundBar::AssertValid() const
{
	CDialogBar::AssertValid();
}

void CBTGVBackgroundBar::Dump(CDumpContext& dc) const
{
	CDialogBar::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBTGVBackgroundBar message handlers

void CBTGVBackgroundBar::OnInit(void)
{
	CSliderCtrl *sRed, *sGreen, *sBlue;
	CEdit *eRed, *eGreen, *eBlue;
	char buffer[20];
	
	sRed = (CSliderCtrl *)GetDlgItem(ID_BGREDSLIDER);
	sGreen = (CSliderCtrl *)GetDlgItem(ID_BGGREENSLIDER);
	sBlue = (CSliderCtrl *)GetDlgItem(ID_BGBLUESLIDER);

	sRed->SetRange(0, 255);
	sGreen->SetRange(0, 255);
	sBlue->SetRange(0, 255);

	sRed->SetPos(0);
	sGreen->SetPos(0);
	sBlue->SetPos(0);

	eRed = (CEdit *)GetDlgItem(IDC_EDIT1);
	eGreen = (CEdit *)GetDlgItem(IDC_EDIT2);
	eBlue = (CEdit *)GetDlgItem(IDC_EDIT3);

	eRed->LimitText(3);
	eGreen->LimitText(3);
	eBlue->LimitText(3);

	eRed->SetWindowText(itoa(sRed->GetPos(), buffer, 10));
	eGreen->SetWindowText(itoa(sGreen->GetPos(), buffer, 10));
	eBlue->SetWindowText(itoa(sBlue->GetPos(), buffer, 10));

	bUpdate = 0;
	bFloating = 44;
}

void CBTGVBackgroundBar::OnUpdate(void)
{
	CSliderCtrl *sRed, *sGreen, *sBlue;
	CEdit *eRed, *eGreen, *eBlue;
	char buffer[20];
	CString tRed, tGreen, tBlue;

	sRed = (CSliderCtrl *)GetDlgItem(ID_BGREDSLIDER);
	sGreen = (CSliderCtrl *)GetDlgItem(ID_BGGREENSLIDER);
	sBlue = (CSliderCtrl *)GetDlgItem(ID_BGBLUESLIDER);
	
	eRed = (CEdit *)GetDlgItem(IDC_EDIT1);
	eGreen = (CEdit *)GetDlgItem(IDC_EDIT2);
	eBlue = (CEdit *)GetDlgItem(IDC_EDIT3);

	eRed->GetWindowText(tRed);
	eGreen->GetWindowText(tGreen);
	eBlue->GetWindowText(tBlue);
	
	// Has the slider changed?
	if((atoi(tRed) == red) && (sRed->GetPos() != red))
	{
		red = sRed->GetPos();
		bUpdate = 1;

		eRed->SetWindowText(itoa(sRed->GetPos(), buffer, 10));
	}

	// Has the edit changed?
	else if((atoi(tRed) != red) && (sRed->GetPos() == red))
	{
		red = atoi(tRed);
		if (red > 255) red = 255;

		bUpdate = 1;

		sRed->SetPos(red);
	}

	// Has the slider changed?
	if((atoi(tGreen) == green) && (sGreen->GetPos() != green))
	{
		green = sGreen->GetPos();
		bUpdate = 1;

		eGreen->SetWindowText(itoa(sGreen->GetPos(), buffer, 10));
	}

	// Has the edit changed?
	else if((atoi(tGreen) != green) && (sGreen->GetPos() == green))
	{
		green = atoi(tGreen);
		if (green > 255) green = 255;
		bUpdate = 1;

		sGreen->SetPos(green);
	}

	// Has the slider changed?
	if((atoi(tBlue) == blue) && (sBlue->GetPos() != blue))
	{
		blue = sBlue->GetPos();
		if (blue > 255) blue = 255;
		bUpdate = 1;

		eBlue->SetWindowText(itoa(sBlue->GetPos(), buffer, 10));
	}

	// Has the edit changed?
	else if((atoi(tBlue) != blue) && (sBlue->GetPos() == blue))
	{
		blue = atoi(tBlue);
		bUpdate = 1;

		sBlue->SetPos(blue);
	}

	if(bFloating != IsFloating())
	{
		bUpdate = 1;
		bFloating = IsFloating();
	}
	
	if(bUpdate)
	{
		bUpdate = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBTGVDialogBar

BEGIN_MESSAGE_MAP(CBTGVDialogBar, CDialogBar)
	//{{AFX_MSG_MAP(CBTGVDialogBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBTGVDialogBar construction/destruction

CBTGVDialogBar::CBTGVDialogBar()
{
	red = green = blue = alpha = brightness = 255;
}

CBTGVDialogBar::~CBTGVDialogBar()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBTGVDialogBar diagnostics

#ifdef _DEBUG
void CBTGVDialogBar::AssertValid() const
{
	CDialogBar::AssertValid();
}

void CBTGVDialogBar::Dump(CDumpContext& dc) const
{
	CDialogBar::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBTGVDialogBar message handlers

void CBTGVDialogBar::OnInit(void)
{
	CSliderCtrl *sRed, *sGreen, *sBlue, *sAlpha, *sBrightness;
	CEdit *eRed, *eGreen, *eBlue, *eAlpha, *eBrightness;
	char buffer[20];
	
	sRed = (CSliderCtrl *)GetDlgItem(ID_REDSLIDER);
	sGreen = (CSliderCtrl *)GetDlgItem(ID_GREENSLIDER);
	sBlue = (CSliderCtrl *)GetDlgItem(ID_BLUESLIDER);
	sAlpha = (CSliderCtrl *)GetDlgItem(ID_ALPHASLIDER);
	sBrightness = (CSliderCtrl *)GetDlgItem(ID_BRIGHTNESSSLIDER);

	sRed->SetRange(0, 255);
	sGreen->SetRange(0, 255);
	sBlue->SetRange(0, 255);
	sAlpha->SetRange(0, 255);
	sBrightness->SetRange(0, 255);

	sRed->SetPos(255);
	sGreen->SetPos(255);
	sBlue->SetPos(255);
	sAlpha->SetPos(255);
	sBrightness->SetPos(255);

	eRed = (CEdit *)GetDlgItem(IDC_EDIT1);
	eGreen = (CEdit *)GetDlgItem(IDC_EDIT2);
	eBlue = (CEdit *)GetDlgItem(IDC_EDIT3);
	eAlpha = (CEdit *)GetDlgItem(IDC_EDIT4);
	eBrightness = (CEdit *)GetDlgItem(IDC_EDIT5);

	eRed->LimitText(3);
	eGreen->LimitText(3);
	eBlue->LimitText(3);
	eAlpha->LimitText(3);
	eBrightness->LimitText(3);

	eRed->SetWindowText(itoa(sRed->GetPos(), buffer, 10));
	eGreen->SetWindowText(itoa(sGreen->GetPos(), buffer, 10));
	eBlue->SetWindowText(itoa(sBlue->GetPos(), buffer, 10));
	eAlpha->SetWindowText(itoa(sAlpha->GetPos(), buffer, 10));
	eBrightness->SetWindowText(itoa(sBrightness->GetPos(), buffer, 10));

	bUpdate = 0;
	bFloating = 44;
}

void CBTGVDialogBar::OnUpdate(void)
{
	CSliderCtrl *sRed, *sGreen, *sBlue, *sAlpha, *sBrightness;
	CButton *pButton;
	CEdit *eRed, *eGreen, *eBlue, *eAlpha, *eBrightness;
	char buffer[20];
	CString tRed, tGreen, tBlue, tAlpha, tBrightness;

	sRed = (CSliderCtrl *)GetDlgItem(ID_REDSLIDER);
	sGreen = (CSliderCtrl *)GetDlgItem(ID_GREENSLIDER);
	sBlue = (CSliderCtrl *)GetDlgItem(ID_BLUESLIDER);
	sAlpha = (CSliderCtrl *)GetDlgItem(ID_ALPHASLIDER);
	sBrightness = (CSliderCtrl *)GetDlgItem(ID_BRIGHTNESSSLIDER);
	pButton = (CButton *)GetDlgItem(IDC_COLORPREVIEW);

	eRed = (CEdit *)GetDlgItem(IDC_EDIT1);
	eGreen = (CEdit *)GetDlgItem(IDC_EDIT2);
	eBlue = (CEdit *)GetDlgItem(IDC_EDIT3);
	eAlpha = (CEdit *)GetDlgItem(IDC_EDIT4);
	eBrightness = (CEdit *)GetDlgItem(IDC_EDIT5);

	eRed->GetWindowText(tRed);
	eGreen->GetWindowText(tGreen);
	eBlue->GetWindowText(tBlue);
	eAlpha->GetWindowText(tAlpha);
	eBrightness->GetWindowText(tBrightness);

	// Has the slider changed?
	if((atoi(tRed) == red) && (sRed->GetPos() != red))
	{
		red = sRed->GetPos();
		bUpdate = 1;

		eRed->SetWindowText(itoa(sRed->GetPos(), buffer, 10));
	}

	// Has the edit changed?
	else if((atoi(tRed) != red) && (sRed->GetPos() == red))
	{
		red = atoi(tRed);
		if (red > 255) red = 255;

		bUpdate = 1;

		sRed->SetPos(red);
	}

	// Has the slider changed?
	if((atoi(tGreen) == green) && (sGreen->GetPos() != green))
	{
		green = sGreen->GetPos();
		bUpdate = 1;

		eGreen->SetWindowText(itoa(sGreen->GetPos(), buffer, 10));
	}

	// Has the edit changed?
	else if((atoi(tGreen) != green) && (sGreen->GetPos() == green))
	{
		green = atoi(tGreen);
		if (green > 255) green = 255;
		bUpdate = 1;

		sGreen->SetPos(green);
	}

	// Has the slider changed?
	if((atoi(tBlue) == blue) && (sBlue->GetPos() != blue))
	{
		blue = sBlue->GetPos();
		if (blue > 255) blue = 255;
		bUpdate = 1;

		eBlue->SetWindowText(itoa(sBlue->GetPos(), buffer, 10));
	}

	// Has the edit changed?
	else if((atoi(tBlue) != blue) && (sBlue->GetPos() == blue))
	{
		blue = atoi(tBlue);
		bUpdate = 1;

		sBlue->SetPos(blue);
	}

	// Has the slider changed?
	if((atoi(tAlpha) == alpha) && (sAlpha->GetPos() != alpha))
	{
		alpha = sAlpha->GetPos();
		if (alpha > 255) alpha = 255;
		bUpdate = 1;

		eAlpha->SetWindowText(itoa(sAlpha->GetPos(), buffer, 10));
	}

	// Has the edit changed?
	else if((atoi(tAlpha) != alpha) && (sAlpha->GetPos() == alpha))
	{
		alpha = atoi(tAlpha);
		bUpdate = 1;

		sAlpha->SetPos(alpha);
	}

	// Has the slider changed?
	if((atoi(tBrightness) == brightness) && (sBrightness->GetPos() != brightness))
	{
		brightness = sBrightness->GetPos();
		if (brightness > 255) brightness = 255;
		bUpdate = 1;

		eBrightness->SetWindowText(itoa(sBrightness->GetPos(), buffer, 10));
	}

	// Has the edit changed?
	else if((atoi(tBrightness) != brightness) && (sBrightness->GetPos() == brightness))
	{
		brightness = atoi(tBrightness);
		bUpdate = 1;

		sBrightness->SetPos(brightness);
	}

	if(bFloating != IsFloating())
	{
		bUpdate = 1;
		bFloating = IsFloating();
	}
	
	if(bUpdate)
	{
		CDC *pDC = pButton->GetDC();
		CRect tempRect;
		pButton->GetClientRect(&tempRect);
		CBrush tempBrush(RGB(red, green, blue));
		pDC->SelectObject(&tempBrush);
		pDC->Rectangle(tempRect.left, tempRect.top, tempRect.right, tempRect.bottom);

		bUpdate = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBTGVisibleBar

BEGIN_MESSAGE_MAP(CBTGVisibleBar, CDialogBar)
	//{{AFX_MSG_MAP(CBTGVisibleBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBTGVisibleBar construction/destruction

CBTGVisibleBar::CBTGVisibleBar()
{
	bStars = 1;
	renderMode = CBTGDoc::btgRenderMode_Realistic;
}

CBTGVisibleBar::~CBTGVisibleBar()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBTGVisibleBar diagnostics

#ifdef _DEBUG
void CBTGVisibleBar::AssertValid() const
{
	CDialogBar::AssertValid();
}

void CBTGVisibleBar::Dump(CDumpContext& dc) const
{
	CDialogBar::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBTGVisibleBar message handlers

void CBTGVisibleBar::OnInit(void)
{
	CButton *check2, *radio2, *radio3;
	
	check2 = (CButton *)GetDlgItem(IDC_CHECK2);
	radio2 = (CButton *)GetDlgItem(IDC_RADIO2);
	radio3 = (CButton *)GetDlgItem(IDC_RADIO3);

	check2->SetCheck(TRUE);
	radio2->SetCheck(TRUE);
	radio3->SetCheck(FALSE);
}

void CBTGVisibleBar::OnUpdate(void)
{
	CButton *check2, *radio2, *radio3;
	
	check2 = (CButton *)GetDlgItem(IDC_CHECK2);
	radio2 = (CButton *)GetDlgItem(IDC_RADIO2);	
	radio3 = (CButton *)GetDlgItem(IDC_RADIO3);

	bStars = check2->GetCheck();
	
	if (radio2->GetCheck())
	{
		renderMode = CBTGDoc::btgRenderMode_Realistic;
	}
	if (radio3->GetCheck())
	{
		renderMode = CBTGDoc::btgRenderMode_3D;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBTGBitmapBar

BEGIN_MESSAGE_MAP(CBTGBitmapBar, CDialogBar)
	//{{AFX_MSG_MAP(CBTGBitmapBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBTGBitmapBar construction/destruction

CBTGBitmapBar::CBTGBitmapBar()
{
	curFileName = 0;
}

CBTGBitmapBar::~CBTGBitmapBar()
{
	if(curFileName)
		delete [] curFileName;
}

/////////////////////////////////////////////////////////////////////////////
// CBTGBitmapBar diagnostics

#ifdef _DEBUG
void CBTGBitmapBar::AssertValid() const
{
	CDialogBar::AssertValid();
}

void CBTGBitmapBar::Dump(CDumpContext& dc) const
{
	CDialogBar::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBTGBitmapBar message handlers


void CBTGBitmapBar::OnInit(void)
{
	HANDLE hSearch;
	WIN32_FIND_DATA	fileInfo;
	CComboBox *pCBox;
	CTGAContainer *pContainer;
	
	pCBox = (CComboBox *)GetDlgItem(IDC_COMBO1);
	
	hSearch = FindFirstFile(BITMAP_SEARCH_DIRECTORY, &fileInfo);

	if(hSearch == INVALID_HANDLE_VALUE)
	{
		FindClose(hSearch);
		return;
	}

	// Add this one.
	pCBox->AddString(fileInfo.cFileName);

	// Load the .TGA.
	pContainer = new CTGAContainer(fileInfo.cFileName);
	gTGAFileList.AddNode(pContainer);
	
	while(FindNextFile(hSearch, &fileInfo))
	{
		// Add this one.
		pCBox->AddString(fileInfo.cFileName);

		// Load the .BMP.
		pContainer = new CTGAContainer(fileInfo.cFileName);
		gTGAFileList.AddNode(pContainer);
	}
	
	FindClose(hSearch);

	// Select the first one.
	pCBox->SetCurSel(0);
}

void CBTGBitmapBar::OnUpdate(void)
{
	CComboBox *pCBox;
	CString tempStr;

	pCBox = (CComboBox *)GetDlgItem(IDC_COMBO1);

	int index = pCBox->GetCurSel();
	pCBox->GetLBText(index, tempStr);

	if(curFileName)
	{
		if(!strcmp(curFileName, tempStr))
			return;
	}
	
	if(curFileName)
		delete [] curFileName;

	curFileName = new char [strlen(tempStr) + 1];
	strcpy(curFileName, tempStr);
}
