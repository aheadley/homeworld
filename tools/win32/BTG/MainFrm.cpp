// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "BTG.h"

#include "MainFrm.h"
#include "btgdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CBTGVisibleBar *gpm_wndVisibleBar;
CBTGVDialogBar *gpm_wndDialogBar;
CBTGVBackgroundBar *gpm_wndBackgroundBar;
ccList gTGAFileList;					// Holds Bitmaps.
CBTGDoc *gBTGDoc;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_COLOR_CONTROL, OnViewColorControl)
	ON_UPDATE_COMMAND_UI(ID_BTG_POLYCOUNT, OnUpdateBtgPolyCount)
	ON_UPDATE_COMMAND_UI(ID_BTG_VERTEXMODE, OnUpdateBtgVertexmode)
	ON_UPDATE_COMMAND_UI(ID_BTG_POLYMODE, OnUpdateBtgPolymode)
	ON_UPDATE_COMMAND_UI(ID_BTG_STARMODE, OnUpdateBtgStarmode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	btgDocMode = CBTGDoc::btgDocMode_EditBTGVertices;
	bPolyCount = FALSE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
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

	if(!m_wndDialogBar.Create(this, IDD_BTGVPROPERTIESCTRL, CBRS_RIGHT, IDD_BTGVPROPERTIESCTRL))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	if(!m_wndBackgroundBar.Create(this, IDD_BTGBGCOLOUR, CBRS_RIGHT, IDD_BTGBGCOLOUR))
	{
		TRACE0("Failed to create status bar\n");
		return -1;		// fail to create
	}

	if(!m_wndVisibleBar.Create(this, IDD_BTGVISIBLES, CBRS_RIGHT, IDD_BTGVISIBLES))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	if(!m_wndBitmapBar.Create(this, IDD_BTGBITMAPS, CBRS_RIGHT, IDD_BTGBITMAPS))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	m_wndDialogBar.SetBarStyle(m_wndDialogBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);

	m_wndBackgroundBar.SetBarStyle(m_wndBackgroundBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);

	m_wndVisibleBar.SetBarStyle(m_wndVisibleBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);

	m_wndBitmapBar.SetBarStyle(m_wndBitmapBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);

	m_wndToolBar.EnableDocking(CBRS_ALIGN_TOP);
	m_wndDialogBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndBackgroundBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndVisibleBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndBitmapBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	DockControlBar(&m_wndDialogBar);
	DockControlBar(&m_wndBackgroundBar);
	DockControlBar(&m_wndVisibleBar);
	DockControlBar(&m_wndBitmapBar);

	m_wndDialogBar.OnInit();
	m_wndBackgroundBar.OnInit();
	m_wndVisibleBar.OnInit();
	m_wndBitmapBar.OnInit();

	gpm_wndVisibleBar = &m_wndVisibleBar;
	gpm_wndDialogBar = &m_wndDialogBar;
	gpm_wndBackgroundBar = &m_wndBackgroundBar;

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		| WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;

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

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	CBTGDoc *pDoc = (CBTGDoc *)GetActiveDocument();
	static signed long bgRed = 0;
	static signed long bgGreen = 0;
	static signed long bgBlue = 0;

	m_wndDialogBar.OnUpdate();
	m_wndBackgroundBar.OnUpdate();
	m_wndVisibleBar.OnUpdate();
	m_wndBitmapBar.OnUpdate();

	pDoc->btgvUpdateMasterColor(m_wndDialogBar.red, m_wndDialogBar.green, m_wndDialogBar.blue, m_wndDialogBar.alpha, m_wndDialogBar.brightness);
	pDoc->btgUpdateVisible(m_wndVisibleBar.bStars, m_wndVisibleBar.renderMode);
	pDoc->btgUpdateBitmap(m_wndBitmapBar.curFileName);
	pDoc->btgvUpdateBackgroundColor(m_wndBackgroundBar.red, m_wndBackgroundBar.green, m_wndBackgroundBar.blue);
			
	return CFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::OnViewColorControl() 
{
	
}

void CMainFrame::OnUpdateBtgPolyCount(CCmdUI* pCmdUI)
{
	bPolyCount = !bPolyCount;

	if (bPolyCount)
	{
		char buf[64];
		CBTGDoc* pDoc = gBTGDoc;
		sprintf(buf, "%d polys / %d verts / %0.2f zoom",
				pDoc->btgpNumExisting(), pDoc->btgvNumExisting(), pDoc->zoomVal);
		m_wndStatusBar.SetPaneText(0, buf);
	}
	else
	{
		switch (btgDocMode)
		{
		case CBTGDoc::btgDocMode_EditBTGVertices:
			m_wndStatusBar.SetPaneText(0, "Vertex Edit Mode");
			break;

		case CBTGDoc::btgDocMode_EditBTGStars:
			m_wndStatusBar.SetPaneText(0, "Star Edit Mode");
			break;

		case CBTGDoc::btgDocMode_EditBTGPolygons:
			m_wndStatusBar.SetPaneText(0, "Polygon Edit Mode");
			break;

		default:
			break;
		}
	}
}

void CMainFrame::OnUpdateBtgVertexmode(CCmdUI* pCmdUI) 
{
	btgDocMode = CBTGDoc::btgDocMode_EditBTGVertices;

	m_wndStatusBar.SetPaneText(0, "Vertex Edit Mode");
}

void CMainFrame::OnUpdateBtgPolymode(CCmdUI* pCmdUI) 
{
	btgDocMode = CBTGDoc::btgDocMode_EditBTGPolygons;
	
	m_wndStatusBar.SetPaneText(0, "Polygon Edit Mode");
}

void CMainFrame::OnUpdateBtgStarmode(CCmdUI* pCmdUI) 
{
	btgDocMode = CBTGDoc::btgDocMode_EditBTGStars;

	m_wndStatusBar.SetPaneText(0, "Star Edit Mode");
}
