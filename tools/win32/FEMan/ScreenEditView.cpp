// ScreenEditView.cpp : implementation file
//

#include "stdafx.h"
#include "FEMan.h"
#include "ScreenEditView.h"
#include "FEManDoc.h"
#include "ScreenObjectRectPropertiesDlg.h"
#include "ScreenObjectBitmapPropertiesDlg.h"
#include "StringObjectStringPropertiesDlg.h"
#include "GridOptionsDlg.h"
#include "screen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScreenEditView

IMPLEMENT_DYNCREATE(CScreenEditView, CView)

CScreenEditView::CScreenEditView()
{
	lButtonStatus = LBS_NORMAL;
	mousePosStatus = LBS_NORMAL;
}

CScreenEditView::~CScreenEditView()
{
}


BEGIN_MESSAGE_MAP(CScreenEditView, CView)
	//{{AFX_MSG_MAP(CScreenEditView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SED_NEW_RECTANGLE, OnSedNewRectangle)
	ON_COMMAND(ID_SOR_DELETESELECTEDSCREENS, OnSorDeleteselectedscreens)
	ON_COMMAND(ID_SOR_PROPERTIES, OnSorProperties)
	ON_COMMAND(ID_SOR_BRINGTOFRONT, OnSorBringtofront)
	ON_COMMAND(ID_SOR_SENDTOBACK, OnSorSendtoback)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_SED_NEW_BITMAP, OnSedNewBitmap)
	ON_COMMAND(ID_SOB_PROPERTIES, OnSobProperties)
	ON_COMMAND(ID_SED_NEW_TEXTSTRING, OnSedNewTextstring)
	ON_COMMAND(ID_SOS_PROPERTIES, OnSosProperties)
	ON_COMMAND(ID_SOB_DELETE, OnSobDelete)
	ON_COMMAND(ID_SOS_DELETESELECTEDSCREENS, OnSosDeleteselectedscreens)
	ON_COMMAND(ID_SOS_SIZETOCONTENT, OnSosSizetocontent)
	ON_COMMAND(ID_SOB_SENDTOBACK, OnSobSendtoback)
	ON_COMMAND(ID_SOB_BRINGTOFRONT, OnSobBringtofront)
	ON_COMMAND(ID_SOS_BRINGTOFRONT, OnSosBringtofront)
	ON_COMMAND(ID_SOS_SENDTOBACK, OnSosSendtoback)
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_SOR_COPY, OnSorCopy)
	ON_COMMAND(ID_SOR_CUT, OnSorCut)
	ON_COMMAND(ID_SOS_COPY, OnSosCopy)
	ON_COMMAND(ID_SOS_CUT, OnSosCut)
	ON_COMMAND(ID_SOB_COPY, OnSobCopy)
	ON_COMMAND(ID_SOB_CUT, OnSobCut)
	ON_COMMAND(ID_SED_PASTE, OnSedPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_SED_GRID, OnSedGrid)
	ON_COMMAND(ID_SED_GRIDOPTIONS, OnSedGridoptions)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScreenEditView drawing

void CScreenEditView::OnDraw(CDC* pDC)
{	
	CFEManDoc* pDoc = GetDocument();
	pDC->SetViewportOrg(DEFAULT_SCREEN_OFFSET_X, DEFAULT_SCREEN_OFFSET_Y);

	if(pDoc->gridVisible)
	{
		DrawGrid(pDC);
	}

	DrawScreenObjects(pDC);

	// This is tracked in real coordinates.
	pDC->SetViewportOrg(0, 0);

	if(lButtonStatus == LBS_WINDOWCLICK)
	{
		DrawSelectWindow(pDC);	
	}
}

/////////////////////////////////////////////////////////////////////////////
// Alex added this stuff...

void CScreenEditView::DrawScreenObjects(CDC *pDC)
{
	Screen *pWorkScreen;
	ScreenObject *pScreenObject;
	CFEManDoc* pDoc = GetDocument();

	// Only draw the screen object if there is one screen selected.
	if(pDoc->GetNumSelectedScreenIcons() != 1)
		return;

	//CPen defaultBackgroundPen(PS_SOLID, 1, RGB(0, 0, 0));
	//CBrush defaultBackgroundBrush(RGB(156, 125, 97));
	
	pWorkScreen = pDoc->GetFirstSelectedScreen();

	pScreenObject = pWorkScreen->GetFirstScreenObject();

	while(pScreenObject)
	{
		pScreenObject->Draw(pDC);

		pScreenObject = (ScreenObject *)pScreenObject->GetNext();
	}
}

