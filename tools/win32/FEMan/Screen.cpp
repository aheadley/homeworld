/*
** SCREEN.CPP : Code to control screens.
*/

#include "MainFrm.h"
#include "FEMan.h"
#include "stdafx.h"
#include "FEMan.h"
#include "ScreenEditView.h"
#include "assert.h"

#include "screen.h"
#include "ccList.h"

#include "bmp.h"

// Variable used to get screenID's.

signed long globalScreenIDMaster = 0;

extern unsigned long languageToView;

////////////////////////////////////////////////////////////////////
// ScreenLinkContainer stuff...

ScreenLinkContainer::~ScreenLinkContainer(void)
{
}

////////////////////////////////////////////////////////////////////
// ScreenLink stuff...

ScreenLink::ScreenLink(signed long t, Screen *parent)
{
	status = SLS_NORMAL;

	type = t;

	linkedTo = 0;

	myScreen = parent;

	SetLinkPos();

	myContainer = 0;

	strcpy(name, DEFAULT_SCREENLINKNAME);
}

ScreenLink::ScreenLink(Screen *parent)
{
	status = SLS_NORMAL;

	linkedTo = 0;

	myScreen = parent;

	SetLinkPos();

	myContainer = 0;

	strcpy(name, "");
}

ScreenLink::~ScreenLink(void)
{
	if(linkedTo)
		Detach();
}

void ScreenLink::SetLinkDst(CPoint *c)
{
	SetLinkDst(c->x, c->y);
}

void ScreenLink::SetLinkDst(signed long x, signed long y)
{
	CPoint workPoint;

	dst.x = x;
	dst.y = y;

	if(linkedTo)
	{
		workPoint.x = ((dst.x - src.x) / 3) + src.x;
		workPoint.y = ((dst.y - src.y) / 3) + src.y;
	}
	else
	{
		workPoint = dst;
	}

	bounds.left = workPoint.x - SL_DEFAULT_DB_SIZE / 2;
	bounds.top = workPoint.y - SL_DEFAULT_DB_SIZE / 2;
	bounds.right = workPoint.x + SL_DEFAULT_DB_SIZE / 2;
	bounds.bottom = workPoint.y + SL_DEFAULT_DB_SIZE / 2;
}

void ScreenLink::SetLinkSrc(signed long x, signed long y)	
{
	src.x = x;
	src.y = y;

	SetLinkDst(&dst);
}

void ScreenLink::SetLinkPos(void)
{
	CRect *pRect;

	pRect = myScreen->GetIconBounds();

	src = pRect->CenterPoint();

	switch(type)
	{
	case ScreenLink::SLT_DEFAULT_PREV:
		src.x -= pRect->Width() / 2;
		break;

	case ScreenLink::SLT_DEFAULT_NEXT:
		src.x += pRect->Width() / 2;
		break;

	case ScreenLink::SLT_EXTRA:
		src.y += pRect->Height() / 2;
		break;
	}

	// If there is no screen dest, choose default dest.
	if(!linkedTo)
	{
		switch(type)
		{
		case ScreenLink::SLT_DEFAULT_PREV:
			SetLinkDst(src.x - SL_DEFAULT_LENGTH, src.y);
			break;

		case ScreenLink::SLT_DEFAULT_NEXT:
			SetLinkDst(src.x + SL_DEFAULT_LENGTH, src.y);
			break;

		case ScreenLink::SLT_EXTRA:
			SetLinkDst(src.x, src.y + SL_DEFAULT_LENGTH);
			break;
		}
	}
	else
	{
		// Force an update of the selection box.
		switch(type)
		{
		case ScreenLink::SLT_DEFAULT_PREV:
			SetLinkDst(&dst);
			break;

		case ScreenLink::SLT_DEFAULT_NEXT:
			SetLinkDst(&dst);
			break;

		case ScreenLink::SLT_EXTRA:
			SetLinkDst(&dst);
			break;
		}
	}
}

void ScreenLink::AttachTo(Screen *pScreen)
{
	ScreenLinkContainer *slc;
	ccList *pWorkList;
	
	// Detach if I'm already attached.
	if(linkedTo)
		Detach();

	// Don't attach to myself.
	if(pScreen == myScreen)
	{
		SetLinkPos();
		return;
	}
	
	// Point to my screen.
	linkedTo = pScreen;
	
	// Create a container.
	slc = new ScreenLinkContainer;

	// Set up the screen container.
	slc->SetMyLink(this);

	// Get the incoming list from the screen.
	pWorkList = pScreen->GetInboundLinkList();

	// Add the container to the inbound list of the screen.
	pWorkList->AddNode(slc);

	myContainer = slc;

	SetLinkPos();
}

void ScreenLink::Detach(void)
{
	ccList *pWorkList;
	
	// Don't detach if not attached.
	if(!linkedTo)
		return;
	
	// If I'm attached, I should have a container.
	ASSERT(myContainer);

	pWorkList = linkedTo->GetInboundLinkList();

	pWorkList->PurgeNode(myContainer);

	myContainer = 0;

	linkedTo = 0;

	SetLinkPos();
}

////////////////////////////////////////////////////////////////////
// Screen stuff...

