// HWVideoTweakerDlg.cpp : implementation file
//

#define WIN32_LEAN_AND_MEAN
#define STRICT
#define D3D_OVERLOADS

#include <string>

#include "stdafx.h"
#include "HWVideoTweaker.h"
#include "HWVideoTweakerDlg.h"
#include "devstats.h"

#include "/homeworld/src/rgl/d3d/ddraw.h"
#include "/homeworld/src/rgl/d3d/d3drm.h"
#include "crc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//where registry entries might be
#define BASEKEYNAME "SOFTWARE\\Sierra On-Line\\Homeworld"
#define DEMOKEYNAME "SOFTWARE\\Sierra On-Line\\Homeworld Downloadable"

//devstats.dat header version string
#define TWEAKERVERSIONSTRING "1.1 tweak"

//convenient macros for obtaining combo & list box objects
#define GET_COMBOBOX(name) CComboBox* name = (CComboBox*)GetDlgItem(IDC_COMBO1)
#define GET_LISTBOX(name)  CListBox* name  = (CListBox*)GetDlgItem(IDC_LIST2)

//structure for device detection (crc generation)
typedef struct
{
    DWORD vendor;
    WORD  product;
    DWORD device;
} hwDat;

hwDat dat;
DWORD devCRC, devCRC1;

//
//features of devstats.dat
//
typedef struct
{
    char*  name;
    int    whichFlag;
    DWORD  bit;
    char*  comment;
} devstats_listbox;

devstats_listbox devstatsList[] =
{
    "OpenGL",                                   0, DEVSTAT_NOGL_9X, "gl",
    "OpenGL, Win95 only",                       1, DEVSTAT2_NOGL_95, "gl 95",
    "Direct3D",                                 0, DEVSTAT_NOD3D_9X, "d3d",
    "Direct3D, Win95 only",                     1, DEVSTAT2_NOD3D_95, "d3d 95",
    "3dfx OpenGL",                              0, DEVSTAT_NO_3DFXGL, "3dfxgl",
    "32bit modes",                              0, DEVSTAT_NO_32BIT, "32bit",
    "OpenGL 32bit modes",                       0, DEVSTAT_NO_32BIT_GL, "32bit gl",
    "OpenGL DirectDraw",                        0, DEVSTAT_NO_DDRAW, "ddraw",
    "OpenGL double buffer actually double",     0, DEVSTAT_GL_TRIPLE, "triple",
    "software DirectDraw",                      0, DEVSTAT_NO_DDRAWSW, "sw ddraw",
    "fast front end",                           0, DEVSTAT_NOFFE, "ffe",
    "fast front end, OpenGL only",              0, DEVSTAT_NOFFE_GL, "gl ffe",
    "fast front end, Direct3D only",            0, DEVSTAT_NOFFE_D3D, "d3d ffe",
    "paletted textures",                        0, DEVSTAT_NOPAL, "pal",
    "antialiasing",                             0, DEVSTAT_NO_AA, "aa",
    "read textures from hardware",              0, DEVSTAT_NO_GETTEXIMAGE, "teximage",
    "point smoothing",                          0, DEVSTAT_NO_POINTSMOOTH, "pointsmooth",
    "additive alpha",                           1, DEVSTAT2_NO_ADDITIVE, "add",
    "iterated alpha",                           1, DEVSTAT2_NO_IALPHA, "ialpha",
    "DrawPixels",                               1, DEVSTAT2_NO_DRAWPIXELS, "drawpixels",
    "user clipping planes",                     1, DEVSTAT2_NO_USERCLIP, "userclip",
    "dither while alpha blending",              1, DEVSTAT2_NO_DALPHA, "dalpha",
    "background colour",                        0, DEVSTAT_NO_BGCOLOUR, "bg",
    "background colour, Direct3D only",         0, DEVSTAT_NOD3D_BGCOLOUR, "d3d bg",
    "don't nudge fonts in y",                   0, DEVSTAT_YADJUST, "yadj",
    "don't nudge fonts in x",                   0, DEVSTAT_XADJUST, "xadj",
    "unfiltered fonts",                         0, DEVSTAT_NEGXADJUST, "-xadj",
    "mode disabling doesn't affect software",   0, DEVSTAT_DISABLE_SW, "sw disable",
    "mixed depth modes",                        1, DEVSTAT2_BROKEN_DEPTH, "depth",
    "small textures",                           0, DEVSTAT_LARGE_TEXTURES, "texmin",

    "16bit 640x480",                            0, DEVSTAT_NO_64048016, "!640x480x16",
    "16bit 800x600",                            0, DEVSTAT_NO_80060016, "!800x600x16",
    "16bit 1024x768",                           0, DEVSTAT_NO_102476816, "!1024x768x16",
    "16bit 1280x1024",                          0, DEVSTAT_NO_1280102416, "!1280x1024x16",
    "16bit 1600x1200",                          0, DEVSTAT_NO_1600120016, "!1600x1200x16",
    "32bit 640x480",                            0, DEVSTAT_NO_64048032, "!640x480x32",
    "32bit 800x600",                            0, DEVSTAT_NO_80060032, "!800x600x32",
    "32bit 1024x768",                           0, DEVSTAT_NO_102476832, "!1024x768x32",
    "32bit 1280x1024",                          0, DEVSTAT_NO_1280102432, "!1280x1024x32",
    "32bit 1600x1200",                          0, DEVSTAT_NO_1600120032, "!1600x1200x32",

    "", 0, 0, ""
};

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHWVideoTweakerDlg dialog

