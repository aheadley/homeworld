// ChooseOutputPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseOutputPage dialog

class CChooseOutputPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CChooseOutputPage)

// Construction
public:
	CChooseOutputPage();
	~CChooseOutputPage();

// Dialog Data
	//{{AFX_DATA(CChooseOutputPage)
	enum { IDD = IDD_CHOOSEOUTPUT };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CChooseOutputPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CChooseOutputPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
