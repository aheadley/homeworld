// ChooseFontPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseFontPage dialog

class CChooseFontPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CChooseFontPage)

// Construction
public:
	CChooseFontPage();
	~CChooseFontPage();

// Dialog Data
	//{{AFX_DATA(CChooseFontPage)
	enum { IDD = IDD_CHOOSEFONT };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CChooseFontPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CChooseFontPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
