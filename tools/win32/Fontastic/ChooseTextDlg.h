// ChooseTextDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseTextDlg dialog

class CChooseTextDlg : public CDialog
{
// Construction
public:
	CChooseTextDlg(CWnd* pParent = NULL);   // standard constructor
	CString textTemplate;

// Dialog Data
	//{{AFX_DATA(CChooseTextDlg)
	enum { IDD = IDD_CHOOSETEXT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseTextDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseTextDlg)
	afx_msg void OnFromfile();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEdit1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
