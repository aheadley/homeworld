/*
** SCREEN.H : Header file for SCREEN.C.
*/

#ifndef __SCREEN_H
#define __SCREEN_H

#include "ccList.h"
#include "assert.h"

#include "bmp.h"

#define SL_DEFAULT_LENGTH				(30)
#define SL_DEFAULT_DB_SIZE				(14)

#define DEFAULT_SCREENNAME				"<Untitled>"
#define DEFAULT_SCREENNAME_BORDER_X		(20)
#define DEFAULT_SCREENNAME_BORDER_Y		(20)

#define DEFAULT_PAGE_SIZE_X				(100)
#define DEFAULT_PAGE_SIZE_Y				(100)

#define DEFAULT_PAGE_NAME				"PageBackground"
#define DEFAULT_SCREEN_OBJECT_NAME		"<Untitled>"

#define SCREEN_OBJECT_BORDER_CHECK_W	(10)
#define SCREEN_OBJECT_BORDER_CHECK_H	(10)

#define DEFAULT_GIZMO_RECT_W			(10)
#define DEFAULT_GIZMO_RECT_H			(10)

#define MIN_SCREENOBJECTRECT_W			(5)
#define MIN_SCREENOBJECTRECT_H			(5)

#define DEFAULT_SCREENLINKNAME			"<Untitled>"

#define NUMBER_LANGUAGES                 6

//string justification
#define JUST_LEFT                       0
#define JUST_RIGHT                      1
#define JUST_CENTRE                     2

extern signed long globalScreenIDMaster;

class Screen;
class ScreenLink;

class ScreenLinkContainer : public ccNode
{
private:
	ScreenLink *myLink;

protected:
public:
	inline ScreenLink *GetMyLink(void)			{ return(myLink); }
	inline void SetMyLink(ScreenLink *sl)		{ myLink = sl; }

	inline ScreenLinkContainer(void)			{ myLink = 0; }
	~ScreenLinkContainer(void);
};

class ScreenLink : public ccNode
{
private:
	signed long status;
	signed long type;

	Screen *myScreen;
	Screen *linkedTo;

	CRect bounds;	// The bounds of the drag box for this screenLink.

	CPoint src;
	CPoint dst;

	char name[255];

	ScreenLinkContainer *myContainer;

	signed long linkedToID;

protected:
public:
	enum tagScreenLinkStatus
	{
		SLS_NORMAL,
		SLS_INACTIVE,
		SLS_SELECTED,

		SLS_LAST_SLS
	};

	enum tagScreenLinkType
	{
		SLT_DEFAULT_PREV,
		SLT_DEFAULT_NEXT,
		SLT_EXTRA,

		SLT_LAST_SLT
	};

	inline signed long GetLinkedToID(void)		{ return(linkedToID); }
	inline void SetLinkedToID(signed long t)		{ linkedToID = t; }

	inline signed long GetType(void)		{ return(type); }
	inline void SetType(signed long t)		{ type = t; }

	inline unsigned long GetStatus(void)	{ return(status); }
	inline void SetStatus(signed long s)	{ status = s; }

	inline Screen *GetMyScreen(void)		{ return(myScreen); }
	inline Screen *LinkedTo(void)			{ return(linkedTo); }

	inline CRect *GetBounds(void)			{ return(&bounds); }

	inline BOOL IsReadOnly(void)			{ return((type == SLT_DEFAULT_PREV) || (type == SLT_DEFAULT_NEXT)); }

	inline CPoint *GetLinkSrc(void)			{ return(&src); }
	inline void SetLinkSrc(CPoint *c)		{ SetLinkSrc(c->x, c->y); }
	void SetLinkSrc(signed long x, signed long y);
	
	inline CPoint *GetLinkDst(void)			{ return(&dst); }
	void SetLinkDst(CPoint *c);
	void SetLinkDst(signed long x, signed long y);

	inline char *GetName(void)				{ return(name); }
	inline void SetName(char *n)			{ strcpy(name, n); }

	ScreenLink(signed long t=ScreenLink::SLT_EXTRA, Screen *parent=0);
	ScreenLink(Screen *parent);
	~ScreenLink(void);

	void SetLinkPos(void);

	void AttachTo(Screen *pScreen);
	void Detach(void);
};