Screen::Screen(CDC *pDC, signed long x, signed long y, char *name)
{
	////////////////////////////////////////////////////////////////////
	// Screen icon stuff...
	
	ScreenLink *pWorkScreenLink;
	
	iconPos.x = iconPos.y = 0;
	iconBounds.left = iconBounds.top = iconBounds.right = iconBounds.bottom = 0;

	nameSize.cx = nameSize.cy = 0;
	screenName = 0;
	userInfo = 0;
	
	SetScreenName(pDC, name);
	
	SetScreenPos(x, y);

	screenStatus = Screen::SS_NORMAL;

	// Add the default prev screenlink.
	pWorkScreenLink = new ScreenLink(ScreenLink::SLT_DEFAULT_PREV, this);
	screenLinkList.AddNode(pWorkScreenLink);
	defPrev = pWorkScreenLink;

	// Add the default next screenlink.
	pWorkScreenLink = new ScreenLink(ScreenLink::SLT_DEFAULT_NEXT, this);
	screenLinkList.AddNode(pWorkScreenLink);
	defNext = pWorkScreenLink;

	SetScreenID();

	////////////////////////////////////////////////////////////////////
	// Actual Screen stuff...
	
	// Set up the background object of the screen.
	ScreenObjectRect *pBackgroundObject = new ScreenObjectRect(DEFAULT_PAGE_NAME);

	// Set up the colors for the page.
	pBackgroundObject->SetBorderColor(RGB(0, 0, 0));
	pBackgroundObject->SetContentColor(RGB(183, 141, 121));

	screenObjectList.AddNode(pBackgroundObject);
}

Screen::Screen(void)
{
	// This is the loaded construct.
	defNext = defPrev = 0;

	iconBounds.left = iconBounds.top = iconBounds.right = iconBounds.bottom;

	iconPos.x = iconPos.y = 0;

	nameSize.cx = nameSize.cy = 0;

	screenID = 0;

	screenName = 0;
	userInfo = 0;

	screenStatus = SS_NORMAL;

	selectedLink = 0;
}

Screen::~Screen(void)
{
	ScreenLinkContainer *pWorkContainer, *pKillContainer;
	ScreenLink *pWorkLink;

	pWorkContainer = (ScreenLinkContainer *)inboundLinkList.GetHead();

	while(pWorkContainer)
	{
		pWorkLink = pWorkContainer->GetMyLink();

		pKillContainer = pWorkContainer;

		pWorkContainer = (ScreenLinkContainer *)pWorkContainer->GetNext();
		
		pWorkLink->Detach();
	}

	if(screenName)
	{
		delete [] screenName;
	}

	if(userInfo)
	{
		delete [] userInfo;
	}
}

void Screen::SetScreenPos(signed long x, signed long y)
{
	iconPos.x = x;
	iconPos.y = y;

	iconBounds.left = iconPos.x - (DEFAULT_SCREENNAME_BORDER_X / 2);
	iconBounds.top = iconPos.y - (DEFAULT_SCREENNAME_BORDER_Y / 2);

	iconBounds.right = iconPos.x + nameSize.cx + (DEFAULT_SCREENNAME_BORDER_X / 2);
	iconBounds.bottom = iconPos.y + nameSize.cy + (DEFAULT_SCREENNAME_BORDER_Y / 2);
}

void Screen::SetScreenName(CDC *pDC, char *name)
{
	// Need to delete the old one?
	if(screenName)
	{
		delete [] screenName;
	}
	
	if(name)
	{
		// User supplied name.
		screenName = new char[strlen(name) + 1];
		strcpy(screenName, name);
	}
	else
	{
		// Use the default.
		screenName = new char[strlen(DEFAULT_SCREENNAME) + 1];
		strcpy(screenName, DEFAULT_SCREENNAME);
	}

	//CSize GetTextExtent( LPCTSTR lpszString, int nCount ) const;
	nameSize = pDC->GetTextExtent(screenName, strlen(screenName));

	SetScreenPos(iconPos.x, iconPos.y);

	// loop through all the screenlinks and update them.
	ScreenLink *pWorkLink = (ScreenLink *)screenLinkList.GetHead();

	while(pWorkLink)
	{
		pWorkLink->SetLinkPos();

		pWorkLink = (ScreenLink *)pWorkLink->GetNext();
	}
}

ScreenLink *Screen::AddScreenLink(void)
{
	ScreenLink *pLink;

	pLink = new ScreenLink(this);

	screenLinkList.AddNode(pLink);

	return(pLink);
}

void Screen::AddExtraScreenLink(void)
{
	ScreenLink *pLink;

	pLink = new ScreenLink(ScreenLink::SLT_EXTRA, this);

	screenLinkList.AddNode(pLink);
}

void Screen::DeleteScreenLink(ScreenLink *sl)
{
	// Is this one of my screenlinks?
	ASSERT(sl->GetMyScreen() == this);

	// Is this one of the default links?
	ASSERT(!(GetSelectedLink() == GetDefPrevLink()));
	ASSERT(!(GetSelectedLink() == GetDefNextLink()));

	screenLinkList.PurgeNode(sl);
}

void Screen::SelectScreenLink(ScreenLink *s)
{
	s->SetStatus(ScreenLink::SLS_SELECTED);

	selectedLink = s;
}

void Screen::DeSelectScreenLink(ScreenLink *s)
{
	s->SetStatus(ScreenLink::SLS_NORMAL);

	selectedLink = 0;
}

CRect *Screen::GetPageBounds(void)
{
	ScreenObject *pSO;

	pSO = (ScreenObject *)screenObjectList.GetHead();

	while(pSO)
	{
		if(!(strcmp(pSO->GetName(), DEFAULT_PAGE_NAME)))
		{
			return(((ScreenObjectRect *)pSO)->GetBounds());
		}

		pSO = (ScreenObject *)pSO->GetNext();
	}

	return(0);
}

void Screen::SetPageBounds(CRect *r)
{
}

