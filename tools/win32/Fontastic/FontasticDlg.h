// FontasticDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFontasticDlg dialog

class CFontasticDlg : public CDialog
{
// Construction
public:
	CFontasticDlg(CWnd* pParent = NULL);	// standard constructor
	void SaveFontFile(CString &fileName);
	char *FixFontName(LOGFONT *tFont);

// Dialog Data
	//{{AFX_DATA(CFontasticDlg)
	enum { IDD = IDD_FONTASTIC_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontasticDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CFontasticDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnChoosefont();
	afx_msg void OnSaveas();
	afx_msg void OnEnableantialiasing();
	afx_msg void OnEnabledropshadow();
	afx_msg void OnChoosetext();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