CHWVideoTweakerDlg::CHWVideoTweakerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHWVideoTweakerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHWVideoTweakerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHWVideoTweakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHWVideoTweakerDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHWVideoTweakerDlg, CDialog)
	//{{AFX_MSG_MAP(CHWVideoTweakerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_LBN_SELCHANGE(IDC_LIST2, OnSelchangeList2)
	ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
	ON_BN_CLICKED(IDC_CRC, OnCrc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHWVideoTweakerDlg message handlers

//
// add all the features of devstats.dat from our convenient structure
//
void populateFeatureBox(CListBox* box)
{
    int index;

    for (index = 0; devstatsList[index].bit != 0; index++)
    {
        box->AddString(devstatsList[index].name);
    }
}

//
// add a particular video card to the vidcard combo box
//
void CHWVideoTweakerDlg::addVideoCard(int index, CComboBox* box, DWORD crc0, DWORD stats0, DWORD crc1, DWORD stats1, char* name)
{
    crcs[0][index] = crc0;
    crcs[1][index] = crc1;
    stats[0][index] = stats0;
    stats[1][index] = stats1;
    box->AddString(name);
}

//
// append a trailing backslash to a string if there isn't yet one there
//
void possiblyAppendBackslash(string& str)
{
    if (str.length() < 1)
    {
        return;
    }
    if (str[str.length() - 1] != '\\')
    {
        str.append("\\");
    }
}

//
// extract device name from a line in devstats.dat.
// won't add multiple or trailing spaces / tabs
//
void extractNameFromLine(string& name, string& line)
{
    int pos;
    BOOL spacing;

    name = "";
    pos = 36;
    if (line.length() <= pos)
    {
        return;
    }
    spacing = FALSE;
    while (line[pos] != ';' && line[pos] >= ' ')
    {
        if (spacing && (line[pos] == ' ' || line[pos] == '\t'))
        {
            pos++;
        }
        else
        {
            if (spacing)
            {
                name += ' ';
            }
            name += line[pos++];
            if (line[pos] == ' ' || line[pos] == '\t')
            {
                spacing = TRUE;
            }
            else
            {
                spacing = FALSE;
            }
        }
    }
}

//
// determine whether HWVideoTweaker already knows about a
// device with a particular crc1
//
BOOL CHWVideoTweakerDlg::dupCRC(DWORD crc, int maxIndex)
{
    int index;

    for (index = 0; index < maxIndex; index++)
    {
        if (crcs[1][index] == crc)
        {
            return TRUE;
        }
    }
    return FALSE;
}

//
// copy a file, slowly.
// will abort if !overwrite & file already exists
//
BOOL copyFile(string& outName, string& inName, BOOL overwrite)
{
    int ch;

    if (!overwrite)
    {
        //check if file exists
        FILE* test = fopen(outName.c_str(), "rb");
        if (test != NULL)
        {
            //file exists, abort
            fclose(test);
            return TRUE;
        }
    }
    //open input
    FILE* in = fopen(inName.c_str(), "rb");
    if (in == NULL)
    {
        return FALSE;
    }
    //open output
    FILE* out = fopen(outName.c_str(), "wb");
    if (out == NULL)
    {
        fclose(in);
        return FALSE;
    }
    //copy the file, 1 byte at a time
    for (;;)
    {
        ch = fgetc(in);
        if (feof(in))
        {
            break;
        }
        fputc(ch, out);
    }
    //done
    fclose(out);
    fclose(in);

    return TRUE;
}

//
// add all the devices in devstats.dat to the vidcard combo box
//
void CHWVideoTweakerDlg::addDevstatsDevices(CComboBox* box)
{
    //first check for HW_Data registry entry
    int   deviceIndex;
    HKEY  key;
    DWORD type, length;
    BYTE  data[1024];
    BOOL  error;
    char* root;
    char  line[1024];
    string name;
    int   len;
    
    error = TRUE;
    //first check for full-release Homeworld registry entry
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, BASEKEYNAME,
                     0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS)
    {
        length = 256;
        if (RegQueryValueEx(key, "HW_Data", 0, &type, data, &length) == ERROR_SUCCESS)
        {
            if (type == REG_SZ)
            {
                devstatsPath = (char*)data;
                possiblyAppendBackslash(devstatsPath);
                error = FALSE;
            }
        }
        RegCloseKey(key);
    }

    if (error)
    {
        //next check for installed downloadable demo
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, DEMOKEYNAME,
                         0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS)
        {
            length = 256;
            if (RegQueryValueEx(key, "HW_Data", 0, &type, data, &length) == ERROR_SUCCESS)
            {
                if (type == REG_SZ)
                {
                    devstatsPath = (char*)data;
                    possiblyAppendBackslash(devstatsPath);
                    error = FALSE;
                }
            }
            RegCloseKey(key);
        }
    }
    
    if (error)
    {
        //then check HW_Root environment variable
        error = TRUE;
        root = getenv("HW_Root");
        if (root != NULL)
        {
            devstatsPath = root;
            possiblyAppendBackslash(devstatsPath);
            devstatsPath += "exe\\";
            error = FALSE;
        }
    }

    if (error)
    {
        //default to current directory
        devstatsPath = "";
        error = FALSE;
    }

    //complete pathname
    devstatsPath += "devstats.dat";
    //generate backup pathname
    devstatsBackupPath = devstatsPath;
    len = devstatsBackupPath.length();
    devstatsBackupPath[len - 3] = 'b';
    devstatsBackupPath[len - 1] = 'k';
    //backup devstats
    if (!copyFile(devstatsBackupPath, devstatsPath, FALSE))
    {
        MessageBox("couldn't backup devstats.dat", "Write Failed", MB_ICONERROR);
        PostQuitMessage(0);
        return;
    }
    FILE* in = fopen(devstatsPath.c_str(), "rt");
    if (in == NULL)
    {
        //fatally fail
        MessageBox("cannot find devstats.dat.  You must install Homeworld.", "Missing File", MB_ICONERROR);
        PostQuitMessage(0);
        return;
    }
    
    //read in the devstats info
    for (deviceIndex = 0, nFileDevices = 0;;)
    {
        if (fgets(line, 1023, in) == NULL)
        {
            break;
        }
        if (line[0] == ';')
        {
            continue;
        }
        if (strlen(line) < 4)
        {
            continue;
        }

        //extract device name & trim trailing spaces
        extractNameFromLine(name, string(line));
        
        //scan crc & stats info
        sscanf(line, "%X %X %X %X", &crc0, &stats0, &crc1, &stats1);

        if (crc0 != 0x00000000 || crc1 != 0x00000000)
        {
            //add device to our master list of all devs, dups and all
            fileCrcs[0].push_back(crc0);
            fileCrcs[1].push_back(crc1);
            fileStats[0].push_back(stats0);
            fileStats[1].push_back(stats1);
            fileNames.push_back(name);
            nFileDevices++;
        }
        //noname device, it's a compatibility dup
        if (name.length() < 4)
        {
            continue;
        }

        //named & crc1'd ... add to combo box
        if (crc1 != 0x00000000)
        {
            if (!dupCRC(crc1, deviceIndex))
            {
                crcs[0].push_back(crc0);
                crcs[1].push_back(crc1);
                stats[0].push_back(stats0);
                stats[1].push_back(stats1);
                dirty.push_back(FALSE);
                box->AddString(name.c_str());
                deviceIndex++;
            }
        }
    }

    fclose(in);

    nDevices = deviceIndex;
}