void Screen::SetPageWidth(unsigned long w)
{
	CRect *pWorkRect = GetPageBounds();

	pWorkRect->right = pWorkRect->left + w;
}


void Screen::SetPageHeight(unsigned long h)
{
	CRect *pWorkRect = GetPageBounds();

	pWorkRect->bottom = pWorkRect->top + h;
}

// The way this works, because it's a little cryptic, is that it checks to see if the
// specified point is withing the rectangle supplied.  If it is, we store that pointer.
// If not, keep going.  The key is, DON'T EXIT when you find an object.  The theory being,
// because the ScreenObjects are stored in the list from back to front, you end up
// selecting the topmost one that the mouse is in, in the end. -- Alex.

ScreenObject *Screen::PointOnScreenObject(CPoint *cPoint)
{
	ScreenObject *pScreenObject = GetFirstScreenObject(), *pRetObject = 0;
	CPoint workPoint;
	CRect workRect;
	
	workPoint = *cPoint;
	workPoint.Offset(-DEFAULT_SCREEN_OFFSET_X, -DEFAULT_SCREEN_OFFSET_Y);

	while(pScreenObject)
	{
		workRect = *pScreenObject->GetBounds();

		workRect.InflateRect(SCREEN_OBJECT_BORDER_CHECK_W / 2, SCREEN_OBJECT_BORDER_CHECK_H / 2);

		if(workRect.PtInRect(workPoint))
		{
			pRetObject = pScreenObject;
		}

		pScreenObject = (ScreenObject *)pScreenObject->GetNext();
	}

	return(pRetObject);
}

unsigned long Screen::GetNumSelectedScreenObjects(void)
{
	ScreenObject *pScreenObject;
	unsigned long numSelected = 0;

	pScreenObject = GetFirstSelectedScreenObject();

	while(pScreenObject)
	{
		if(pScreenObject->GetStatus() == ScreenObject::SOS_SELECTED)
			numSelected++;
		
		pScreenObject = (ScreenObject *)pScreenObject->GetNext();
	}

	return(numSelected);
}

ScreenObject *Screen::GetFirstSelectedScreenObject(void)
{
	ScreenObject *pScreenObject;

	pScreenObject = GetFirstScreenObject();

	while(pScreenObject)
	{
		if(pScreenObject->GetStatus() == ScreenObject::SOS_SELECTED)
			return(pScreenObject);
		
		pScreenObject = (ScreenObject *)pScreenObject->GetNext();
	}

	return(0);
}

void Screen::SelectScreenObject(ScreenObject *so)
{
	so->SetStatus(ScreenObject::SOS_SELECTED);
}

void Screen::DeSelectScreenObject(ScreenObject *so)
{
	so->SetStatus(ScreenObject::SOS_NORMAL);
}

void Screen::ClearAllSelectedScreenObjects(void)
{
	ScreenObject *pScreenObject;

	pScreenObject = GetFirstSelectedScreenObject();

	while(pScreenObject)
	{
		DeSelectScreenObject(pScreenObject);

		pScreenObject = (ScreenObject *)pScreenObject->GetNext();
	}
}

void Screen::SelectScreenObjectsInRect(CRect *pRect)
{
	ScreenObject *pScreenObject;
	CRect *pWorkRect;
	
	CRect workRect = *pRect, dstRect;

	workRect.NormalizeRect();

	workRect.OffsetRect(-DEFAULT_SCREEN_OFFSET_X, -DEFAULT_SCREEN_OFFSET_Y);

	pScreenObject = GetFirstScreenObject();

	while(pScreenObject)
	{
		pWorkRect = pScreenObject->GetBounds();

		if((pWorkRect->left > workRect.left) &&
			(pWorkRect->right < workRect.right) &&
			(pWorkRect->top > workRect.top) &&
			(pWorkRect->bottom < workRect.bottom))
		{
			SelectScreenObject(pScreenObject);
			pScreenObject = (ScreenObject *)pScreenObject->GetNext();
			continue;
		}
		else
		{
			DeSelectScreenObject(pScreenObject);
		}
		
		pScreenObject = (ScreenObject *)pScreenObject->GetNext();
	}
}

void Screen::RelocateSelectedScreensObjects(CPoint *pPressedAt, CPoint *pPoint)
{
	ScreenObject *pScreenObject;
	CPoint workPoint, *pWorkPoint;

	workPoint.x = pPoint->x - pPressedAt->x;
	workPoint.y = pPoint->y - pPressedAt->y;

	pScreenObject = GetFirstSelectedScreenObject();

	while(pScreenObject)
	{
		if(pScreenObject->GetStatus() == ScreenObject::SOS_SELECTED)
		{
			pWorkPoint = pScreenObject->GetPos();

			pScreenObject->SetPos(pWorkPoint->x + workPoint.x, pWorkPoint->y + workPoint.y);
		}

		pScreenObject = (ScreenObject *)pScreenObject->GetNext();
	}
}

void Screen::SetExplicitScreensObjectPosition(CPoint *pPoint)
{
	ScreenObject *pScreenObject;
	
	pScreenObject = GetFirstSelectedScreenObject();

	while(pScreenObject)
	{
		if(pScreenObject->GetStatus() == ScreenObject::SOS_SELECTED)
		{
			pScreenObject->SetPos(pPoint->x - DEFAULT_SCREEN_OFFSET_X, pPoint->y - DEFAULT_SCREEN_OFFSET_Y);
		}

		pScreenObject = (ScreenObject *)pScreenObject->GetNext();
	}
}

