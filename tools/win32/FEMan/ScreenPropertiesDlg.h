// ScreenPropertiesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScreenPropertiesDlg dialog

class CScreenPropertiesDlg : public CDialog
{
// Construction
public:
	CScreenPropertiesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CScreenPropertiesDlg)
	enum { IDD = IDD_SCREENPROPERTIES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	char screenName[255];
	unsigned long screenWidth;
	unsigned long screenHeight;
	char *userInfo;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScreenPropertiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScreenPropertiesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