//
// do initial startup-y things
//
BOOL CHWVideoTweakerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
    //populate boxes & read devstats, etc.
    ReinitTables(TRUE);

    //initial settings
    OnSelchangeCombo1();
    OnSelchangeList2();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

//
// handle the fact that our "about" menu item is in the system menu
//
void CHWVideoTweakerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CHWVideoTweakerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHWVideoTweakerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//
// callback for DirectDraw device enumeration & crc generation
//
BOOL WINAPI hwdDirectDraw_cb(GUID FAR* lpGUID, LPSTR desc, LPSTR name, LPVOID context)
{
    LPDIRECTDRAW ddraw1;
    LPDIRECTDRAW4 ddraw4;
    LPDIRECT3D3 d3dObject;
    DDDEVICEIDENTIFIER dddi;

    if (FAILED(DirectDrawCreate(lpGUID, &ddraw1, NULL)))
    {
        return D3DENUMRET_OK;
    }
    if (FAILED(ddraw1->QueryInterface(IID_IDirectDraw4, (void**)&ddraw4)))
    {
        ddraw1->Release();
        return D3DENUMRET_OK;
    }
    if (FAILED(ddraw4->QueryInterface(IID_IDirect3D3, (void**)&d3dObject)))
    {
        ddraw4->Release();
        ddraw1->Release();
        return D3DENUMRET_OK;
    }

    if (FAILED(ddraw4->GetDeviceIdentifier(&dddi, 0)))
    {
        ddraw4->Release();
        ddraw1->Release();
        return D3DENUMRET_OK;
    }

    //generate the CRC
    memset(&dat, 0, sizeof(hwDat));
    dat.vendor  = dddi.dwVendorId;
    dat.product = 4;//HIWORD(dddi.liDriverVersion.HighPart);
    dat.device  = dddi.dwDeviceId;
    if (devCRC == 0)
    {
        devCRC = (DWORD)crc32Compute((BYTE*)&dat, sizeof(hwDat));
    }
    else
    {
        devCRC1 = (DWORD)crc32Compute((BYTE*)&dat, sizeof(hwDat));
    }

    return D3DENUMRET_OK;
}

