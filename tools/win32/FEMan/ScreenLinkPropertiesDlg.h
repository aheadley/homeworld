// ScreenLinkPropertiesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScreenLinkPropertiesDlg dialog

class CScreenLinkPropertiesDlg : public CDialog
{
// Construction
public:
	CScreenLinkPropertiesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CScreenLinkPropertiesDlg)
	enum { IDD = IDD_SL_PROPERTIES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	char screenLinkName[255];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScreenLinkPropertiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScreenLinkPropertiesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChangeEdit1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