class ScreenObject : public ccNode
{
private:
protected:
	signed long status;
	signed long type;

	char name[255];
	char *userInfo;

	CPoint pos;
	CRect bounds;

public:
	enum tagScreenObjectStatus
	{
		SOS_NORMAL,
		SOS_INACTIVE,
		SOS_SELECTED,

		SOS_LAST_SOS
	};

	enum tagScreenObjectTypes
	{
		SOT_SCREENOBJECTRECT,
		SOT_SCREENOBJECTBITMAP,
		SOT_SCREENOBJECTSTRING,

		SOT_LAST_SOT,
	};

	ScreenObject(char *n);
	ScreenObject(void); // This is the loaded construct.
	~ScreenObject(void);

	inline signed long GetType(void)			{ return(type); }
	inline void SetType(signed long t)			{ type = t; }

	inline signed long GetStatus(void)			{ return(status); }
	inline void SetStatus(signed long s)		{ status = s; }

	inline char *GetName(void)					{ return(name); }
	inline void SetName(char *n)				{ strcpy(name, n); }

	inline void FlushMyUserInfo(void)			{ userInfo = 0; }
	inline char *GetMyUserInfo(void)			{ return(userInfo); }
	void SetMyUserInfo(char *name);

	inline CPoint *GetPos(void)					{ return(&pos); }
	void SetPos(signed long x, signed long y);

	inline CRect *GetBounds(void)				{ return(&bounds); }
	virtual inline void SetBounds(CRect *r)				{ bounds = *r; }
	
	virtual void Draw(CDC *pDC) = 0;

	virtual void DrawSelectionBorder(CDC *pDC);
	
	virtual signed long CheckSelectionDragGizmos(CPoint *pPoint);

	void Resize(CPoint *pPressedAt, CPoint *pPoint, signed long lButtonStatus);

	inline virtual ScreenObject *Copy(CDC *pDC) { return(0); }
};

class ScreenObjectRect : public ScreenObject
{
private:
	// Border Info.
	unsigned long borderType;
	unsigned long borderColor;
	unsigned long borderThickness;

	// Content Info.
	unsigned long contentType;
	unsigned long contentColor;

protected:
public:
	enum tagBorderTypes
	{
		BT_NORMAL,
		BT_NONE,

		BT_LAST_BT
	};

	enum tagContentTypes
	{
		CT_NORMAL,
		CT_NONE,

		CT_LAST_CT
	};

	ScreenObjectRect(char *n, signed long l = 0, signed long t = 0, signed long r = 100, signed long b = 100);
	ScreenObjectRect(void); // This is the loaded construct.
	~ScreenObjectRect(void);
	
	inline void SetBounds(CRect *r)				{ bounds = *r; }
	inline void SetBounds(signed long l, signed long t, signed long r, signed long b)
		{ bounds.SetRect(l, t, r, b); }
	inline void SetBoundsWidth(signed long w)	{ bounds.right = bounds.left + w; }
	inline void SetBoundsHeight(signed long h)	{ bounds.bottom = bounds.top + h; }

	inline unsigned long GetBorderColor(void)	{ return(borderColor); }
	inline void SetBorderColor(unsigned long c)	{ borderColor = c; }

	inline unsigned long GetContentColor(void)	{ return(contentColor); }
	inline void SetContentColor(unsigned long c){ contentColor = c; }

	inline unsigned long GetBorderType(void)	{ return(borderType); }
	inline void SetBorderType(unsigned long t)	{ borderType = t; }

	inline unsigned long GetContentType(void)	{ return(contentType); }
	inline void SetContentType(unsigned long t)	{ contentType = t; }

	inline unsigned long GetBorderWidth(void)	{ return(borderThickness); }
	inline void SetBorderWidth(unsigned long w)	{ borderThickness = w; }

	inline unsigned long GetContentWidth(void)	{ return(bounds.Width()); }
	inline void SetContentWidth(unsigned long w)	{ SetBoundsWidth(w); }

	inline unsigned long GetContentHeight(void)	{ return(bounds.Height()); }
	inline void SetContentHeight(unsigned long h)	{ SetBoundsHeight(h); }
	
	virtual signed long CheckSelectionDragGizmos(CPoint *pPoint);
	
	virtual void Draw(CDC *pDC);