//
// find user's video card in our vidcard combo box
//
int CHWVideoTweakerDlg::FindVideoCardIndex(void)
{
    int index;

    //enumerate available devices
    devCRC = devCRC1 = 0;
    (void)DirectDrawEnumerate(hwdDirectDraw_cb, NULL);

    if (devCRC == 0)
    {
        //no devices were enumerated
        return -1;
    }

    //search thru available devices for matching CRC
    for (index = 0; index < nDevices; index++)
    {
        if (crcs[1][index] == devCRC)
        {
            return index;
        }
    }
    return -1;
}

//
// repopulates combo & list boxes w/ device & feature info.
// called at startup or on Restore Defaults
//
void CHWVideoTweakerDlg::ReinitTables(BOOL enumerate)
{
    int cIndex, vIndex, fIndex;

    //empty out our vectors, etc
    for (int index = 0; index < 2; index++)
    {
        fileCrcs[index].erase(fileCrcs[index].begin(), fileCrcs[index].end());
        fileStats[index].erase(fileStats[index].begin(), fileStats[index].end());
        crcs[index].erase(crcs[index].begin(), crcs[index].end());
        stats[index].erase(stats[index].begin(), stats[index].end());
    }
    dirty.erase(dirty.begin(), dirty.end());
    devstatsPath.erase(devstatsPath.begin(), devstatsPath.end());
    devstatsBackupPath.erase(devstatsBackupPath.begin(), devstatsBackupPath.end());
    
    GET_COMBOBOX(pComboBox);
    //get current index
    cIndex = pComboBox->GetCurSel();
    //clear contents
    pComboBox->ResetContent();
    //populate w/ devstats.dat
    addDevstatsDevices(pComboBox);
    //obtain video card index
    vIndex = FindVideoCardIndex();
    //select something
    if (enumerate && vIndex != -1)
    {
        //enumeration was successful, select
        pComboBox->SetCurSel(vIndex);
    }
    else if (cIndex != -1)
    {
        //previous selection was valid
        pComboBox->SetCurSel(cIndex);
    }
    else
    {
        //default to the first card
        pComboBox->SetCurSel(0);
    }

    GET_LISTBOX(pFeatureBox);
    //get current index
    fIndex = pFeatureBox->GetCurSel();
    //clear contents
    pFeatureBox->ResetContent();
    //populate w/ features
    populateFeatureBox(pFeatureBox);
    //select something
    if (fIndex == -1)
    {
        pFeatureBox->SetCurSel(0);
    }
    else
    {
        pFeatureBox->SetCurSel(fIndex);
    }

    //initial settings
    OnSelchangeCombo1();
    OnSelchangeList2();
}

