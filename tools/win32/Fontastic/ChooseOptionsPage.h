// ChooseOptionsPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseOptionsPage dialog

class CChooseOptionsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CChooseOptionsPage)

// Construction
public:
	CChooseOptionsPage();
	~CChooseOptionsPage();

// Dialog Data
	//{{AFX_DATA(CChooseOptionsPage)
	enum { IDD = IDD_CHOOSEOPTIONS };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CChooseOptionsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CChooseOptionsPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
