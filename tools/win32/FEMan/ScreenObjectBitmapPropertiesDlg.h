// ScreenObjectBitmapPropertiesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScreenObjectBitmapPropertiesDlg dialog

class CScreenObjectBitmapPropertiesDlg : public CDialog
{
// Construction
public:
	CScreenObjectBitmapPropertiesDlg(CWnd* pParent = NULL);   // standard constructor

	unsigned long contentVisible;

	char *userInfo;

// Dialog Data
	//{{AFX_DATA(CScreenObjectBitmapPropertiesDlg)
	enum { IDD = IDD_SOB_PROPERITES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScreenObjectBitmapPropertiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScreenObjectBitmapPropertiesDlg)
	afx_msg void OnCheck1();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