	virtual void DrawSelectionBorder(CDC *pDC);

	ScreenObject *Copy(CDC *pDC);
};

class ScreenObjectBitmap : public ScreenObject
{
private:
	BMPFile myBmpFile;
	HBITMAP myBitmap;

	// Content Info.
	unsigned long contentType;

protected:
public:
	enum tagContentTypes
	{
		CT_NORMAL,
		CT_NONE,

		CT_LAST_CT
	};
	
	inline HBITMAP GetMyBitmap(void)			{ return(myBitmap); }
	inline BMPFile *GetMyBmpFile(void)			{ return(&myBmpFile); }

	inline unsigned long GetContentType(void)	{ return(contentType); }
	inline void SetContentType(unsigned long t)	{ contentType = t; }

	inline void SetFileName(char *n)			{ strcpy(myBmpFile.fileName, n); }

	BOOL LoadBitmap(CDC *pDC, char *fileName);
	void KillBitmap(void);

	inline char *GetFileName(void)				{ return(myBmpFile.fileName); }

	ScreenObjectBitmap(char *n, signed long x, signed long y);
	ScreenObjectBitmap(void); // This is the loaded construct.
	~ScreenObjectBitmap(void);
	
	inline unsigned long GetContentWidth(void)	{ return(bounds.Width()); }
	inline unsigned long GetContentHeight(void)	{ return(bounds.Height()); }
	
	virtual void Draw(CDC *pDC);

	ScreenObject *Copy(CDC *pDC);
};

class ScreenObjectString : public ScreenObject
{
private:
    unsigned long justification;
	char *myString[NUMBER_LANGUAGES];

	LOGFONT myFont;

	// Content Info.
	unsigned long contentType;

	unsigned long textColor;
	unsigned long textBackground;

	unsigned long autoSizeToContent;

protected:
public:
	enum tagContentTypes
	{
		CT_NORMAL,
		CT_MASKED,
		CT_NONE,

		CT_LAST_CT
	};
	
	inline unsigned long GetAutoSizeToContent(void)		{ return(autoSizeToContent); }
	inline void SetAutoSizeToContent(unsigned long asc)	{ autoSizeToContent = asc; }
	
	inline char *GetMyString(unsigned long sIndex)		{ assert(sIndex < NUMBER_LANGUAGES); return(myString[sIndex]); }
	void SetMyString(char *s, unsigned long sIndex);
	
	inline LOGFONT *GetMyFont(void)		{ return(&myFont); }
	inline void SetMyFont(LOGFONT *l)	{ myFont = *l; }

	inline unsigned long GetContentType(void)	{ return(contentType); }
	inline void SetContentType(unsigned long t)	{ contentType = t; }

	inline unsigned long GetTextColor(void)		{ return(textColor); }
	inline void SetTextColor(unsigned long t)	{ textColor = t; }

	inline unsigned long GetTextBackground(void)		{ return(textBackground); }
	inline void SetTextBackground(unsigned long t)	{ textBackground = t; }

    inline unsigned long GetJustification(void)        { return(justification); }
    inline void SetJustification(unsigned long j)   { justification = j; }

	ScreenObjectString(char *n, signed long x, signed long y, char *s=NULL);
	ScreenObjectString(void); // This is the loaded construct.
	~ScreenObjectString(void);
	
	inline unsigned long GetContentWidth(void)	{ return(bounds.Width()); }
	inline unsigned long GetContentHeight(void)	{ return(bounds.Height()); }
	
	void SetStringBounds(CDC *pDC);

	virtual void Draw(CDC *pDC);

	signed long CheckSelectionDragGizmos(CPoint *pPoint);

	inline void FlushMyString(void)		{ unsigned long index; for (index = 0; index < NUMBER_LANGUAGES; index++) {myString[index] = 0;} }

	char *GetFontName(void);
	
	ScreenObject *Copy(CDC *pDC);
};

class Screen : public ccNode
{
private:
	////////////////////////////////////////////////////////////////////////
	// Screen Icon stuff...

	CPoint iconPos;
	CRect iconBounds;

	CSize nameSize;
	char *screenName;
	char *userInfo;

	signed long screenStatus;

	ScreenLink *defPrev;
	ScreenLink *defNext;

	ScreenLink *selectedLink;