void Screen::DeleteSelectedScreenObjects(void)
{
	ScreenObject *pScreenObject = GetFirstScreenObject();
	ScreenObject *pKillObject;

	while(pScreenObject)
	{
		pKillObject = pScreenObject;
		pScreenObject = (ScreenObject *)pScreenObject->GetNext();

		if(pKillObject->GetStatus() == ScreenObject::SOS_SELECTED)
		{
			// Don't delete the default page.
			if(!IsObjectDefaultPage(pKillObject))
			{
				screenObjectList.PurgeNode(pKillObject);
			}
		}
	}
}

void Screen::SetScreenID(void)
{
	screenID = globalScreenIDMaster;
	globalScreenIDMaster ++;
}

void Screen::SetMyUserInfo(char *name)
{
	if(userInfo)
		delete [] userInfo;

	userInfo = 0;

	if(!name)
		return;
	
	userInfo = new char [strlen(name) + 1];

	strcpy(userInfo, name);
}

//////////////////////////////////////////////////////////////////////////////////////
// ScreenObject stuff here...

ScreenObject::ScreenObject(char *n)
{
	status = SOS_NORMAL;

	strcpy(name, n);

	pos.x = pos.y = 0;
	bounds.SetRect(0, 0, 100, 100);

	userInfo = 0;
}

ScreenObject::ScreenObject(void)
{
	bounds.left = bounds.top = bounds.right = bounds.bottom = 0;
	strcpy(name, "");
	pos.x = pos.y = 0;
	status = SOS_NORMAL;
	type = 0;
	userInfo = 0;
}

ScreenObject::~ScreenObject(void)
{
	if(userInfo)
		delete [] userInfo;
}

void ScreenObject::DrawSelectionBorder(CDC *pDC)
{
	CPen selectionBorder(PS_DASHDOTDOT, 1, RGB(0xFF, 0x00, 0x00));
	CBrush selectionBrush;
	CRect workRect;

	pDC->SetTextColor(RGB(0xFF, 0xFF, 0xFF));
	pDC->SetBkColor(RGB(0xFF, 0xFF, 0xFF));
	
	selectionBrush.CreateStockObject(HOLLOW_BRUSH);

	pDC->SelectObject(selectionBorder);
	pDC->SelectObject(selectionBrush);

	workRect = bounds;

	pDC->Rectangle(workRect);

	workRect.left ++;
	workRect.top ++;
	workRect.right --;
	workRect.bottom --;
	pDC->Rectangle(workRect);

	workRect.left ++;
	workRect.top ++;
	workRect.right --;
	workRect.bottom --;
	pDC->Rectangle(workRect);
}

void ScreenObject::SetPos(signed long x, signed long y)
{
	signed long w, h;

	w = bounds.Width();
	h = bounds.Height();

	pos.x = x;
	pos.y = y;

	bounds.SetRect(x, y, x + w, y + h);
}

signed long ScreenObject::CheckSelectionDragGizmos(CPoint *pPoint)
{
	return(CScreenEditView::LBS_SCREENOBJECTCLICK);
}

void ScreenObject::Resize(CPoint *pPressedAt, CPoint *pPoint, signed long lButtonStatus)
{
#if 1
	CPoint workPoint;

	workPoint.x = pPoint->x - pPressedAt->x;
	workPoint.y = pPoint->y - pPressedAt->y;

	switch(lButtonStatus)
	{
	case CScreenEditView::LBS_IN_GIZMO_TOP_LEFT:
		if(bounds.left + workPoint.x < bounds.right - MIN_SCREENOBJECTRECT_W)
		{
			bounds.left += workPoint.x;

			pos.x = bounds.left;
		}

		if(bounds.top + workPoint.y < bounds.bottom - MIN_SCREENOBJECTRECT_H)
		{
			bounds.top += workPoint.y;

			pos.y = bounds.top;
		}
		break;

	case CScreenEditView::LBS_IN_GIZMO_TOP_MIDDLE:
		if(bounds.top + workPoint.y < bounds.bottom - MIN_SCREENOBJECTRECT_H)
		{
			bounds.top += workPoint.y;

			pos.y = bounds.top;
		}
		break;

	case CScreenEditView::LBS_IN_GIZMO_TOP_RIGHT:
		if(bounds.right + workPoint.x > bounds.left + MIN_SCREENOBJECTRECT_W)
		{
			bounds.right += workPoint.x;
		}

		if(bounds.top + workPoint.y < bounds.bottom - MIN_SCREENOBJECTRECT_H)
		{
			bounds.top += workPoint.y;

			pos.y = bounds.top;
		}
		break;

	case CScreenEditView::LBS_IN_GIZMO_MIDDLE_LEFT:
		if(bounds.left + workPoint.x < bounds.right - MIN_SCREENOBJECTRECT_W)
		{
			bounds.left += workPoint.x;

			pos.x = bounds.left;
		}
		break;

	case CScreenEditView::LBS_IN_GIZMO_MIDDLE_RIGHT:
		if(bounds.right + workPoint.x > bounds.left + MIN_SCREENOBJECTRECT_W)
		{
			bounds.right += workPoint.x;
		}
		break;

	case CScreenEditView::LBS_IN_GIZMO_BOTTOM_LEFT:
		if(bounds.left + workPoint.x < bounds.right - MIN_SCREENOBJECTRECT_W)
		{
			bounds.left += workPoint.x;

			pos.x = bounds.left;
		}

		if(bounds.bottom + workPoint.y > bounds.top + MIN_SCREENOBJECTRECT_H)
		{
			bounds.bottom += workPoint.y;
		}
		break;

	case CScreenEditView::LBS_IN_GIZMO_BOTTOM_MIDDLE:
		if(bounds.bottom + workPoint.y > bounds.top + MIN_SCREENOBJECTRECT_H)
		{
			bounds.bottom += workPoint.y;
		}
		break;

	case CScreenEditView::LBS_IN_GIZMO_BOTTOM_RIGHT:
		if(bounds.right + workPoint.x > bounds.left + MIN_SCREENOBJECTRECT_W)
		{
			bounds.right += workPoint.x;
		}

		if(bounds.bottom + workPoint.y > bounds.top + MIN_SCREENOBJECTRECT_H)
		{
			bounds.bottom += workPoint.y;
		}
		break;
	}
#endif
}

