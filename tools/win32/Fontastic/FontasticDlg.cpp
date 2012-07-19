// FontasticDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fontastic.h"
#include "FontasticDlg.h"
#include "ChooseTextDlg.h"
#include "hff.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFontasticDlg dialog

CFontasticDlg::CFontasticDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFontasticDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFontasticDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFontasticDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFontasticDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFontasticDlg, CDialog)
	//{{AFX_MSG_MAP(CFontasticDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHOOSEFONT, OnChoosefont)
	ON_BN_CLICKED(IDC_SAVEAS, OnSaveas)
	ON_BN_CLICKED(IDC_ENABLEANTIALIASING, OnEnableantialiasing)
	ON_BN_CLICKED(IDC_ENABLEDROPSHADOW, OnEnabledropshadow)
	ON_BN_CLICKED(IDC_CHOOSETEXT, OnChoosetext)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontasticDlg message handlers

BOOL CFontasticDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CButton *tempButton;

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	tempButton = (CButton *)GetDlgItem(IDC_ENABLEANTIALIASING);

	tempButton->SetCheck(TRUE);

	// Check the light antialiasing radio.
	tempButton = (CButton *)GetDlgItem(IDC_RADIO1);
	tempButton->SetCheck(TRUE);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFontasticDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CFont fTemp;
		CStatic *pStatic = (CStatic *)GetDlgItem(IDC_PREVIEW);

		fTemp.CreateFontIndirect(&gFont);

		pStatic->SetFont(&fTemp);
		
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFontasticDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CFontasticDlg::OnChoosefont() 
{
	CFontDialog dlgTemp(&gFont, CF_EFFECTS | CF_SCREENFONTS, NULL, this);

	dlgTemp.DoModal();

	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_PREVIEW);

	Invalidate();
}

void CFontasticDlg::OnSaveas() 
{
	static char BASED_CODE szFilter[] = "Homeworld Font Files (*.hff)|*.hff|All Files (*.*)|*.*||";
	char *tempFileName;

	tempFileName = FixFontName(&gFont);
	
	CFileDialog dlgTemp(FALSE, ".HFF", tempFileName, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilter, this);

	delete tempFileName;

	int nResult = dlgTemp.DoModal();

	if(nResult == IDOK)
	{
		// Save the file here.
		CString strTemp = dlgTemp.GetFileName();

		SaveFontFile(strTemp);
		
		CDialog::OnOK();
	}
}

void CFontasticDlg::OnEnableantialiasing() 
{
	enableAntialiasing = 1 - enableAntialiasing;
}

void CFontasticDlg::OnEnabledropshadow() 
{
#if 1
	enableDropshadow = 1 - enableDropshadow;
#else
	CButton *pButton = (CButton *)GetDlgItem(IDC_ENABLEDROPSHADOW);

	pButton->SetCheck(0);

	MessageBox("Dropshadow not supported yet.");
#endif
}

void CFontasticDlg::OnChoosetext() 
{
	CChooseTextDlg dlgTemp;
		
	dlgTemp.textTemplate = chooseTextTemplate;
	
	int nResult = dlgTemp.DoModal();

	if(nResult == IDOK)
	{
		chooseTextTemplate = dlgTemp.textTemplate;
	}
}

void CFontasticDlg::SaveFontFile(CString &fileName)
{
	SaveHomeworldFontFile	shffTemp;

	shffTemp.SaveHFF(fileName);
}

char *CFontasticDlg::FixFontName(LOGFONT *tFont)
{
	char *pRet, pPtSize[10];
	unsigned long lenPRet = 0;
	char bold = 0, underline = 0, strikeout = 0, italics = 0;
	unsigned long i;
	
	pRet = new char [strlen("default.hff") + 1];

	strcpy(pRet, "default.hff");

	if(!tFont->lfFaceName)
		return(pRet);

	if(abs(tFont->lfHeight) < 1)
		return(pRet);

	delete [] pRet;

	lenPRet += strlen(tFont->lfFaceName) + 4;

	// Add underscore.
	lenPRet ++;

	// Add bold?
	if(tFont->lfWeight == 700)
	{
		lenPRet ++;
		bold = 1;
	}

	if(tFont->lfUnderline)
	{
		lenPRet ++;
		underline = 1;
	}

	if(tFont->lfStrikeOut)
	{
		lenPRet ++;
		strikeout = 1;
	}

	if(tFont->lfItalic)
	{
		lenPRet ++;
		italics = 1;
	}

	// Add point size.
	lenPRet += strlen(itoa(abs(tFont->lfHeight), pPtSize, 10));

	pRet = new char [lenPRet + 1];

	strcpy(pRet, tFont->lfFaceName);

	strcat(pRet, "_");

	if(bold)
	{
		strcat(pRet, "b");
	}

	if(underline)
	{
		strcat(pRet, "u");
	}

	if(strikeout)
	{
		strcat(pRet, "s");
	}

	if(italics)
	{
		strcat(pRet, "i");
	}

	strcat(pRet, pPtSize);

	strcat(pRet, ".hff");

	for(i=0;i<strlen(pRet);i++)
	{
		if(pRet[i] == ' ')
			pRet[i] = '_';
	}

	return(pRet);
}

void CFontasticDlg::OnRadio1() 
{
	gAntialiasingLevel = 1;
	
}

void CFontasticDlg::OnRadio2() 
{
	gAntialiasingLevel = 2;
}