//
// update feature list & radio buttons, etc. when vidcard selection changes
//
void CHWVideoTweakerDlg::OnSelchangeCombo1() 
{
    //selected a new card
    BOOL state;
    GET_COMBOBOX(pComboBox);
    GET_LISTBOX(pListBox);
    int index  = pComboBox->GetCurSel();
    int lIndex = pListBox->GetCurSel();
    if (index == -1)
    {
        crc0 = crc1 = 0;
        stats0 = stats1 = 0;
        return;
    }

    //set stats & crc appropriately
    crc0 = crcs[0][index];
    crc1 = crcs[1][index];
    stats0 = stats[0][index];
    stats1 = stats[1][index];

    //prepend asterisks for enabled features
    for (index = 0; devstatsList[index].bit != 0; index++)
    {
        state = TRUE;
        if (devstatsList[index].whichFlag == 0)
        {
            if (stats0 & devstatsList[index].bit)
            {
                state = FALSE;
            }
        }
        else
        {
            if (stats1 & devstatsList[index].bit)
            {
                state = FALSE;
            }
        }
        updateFeatureString(pListBox, index, state);
    }
    if (lIndex == -1)
    {
        pListBox->SetCurSel(0);
    }
    else
    {
        pListBox->SetCurSel(lIndex);
    }

    //make radio buttons agree w/ current selection
    OnSelchangeList2();
}

//
// update radio buttons, etc. when feature selection changes
//
void CHWVideoTweakerDlg::OnSelchangeList2() 
{
    //have selected a different feature, make radio buttons agree w/ stats bits
    CButton* button0;
    CButton* button1;
    int index, whichFlag;
    DWORD bit;

    GET_LISTBOX(pListBox);
    button0 = (CButton*)GetDlgItem(IDC_RADIO1);
    button1 = (CButton*)GetDlgItem(IDC_RADIO2);

    index = pListBox->GetCurSel();
    if (index == -1)
    {
        //no selection
        return;
    }

    bit = devstatsList[index].bit;
    whichFlag = devstatsList[index].whichFlag;

    //match enable / disable feature radio buttons to stats state
    if (whichFlag == 0)
    {
        if (!(stats0 & bit))
        {
            button0->SetCheck(1);
            button1->SetCheck(0);
        }
        else
        {
            button0->SetCheck(0);
            button1->SetCheck(1);
        }
    }
    else
    {
        if (!(stats1 & bit))
        {
            button0->SetCheck(1);
            button1->SetCheck(0);
        }
        else
        {
            button0->SetCheck(0);
            button1->SetCheck(1);
        }
    }
}

//
// prepend '*' for disabled features in the feature list box.
// handles setting list box string & resetting selection
//
void CHWVideoTweakerDlg::updateFeatureString(CListBox* box, int index, BOOL state)
{
    char str[256];
    
    box->DeleteString(index);
    if (!state)
    {
        str[0] = '*';
        str[1] = ' ';
        str[2] = '\0';
    }
    else
    {
        str[0] = '\0';
    }
    strcat(str, devstatsList[index].name);
    box->InsertString(index, str);
    box->SetCurSel(index);
}

