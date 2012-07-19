// FlowTreeView.cpp : implementation file
//

#include "stdafx.h"
#include "FEMan.h"
#include "FlowTreeView.h"
#include "ScreenPropertiesDlg.h"
#include "ScreenLinkPropertiesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFlowTreeView

IMPLEMENT_DYNCREATE(CFlowTreeView, CView)

CFlowTreeView::CFlowTreeView()
{
	lButtonStatus = LBS_NORMAL;
}

CFlowTreeView::~CFlowTreeView()
{
}


BEGIN_MESSAGE_MAP(CFlowTreeView, CView)
	//{{AFX_MSG_MAP(CFlowTreeView)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_FTD_NEW_SCREEN, OnFtdNewScreen)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_SI_DELETESCREEN, OnSiDeletescreen)
	ON_COMMAND(ID_SI_PROPERTIES, OnSiProperties)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_SI_NEW_SCREENLINK, OnSiNewScreenlink)
	ON_COMMAND(ID_SL_DELETESCREENLINK, OnSlDeletescreenlink)
	ON_COMMAND(ID_SL_PROPERTIES, OnSlProperties)
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFlowTreeView drawing

void CFlowTreeView::RegionUpdate(CRect *pRect)
{
	//InvalidateRect(*pRect, TRUE);
	Invalidate();
}

void CFlowTreeView::OnDraw(CDC* pDC)
{
	// Draw all the Screen Icons.
	DrawScreenLinks(pDC);
	DrawScreenIcons(pDC);

	if(lButtonStatus == LBS_WINDOWCLICK)
	{
		DrawSelectWindow(pDC);	
	}
}

// Alex added this stuff...
void CFlowTreeView::DrawScreenLinks(CDC *pDC)
{
	Screen *pWorkScreen;
	ScreenLink *pWorkLink;
	CPoint *pSrc, *pDst;
	CFEManDoc* pDoc = GetDocument();

	pWorkScreen = pDoc->GetFirstScreen();
	
	while(pWorkScreen)
	{
		pWorkLink = pWorkScreen->GetFirstScreenLink();
		
		CPen defPrevPen(PS_SOLID, 1, RGB(255, 0, 0));
		CPen defNextPen(PS_SOLID, 1, RGB(0, 255, 0));
		CPen defExtraPen(PS_SOLID, 1, RGB(0, 0, 0));

		CPen defPrevPen_sel(PS_SOLID, 3, RGB(255, 0, 0));
		CPen defNextPen_sel(PS_SOLID, 3, RGB(0, 255, 0));
		CPen defExtraPen_sel(PS_SOLID, 3, RGB(0, 0, 0));

		while(pWorkLink)
		{
			CRect *pRect;
			
			pRect = pWorkScreen->GetIconBounds();

			switch(pWorkLink->GetType())
			{
				case ScreenLink::SLT_DEFAULT_PREV:
					pSrc = pWorkLink->GetLinkSrc();
					pDst = pWorkLink->GetLinkDst();

					if(pWorkLink->GetStatus() == ScreenLink::SLS_SELECTED)
						pDC->SelectObject(defPrevPen_sel);
					else
						pDC->SelectObject(defPrevPen);

					pDC->MoveTo(*pSrc);
					pDC->LineTo(*pDst);

					pRect = pWorkLink->GetBounds();
					pDC->Rectangle(pRect);
				break;

				case ScreenLink::SLT_DEFAULT_NEXT:
					pSrc = pWorkLink->GetLinkSrc();
					pDst = pWorkLink->GetLinkDst();

					if(pWorkLink->GetStatus() == ScreenLink::SLS_SELECTED)
						pDC->SelectObject(defNextPen_sel);
					else
						pDC->SelectObject(defNextPen);
					pDC->MoveTo(*pSrc);
					pDC->LineTo(*pDst);

					pRect = pWorkLink->GetBounds();
					pDC->Rectangle(pRect);
				break;

				case ScreenLink::SLT_EXTRA:
					pSrc = pWorkLink->GetLinkSrc();
					pDst = pWorkLink->GetLinkDst();

					if(pWorkLink->GetStatus() == ScreenLink::SLS_SELECTED)
						pDC->SelectObject(defExtraPen_sel);
					else
						pDC->SelectObject(defExtraPen);

					pDC->MoveTo(*pSrc);
					pDC->LineTo(*pDst);

					pRect = pWorkLink->GetBounds();
					pDC->Rectangle(pRect);
				break;
			}

			pWorkLink = (ScreenLink *)pWorkLink->GetNext();
		}	
		pWorkScreen = (Screen *)pWorkScreen->GetNext();
	}
}

