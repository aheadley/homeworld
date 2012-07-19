// ScreenObjectRectPropertiesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScreenObjectRectPropertiesDlg dialog

class CScreenObjectRectPropertiesDlg : public CDialog
{
// Construction
public:
	unsigned long borderColor;
	unsigned long contentColor;

	unsigned long borderVisible;
	unsigned long contentVisible;

	unsigned long borderWidth;

	unsigned long contentWidth;
	unsigned long contentHeight;

	char *userInfo;

	CScreenObjectRectPropertiesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CScreenObjectRectPropertiesDlg)
	enum { IDD = IDD_SOR_PROPERITES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScreenObjectRectPropertiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScreenObjectRectPropertiesDlg)
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnCheck1();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheck2();
	afx_msg void OnChangeEdit6();
	afx_msg void OnChangeEdit1();
	afx_msg void OnChangeEdit4();
	afx_msg void OnChangeEdit5();
	afx_msg void OnChangeEdit11();
	afx_msg void OnChangeEdit12();
	afx_msg void OnChangeEdit7();
	afx_msg void OnChangeEdit8();
	afx_msg void OnChangeEdit9();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