	ccList screenLinkList;
	ccList inboundLinkList;

	signed long screenID;			// This screen's unique ID #.

	///////////////////////////////////////////////////////////////////////////
	// Actual Screen stuff...

		ccList screenObjectList;

protected:
public:
	enum tagScreenStatus
	{
		SS_NORMAL,
		SS_INACTIVE,
		SS_SELECTED,

		SS_LAST_SS
	};

	////////////////////////////////////////////////////////////////////////
	// Screen Icon stuff...

	inline CPoint *GetIconPos(void)				{ return(&iconPos); }
	inline signed long GetIconX(void)			{ return(iconPos.x); }
	inline signed long GetIconY(void)			{ return(iconPos.y); }
	inline CRect *GetIconBounds(void)			{ return(&iconBounds); }

	inline CSize *GetNameSize(void)				{ return(&nameSize); }
	inline char *GetName(void)					{ return(screenName); }

	inline char *GetMyUserInfo(void)			{ return(userInfo); }
	void SetMyUserInfo(char *name);
	
	inline signed long GetScreenStatus(void)	{ return(screenStatus); }
	inline void SetScreenStatus(signed long s)	{ screenStatus = s; }

	inline unsigned long GetNumScreenLinks(void){ return(screenLinkList.GetNumElements()); }
	inline ScreenLink *GetFirstScreenLink(void)	{ return((ScreenLink *)screenLinkList.GetHead()); }

	inline ScreenLink *GetSelectedLink(void)	{ return(selectedLink); }
	inline void SetSelectedLink(ScreenLink *s)	{ selectedLink = s; }

	inline ScreenLink *GetDefPrevLink(void)		{ return(defPrev); }
	inline ScreenLink *GetDefNextLink(void)		{ return(defNext); }

	inline ccList *GetInboundLinkList(void)		{ return(&inboundLinkList); }

	inline signed long GetScreenID(void)		{ return(screenID); }
	void SetScreenID(void);
	inline void SetScreenID(signed long ID)		{ screenID = ID; } // If it's loaded.

	void SelectScreenLink(ScreenLink *s);
	void DeSelectScreenLink(ScreenLink *s);
	
	Screen(CDC *pDC, signed long x=0, signed long y=0, char *name=0);
	Screen(void); // This is the loaded construct.
	~Screen(void);

	ScreenLink *AddScreenLink(void);
	
	void AddExtraScreenLink(void);

	void DeleteScreenLink(ScreenLink *sl);

	void SetScreenPos(signed long x, signed long y);
	void SetScreenName(CDC *pDC, char *name);

	///////////////////////////////////////////////////////////////////////////
	// Actual Screen stuff...

	inline ccList *GetScreenObjectList(void)				{ return(&screenObjectList); }

	inline unsigned long GetNumScreenObjects(void)			{ return(screenObjectList.GetNumElements()); }
	
	unsigned long GetNumSelectedScreenObjects(void);

	inline BOOL IsSelectedScreenObject(ScreenObject *s)		{ return(s->GetStatus() == ScreenObject::SOS_SELECTED); }
	
	inline ScreenObject *GetFirstScreenObject(void)			{ return((ScreenObject *)screenObjectList.GetHead()); }

	inline BOOL IsObjectDefaultPage(ScreenObject *s)		{ return(!strcmp(s->GetName(), DEFAULT_PAGE_NAME)); }

	inline void AddScreenObject(ScreenObject *so)			{ screenObjectList.AddNode(so); }
	
	void SelectScreenObject(ScreenObject *so);
	void DeSelectScreenObject(ScreenObject *so);
	
	void SelectScreenObjectsInRect(CRect *pRect);

	void RelocateSelectedScreensObjects(CPoint *pPressedAt, CPoint *pPoint);
	void SetExplicitScreensObjectPosition(CPoint *pPoint);
	
	ScreenObject *GetFirstSelectedScreenObject(void);

	ScreenObject *PointOnScreenObject(CPoint *cPoint);

	CRect *GetPageBounds(void);
	void SetPageBounds(CRect *r);

	void SetPageWidth(unsigned long w);
	void SetPageHeight(unsigned long h);

	void ClearAllSelectedScreenObjects(void);

	void DeleteSelectedScreenObjects(void);
};

#endif // __SCREEN_H