void CFlowTreeView::DrawScreenIcons(CDC *pDC)
{
	CPoint *pWorkPoint;
	CRect *pWorkRect;
	CSize *pWorkSize;
	char *pWorkName;
	Screen *pWorkScreen;
	
	CFEManDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	// Create the pens we need to draw the screen icons in both thier selected and
	// non-selected forms.
	CPen normalPen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen selectedPen(PS_SOLID, 3, RGB(255, 0, 0));
	
	pWorkScreen = pDoc->GetFirstScreen();

	while(pWorkScreen)
	{
		pWorkPoint = pWorkScreen->GetIconPos();
		pWorkRect = pWorkScreen->GetIconBounds();
		pWorkSize = pWorkScreen->GetNameSize();
		pWorkName = pWorkScreen->GetName();

		// Check to see if the screen icon is selected.  If it is, draw it with
		// a thicker border.
		if(pWorkScreen->GetScreenStatus() == Screen::SS_SELECTED)
		{
			pDC->SelectObject(selectedPen);
		}
		else
		{
			pDC->SelectObject(normalPen);
		}
		
		// Draw the bounds of the icon.
		pDC->Rectangle(pWorkRect->left, pWorkRect->top, pWorkRect->right, pWorkRect->bottom);

		// Draw the text in the icon.
		pDC->SetTextAlign(TA_CENTER | TA_TOP);
		pDC->TextOut(pWorkPoint->x + (pWorkSize->cx / 2), pWorkPoint->y, pWorkName, strlen(pWorkName));
		
		pWorkScreen = (Screen *)pWorkScreen->GetNext();
	}
}

void CFlowTreeView::DrawSelectWindow(CDC *pDC)
{
	CPen selectWindowPen(PS_DOT, 1, RGB(0xff, 0, 0));

	pDC->SelectStockObject(HOLLOW_BRUSH);

	pDC->SelectObject(selectWindowPen);

	pDC->Rectangle(&selectionWindow);
}

/////////////////////////////////////////////////////////////////////////////
// CFlowTreeView printing

BOOL CFlowTreeView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFlowTreeView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFlowTreeView::OnPrint( CDC *pDC, CPrintInfo *pInfo )
{
	// Print headers and/or footers, if desired.
	// Find portion of document corresponding to pInfo->m_nCurPage.
	OnDraw( pDC );
}

void CFlowTreeView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CFlowTreeView diagnostics

#ifdef _DEBUG
void CFlowTreeView::AssertValid() const
{
	CView::AssertValid();
}

void CFlowTreeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFEManDoc* CFlowTreeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFEManDoc)));
	return (CFEManDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFlowTreeView message handlers

void CFlowTreeView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen;
	ScreenLink *pWorkLink;
	
	// Save the position of this last right click.
	lastRightClick = point;

	// Keep this click in client coordinates?
	ScreenToClient(&lastRightClick);
	
	// Three types of context menus in this pane;
	// 1: Default context menu.
	// 2: Screen context menu.
	// 3: Screen Link context menu.

	// Load the Screen context menu.
	pWorkScreen = pDoc->PointOnScreenIcon(&lastRightClick);
	pWorkLink = pDoc->PointOnScreenLink(&lastRightClick);

	if(pWorkScreen)
	{	
		CMenu contextMenu, *pContextMenu;

		if(!pDoc->IsSelectedScreenIcon(pWorkScreen))
		{
			pDoc->ClearAllSelectedScreenIcons();
		}
		
		pDoc->SelectScreenIcon(pWorkScreen);

		Invalidate();

		contextMenu.LoadMenu(IDR_SCREENICON);

		pContextMenu = contextMenu.GetSubMenu(0);

		// Disable properties if there is more than one item selected.
		if(pDoc->GetNumSelectedScreenIcons() > 1)
		{
			pContextMenu->EnableMenuItem(ID_SI_PROPERTIES, MF_GRAYED);
			pContextMenu->EnableMenuItem(ID_SI_NEW_SCREENLINK, MF_GRAYED);
		}
		
		pContextMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, GetParent());
	}

	// Load the Screen Link context menu.
	else if(pWorkLink)
	{
		CMenu contextMenu, *pContextMenu;
		
		pWorkScreen = pWorkLink->GetMyScreen();
		
		if(!pDoc->IsSelectedScreenIcon(pWorkScreen))
		{
			pDoc->ClearAllSelectedScreenIcons();
		}

		pDoc->SelectScreenIcon(pWorkScreen);

		pWorkScreen->SelectScreenLink(pWorkLink);

		Invalidate();

		contextMenu.LoadMenu(IDR_SCREENLINK);

		pContextMenu = contextMenu.GetSubMenu(0);

		// Disable properties if there is more than one item selected.
		if(pDoc->GetNumSelectedScreenIcons() > 1)
		{
			pContextMenu->EnableMenuItem(ID_SL_PROPERTIES, MF_GRAYED);
			pContextMenu->EnableMenuItem(ID_SL_DELETESCREENLINK, MF_GRAYED);
		}

		if(pWorkLink->IsReadOnly())
		{
			pContextMenu->EnableMenuItem(ID_SL_DELETESCREENLINK, MF_GRAYED);
		}
		
		pContextMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, GetParent());
	}
	// Load the default context menu.
	else
	{
		CMenu contextMenu, *pContextMenu;

		pDoc->ClearAllSelectedScreenIcons();
		
		Invalidate();

		contextMenu.LoadMenu(IDR_FLOWTREEDEFAULT);

		pContextMenu = contextMenu.GetSubMenu(0);

		pContextMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, GetParent());
	}
}

void CFlowTreeView::OnFtdNewScreen() 
{
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen;
	CDC *pDC;

	pDC = GetDC();

	pWorkScreen = pDoc->CreateScreen(pDC, &lastRightClick);

	pDoc->SelectScreenIcon(pWorkScreen);
	
	RegionUpdate(pWorkScreen->GetIconBounds());

	pDoc->SetModifiedFlag();
}


void CFlowTreeView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen;
	ScreenLink *pWorkLink;

	// Left button is pressed.  We have several choices.
	// 1: Pressed on nothing.  Start dragging a selection window.
	// 2: Pressed on a Screen.  Select the screen and drag it.
	// 3: Pressed on a Screen Link.  Select the link and drag it.
	// 4: Pressed on an already selected Screen.  Drag all selected items.
	// 5: Pressed on an already selected Screen Link.  Drag all selected items.

	lastLeftClick = point;
	
	switch(lButtonStatus)
	{
	case LBS_NORMAL:		// Nothing special.
		// Find out where I clicked and choose one of the tracking modes.
		
		pWorkScreen = pDoc->PointOnScreenIcon(&point);
		pWorkLink = pDoc->PointOnScreenLink(&point);
		
		if(pWorkScreen)
		{
			// Clicked on a screen icon.

			// If nothing selected, OR this screen is not 
			// selected, clear all other selections and drag this screen.
			if((pDoc->GetNumSelectedScreenIcons() == 0) || 
				(!pDoc->IsSelectedScreenIcon(pWorkScreen)))
			{
				pDoc->ClearAllSelectedScreenIcons();

				pDoc->SelectScreenIcon(pWorkScreen);
			}

			// If this screen selected, drag all selected items.
			lButtonStatus = LBS_SCREENCLICK;
		}
		else if (pDoc->PointOnScreenLink(&point))
		{
			// Clicked on a screen link.

			// Clear all screen selections, select the screen we're on 
			// and the screenlink we're on and drag the link.
			pDoc->ClearAllSelectedScreenIcons();

			pWorkScreen = pWorkLink->GetMyScreen();
			
			pDoc->SelectScreenIcon(pWorkScreen);

			pWorkScreen->SelectScreenLink(pWorkLink);

			pWorkLink->Detach();
			pWorkLink->SetLinkDst(&point);
			
			// If this screen selected, drag all selected items.
			lButtonStatus = LBS_LINKCLICK;
		}
		else 
		{
			// Clicked on the flow tree window.

			// Clear all the selections and start dragging a selection window.
			pDoc->ClearAllSelectedScreenIcons();

			lButtonStatus = LBS_WINDOWCLICK;

			selectionWindow.left = lastLeftClick.x;
			selectionWindow.top = lastLeftClick.y;

			selectionWindow.right = lastLeftClick.x;
			selectionWindow.bottom = lastLeftClick.y;
		}

		// Make sure we redraw to reflect the changes.
		Invalidate();
		break;

	case LBS_WINDOWCLICK:	// Clicked on a window.
	case LBS_SCREENCLICK:	// Clicked on a screen.
	case LBS_LINKCLICK:		// Clicked on a screenlink.
		// In a tracking mode.  Probably do nothing.
		break;
	}

	ChooseMouseCursor();

	pDoc->SetModifiedFlag();
	
	CView::OnLButtonDown(nFlags, point);
}

