// HWVideoTweakerDlg.h : header file
//

#if !defined(AFX_HWVIDEOTWEAKERDLG_H__947925C9_6FA9_11D3_9146_00A0C982C829__INCLUDED_)
#define AFX_HWVIDEOTWEAKERDLG_H__947925C9_6FA9_11D3_9146_00A0C982C829__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <vector>
#include <string>
#include <algorithm>

using namespace std;

typedef vector<string> vec_string;
typedef vector<DWORD> vec_dword;
typedef vector<BOOL> vec_bool;

#pragma warning (disable : 4786)

/////////////////////////////////////////////////////////////////////////////
// CHWVideoTweakerDlg dialog

class CHWVideoTweakerDlg : public CDialog
{
// Construction
public:
	CHWVideoTweakerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CHWVideoTweakerDlg)
	enum { IDD = IDD_HWVIDEOTWEAKER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHWVideoTweakerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CHWVideoTweakerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeCombo1();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnSelchangeList2();
	virtual void OnOK();
	afx_msg void OnDefault();
	afx_msg void OnCrc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    string devstatsPath;
    string devstatsBackupPath;

    DWORD crc0, crc1;
    DWORD stats0, stats1;

    int nFileDevices, nDevices;

    vec_dword  fileCrcs[2];
    vec_dword  fileStats[2];
    vec_string fileNames;

    vec_dword crcs[2];
    vec_dword stats[2];
    vec_bool  dirty;

    void ReinitTables(BOOL enumerate);
    int  FindVideoCardIndex(void);

    void updateFeatureString(CListBox* box, int index, BOOL state);

    BOOL dupCRC(DWORD crc, int maxIndex);
    void addVideoCard(int index, CComboBox* box, DWORD crc0, DWORD stats0, DWORD crc1, DWORD stats1, char* name);
    void addDevstatsDevices(CComboBox* box);
    void setDirtyFlag(void);
    void setStatsBit(int whichStats, DWORD bit, BOOL state);

    void outputDevstats(void);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HWVIDEOTWEAKERDLG_H__947925C9_6FA9_11D3_9146_00A0C982C829__INCLUDED_)
