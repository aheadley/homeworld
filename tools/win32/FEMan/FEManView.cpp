// FEManView.cpp : implementation of the CFEManView class
//

#include "stdafx.h"
#include "FEMan.h"

#include "FEManDoc.h"
#include "FEManView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFEManView

IMPLEMENT_DYNCREATE(CFEManView, CView)

BEGIN_MESSAGE_MAP(CFEManView, CView)
	//{{AFX_MSG_MAP(CFEManView)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFEManView construction/destruction

CFEManView::CFEManView()
{
	// TODO: add construction code here

}

CFEManView::~CFEManView()
{
}

BOOL CFEManView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CFEManView drawing

void CFEManView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CFEManView printing

BOOL CFEManView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFEManView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFEManView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CFEManView diagnostics

#ifdef _DEBUG
void CFEManView::AssertValid() const
{
	CView::AssertValid();
}

void CFEManView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFEManDoc* CFEManView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFEManDoc)));
	return (CFEManDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFEManView message handlers

/////////////////////////////////////////////////////////////////////////////

void CFEManView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	CView::OnPrint(pDC, pInfo);
}

void CFEManView::OnFilePrint() 
{
}