void ScreenObject::SetMyUserInfo(char *name)
{
	if(userInfo)
		delete [] userInfo;

	userInfo = 0;
	
	if(!name)
		return;

	userInfo = new char [strlen(name) + 1];

	strcpy(userInfo, name);
}

//////////////////////////////////////////////////////////////////////////////////////
// ScreenObjectRect stuff here...

ScreenObjectRect::ScreenObjectRect(char *n, signed long l, signed long t, signed long r, signed long b) : ScreenObject(n)
{
	SetPos(l, t);
	SetBounds(l, t, r, b);

	SetType(SOT_SCREENOBJECTRECT);

		// Border Info.
	borderType = BT_NORMAL;
	borderColor	= RGB(0, 0, 0);
	borderThickness = 1;

	// Content Info.
	contentType = CT_NORMAL;
	contentColor = RGB(200, 200, 200);
}

ScreenObjectRect::ScreenObjectRect(void)
{
	borderColor = borderThickness = 0;
	borderType = BT_NORMAL;

	contentColor = 0;
	contentType = CT_NORMAL;
}

ScreenObjectRect::~ScreenObjectRect(void)
{
}

void ScreenObjectRect::Draw(CDC *pDC)
{	
	// Create the pen for the bounds.
	CPen borderPen;
	CBrush contentBrush;

	switch(borderType)
	{
	case BT_NORMAL:
		borderPen.CreatePen(PS_SOLID, borderThickness, borderColor);
		break;

	case BT_NONE:
		// even if the border is not visible, we want the rectangle to be visible
		borderPen.CreatePen(PS_DOT, borderThickness, BLACK_PEN);
		//borderPen.CreateStockObject(NULL_PEN);
		break;
	}

	switch(contentType)
	{
	case CT_NORMAL:
		contentBrush.CreateSolidBrush(contentColor);
		break;
		
	case CT_NONE:
		contentBrush.CreateStockObject(HOLLOW_BRUSH);
		break;
	}

	pDC->SelectObject(borderPen);
	pDC->SelectObject(contentBrush);
	
	pDC->Rectangle(bounds);

	// If this screen object is selected, draw the selection border.
	if(status == SOS_SELECTED)
	{
		DrawSelectionBorder(pDC);
		//DrawSelectionGizmos(pDC);
	}
}

signed long ScreenObjectRect::CheckSelectionDragGizmos(CPoint *pPoint)
{
	// Check each of the 8 gizmos.  If the mouse is not in any of them, just return
	// the default LBS_SCREENOBJECTCLICK.  Otherwise, return which one it's in.

	CRect gizmoRect;
	
	gizmoRect.SetRect(-DEFAULT_GIZMO_RECT_W / 2, -DEFAULT_GIZMO_RECT_H / 2, DEFAULT_GIZMO_RECT_W / 2, DEFAULT_GIZMO_RECT_H / 2);

	gizmoRect.OffsetRect(bounds.left, bounds.top);

	CPoint workPoint = *pPoint;

	workPoint.x -= DEFAULT_SCREEN_OFFSET_X;
	workPoint.y -= DEFAULT_SCREEN_OFFSET_Y;

	/////////////////////////////////////////////////////////////////
	
	// Check the top left one.
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_TOP_LEFT);

	// Check the top right one.
	gizmoRect.OffsetRect(bounds.Width(), 0);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_TOP_RIGHT);

	/////////////////////////////////////////////////////////////////

	// Check the bottom right one.
	gizmoRect.OffsetRect(0, bounds.Height());
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_BOTTOM_RIGHT);

	// Check the bottom left one.
	gizmoRect.OffsetRect(-bounds.Width(), 0);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_BOTTOM_LEFT);

	///////////////////////////////////////////////////////////////////

	// Check the top middle one.
	gizmoRect.SetRect(bounds.left + DEFAULT_GIZMO_RECT_W / 2, bounds.top - DEFAULT_GIZMO_RECT_H / 2, bounds.right - DEFAULT_GIZMO_RECT_W / 2, bounds.top + DEFAULT_GIZMO_RECT_H / 2);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_TOP_MIDDLE);

	// Check the bottom middle one.
	gizmoRect.SetRect(bounds.left + DEFAULT_GIZMO_RECT_W / 2, bounds.bottom - DEFAULT_GIZMO_RECT_H / 2, bounds.right - DEFAULT_GIZMO_RECT_W / 2, bounds.bottom + DEFAULT_GIZMO_RECT_H / 2);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_BOTTOM_MIDDLE);

	// Check the middle left one.
	gizmoRect.SetRect(bounds.left - DEFAULT_GIZMO_RECT_W / 2, bounds.top + DEFAULT_GIZMO_RECT_H / 2, bounds.left + DEFAULT_GIZMO_RECT_W / 2, bounds.bottom - DEFAULT_GIZMO_RECT_H / 2);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_MIDDLE_LEFT);

	// Check the middle right one.
	gizmoRect.SetRect(bounds.right - DEFAULT_GIZMO_RECT_W / 2, bounds.top + DEFAULT_GIZMO_RECT_H / 2, bounds.right + DEFAULT_GIZMO_RECT_W / 2, bounds.bottom - DEFAULT_GIZMO_RECT_H / 2);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_MIDDLE_RIGHT);

	return(ScreenObject::CheckSelectionDragGizmos(pPoint));
}

