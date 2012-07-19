// GridOptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGridOptionsDlg dialog

class CGridOptionsDlg : public CDialog
{
// Construction
public:
	CGridOptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGridOptionsDlg)
	enum { IDD = IDD_GRIDOPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	unsigned long gridActive;
	unsigned long gridVisible;
	unsigned long gridSpacing;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGridOptionsDlg)
	afx_msg void OnCheck1();
	afx_msg void OnCheck2();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEdit1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