void CFlowTreeView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen;
	ScreenLink *pWorkLink;
		
	// Check to see if we're in link drag mode
	if(lButtonStatus == LBS_LINKCLICK)
	{
		pWorkScreen = pDoc->PointOnScreenIcon(&point);
		
		// Check to see if we released on a screen.
		if(pWorkScreen)
		{
			// Attach the link.
			Screen *pTempScreen = pDoc->GetFirstSelectedScreen();
			pWorkLink = pTempScreen->GetSelectedLink();

			pWorkLink->AttachTo(pWorkScreen);

			CalculateScreenLinkName(pWorkLink);
		}
		else
		{
			// Reset the link to the default pos.
			pWorkScreen = pDoc->GetFirstSelectedScreen();

			pWorkLink = pWorkScreen->GetSelectedLink();

			pWorkLink->Detach();
			pWorkLink->SetLinkPos();
		}
	}
		
	// We're done tracking the mouse while the left button is down.
	lButtonStatus = LBS_NORMAL;
	
	pDoc->SetModifiedFlag();

	ChooseMouseCursor();

	Invalidate();
}

void CFlowTreeView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CFEManDoc* pDoc = GetDocument();
	
	switch(lButtonStatus)
	{
	case LBS_NORMAL:		// Nothing special.
		// Nothing to do with the left button.
		break;

	case LBS_WINDOWCLICK:	// Clicked on a window.
		// Track the selection window.

		// Update the current selection rectangle to stretch from the last left click
		// position to the current mouse position.

		selectionWindow.right = point.x;
		selectionWindow.bottom = point.y;

		// Let the document select all the screens within this rect.
		pDoc->SelectScreenIconsInRect(&selectionWindow);

		// Make sure we redraw this box.
		Invalidate();
		break;

	case LBS_LINKCLICK:		// Clicked on a screenlink.
		// Should be okay, because there is only one screen selected when we're dragging
		// links.
		//pDoc->RelocateSelectedScreenLinks(&lastLeftClick, &point);
		pDoc->RelocateSelectedScreenLink(&lastLeftClick, &point);

		// Make sure we only move the difference.
		lastLeftClick = point; // I'm cheating.  So sue me.  I'm fired.

		Invalidate();
		break;
	
	case LBS_SCREENCLICK:	// Clicked on a screen.
		// Move all selected objects.

		pDoc->RelocateSelectedScreens(&lastLeftClick, &point);

		// Make sure we only move the difference.
		lastLeftClick = point; // I'm cheating.  So sue me.  I'm fired.

		Invalidate();
		break;
	}
	
	ChooseMouseCursor();

	CView::OnMouseMove(nFlags, point);
}

void CFlowTreeView::OnSiDeletescreen() 
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->DeleteSelectedScreens();

	Invalidate();

	pDoc->UpdateScreenEditView();

	pDoc->SetModifiedFlag();
}

void CFlowTreeView::OnSiProperties() 
{
	DoScreenProperties();
}

void CFlowTreeView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	DoScreenProperties();
}

void CFlowTreeView::OnSiNewScreenlink() 
{
	Screen *pWorkScreen;
	CFEManDoc* pDoc = GetDocument();

	// Safe because there should only be one selected screen here.
	pWorkScreen = pDoc->GetFirstSelectedScreen();

	pWorkScreen->AddExtraScreenLink();

	Invalidate();

	pDoc->SetModifiedFlag();
}

void CFlowTreeView::OnSlDeletescreenlink() 
{
	Screen *pWorkScreen;
	CFEManDoc* pDoc = GetDocument();

	// Safe because there should only be one selected screen here, and the link
	// we've chosen to delete is not a default link.
	pWorkScreen = pDoc->GetFirstSelectedScreen();

	pWorkScreen->DeleteScreenLink(pWorkScreen->GetSelectedLink());

	Invalidate();

	pDoc->SetModifiedFlag();
}