void ScreenObjectRect::DrawSelectionBorder(CDC *pDC)
{
	// Call the base selection window to get the border.
	ScreenObject::DrawSelectionBorder(pDC);
	
	CPen gizmoPen;
	CBrush gizmoBrush;
	CRect gizmoRect;

	gizmoPen.CreateStockObject(BLACK_PEN);
	gizmoBrush.CreateStockObject(WHITE_BRUSH);
	gizmoRect.SetRect(-DEFAULT_GIZMO_RECT_W / 2, -DEFAULT_GIZMO_RECT_H / 2, DEFAULT_GIZMO_RECT_W / 2, DEFAULT_GIZMO_RECT_H / 2);

	gizmoRect.OffsetRect(bounds.left, bounds.top);

	pDC->SelectObject(gizmoPen);

	pDC->SelectObject(gizmoBrush);
}

ScreenObject *ScreenObjectRect::Copy(CDC *pDC)
{
	char tempStr[1000];
	
	ScreenObjectRect *tempObject;

	tempObject = new ScreenObjectRect;

	*tempObject = *this;

	if(!strcmp(tempObject->name, DEFAULT_PAGE_NAME))
	{
		tempObject->SetName(DEFAULT_SCREEN_OBJECT_NAME);
	}

	if(tempObject->GetMyUserInfo())
	{
		strcpy(tempStr, tempObject->GetMyUserInfo());

		tempObject->FlushMyUserInfo();

		tempObject->SetMyUserInfo(tempStr);
	}

	return((ScreenObject *)tempObject);
}

//////////////////////////////////////////////////////////////////////////////////////
// ScreenObjectBitmap stuff here...

ScreenObjectBitmap::ScreenObjectBitmap(char *n, signed long x, signed long y) : ScreenObject(n)
{
	SetPos(x, y);

	SetType(SOT_SCREENOBJECTBITMAP);

	memset(&myBmpFile, 0x00, sizeof(BMPFile));

	contentType = CT_NORMAL;

	myBitmap = 0;
}

ScreenObjectBitmap::ScreenObjectBitmap(void)
{
	contentType = CT_NORMAL;
	
	memset(&myBmpFile, 0x00, sizeof(BMPFile));
}

ScreenObjectBitmap::~ScreenObjectBitmap(void)
{
	KillBitmap();
}

BOOL ScreenObjectBitmap::LoadBitmap(CDC *pDC, char *fileName)
{
	// Make sure nothing is loaded.
	KillBitmap();
	
	// Load the .BMP file.
	if(!LoadBMPFile(fileName, &myBmpFile))
		return(0);

	// STUPID WINDOWS!!!
	typedef struct tagmyBITMAPINFO
	{
		BITMAPINFOHEADER    bmiHeader;
		RGBQUAD             bmiColors[256];
	} myBITMAPINFO;

	myBITMAPINFO bmInfo;

	bmInfo.bmiHeader = myBmpFile.bmih;
	for( unsigned long i=0 ; i<myBmpFile.bmih.biClrUsed ; i++ )
		bmInfo.bmiColors[i] = myBmpFile.paletteBits[i];
	
	// Create the DD bitmap.
	myBitmap = CreateDIBitmap(pDC->GetSafeHdc(), &myBmpFile.bmih, CBM_INIT,
		myBmpFile.bitmapBits, (BITMAPINFO *)&bmInfo, DIB_RGB_COLORS);

	// Set up the bounds.
	bounds.right = myBmpFile.bmih.biWidth + bounds.left;
	bounds.bottom = myBmpFile.bmih.biHeight + bounds.top;

	return(1);
}

void ScreenObjectBitmap::KillBitmap(void)
{
	KillBMPFile(&myBmpFile);

	if(myBitmap)
	{
		::DeleteObject(myBitmap);

		myBitmap = 0;
	}
}

void ScreenObjectBitmap::Draw(CDC *pDC)
{
	// Create the compatible device context.
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(pDC);
	
	// Get a CBitmap pointer from the handle.
	CBitmap *tempBitmap = CBitmap::FromHandle(myBitmap);
	
	switch(contentType)
	{
	case CT_NORMAL:
		// Select the bitmap for drawing.
		dcCompatible.SelectObject(tempBitmap);
		break;
		
	case CT_NONE:
		break;
	}
	
	// Draw it!!
	pDC->BitBlt(bounds.left, bounds.top, bounds.Width(), bounds.Height(), &dcCompatible, 0, 0, SRCCOPY);

	// If this screen object is selected, draw the selection border.
	if(status == SOS_SELECTED)
	{
		DrawSelectionBorder(pDC);
	}
}