void CScreenEditView::DrawSelectWindow(CDC *pDC)
{
	CPen selectWindowPen(PS_DOT, 1, RGB(0xFF, 0x00, 0x00));

	pDC->SelectStockObject(HOLLOW_BRUSH);

	pDC->SelectObject(selectWindowPen);

	pDC->Rectangle(&selectionWindow);
}

void CScreenEditView::DrawGrid(CDC *pDC)
{
	CFEManDoc* pDoc = GetDocument();
	unsigned long i, screenWidth, screenHeight, spacing;

	CRect tempRect;
	GetClientRect(&tempRect);
	screenWidth = tempRect.Width();
	screenHeight = tempRect.Height();

	spacing = pDoc->gridSpacing;

	if(spacing == 0)
		spacing ++;

	for( i=0 ; i<screenWidth ; i+=spacing )
	{
		pDC->MoveTo(i, 0);
		pDC->LineTo(i, screenHeight);
	}

	for( i=0 ; i<screenHeight ; i+=spacing )
	{
		pDC->MoveTo(0, i);
		pDC->LineTo(screenWidth, i);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CScreenEditView diagnostics

#ifdef _DEBUG
void CScreenEditView::AssertValid() const
{
	CView::AssertValid();
}

void CScreenEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFEManDoc* CScreenEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFEManDoc)));
	return (CFEManDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CScreenEditView message handlers

void CScreenEditView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen;
	ScreenObject *pScreenObject;
	
	// Only draw the screen object if there is one screen selected.
	if(pDoc->GetNumSelectedScreenIcons() != 1)
		return;
	
	// Left button is pressed.  We have several choices.
	// 1: Pressed on nothing.  Start dragging a selection window.
	// 2: Pressed on a ScreenObject.  Select the screen object and drag it.
	// 4: Pressed on an already selected Screen Object.  Drag all selected items.
	
	if(pDoc->gridActive)
	{
		CPoint tempPoint;

		tempPoint.x = point.x - DEFAULT_SCREEN_OFFSET_X;
		tempPoint.y = point.y - DEFAULT_SCREEN_OFFSET_Y;

		unsigned long xRes = tempPoint.x / pDoc->gridSpacing;
		unsigned long yRes = tempPoint.y / pDoc->gridSpacing;

		lastLeftClick.x = pDoc->gridSpacing * xRes;
		lastLeftClick.y = pDoc->gridSpacing * yRes;

		lastLeftClick.x += DEFAULT_SCREEN_OFFSET_X;
		lastLeftClick.y += DEFAULT_SCREEN_OFFSET_Y;
	}
	else
	{
		lastLeftClick = point;
	}
	
	switch(lButtonStatus)
	{
	case LBS_NORMAL:		// Nothing special.
		// Find out where I clicked and choose one of the tracking modes.
		
		pWorkScreen = pDoc->GetFirstSelectedScreen();
		pScreenObject = pWorkScreen->PointOnScreenObject(&point);
		
		if(pScreenObject)
		{
			// Clicked on a screen object.

			// If nothing selected, OR this screen object is not
			// selected, clear all other selections and drag this screen.
			if((pWorkScreen->GetNumSelectedScreenObjects() == 0) ||
				(!pWorkScreen->IsSelectedScreenObject(pScreenObject)))
			{
				pWorkScreen->ClearAllSelectedScreenObjects();

				pWorkScreen->SelectScreenObject(pScreenObject);
			}

			// Only do this if there is just one screen object selected.
			if(pWorkScreen->GetNumSelectedScreenObjects() == 1)
			{
				lButtonStatus = pScreenObject->CheckSelectionDragGizmos(&point);
			}
			else
			{
				// If this screen selected, drag all selected items.
				lButtonStatus = LBS_SCREENOBJECTCLICK;	
			}
		}
		
		else
		{
			// Didn't click on a screen object.  Start Dragging the selection widow.

			// Clear all the selections and start dragging a selection window.
			pWorkScreen->ClearAllSelectedScreenObjects();

			lButtonStatus = LBS_WINDOWCLICK;

			selectionWindow.left = lastLeftClick.x;
			selectionWindow.top = lastLeftClick.y;

			selectionWindow.right = lastLeftClick.x;
			selectionWindow.bottom = lastLeftClick.y;
		}

		// Make sure we redraw to reflect the changes.
		Invalidate();
		break;

	case LBS_SCREENOBJECTCLICK:	// Clicked on a screen object.
			// In a tracking mode.  Probably do nothing.
		break;
	}

	ChooseMouseCursor(&point);
}

void CScreenEditView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// We're done tracking the mouse while the left button is down.
	lButtonStatus = LBS_NORMAL;

	ChooseMouseCursor(&point);

	Invalidate();
}

void CScreenEditView::OnMouseMove(UINT nFlags, CPoint point)
{
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen;

	// Only draw the screen object if there is one screen selected.
	if(pDoc->GetNumSelectedScreenIcons() != 1)
		return;

	CPoint gridPoint, tempPoint;
	unsigned long xRes, yRes;

	if(pDoc->gridActive)
	{
		tempPoint.x = point.x - DEFAULT_SCREEN_OFFSET_X;
		tempPoint.y = point.y - DEFAULT_SCREEN_OFFSET_Y;

		xRes = tempPoint.x / pDoc->gridSpacing;
		yRes = tempPoint.y / pDoc->gridSpacing;

		gridPoint.x = pDoc->gridSpacing * xRes;
		gridPoint.y = pDoc->gridSpacing * yRes;

		gridPoint.x += DEFAULT_SCREEN_OFFSET_X;
		gridPoint.y += DEFAULT_SCREEN_OFFSET_Y;
	}
	else
	{
		gridPoint = point;
	}

	pWorkScreen = pDoc->GetFirstSelectedScreen();
	
	switch(lButtonStatus)
	{
	case LBS_NORMAL:		// Nothing special.
		// Nothing to do with the left button.
		ChooseMouseCursor(&point);
		break;

	case LBS_WINDOWCLICK:	// Clicked on a window.
		// Track the selection window.

		// Update the current selection rectangle to stretch from the last left click
		// position to the current mouse position.

		selectionWindow.right = point.x;
		selectionWindow.bottom = point.y;

		// Let the document select all the screens within this rect.
		pWorkScreen->SelectScreenObjectsInRect(&selectionWindow);

		// Make sure we redraw this box.
		Invalidate();
		break;
	
	case LBS_SCREENOBJECTCLICK:	// Clicked on a screen object.
		// Move all selected objects.
		
		if(pDoc->gridActive)
		{
			pWorkScreen->SetExplicitScreensObjectPosition(&gridPoint);
		}
		else
		{
			pWorkScreen->RelocateSelectedScreensObjects(&lastLeftClick, &gridPoint);
		}

		// Make sure we only move the difference.
		lastLeftClick = gridPoint; // I'm cheating.  So sue me.  I'm fired.

		Invalidate();
		break;

	case LBS_IN_GIZMO_TOP_LEFT:
	case LBS_IN_GIZMO_TOP_MIDDLE:
	case LBS_IN_GIZMO_TOP_RIGHT:
	case LBS_IN_GIZMO_MIDDLE_LEFT:
	case LBS_IN_GIZMO_MIDDLE_RIGHT:
	case LBS_IN_GIZMO_BOTTOM_LEFT:
	case LBS_IN_GIZMO_BOTTOM_MIDDLE:
	case LBS_IN_GIZMO_BOTTOM_RIGHT:
		// Only get these if only one screen object selected.
		ScreenObject *so = (ScreenObject *)pWorkScreen->GetFirstSelectedScreenObject();

		so->Resize(&lastLeftClick, &gridPoint, lButtonStatus);

		// Make sure we only move the difference.
		lastLeftClick = gridPoint; // I'm cheating.  So sue me.  I'm fired.

		Invalidate();
		break;
	}
}

