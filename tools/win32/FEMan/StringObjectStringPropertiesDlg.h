// StringObjectStringPropertiesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStringObjectStringPropertiesDlg dialog

#include "screen.h"

class CStringObjectStringPropertiesDlg : public CDialog
{
// Construction
public:
	CStringObjectStringPropertiesDlg(CWnd* pParent = NULL);   // standard constructor

    unsigned long justification;
	unsigned long contentVisible;
	LOGFONT myFont;
	char *myString[NUMBER_LANGUAGES];

	unsigned long textColor;
	unsigned long textBackground;

	unsigned long autoSizeToContent;

	char *userInfo;

// Dialog Data
	//{{AFX_DATA(CStringObjectStringPropertiesDlg)
	enum { IDD = IDD_SOS_PROPERITES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStringObjectStringPropertiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStringObjectStringPropertiesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButton1();
	virtual void OnOK();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnChangeEdit1();
	afx_msg void OnChangeEdit4();
	afx_msg void OnChangeEdit5();
	afx_msg void OnChangeEdit6();
	afx_msg void OnChangeEdit7();
	afx_msg void OnChangeEdit8();
	afx_msg void OnButton3();
	afx_msg void OnButton2();
	afx_msg void OnCheck1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