ScreenObject *ScreenObjectBitmap::Copy(CDC *pDC)
{
	char tempStr[1000];

	ScreenObjectBitmap *tempObject;

	tempObject = new ScreenObjectBitmap;

	*tempObject = *this;

	char tempFileName[255];

	// Save the file name.
	strcpy(tempFileName, tempObject->myBmpFile.fileName);

	// Clear out the bitmap DON'T DELETE IT!!!
	memset(&tempObject->myBmpFile, 0x00, sizeof(BMPFile));

	tempObject->myBitmap = 0;

	tempObject->LoadBitmap(pDC, tempFileName);

	if(tempObject->GetMyUserInfo())
	{
		strcpy(tempStr, tempObject->GetMyUserInfo());

		tempObject->FlushMyUserInfo();

		tempObject->SetMyUserInfo(tempStr);
	}

	return(tempObject);	
}

//////////////////////////////////////////////////////////////////////////////////////
// ScreenObjectString stuff here...

ScreenObjectString::ScreenObjectString(char *n, signed long x, signed long y, char *s) : ScreenObject(n)
{
	SetPos(x, y);

	SetType(SOT_SCREENOBJECTSTRING);

    memset(myString, 0, sizeof(myString));
//	myString = 0;

	contentType = CT_NORMAL;

	SetMyString(s, languageToView);

	textColor = 0;
	textBackground = 0x00FFFFFF;

	myFont = gDefaultFont;

	autoSizeToContent = 1;
    justification = JUST_CENTRE;
}

ScreenObjectString::ScreenObjectString(void)
{
	contentType = CT_NORMAL;
	myFont = gDefaultFont;

    memset(myString, 0, sizeof(myString));
//	myString = 0;

	textBackground = 0;
	textColor = 0;

	autoSizeToContent = 0;
    justification = JUST_CENTRE;
}

ScreenObjectString::~ScreenObjectString(void)
{
    unsigned long index;

    for (index = 0; index < NUMBER_LANGUAGES; index++)
    {                                                       //free all strings
    	if(myString[index])
    	{
    		delete [] myString[index];

    		myString[index] = 0;
    	}
    }
}

/*-----------------------------------------------------------------------------
    Name        : ScreenObjectString::SetMyString
    Description : Set the string for a particular language for a string object.
    Inputs      : s - string to set
                  sIndex - index of language (0..NUMBER_LANGUAGES)
    Outputs     :
    Return      :
    Note        : any strings other than sIndex that do not have a string
                    associated with them already will be set to the specified
                    string as well.
----------------------------------------------------------------------------*/
void ScreenObjectString::SetMyString(char *s, unsigned long sIndex)
{
    unsigned long index;
	// Don't bother to do anything if !s.

    for (index = 0; index < NUMBER_LANGUAGES; index++)
    {
        if (s == NULL)
        {                                                   //if setting string to NULL
            if (myString[index] != NULL && index == sIndex)
            {                                               //if this string is not NULL
                delete [] myString[index];
                myString[index] = NULL;
            }
        }
        else
        {                                                   //string is not NULL
            if (myString[index] == NULL || index == sIndex)
            {                                               //if we should set this string
                if (myString[index] != NULL)
                {
                    delete [] myString[index];
                    myString[index] = NULL;
                }
        		// Allocate and copy over the string.
        		myString[index] = new char [strlen(s) + 1];

        		strcpy(myString[index], s);
            }
        }
    }
/*
	if(s)
	{
		if(myString)
		{
			delete [] myString;

			myString = 0;
		}
		
		// Allocate and copy over the string.
		myString = new char [strlen(s) + 1];

		strcpy(myString, s);
	}
*/
}

void ScreenObjectString::SetStringBounds(CDC *pDC)
{
	CSize tempSize;
	unsigned long language, i, numLines=1, longestOne = 0, thisOne = 0;
    unsigned long maxNumLines = 1, maxLongestOne = 0, maxThisOne = 0;
    unsigned long nHits = 0;
    signed long longestLanguage = -1;
    CSize biggestSize(-1, -1);

    for (language = 0; language < NUMBER_LANGUAGES; language++)
    {
        if (myString[language] == NULL)
        {
            continue;
        }
        nHits++;
        numLines=1, longestOne = 0, thisOne = 0;
    	for( i=0 ; i<strlen(myString[language]) ; i++, thisOne++)
    	{
    		if(myString[language][i] == '\n')
    		{
    			numLines ++;
    			
    			if(thisOne > longestOne)
    			{
    				longestOne = thisOne;
    			}

    			thisOne = 0;
    		}
    	}
        tempSize = pDC->GetOutputTextExtent(myString[language], strlen(myString[language]));
        if (tempSize.cx >= biggestSize.cx)
        {
            longestLanguage = language;
            biggestSize.cx = tempSize.cx;
        }
		if(thisOne > longestOne)
		{
			longestOne = thisOne;
		}

        maxNumLines = max(maxNumLines, numLines);
        maxLongestOne = max(maxLongestOne, longestOne);
        maxThisOne = max(maxThisOne, thisOne);
    }

    assert(nHits > 0);

	if(maxThisOne > maxLongestOne)
		maxLongestOne = maxThisOne;

	tempSize = pDC->GetOutputTextExtent(myString[longestLanguage], maxLongestOne);
	
	if(tempSize.cx < MIN_SCREENOBJECTRECT_W)
		tempSize.cx = MIN_SCREENOBJECTRECT_W;

	if(tempSize.cy < MIN_SCREENOBJECTRECT_H)
		tempSize.cy = MIN_SCREENOBJECTRECT_H;

	tempSize.cy *= maxNumLines;

	bounds.right = bounds.left + tempSize.cx;
	bounds.bottom = bounds.top + tempSize.cy;
}