void CFlowTreeView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	// Was working on tab controls for Flow Tree window.  Later. -- Alex
}

void CFlowTreeView::DoScreenLinkPropertiesDlg(void)
{
	Screen *pWorkScreen;
	ScreenLink *pWorkLink;
	CFEManDoc* pDoc = GetDocument();

	pWorkScreen = pDoc->GetFirstSelectedScreen();
	
	if(pWorkScreen)
	{
		pWorkLink = pWorkScreen->GetSelectedLink();

		pDoc->ClearAllSelectedScreenIcons();

		pDoc->SelectScreenIcon(pWorkScreen);

		Invalidate();
		
		// Load up the screen properties dialog.
		CScreenLinkPropertiesDlg screenLinkProperties;

		// Set the screen name.
		strcpy(screenLinkProperties.screenLinkName, pWorkLink->GetName());

		if(screenLinkProperties.DoModal() == IDOK)
		{
			pWorkLink->SetName(screenLinkProperties.screenLinkName);

			Invalidate();
			pDoc->UpdateScreenEditView();
		}

		pDoc->SetModifiedFlag();
	}
}

void CFlowTreeView::DoScreenProperties(void)
{
	Screen *pWorkScreen;
	CFEManDoc* pDoc = GetDocument();

	pWorkScreen = pDoc->GetFirstSelectedScreen();
	
	if(pWorkScreen)
	{
		pDoc->ClearAllSelectedScreenIcons();

		pDoc->SelectScreenIcon(pWorkScreen);

		Invalidate();
		
		// Load up the screen properties dialog.
		CScreenPropertiesDlg screenProperties;

		// Set the screen name.
		strcpy(screenProperties.screenName, pWorkScreen->GetName());

		screenProperties.screenWidth = pWorkScreen->GetPageBounds()->Width();
		screenProperties.screenHeight = pWorkScreen->GetPageBounds()->Height();

		// Set the user info.
		if(pWorkScreen->GetMyUserInfo())
		{
			screenProperties.userInfo = new char [strlen(pWorkScreen->GetMyUserInfo()) + 1];
			strcpy(screenProperties.userInfo, pWorkScreen->GetMyUserInfo());
		}
		
		if(screenProperties.DoModal() == IDOK)
		{
			CDC *pDC = GetDC();
			pWorkScreen->SetScreenName(pDC, screenProperties.screenName);
			pWorkScreen->SetMyUserInfo(screenProperties.userInfo);

			pWorkScreen->SetPageWidth(screenProperties.screenWidth);
			pWorkScreen->SetPageHeight(screenProperties.screenHeight);

			Invalidate();
			pDoc->UpdateScreenEditView();
		}

		pDoc->SetModifiedFlag();

		if(screenProperties.userInfo)
			delete [] screenProperties.userInfo;
	}
}

void CFlowTreeView::OnSlProperties() 
{
	CFEManDoc *pDoc = (CFEManDoc *)GetDocument();
	
	DoScreenLinkPropertiesDlg();

	pDoc->SetModifiedFlag();
}

BOOL CFlowTreeView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if(lButtonStatus == LBS_NORMAL)
		return CView::OnSetCursor(pWnd, nHitTest, message);

	return(TRUE);
}

void CFlowTreeView::ChooseMouseCursor(void)
{
	switch(lButtonStatus)
	{
	case LBS_LINKCLICK:
		::SetCursor(theApp.LoadStandardCursor(IDC_SIZEALL));
		break;

	case LBS_SCREENCLICK:
		::SetCursor(theApp.LoadStandardCursor(IDC_SIZEALL));
		break;

	default:
		::SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
		break;
	}
}

void CFlowTreeView::CalculateScreenLinkName(ScreenLink *sl)
{
	char tempName[1000];

	strcpy(tempName, sl->GetMyScreen()->GetName());

	if(sl->LinkedTo())
	{
		strcat(tempName, "-TO-");

		strcat(tempName, sl->LinkedTo()->GetName());
	}

	sl->SetName(tempName);
}

void CFlowTreeView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	gSplitWidth = cx;
}