void CScreenEditView::DoContextMenu(CPoint point)
{
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen;
	ScreenObject *pScreenObject;
	
	// Save the position of this last right click.
	lastRightClick = point;

	// Keep this click in client coordinates?
	ScreenToClient(&lastRightClick);
	
	// These are the types of context menus in this pane;
	// 1: Default context menu.
	// 2: Screen Object context menu.
	
	// Load the Screen Object context menu.
	pWorkScreen = pDoc->GetFirstSelectedScreen();

	if(!pWorkScreen)
		return;

	pScreenObject = pWorkScreen->PointOnScreenObject(&lastRightClick);
	
	if(pScreenObject)
	{	
		CMenu contextMenu, *pContextMenu;

		if(!pWorkScreen->IsSelectedScreenObject(pScreenObject))
		{
			pWorkScreen->ClearAllSelectedScreenObjects();
		}
		
		pWorkScreen->SelectScreenObject(pScreenObject);

		Invalidate();

		if(pScreenObject->GetType() == ScreenObject::SOT_SCREENOBJECTRECT)
		{
			contextMenu.LoadMenu(IDR_SCREENOBJECTRECT);

			pContextMenu = contextMenu.GetSubMenu(0);

			// Disable properties if there is more than one item selected.
			if(pWorkScreen->GetNumSelectedScreenObjects() > 1)
			{
				pContextMenu->EnableMenuItem(ID_SOR_PROPERTIES, MF_GRAYED);
			}

			// Disable delete if one of the screens is the default page.
			// Delete screen won't delete this screen anyway.
			if(pWorkScreen->IsObjectDefaultPage(pScreenObject))
			{
				pContextMenu->EnableMenuItem(ID_SOR_DELETESELECTEDSCREENS, MF_GRAYED);
			}
			
			pContextMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, GetParent());
		}

		else if(pScreenObject->GetType() == ScreenObject::SOT_SCREENOBJECTBITMAP)
		{
			contextMenu.LoadMenu(IDR_SCREENOBJECTBITMAP);

			pContextMenu = contextMenu.GetSubMenu(0);

			// Disable properties if there is more than one item selected.
			if(pWorkScreen->GetNumSelectedScreenObjects() > 1)
			{
				pContextMenu->EnableMenuItem(ID_SOB_PROPERTIES, MF_GRAYED);
			}

			pContextMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, GetParent());
		}

		else if(pScreenObject->GetType() == ScreenObject::SOT_SCREENOBJECTSTRING)
		{
			contextMenu.LoadMenu(IDR_SCREENOBJECTSTRING);

			pContextMenu = contextMenu.GetSubMenu(0);

			// Disable properties if there is more than one item selected.
			if(pWorkScreen->GetNumSelectedScreenObjects() > 1)
			{
				pContextMenu->EnableMenuItem(ID_SOS_PROPERTIES, MF_GRAYED);
			}

			pContextMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, GetParent());
		}
	}

	// Load the default context menu.
	else
	{
		CMenu contextMenu, *pContextMenu;
		
		pWorkScreen->ClearAllSelectedScreenObjects();
		
		Invalidate();

		contextMenu.LoadMenu(IDR_SCREENEDITDEFAULT);

		pContextMenu = contextMenu.GetSubMenu(0);

		pContextMenu->CheckMenuItem(ID_SED_GRID, MF_BYCOMMAND | (pDoc->gridActive ? MF_CHECKED : MF_UNCHECKED));

		pContextMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, GetParent());
	}
}

void CScreenEditView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	DoContextMenu(point);
}

void CScreenEditView::OnSedNewRectangle()
{
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen = pDoc->GetFirstSelectedScreen();
	
	CPoint workPoint = lastRightClick;

	ScreenObjectRect *sor = new ScreenObjectRect(DEFAULT_SCREEN_OBJECT_NAME,
		lastRightClick.x, lastRightClick.y,
		lastRightClick.x + DEFAULT_PAGE_SIZE_X,
		lastRightClick.y + DEFAULT_PAGE_SIZE_Y);

	pWorkScreen->AddScreenObject(sor);	
	
	Invalidate();
}

void CScreenEditView::OnSorDeleteselectedscreens()
{
	CFEManDoc* pDoc = GetDocument();

	Screen *pWorkScreen = pDoc->GetFirstSelectedScreen();

	pDoc->CopySelectedScreensToDeleteBuffer(GetDC());
	
	pWorkScreen->DeleteSelectedScreenObjects();

	Invalidate();
}