//
// set modification flag for a device as features are toggled
//
void CHWVideoTweakerDlg::setDirtyFlag(void)
{
    GET_COMBOBOX(pComboBox);
    int index = pComboBox->GetCurSel();
    if (index != -1)
    {
        dirty[index] = TRUE;
    }
}

//
// set class copies of stats flags, as well as device master flags
//
void CHWVideoTweakerDlg::setStatsBit(int whichStats, DWORD bit, BOOL state)
{
    GET_COMBOBOX(pComboBox);
    int index = pComboBox->GetCurSel();
    //set class convenience copies
    if (whichStats == 0)
    {
        if (state)
        {
            stats0 |= bit;
        }
        else
        {
            stats0 &= ~bit;
        }
    }
    else
    {
        if (state)
        {
            stats1 |= bit;
        }
        else
        {
            stats1 &= ~bit;
        }
    }

    //set device bits
    if (index == -1)
    {
        return;
    }
    stats[0][index] = stats0;
    stats[1][index] = stats1;
}

//
// "enable feature" radio button
//
void CHWVideoTweakerDlg::OnRadio1() 
{
    //we're clearing a bit
    GET_LISTBOX(pListBox);
    int index = pListBox->GetCurSel();
    if (index != -1)
    {
        //update string
        updateFeatureString(pListBox, index, TRUE);
    }

    setStatsBit(devstatsList[index].whichFlag,
                devstatsList[index].bit,
                FALSE);

    setDirtyFlag();
}

//
// "disable feature" radio button
//
void CHWVideoTweakerDlg::OnRadio2() 
{
    //we're setting a bit
    GET_LISTBOX(pListBox);
    int index = pListBox->GetCurSel();
    if (index != -1)
    {
        //update string
        updateFeatureString(pListBox, index, FALSE);
    }

    setStatsBit(devstatsList[index].whichFlag,
                devstatsList[index].bit,
                TRUE);

    setDirtyFlag();
}

//
// restore default devstats.dat by copying devstats.bak onto it
//
void CHWVideoTweakerDlg::OnDefault() 
{
    //copy devstats.bak -> devstats.dat
    if (!copyFile(devstatsPath, devstatsBackupPath, TRUE))
    {
        MessageBox("couldn't restore devstats.dat", "Couldn't restore", MB_ICONERROR);
    }

    //reload dev info
    ReinitTables(TRUE);
}

//
// scan thru devstatsList and return the comment field from
// given stats bit (stats[01] & bitmask), or NULL
//
char* findComent(int whichFlag, DWORD mask)
{
    int index;

    for (index = 0; devstatsList[index].bit != 0; index++)
    {
        if (devstatsList[index].whichFlag == whichFlag &&
            devstatsList[index].bit == mask)
        {
            return devstatsList[index].comment;
        }
    }

    return NULL;
}

