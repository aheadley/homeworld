// FEManDoc.h : interface of the CFEManDoc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CFEMANDOC_H
#define __CFEMANDOC_H

#include "screen.h"
#include "ccList.h"

class CFEManDoc : public CDocument
{
protected: // create from serialization only
	CFEManDoc();
	DECLARE_DYNCREATE(CFEManDoc)

// Attributes
public:

// Operations
public:

// Alex added this.
private:
protected:
	ccList screenList;

	ccList clipboardList;

	ccList undoList;

public:

	signed long gridActive;
	signed long gridVisible;
	signed long gridSpacing;

	//////// Clipboard functions.
	inline ccList *GetClipboardList(void)	{ return(&clipboardList); }

	void DoCopyScreenObject(CDC *pDC);
	void DoCutScreenObject(CDC *pDC);
	void DoPasteScreenObject(CDC *pDC);
	void DoUndoScreenObject(CDC *pDC);

	void CopySelectedScreensToDeleteBuffer(CDC *pDC);
	//////// End of clipboard functions.
	
	inline Screen *GetFirstScreen(void)		{ return((Screen *)screenList.GetHead()); }
	inline unsigned long GetNumScreens(void)	{ return(screenList.GetNumElements()); }
	Screen *GetFirstSelectedScreen(void);

	inline BOOL IsSelectedScreenIcon(Screen *s)	
	{ return(s->GetScreenStatus() == Screen::SS_SELECTED); }

	inline void SelectScreenIcon(Screen *s);
	inline void DeSelectScreenIcon(Screen *s);
	
	Screen *CreateScreen(CDC *pDC, CPoint *pPoint);
	Screen *CreateLoadedScreen(void);
	void DeleteScreen(Screen *pScreen);
	void DeleteSelectedScreens(void);

	Screen *PointOnScreenIcon(CPoint *pPoint);
	ScreenLink *PointOnScreenLink(CPoint *pPoint);

	void SelectScreenIconsInRect(CRect *pRect);

	unsigned long GetNumSelectedScreenIcons(void);

	void ClearAllSelectedScreenIcons(void);

	void RelocateAllScreens(void);
	void RelocateSelectedScreens(CPoint *pPressedAt, CPoint *pPoint);
	void RelocateSelectedScreenLink(CPoint *pPressedAt, CPoint *pPoint);

	void UpdateScreenEditView(void);

	void BringSelectedScreenObjectsToFront(void);
	void SendSelectedScreenObjectsToBack(void);

	void DeleteAbsolutelyEverything(void);

	void SaveEverything(CArchive *ar);
	void SaveScreen(Screen *s, CArchive *ar);
	void SaveScreenLink(ScreenLink *s, CArchive *ar);
	void SaveScreenObject(ScreenObject *s, CArchive *ar);
	void SaveScreenObjectRect(ScreenObjectRect *s, CArchive *ar);
	void SaveScreenObjectBitmap(ScreenObjectBitmap *s, CArchive *ar);
	void SaveScreenObjectString(ScreenObjectString *s, CArchive *ar);

	char *FindTag(char *buffer, char *tag, CArchive *ar);
	char *SkipTag(char *buffer);
	char *GetInt(char *buffer, signed long *num);

	void LoadEverything(CArchive *ar);
	void LoadScreen(CArchive *ar);
	void LoadScreenLink(Screen *s, CArchive *ar);

	void LoadScreenObject(ScreenObject *s, CArchive *ar);
	void LoadScreenObjectRect(Screen *s, CArchive *ar);
	void LoadScreenObjectBitmap(Screen *s, CArchive *ar);
	void LoadScreenObjectString(Screen *s, CArchive *ar);

	Screen *FindScreenWithID(signed long ID);
	void RealizeAllScreenLinks(void);
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFEManDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFEManDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFEManDoc)
	afx_msg void OnFileNew();
	afx_msg void OnFilePrint();
	afx_msg void OnFileSave();
	afx_msg void OnFileExport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif __CFEMANDOC_H