void CScreenEditView::OnSorProperties()
{
	DoScreenObjectRectPropertiesDlg();	

	Invalidate();
}

void CScreenEditView::DoScreenObjectRectPropertiesDlg(void)
{
	CScreenObjectRectPropertiesDlg sorDialog;
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen = pDoc->GetFirstSelectedScreen();
	ScreenObjectRect *sor = (ScreenObjectRect *)pWorkScreen->GetFirstSelectedScreenObject();

	// Set up the dialog info.
	sorDialog.borderColor = sor->GetBorderColor();
	sorDialog.contentColor = sor->GetContentColor();

	sorDialog.borderVisible = (sor->GetBorderType() == ScreenObjectRect::BT_NORMAL);
	sorDialog.contentVisible = (sor->GetContentType() == ScreenObjectRect::CT_NORMAL);

	sorDialog.borderWidth = sor->GetBorderWidth();

	sorDialog.contentWidth = sor->GetContentWidth();
	sorDialog.contentHeight = sor->GetContentHeight();

	if(sor->GetMyUserInfo())
	{
		sorDialog.userInfo = new char [strlen(sor->GetMyUserInfo()) + 1];
		strcpy(sorDialog.userInfo, sor->GetMyUserInfo());
	}
	
	if(sorDialog.DoModal() == IDOK)
	{
		// Update all the info.
		sor->SetBorderColor(sorDialog.borderColor);
		sor->SetContentColor(sorDialog.contentColor);

		sor->SetBorderType((sorDialog.borderVisible ? ScreenObjectRect::BT_NORMAL : ScreenObjectRect::BT_NONE));
		sor->SetContentType((sorDialog.contentVisible ? ScreenObjectRect::CT_NORMAL : ScreenObjectRect::CT_NONE));

		sor->SetBorderWidth(sorDialog.borderWidth);

		sor->SetContentWidth(sorDialog.contentWidth);
		sor->SetContentHeight(sorDialog.contentHeight);

		sor->SetMyUserInfo(sorDialog.userInfo);
	}	

	delete [] sorDialog.userInfo;
}

void CScreenEditView::DoScreenObjectBitmapPropertiesDlg(void)
{
	CScreenObjectBitmapPropertiesDlg sobDialog;
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen = pDoc->GetFirstSelectedScreen();
	ScreenObjectBitmap *sob = (ScreenObjectBitmap *)pWorkScreen->GetFirstSelectedScreenObject();

	// Set up the dialog info.
	sobDialog.contentVisible = (sob->GetContentType() == ScreenObjectBitmap::CT_NORMAL);

	if(sob->GetMyUserInfo())
	{
		sobDialog.userInfo = new char [strlen(sob->GetMyUserInfo()) + 1];
		strcpy(sobDialog.userInfo, sob->GetMyUserInfo());
	}

	if(sobDialog.DoModal() == IDOK)
	{
		// Update all the info.
		sob->SetContentType((sobDialog.contentVisible ? ScreenObjectBitmap::CT_NORMAL : ScreenObjectBitmap::CT_NONE));

		sob->SetMyUserInfo(sobDialog.userInfo);
	}	

	delete [] sobDialog.userInfo;
}

void CScreenEditView::DoScreenObjectStringPropertiesDlg(void)
{
	CStringObjectStringPropertiesDlg sosDialog;
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen = pDoc->GetFirstSelectedScreen();
	ScreenObjectString *sos = (ScreenObjectString *)pWorkScreen->GetFirstSelectedScreenObject();
    unsigned long index;

	// Set up the dialog info.
	sosDialog.contentVisible = sos->GetContentType();
	
	sosDialog.myFont = *sos->GetMyFont();
	
    for (index = 0; index < NUMBER_LANGUAGES; index++)
    {
        if (sos->GetMyString(index) == NULL)
        {
            sosDialog.myString[index] = NULL;
        }
        else
        {
        	sosDialog.myString[index] = new char [strlen(sos->GetMyString(index)) + 1];
        	strcpy(sosDialog.myString[index], sos->GetMyString(index));
        }
    }

	sosDialog.textColor = sos->GetTextColor();
	sosDialog.textBackground = sos->GetTextBackground();

	sosDialog.autoSizeToContent = sos->GetAutoSizeToContent();

    sosDialog.justification = sos->GetJustification();

	if(sos->GetMyUserInfo())
	{
		sosDialog.userInfo = new char [strlen(sos->GetMyUserInfo()) + 1];
		strcpy(sosDialog.userInfo, sos->GetMyUserInfo());
	}

	if(sosDialog.DoModal() == IDOK)
	{
		// Update all the info.
		sos->SetContentType(sosDialog.contentVisible);

		sos->SetMyFont(&sosDialog.myFont);

        for (index = 0; index < NUMBER_LANGUAGES; index++)
        {
    		sos->SetMyString(sosDialog.myString[index], index);
        	delete [] sosDialog.myString[index];
        }

        sos->SetJustification(sosDialog.justification);

		sos->SetTextColor(sosDialog.textColor);
		sos->SetTextBackground(sosDialog.textBackground);

		sos->SetAutoSizeToContent(sosDialog.autoSizeToContent);

		sos->SetMyUserInfo(sosDialog.userInfo);
	}	


	delete [] sosDialog.userInfo;
}