//
// write devstats.dat, but only if something was actually updated.
// rather comments are generated, too, because devstats was always
// intended to be human-editable
//
void CHWVideoTweakerDlg::outputDevstats(void)
{
    int   index, fIndex, bitIndex, len;
    int   isDirty, modeDisabling;
    char  buf[256];
    DWORD bit;

    //check to see if anything was updated
    for (index = 0, isDirty = 0; index < nDevices; index++)
    {
        if (dirty[index])
        {
            isDirty++;
        }
    }

    if (!isDirty)
    {
        //nothing updated, don't bother writing
        return;
    }

    //go ahead with output

    //make fileStats agree w/ stats
    for (index = 0; index < nDevices; index++)
    {
        if (dirty[index])
        {
            //scan for other entries of same device
            for (fIndex = 0; fIndex < nFileDevices; fIndex++)
            {
                if (fileCrcs[1][fIndex] == crcs[1][index])
                {
                    //crc1 match
                    fileStats[0][fIndex] = stats[0][index];
                    fileStats[1][fIndex] = stats[1][index];
                }
                else if (fileCrcs[0][fIndex] == crcs[0][index])
                {
                    //crc0 match
                    fileStats[0][fIndex] = stats[0][index];
                    fileStats[1][fIndex] = stats[1][index];
                }
            }
        }
    }

    //
    //output devstats.dat
    //
    FILE* out = fopen(devstatsPath.c_str(), "wt");
    if (out == NULL)
    {
        MessageBox("cannot write devstats.dat", "Can't Open", MB_ICONERROR);
        return;
    }
    
    //devstats.dat file version
    fprintf(out, ";%s\n", TWEAKERVERSIONSTRING);

    //header comments
    isDirty = 0;
    for (fIndex = 0; fIndex < 2; fIndex++)
    {
        for (bitIndex = 0; bitIndex < 32; bitIndex++)
        {
            bit = 1 << bitIndex;
            for (index = 0; devstatsList[index].bit != 0; index++)
            {
                if (devstatsList[index].whichFlag == fIndex &&
                    devstatsList[index].bit == bit)
                {
                    char* comment = devstatsList[index].comment;
                    if (comment != NULL && comment[0] == '!')
                    {
                        comment++;
                        if (!isDirty)
                        {
                            //print a nice separator before display mode bits
                            fprintf(out, ";...\n");
                            isDirty = 1;
                        }
                    }
                    fprintf(out, ";bit %02d (%08X) %s [%s]\n",
                            bitIndex + 1, bit,
                            devstatsList[index].name,
                            comment);
                    break;
                }
            }
        }

        //separator
        fprintf(out, ";...\n");
    }

    //devices
    vec_string::iterator si = fileNames.begin();
    for (index = 0; index < nFileDevices; index++, ++si)
    {
        //device crcs / stats / name
        sprintf(buf, "%08X %08X %08X %08X %s",
                fileCrcs[0][index],
                fileStats[0][index],
                fileCrcs[1][index],
                fileStats[1][index],
                (*si).c_str());
        fprintf(out, "%s", buf);

        //comments
        if (fileStats[0][index] != 0x00000000 || fileStats[1][index] != 0x00000000)
        {
            len = strlen(buf);
            if (len < 58)
            {
                for (fIndex = 0; fIndex < (58 - len); fIndex++)
                {
                    fputc(' ', out);
                }
            }
            
            fprintf(out, " ;");
            isDirty = 0;
            modeDisabling = 0;
            strncpy(buf, "display mode disabling", 255);
            
            for (fIndex = 0; fIndex < 2; fIndex++)
            {
                for (bit = 1; bit != 0; bit <<= 1)
                {
                    if (fileStats[fIndex][index] & bit)
                    {
                        char* comment = findComent(fIndex, bit);
                        if (comment != NULL)
                        {
                            if (comment[0] == '!')
                            {
                                //it's a display mode bit
                                if (modeDisabling)
                                {
                                    //already printed our comment once
                                    isDirty++;
                                    continue;
                                }
                                else
                                {
                                    //generic comment for mode disabling
                                    comment = buf;
                                    modeDisabling = 1;
                                }
                            }
                            if (isDirty)
                            {
                                //multiple comments ...
                                fprintf(out, ", %s", comment);
                            }
                            else
                            {
                                //first comment
                                fprintf(out, " %s", comment);
                            }
                            isDirty++;
                        }
                    }
                }
                
                //this will print a '.' after stats0 if stats1 isn't empty
                if (isDirty &&
                    (fIndex == 0) &&
                    (fileStats[1][index] != 0x00000000))
                {
                    fprintf(out, ".");
                    isDirty = 0;
                }
            }
        }

        //newline
        fprintf(out, "\n");
    }

    fclose(out);
}

//
// "ok" button handler: save devstats.dat, then exit
//
void CHWVideoTweakerDlg::OnOK() 
{
    outputDevstats();
	
	CDialog::OnOK();
}

//
// popup a message box w/ device crc(s)
//
void CHWVideoTweakerDlg::OnCrc() 
{
    char buf[64];
    if (devCRC == 0)
    {
        //no crc found, don't bother with the message box
        return;
    }
    if (devCRC1 == 0)
    {
        sprintf(buf, "%08X", devCRC);
    }
    else
    {
        sprintf(buf, "%08X %08X", devCRC, devCRC1);
    }
    MessageBox(buf, "CRC", MB_ICONINFORMATION);
}
