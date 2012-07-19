// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "FEMan.h"

#include "MainFrm.h"

#include "FlowTreeView.h"
#include "ScreenEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

unsigned long languageToView = 0;
/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_ACTIVATE()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_OPTIONS_CHOOSEDEFAULTFONT, OnOptionsChoosedefaultfont)
	ON_COMMAND(ID_VIEW_LANGUAGE_ENGLISH,    OnOptionsViewLanguageEnglish    )
	ON_COMMAND(ID_VIEW_LANGUAGE_FRENCH,     OnOptionsViewLanguageFrench     )
	ON_COMMAND(ID_VIEW_LANGUAGE_GERMAN,     OnOptionsViewLanguageGerman     )
	ON_COMMAND(ID_VIEW_LANGUAGE_PIGLATIN,   OnOptionsViewLanguagePigLatin   )
	ON_COMMAND(ID_VIEW_LANGUAGE_SWAHILI,    OnOptionsViewLanguageSwahili    )
	ON_COMMAND(ID_VIEW_LANGUAGE_SANSKRIT,   OnOptionsViewLanguageSanskrit   )
	ON_WM_MOVE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

unsigned long identByLanguage[NUMBER_LANGUAGES] =
{
    ID_VIEW_LANGUAGE_ENGLISH,
    ID_VIEW_LANGUAGE_FRENCH,
    ID_VIEW_LANGUAGE_GERMAN,
    ID_VIEW_LANGUAGE_PIGLATIN,
    ID_VIEW_LANGUAGE_SWAHILI,
    ID_VIEW_LANGUAGE_SANSKRIT
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	activeWindowRow = AWR_LAST_AWR;
}

CMainFrame::~CMainFrame()
{
	AfxGetApp()->WriteProfileInt("Size Information", "Application Frame Left", gAppRect.left);
	AfxGetApp()->WriteProfileInt("Size Information", "Application Frame Top", gAppRect.top);
	AfxGetApp()->WriteProfileInt("Size Information", "Application Frame Width", gAppRect.right);
	AfxGetApp()->WriteProfileInt("Size Information", "Application Frame Height", gAppRect.bottom);

	AfxGetApp()->WriteProfileInt("Size Information", "Application Splitter Width", gSplitWidth);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    unsigned long index;

    for (index = 1; index < NUMBER_LANGUAGES; index++)
    {
        GetMenu()->CheckMenuItem(identByLanguage[index], MF_BYCOMMAND | MF_UNCHECKED);
    }
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	int l, t, w, h;

	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	
	l = AfxGetApp()->GetProfileInt("Size Information", "Application Frame Left", -1000);
	t = AfxGetApp()->GetProfileInt("Size Information", "Application Frame Top", -1000);
	w = AfxGetApp()->GetProfileInt("Size Information", "Application Frame Width", -1000);
	h = AfxGetApp()->GetProfileInt("Size Information", "Application Frame Height", -1000);

	if((l != -1000) && (t != -1000) && (w != -1000) && (h != -1000))
	{
		cs.x = l;
		cs.y = t;
		cs.cx = w;
		cs.cy = h;
	}

	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	BOOL bRet;

	// This actually creates the split panes.
	bRet = m_wndSplitter.CreateStatic(this, 1, 2, WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL);	

	if(bRet == FALSE)
		return(FALSE);

	gSplitWidth = AfxGetApp()->GetProfileInt("Size Information", "Application Splitter Width", 300);
	
	// Create and attach the FlowTreeView.
	bRet = m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CFlowTreeView), CSize(gSplitWidth, 0), pContext);

	if(bRet == FALSE)
		return(FALSE);

	// Create and attach the ScreenEditView.
	bRet = m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CScreenEditView), CSize(0, 0), pContext);

	if(bRet == FALSE)
		return(FALSE);

	// Calling the parent actually negates what we are trying to accomplish here.
	//return CFrameWnd::OnCreateClient(lpcs, pContext);

	// Everything was A-OK.
	return(TRUE);
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
}

void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	CFrameWnd::OnSetFocus(pOldWnd);
}

void CMainFrame::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd::OnLButtonDown(nFlags, point);
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	int newActiveWindowRow;
	
	m_wndSplitter.GetActivePane(NULL, &newActiveWindowRow);

	if(newActiveWindowRow != activeWindowRow)
	{
		activeWindowRow = newActiveWindowRow;

		if(activeWindowRow == AWR_SCREENEDIT)
		{
			m_hAccelTable = LoadAccelerators(theApp.m_hInstance, MAKEINTRESOURCE(IDR_SCREENEDIT_ACCEL));
		}
		else // FLOWTREE
		{
			m_hAccelTable = LoadAccelerators(theApp.m_hInstance, MAKEINTRESOURCE(IDR_FLOWTREE_ACCEL));
		}
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}


void CMainFrame::OnOptionsChoosedefaultfont()
{
	CFontDialog dlg(&gDefaultFont);

	if(dlg.DoModal() == IDOK)
	{
		dlg.GetCurrentFont(&gDefaultFont);
	}
}
/*-----------------------------------------------------------------------------
    Name        : CMainFrame::OnOptionsViewLanguageXXX
    Description : Callbacks to set the language to view
    Inputs      : void
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CMainFrame::OnOptionsViewLanguageEnglish()
{
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_UNCHECKED);
    languageToView = 0;
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_CHECKED);
    Invalidate();
}
void CMainFrame::OnOptionsViewLanguageFrench()
{
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_UNCHECKED);
    languageToView = 1;
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_CHECKED);
    Invalidate();
}
void CMainFrame::OnOptionsViewLanguageGerman()
{
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_UNCHECKED);
    languageToView = 2;
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_CHECKED);
    Invalidate();
}
void CMainFrame::OnOptionsViewLanguagePigLatin()
{
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_UNCHECKED);
    languageToView = 3;
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_CHECKED);
    Invalidate();
}
void CMainFrame::OnOptionsViewLanguageSwahili()
{
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_UNCHECKED);
    languageToView = 4;
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_CHECKED);
    Invalidate();
}
void CMainFrame::OnOptionsViewLanguageSanskrit()
{
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_UNCHECKED);
    languageToView = 5;
    GetMenu()->CheckMenuItem(identByLanguage[languageToView], MF_BYCOMMAND | MF_CHECKED);
    Invalidate();
}

void CMainFrame::OnMove(int x, int y)
{
	CFrameWnd::OnMove(x, y);

	gAppRect.left = x;
	gAppRect.top = y;
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
	
	gAppRect.right = cx;
	gAppRect.bottom = cy;
}