void CScreenEditView::DoGridOptionsDlg(void)
{
	CGridOptionsDlg goDialog;
	CFEManDoc* pDoc = GetDocument();
	
	// Set up the dialog info.
	goDialog.gridActive = pDoc->gridActive;
	goDialog.gridVisible = pDoc->gridVisible;
	goDialog.gridSpacing = pDoc->gridSpacing;
	
	if(goDialog.DoModal() == IDOK)
	{
		// Update all the info.
		pDoc->gridActive = goDialog.gridActive;
		pDoc->gridVisible = goDialog.gridVisible;
		pDoc->gridSpacing = goDialog.gridSpacing;
	}	

	Invalidate();
}

void CScreenEditView::OnSorBringtofront()
{	
	CFEManDoc* pDoc = GetDocument();	

	pDoc->BringSelectedScreenObjectsToFront();

	Invalidate();
}

void CScreenEditView::OnSorSendtoback()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->SendSelectedScreenObjectsToBack();

	Invalidate();
}

void CScreenEditView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CPoint tempPoint;

	tempPoint = point;

	ClientToScreen(&tempPoint);
	
	DoContextMenu(tempPoint);	
}

void CScreenEditView::OnSedNewBitmap()
{
	static char BASED_CODE szFilter[] = "Windows Bitmap Files (*.bmp)|*.bmp|All Files (*.*)|*.*||";
	CDC *pDC = GetDC();
	
	// Bring up the dialog to select the file name of the bitmap.
	CFileDialog tempFileDialog(TRUE, ".BMP", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);

	if(tempFileDialog.DoModal() == IDOK)
	{
		CFEManDoc* pDoc = GetDocument();
		Screen *pWorkScreen = pDoc->GetFirstSelectedScreen();
		
		ScreenObjectBitmap *sob = new ScreenObjectBitmap(DEFAULT_SCREEN_OBJECT_NAME, lastRightClick.x, lastRightClick.y);

		if(sob->LoadBitmap(pDC, tempFileDialog.GetPathName().GetBuffer(10)))
		{
			// Successful.
			pWorkScreen->AddScreenObject(sob);
		}
		else
		{
			// Unsuccessful.
			delete(sob);
		}

		tempFileDialog.GetPathName().ReleaseBuffer(-1);
	}
	
	Invalidate();
}

void CScreenEditView::OnSobProperties()
{
	DoScreenObjectBitmapPropertiesDlg();	

	Invalidate();
}

void CScreenEditView::OnSedNewTextstring()
{
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen = pDoc->GetFirstSelectedScreen();
	
	CPoint workPoint = lastRightClick;

	ScreenObjectString *sos = new ScreenObjectString(DEFAULT_SCREEN_OBJECT_NAME,
		lastRightClick.x, lastRightClick.y, DEFAULT_SCREEN_OBJECT_NAME);
		
	pWorkScreen->AddScreenObject(sos);	
	
	Invalidate();
}

void CScreenEditView::OnSosProperties()
{
	DoScreenObjectStringPropertiesDlg();	

	Invalidate();
}

void CScreenEditView::OnSobDelete()
{
	CFEManDoc* pDoc = GetDocument();

	Screen *pWorkScreen = pDoc->GetFirstSelectedScreen();

	pDoc->CopySelectedScreensToDeleteBuffer(GetDC());

	pWorkScreen->DeleteSelectedScreenObjects();

	Invalidate();
}

