// ChooseTextPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseTextPage dialog

class CChooseTextPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CChooseTextPage)

// Construction
public:
	CChooseTextPage();
	~CChooseTextPage();

// Dialog Data
	//{{AFX_DATA(CChooseTextPage)
	enum { IDD = IDD_CHOOSETEXT };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CChooseTextPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CChooseTextPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
