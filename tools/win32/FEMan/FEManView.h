// FEManView.h : interface of the CFEManView class
//
/////////////////////////////////////////////////////////////////////////////

class CFEManView : public CView
{
protected: // create from serialization only
	CFEManView();
	DECLARE_DYNCREATE(CFEManView)

// Attributes
public:
	CFEManDoc* GetDocument();

// Operations
public:

// Alex added this.
private:
	
protected:
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFEManView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFEManView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFEManView)
	afx_msg void OnFilePrint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FEManView.cpp
inline CFEManDoc* CFEManView::GetDocument()
   { return (CFEManDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