void CScreenEditView::OnSosDeleteselectedscreens()
{
	CFEManDoc* pDoc = GetDocument();

	Screen *pWorkScreen = pDoc->GetFirstSelectedScreen();

	pDoc->CopySelectedScreensToDeleteBuffer(GetDC());
	
	pWorkScreen->DeleteSelectedScreenObjects();

	Invalidate();
}

void CScreenEditView::OnSosSizetocontent()
{
	CFEManDoc* pDoc = GetDocument();

	Screen *pWorkScreen = pDoc->GetFirstSelectedScreen();

	ScreenObject *pWorkObject = pWorkScreen->GetFirstSelectedScreenObject();

	while(pWorkObject)
	{
		if(pWorkObject->GetType() == ScreenObject::SOT_SCREENOBJECTSTRING)
		{
			CDC *pDC = GetDC();
			ScreenObjectString *sos = (ScreenObjectString *)pWorkObject;

			sos->SetStringBounds(pDC);
		}

		pWorkObject = (ScreenObject *)pWorkObject->GetNext();
	}

	Invalidate();
}

void CScreenEditView::OnSobSendtoback()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->SendSelectedScreenObjectsToBack();

	Invalidate();
}

void CScreenEditView::OnSobBringtofront()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->BringSelectedScreenObjectsToFront();

	Invalidate();
}

void CScreenEditView::OnSosBringtofront()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->BringSelectedScreenObjectsToFront();

	Invalidate();
}

void CScreenEditView::OnSosSendtoback()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->SendSelectedScreenObjectsToBack();

	Invalidate();
}

BOOL CScreenEditView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen;
	ScreenObject *pWorkObject;

	if(pDoc->GetNumSelectedScreenIcons() != 1)
		return(CView::OnSetCursor(pWnd, nHitTest, message));

	pWorkScreen = pDoc->GetFirstSelectedScreen();

	pWorkObject = pWorkScreen->GetFirstSelectedScreenObject();

	if(pWorkObject)
	{
		//ChooseMouseCursor();

		return(TRUE);
	}
	
	return(CView::OnSetCursor(pWnd, nHitTest, message));
}

void CScreenEditView::ChooseMouseCursor(CPoint *point)
{
	CFEManDoc* pDoc = GetDocument();
	Screen *pWorkScreen;
	ScreenObject *pScreenObject;

	if(lButtonStatus == LBS_NORMAL)
	{
		pWorkScreen = pDoc->GetFirstSelectedScreen();

		if(!pWorkScreen)
			return;

		pScreenObject = pWorkScreen->PointOnScreenObject(point);
		
		if(pScreenObject)
		{
			// Moved over on a screen object.

			// Only do this if there is just one screen object selected.
			if(pScreenObject->GetStatus() == ScreenObject::SOS_SELECTED)
			{
				mousePosStatus = pScreenObject->CheckSelectionDragGizmos(point);
			}
			else
			{
				// If this screen selected, drag all selected items.
				mousePosStatus = LBS_NORMAL;	
			}
		}
		else
		{
			mousePosStatus = LBS_NORMAL;
		}
	}
	else
	{
		mousePosStatus = lButtonStatus;
	}

	switch(mousePosStatus)
	{
	case LBS_SCREENOBJECTCLICK:
		::SetCursor(theApp.LoadStandardCursor(IDC_SIZEALL));
		break;

	case LBS_IN_GIZMO_TOP_LEFT:
	case LBS_IN_GIZMO_BOTTOM_RIGHT:
		::SetCursor(theApp.LoadStandardCursor(IDC_SIZENWSE));
		break;

	case LBS_IN_GIZMO_TOP_RIGHT:
	case LBS_IN_GIZMO_BOTTOM_LEFT:
		::SetCursor(theApp.LoadStandardCursor(IDC_SIZENESW));
		break;

	case LBS_IN_GIZMO_MIDDLE_LEFT:
	case LBS_IN_GIZMO_MIDDLE_RIGHT:
		::SetCursor(theApp.LoadStandardCursor(IDC_SIZEWE));
		break;

	case LBS_IN_GIZMO_TOP_MIDDLE:
	case LBS_IN_GIZMO_BOTTOM_MIDDLE:
		::SetCursor(theApp.LoadStandardCursor(IDC_SIZENS));
	break;

	default:
		::SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
		break;
	}
}

void CScreenEditView::OnSorCopy()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->DoCopyScreenObject(GetDC());

	Invalidate();
}

void CScreenEditView::OnSorCut()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->DoCutScreenObject(GetDC());

	Invalidate();
}

void CScreenEditView::OnSosCopy()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->DoCopyScreenObject(GetDC());

	Invalidate();
}

void CScreenEditView::OnSosCut()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->DoCutScreenObject(GetDC());

	Invalidate();
}

void CScreenEditView::OnSobCopy()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->DoCopyScreenObject(GetDC());

	Invalidate();
}

void CScreenEditView::OnSobCut()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->DoCutScreenObject(GetDC());

	Invalidate();
}

void CScreenEditView::OnSedPaste()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->DoPasteScreenObject(GetDC());

	Invalidate();
}

void CScreenEditView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{	
	CView::OnPrint(pDC, pInfo);
}

void CScreenEditView::OnEditUndo()
{
	// Should check for active view so this works for screen edit too.	
	CFEManDoc* pDoc = GetDocument();

	pDoc->DoUndoScreenObject(GetDC());

	Invalidate();
}

void CScreenEditView::OnEditCut()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->DoCutScreenObject(GetDC());

	Invalidate();
}

void CScreenEditView::OnEditPaste()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->DoPasteScreenObject(GetDC());

	Invalidate();
}

void CScreenEditView::OnEditCopy()
{
	CFEManDoc* pDoc = GetDocument();

	pDoc->DoCopyScreenObject(GetDC());

	Invalidate();	
}

void CScreenEditView::OnSedGrid()
{
	CFEManDoc* pDoc = GetDocument();
	pDoc->gridActive = 1 - pDoc->gridActive;
}

void CScreenEditView::OnSedGridoptions()
{
	DoGridOptionsDlg();
}

void CScreenEditView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CFEManDoc* pDoc = GetDocument();
	CPoint workPoint = lastLeftClick;
	unsigned long moveDist = 0;
	Screen *pWorkScreen;
	ScreenObject *pWorkObject;

	// Only draw the screen object if there is one screen selected.
	if(pDoc->GetNumSelectedScreenIcons() != 1)
		return;

	pWorkScreen = pDoc->GetFirstSelectedScreen();
	
	if(pDoc->gridActive)
	{
		moveDist = pDoc->gridSpacing;
	}
	else
	{
		moveDist = 1;
	}

	switch(nChar)
	{
	case VK_LEFT:
		workPoint.x -= moveDist;
		pWorkScreen->RelocateSelectedScreensObjects(&lastLeftClick, &workPoint);
		lastLeftClick = workPoint;
		break;

	case VK_RIGHT:
		workPoint.x += moveDist;
		pWorkScreen->RelocateSelectedScreensObjects(&lastLeftClick, &workPoint);
		lastLeftClick = workPoint;
		break;

	case VK_UP:
		workPoint.y -= moveDist;
		pWorkScreen->RelocateSelectedScreensObjects(&lastLeftClick, &workPoint);
		lastLeftClick = workPoint;
		break;

	case VK_DOWN:
		workPoint.y += moveDist;
		pWorkScreen->RelocateSelectedScreensObjects(&lastLeftClick, &workPoint);
		lastLeftClick = workPoint;
		break;

	case VK_TAB:
		pWorkObject = pWorkScreen->GetFirstSelectedScreenObject();

		if(pWorkObject)
		{
			pWorkObject = pWorkScreen->GetFirstSelectedScreenObject();

			pWorkObject = (ScreenObject *)pWorkObject->GetNext();

			pWorkScreen->ClearAllSelectedScreenObjects();

			if(pWorkObject)
			{
				pWorkScreen->SelectScreenObject(pWorkObject);
			}
			else
			{
				pWorkScreen->SelectScreenObject(pWorkScreen->GetFirstScreenObject());
			}
		}
		else
		{
			pWorkScreen->SelectScreenObject(pWorkScreen->GetFirstScreenObject());
		}
		break;
	}

	Invalidate();

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