void ScreenObjectString::Draw(CDC *pDC)
{	
    UINT wJustification = DT_TOP;

	if(myString[languageToView])
	{
		pDC->SetBkColor(textBackground);

		CFont tempFont;

		// Not initialized.
		if(!strcmp(myFont.lfFaceName, ""))
		{
			pDC->SelectStockObject(SYSTEM_FONT);
		}
		else
		{
			tempFont.CreateFontIndirect(&myFont);
			
			pDC->SelectObject(&tempFont);
		}

		if(autoSizeToContent)
		{
			SetStringBounds(pDC);
		}

        switch (justification)
        {
            case JUST_LEFT:
                wJustification |= DT_LEFT;
                break;
            case JUST_RIGHT:
                wJustification |= DT_RIGHT;
                break;
            case JUST_CENTRE:
                wJustification |= DT_CENTER;
                break;
        }
		switch(contentType)
		{
		case CT_NORMAL:
			pDC->SetTextColor(textColor);
			pDC->SetBkColor(textBackground);
			pDC->DrawText(myString[languageToView], -1, &bounds, wJustification);
			break;

		case CT_MASKED:
			pDC->SetTextColor(textColor);
			pDC->SetBkMode(TRANSPARENT);
			pDC->DrawText(myString[languageToView], -1, &bounds, wJustification);
			break;

		case CT_NONE:
			break;
		}
	}


	if(status == SOS_SELECTED)
	{
		DrawSelectionBorder(pDC);
		//DrawSelectionGizmos(pDC);
	}
}

signed long ScreenObjectString::CheckSelectionDragGizmos(CPoint *pPoint)
{
	// Check each of the 8 gizmos.  If the mouse is not in any of them, just return
	// the default LBS_SCREENOBJECTCLICK.  Otherwise, return which one it's in.

	// Don't bother if we can't resize it anyway.
	if(autoSizeToContent)
	{
		return(ScreenObject::CheckSelectionDragGizmos(pPoint));
	}

	CRect gizmoRect;
	
	gizmoRect.SetRect(-DEFAULT_GIZMO_RECT_W / 2, -DEFAULT_GIZMO_RECT_H / 2, DEFAULT_GIZMO_RECT_W / 2, DEFAULT_GIZMO_RECT_H / 2);

	gizmoRect.OffsetRect(bounds.left, bounds.top);

	CPoint workPoint = *pPoint;

	workPoint.x -= DEFAULT_SCREEN_OFFSET_X;
	workPoint.y -= DEFAULT_SCREEN_OFFSET_Y;

	/////////////////////////////////////////////////////////////////
	
	// Check the top left one.
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_TOP_LEFT);

	// Check the top right one.
	gizmoRect.OffsetRect(bounds.Width(), 0);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_TOP_RIGHT);

	/////////////////////////////////////////////////////////////////

	// Check the bottom right one.
	gizmoRect.OffsetRect(0, bounds.Height());
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_BOTTOM_RIGHT);

	// Check the bottom left one.
	gizmoRect.OffsetRect(-bounds.Width(), 0);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_BOTTOM_LEFT);

	///////////////////////////////////////////////////////////////////

	// Check the top middle one.
	gizmoRect.SetRect(bounds.left + DEFAULT_GIZMO_RECT_W / 2, bounds.top - DEFAULT_GIZMO_RECT_H / 2, bounds.right - DEFAULT_GIZMO_RECT_W / 2, bounds.top + DEFAULT_GIZMO_RECT_H / 2);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_TOP_MIDDLE);

	// Check the bottom middle one.
	gizmoRect.SetRect(bounds.left + DEFAULT_GIZMO_RECT_W / 2, bounds.bottom - DEFAULT_GIZMO_RECT_H / 2, bounds.right - DEFAULT_GIZMO_RECT_W / 2, bounds.bottom + DEFAULT_GIZMO_RECT_H / 2);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_BOTTOM_MIDDLE);

	// Check the middle left one.
	gizmoRect.SetRect(bounds.left - DEFAULT_GIZMO_RECT_W / 2, bounds.top + DEFAULT_GIZMO_RECT_H / 2, bounds.left + DEFAULT_GIZMO_RECT_W / 2, bounds.bottom - DEFAULT_GIZMO_RECT_H / 2);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_MIDDLE_LEFT);

	// Check the middle right one.
	gizmoRect.SetRect(bounds.right - DEFAULT_GIZMO_RECT_W / 2, bounds.top + DEFAULT_GIZMO_RECT_H / 2, bounds.right + DEFAULT_GIZMO_RECT_W / 2, bounds.bottom - DEFAULT_GIZMO_RECT_H / 2);
	if(gizmoRect.PtInRect(workPoint))
		return(CScreenEditView::LBS_IN_GIZMO_MIDDLE_RIGHT);

	return(ScreenObject::CheckSelectionDragGizmos(pPoint));
}

ScreenObject *ScreenObjectString::Copy(CDC *pDC)
{
	ScreenObjectString *tempObject;
    unsigned int index;

	tempObject = new ScreenObjectString;

	*tempObject = *this;

	char tempStr[500];

	tempObject->FlushMyString();

    for (index = 0; index < NUMBER_LANGUAGES; index++)
    {
        if (myString[index] == NULL)
        {
            continue;
        }
    	strcpy(tempStr, GetMyString(index));

    	tempObject->SetMyString(tempStr, index);
    }

	if(tempObject->GetMyUserInfo())
	{
		strcpy(tempStr, tempObject->GetMyUserInfo());

		tempObject->FlushMyUserInfo();

		tempObject->SetMyUserInfo(tempStr);
	}

	return(tempObject);	
}

char *ScreenObjectString::GetFontName(void)
{
	return(myFont.lfFaceName);
}