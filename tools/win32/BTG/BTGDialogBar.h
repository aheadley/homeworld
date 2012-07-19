/*
** BTGDialogBar.h : Header file for BTGDialogBar.cpp.
*/

#ifndef __BTGDIALOGBAR_H
#define __BTGDIALOGBAR_H

#define BITMAP_SEARCH_DIRECTORY		"\\HOMEWORLD\\DATA\\BTG\\BITMAPS\\*.TGA"
#define BITMAP_LOAD_DIRECTORY		"\\HOMEWORLD\\DATA\\BTG\\BITMAPS\\"

class CBTGVBackgroundBar : public CDialogBar
{
public:
	CBTGVBackgroundBar();

	virtual ~CBTGVBackgroundBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void OnInit(void);
	void OnUpdate(void);

	signed long red, green, blue;

protected:
	BOOL bUpdate, bFloating;

	DECLARE_MESSAGE_MAP()
};

class CBTGVDialogBar : public CDialogBar
{
// Constructor
public:
	CBTGVDialogBar();
	
// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual ~CBTGVDialogBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void OnInit(void);
	void OnUpdate(void);

	signed long red, green, blue, alpha, brightness;

protected:
	BOOL bUpdate, bFloating;
	
// Generated message map functions
protected:
	//{{AFX_MSG(CBTGDialogBar)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CBTGVisibleBar : public CDialogBar
{
// Constructor
public:
	CBTGVisibleBar();
	
// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual ~CBTGVisibleBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	BOOL bStars;
	signed long renderMode;

	void OnInit(void);
	void OnUpdate(void);

protected:
	
// Generated message map functions
protected:
	//{{AFX_MSG(CBTGDialogBar)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CBTGBitmapBar : public CDialogBar
{
// Constructor
public:
	CBTGBitmapBar();
	
// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual ~CBTGBitmapBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	char *curFileName;

	void OnInit(void);
	void OnUpdate(void);

protected:
	
// Generated message map functions
protected:
	//{{AFX_MSG(CBTGDialogBar)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif