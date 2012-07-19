VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmOptions 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "MissionMan Options"
   ClientHeight    =   5844
   ClientLeft      =   48
   ClientTop       =   336
   ClientWidth     =   8244
   Icon            =   "frmOptions.frx":0000
   MaxButton       =   0   'False
   ScaleHeight     =   5844
   ScaleWidth      =   8244
   StartUpPosition =   1  'CenterOwner
   Tag             =   "Options"
   Begin VB.Frame frmTools 
      Caption         =   "Tools"
      Height          =   1515
      Left            =   6240
      TabIndex        =   72
      Top             =   120
      Width           =   1875
      Begin VB.CheckBox Check1 
         Caption         =   "Free View Scale"
         Height          =   195
         Left            =   120
         TabIndex        =   73
         Top             =   1140
         Visible         =   0   'False
         Width           =   1635
      End
      Begin VB.CheckBox chkScale 
         Caption         =   "Free Object Scale"
         Height          =   195
         Left            =   120
         TabIndex        =   8
         Top             =   840
         Width           =   1635
      End
      Begin VB.CheckBox chkStat 
         Caption         =   "Status Bar"
         Height          =   195
         Left            =   120
         TabIndex        =   7
         Top             =   540
         Width           =   1635
      End
      Begin VB.CheckBox chkTool 
         Caption         =   "Toolbar"
         Height          =   195
         Left            =   120
         TabIndex        =   6
         Top             =   240
         Width           =   1635
      End
   End
   Begin VB.Frame fmeCurs 
      Caption         =   "Cursor"
      Height          =   675
      Left            =   6240
      TabIndex        =   69
      Top             =   3660
      Width           =   1875
      Begin VB.ComboBox cmbCol 
         Height          =   315
         Index           =   6
         ItemData        =   "frmOptions.frx":030A
         Left            =   840
         List            =   "frmOptions.frx":0311
         Style           =   2  'Dropdown List
         TabIndex        =   30
         Top             =   240
         Width           =   915
      End
      Begin VB.Label lblCol6 
         Caption         =   "Color:"
         Height          =   195
         Left            =   120
         TabIndex        =   70
         Top             =   300
         Width           =   615
      End
   End
   Begin VB.CommandButton cmdReload 
      Caption         =   "&Reload"
      Height          =   375
      Left            =   120
      TabIndex        =   31
      Tag             =   "OK"
      Top             =   5340
      Width           =   1095
   End
   Begin VB.Frame fmeBand 
      Caption         =   "Bandbox"
      Height          =   675
      Left            =   4200
      TabIndex        =   63
      Top             =   3660
      Width           =   1875
      Begin VB.ComboBox cmbCol 
         Height          =   315
         Index           =   5
         ItemData        =   "frmOptions.frx":0318
         Left            =   840
         List            =   "frmOptions.frx":031F
         Style           =   2  'Dropdown List
         TabIndex        =   28
         Top             =   240
         Width           =   915
      End
      Begin VB.Label lblCol5 
         Caption         =   "Color:"
         Height          =   195
         Left            =   120
         TabIndex        =   64
         Top             =   300
         Width           =   615
      End
   End
   Begin VB.Frame fmeSel 
      Caption         =   "Selection"
      Height          =   675
      Left            =   4200
      TabIndex        =   61
      Top             =   4500
      Width           =   1875
      Begin VB.ComboBox cmbCol 
         Height          =   315
         Index           =   4
         ItemData        =   "frmOptions.frx":0326
         Left            =   840
         List            =   "frmOptions.frx":032D
         Style           =   2  'Dropdown List
         TabIndex        =   29
         Top             =   240
         Width           =   915
      End
      Begin VB.Label lblCol4 
         Caption         =   "Color:"
         Height          =   195
         Left            =   120
         TabIndex        =   62
         Top             =   300
         Width           =   615
      End
   End
   Begin VB.CommandButton cmdSave 
      Caption         =   "&Save"
      Height          =   375
      Left            =   1320
      TabIndex        =   32
      Tag             =   "OK"
      Top             =   5340
      Width           =   1095
   End
   Begin VB.Frame fmePaths 
      Caption         =   "Paths"
      Height          =   1515
      Left            =   120
      TabIndex        =   58
      Top             =   120
      Width           =   5955
      Begin VB.TextBox txtData 
         Enabled         =   0   'False
         Height          =   285
         Left            =   1020
         OLEDropMode     =   1  'Manual
         TabIndex        =   3
         Top             =   660
         Width           =   4815
      End
      Begin VB.CommandButton cmdData 
         Caption         =   "4"
         Enabled         =   0   'False
         BeginProperty Font 
            Name            =   "Webdings"
            Size            =   8.4
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   840
         TabIndex        =   2
         Tag             =   "Apply"
         ToolTipText     =   "Browse"
         Top             =   660
         Width           =   195
      End
      Begin VB.CommandButton cmdObjects 
         Caption         =   "4"
         Enabled         =   0   'False
         BeginProperty Font 
            Name            =   "Webdings"
            Size            =   8.4
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   840
         TabIndex        =   4
         Tag             =   "Apply"
         ToolTipText     =   "Browse"
         Top             =   1080
         Width           =   195
      End
      Begin VB.TextBox txtObjects 
         Enabled         =   0   'False
         Height          =   285
         Left            =   1020
         OLEDropMode     =   1  'Manual
         TabIndex        =   5
         Top             =   1080
         Width           =   4815
      End
      Begin VB.CommandButton cmdWork 
         Caption         =   "4"
         Enabled         =   0   'False
         BeginProperty Font 
            Name            =   "Webdings"
            Size            =   8.4
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   840
         TabIndex        =   0
         Tag             =   "Apply"
         ToolTipText     =   "Browse"
         Top             =   240
         Width           =   195
      End
      Begin VB.TextBox txtWork 
         Enabled         =   0   'False
         Height          =   285
         Left            =   1020
         OLEDropMode     =   1  'Manual
         TabIndex        =   1
         Top             =   240
         Width           =   4815
      End
      Begin VB.Label lblData 
         Caption         =   "Data:"
         Height          =   195
         Left            =   120
         TabIndex        =   71
         Top             =   720
         Width           =   615
      End
      Begin VB.Label lblObjects 
         Caption         =   "Object:"
         Height          =   195
         Left            =   120
         TabIndex        =   60
         Top             =   1140
         Width           =   615
      End
      Begin VB.Label lblWork 
         Caption         =   "Work:"
         Height          =   195
         Left            =   120
         TabIndex        =   59
         Top             =   300
         Width           =   615
      End
   End
   Begin VB.Frame fmeCamera 
      Caption         =   "Camera"
      Height          =   1815
      Left            =   120
      TabIndex        =   49
      Top             =   3360
      Width           =   3915
      Begin VB.CheckBox chkCam 
         Caption         =   "Free Camera"
         Height          =   195
         Left            =   120
         TabIndex        =   26
         Top             =   1440
         Width           =   1635
      End
      Begin VB.TextBox txtFocusX 
         Height          =   285
         Left            =   840
         TabIndex        =   23
         Top             =   960
         Width           =   915
      End
      Begin VB.TextBox txtFocusY 
         Height          =   285
         Left            =   1860
         TabIndex        =   24
         Top             =   960
         Width           =   915
      End
      Begin VB.TextBox txtFocusZ 
         Height          =   285
         Left            =   2880
         TabIndex        =   25
         Top             =   960
         Width           =   915
      End
      Begin VB.TextBox txtEyeZ 
         Height          =   285
         Left            =   2880
         TabIndex        =   22
         Top             =   540
         Width           =   915
      End
      Begin VB.TextBox txtEyeY 
         Height          =   285
         Left            =   1860
         TabIndex        =   21
         Top             =   540
         Width           =   915
      End
      Begin VB.TextBox txtEyeX 
         Height          =   285
         Left            =   840
         TabIndex        =   20
         Top             =   540
         Width           =   915
      End
      Begin VB.ComboBox cmbCol 
         Height          =   315
         Index           =   2
         ItemData        =   "frmOptions.frx":0334
         Left            =   2880
         List            =   "frmOptions.frx":033F
         Style           =   2  'Dropdown List
         TabIndex        =   27
         Top             =   1380
         Width           =   915
      End
      Begin VB.Label lblZC 
         Alignment       =   2  'Center
         Caption         =   "z"
         Height          =   195
         Left            =   2880
         TabIndex        =   55
         Top             =   240
         Width           =   915
      End
      Begin VB.Label lblYC 
         Alignment       =   2  'Center
         Caption         =   "y"
         Height          =   195
         Left            =   1860
         TabIndex        =   54
         Top             =   240
         Width           =   915
      End
      Begin VB.Label lblXC 
         Alignment       =   2  'Center
         Caption         =   "x"
         Height          =   195
         Left            =   840
         TabIndex        =   53
         Top             =   240
         Width           =   915
      End
      Begin VB.Label lblFocus 
         Caption         =   "Focus:"
         Height          =   195
         Left            =   120
         TabIndex        =   52
         Top             =   1020
         Width           =   615
      End
      Begin VB.Label lblEye 
         Caption         =   "Eye:"
         Height          =   195
         Left            =   120
         TabIndex        =   51
         Top             =   600
         Width           =   615
      End
      Begin VB.Label lblCol2 
         Caption         =   "Color:"
         Height          =   195
         Left            =   2160
         TabIndex        =   50
         Top             =   1440
         Width           =   615
      End
   End
   Begin VB.Frame fmeViews 
      Caption         =   "Views"
      Height          =   1395
      Left            =   120
      TabIndex        =   48
      Top             =   1800
      Width           =   3915
      Begin VB.TextBox txtOffsetX 
         Height          =   285
         Left            =   840
         TabIndex        =   9
         Top             =   540
         Width           =   915
      End
      Begin VB.TextBox txtOffsetY 
         Height          =   285
         Left            =   1860
         TabIndex        =   10
         Top             =   540
         Width           =   915
      End
      Begin VB.TextBox txtOffsetZ 
         Height          =   285
         Left            =   2880
         TabIndex        =   11
         Top             =   540
         Width           =   915
      End
      Begin VB.ComboBox cmbCol 
         Height          =   315
         Index           =   0
         ItemData        =   "frmOptions.frx":034E
         Left            =   2880
         List            =   "frmOptions.frx":0355
         Style           =   2  'Dropdown List
         TabIndex        =   13
         Top             =   960
         Width           =   915
      End
      Begin VB.TextBox txtScale 
         Height          =   285
         Left            =   840
         TabIndex        =   12
         Top             =   960
         Width           =   915
      End
      Begin VB.Label lblOffset 
         Caption         =   "Offset:"
         Height          =   195
         Left            =   120
         TabIndex        =   68
         Top             =   600
         Width           =   615
      End
      Begin VB.Label lblXD 
         Alignment       =   2  'Center
         Caption         =   "x"
         Height          =   195
         Left            =   840
         TabIndex        =   67
         Top             =   240
         Width           =   915
      End
      Begin VB.Label lblYD 
         Alignment       =   2  'Center
         Caption         =   "y"
         Height          =   195
         Left            =   1860
         TabIndex        =   66
         Top             =   240
         Width           =   915
      End
      Begin VB.Label lblZD 
         Alignment       =   2  'Center
         Caption         =   "z"
         Height          =   195
         Left            =   2880
         TabIndex        =   65
         Top             =   240
         Width           =   915
      End
      Begin VB.Label lblCol0 
         Caption         =   "Color:"
         Height          =   195
         Left            =   2160
         TabIndex        =   57
         Top             =   1020
         Width           =   615
      End
      Begin VB.Label lblScale 
         Caption         =   "Scale:"
         Height          =   195
         Left            =   120
         TabIndex        =   56
         Top             =   1020
         Width           =   615
      End
   End
   Begin VB.Frame fmeAngle 
      Caption         =   "Angle"
      Height          =   1395
      Left            =   6240
      TabIndex        =   45
      Top             =   1800
      Width           =   1875
      Begin VB.CheckBox chkAngle 
         Caption         =   "Snap Angle"
         Height          =   195
         Left            =   120
         TabIndex        =   17
         Top             =   240
         Width           =   1635
      End
      Begin VB.TextBox txtAngle 
         Height          =   285
         Left            =   840
         TabIndex        =   18
         Top             =   540
         Width           =   915
      End
      Begin VB.ComboBox cmbCol 
         Height          =   315
         Index           =   3
         ItemData        =   "frmOptions.frx":035C
         Left            =   840
         List            =   "frmOptions.frx":0367
         Style           =   2  'Dropdown List
         TabIndex        =   19
         Top             =   960
         Width           =   915
      End
      Begin VB.Label lblAngle 
         Caption         =   "Angle:"
         Height          =   195
         Left            =   120
         TabIndex        =   47
         Top             =   600
         Width           =   615
      End
      Begin VB.Label lblCol3 
         Caption         =   "Color:"
         Height          =   195
         Left            =   120
         TabIndex        =   46
         Top             =   1020
         Width           =   615
      End
   End
   Begin VB.Frame fmeGrid 
      Caption         =   "Grid"
      Height          =   1395
      Left            =   4200
      TabIndex        =   42
      Top             =   1800
      Width           =   1875
      Begin VB.ComboBox cmbCol 
         Height          =   315
         Index           =   1
         ItemData        =   "frmOptions.frx":0376
         Left            =   840
         List            =   "frmOptions.frx":0381
         Style           =   2  'Dropdown List
         TabIndex        =   16
         Top             =   960
         Width           =   915
      End
      Begin VB.TextBox txtSize 
         Height          =   285
         Left            =   840
         TabIndex        =   15
         Top             =   540
         Width           =   915
      End
      Begin VB.CheckBox chkGrid 
         Caption         =   "Snap Grid"
         Height          =   195
         Left            =   120
         TabIndex        =   14
         Top             =   240
         Width           =   1635
      End
      Begin VB.Label lbCol1 
         Caption         =   "Color:"
         Height          =   195
         Left            =   120
         TabIndex        =   44
         Top             =   1020
         Width           =   615
      End
      Begin VB.Label lblSize 
         Caption         =   "Size:"
         Height          =   195
         Left            =   120
         TabIndex        =   43
         Top             =   600
         Width           =   615
      End
   End
   Begin VB.CommandButton cmdOK 
      Caption         =   "&OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   4620
      TabIndex        =   33
      Tag             =   "OK"
      Top             =   5340
      Width           =   1095
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   5820
      TabIndex        =   34
      Tag             =   "Cancel"
      Top             =   5340
      Width           =   1095
   End
   Begin VB.CommandButton cmdApply 
      Caption         =   "&Apply"
      Height          =   375
      Left            =   7020
      TabIndex        =   35
      Tag             =   "Apply"
      Top             =   5340
      Width           =   1095
   End
   Begin VB.PictureBox picOptions 
      BorderStyle     =   0  'None
      Height          =   3780
      Index           =   3
      Left            =   -20000
      ScaleHeight     =   3840.968
      ScaleMode       =   0  'User
      ScaleWidth      =   5745.64
      TabIndex        =   37
      TabStop         =   0   'False
      Top             =   480
      Width           =   5685
      Begin VB.Frame fraSample4 
         Caption         =   "Sample 4"
         Height          =   2022
         Left            =   505
         TabIndex        =   41
         Tag             =   "Sample 4"
         Top             =   502
         Width           =   2033
      End
   End
   Begin VB.PictureBox picOptions 
      BorderStyle     =   0  'None
      Height          =   3780
      Index           =   2
      Left            =   -20000
      ScaleHeight     =   3840.968
      ScaleMode       =   0  'User
      ScaleWidth      =   5745.64
      TabIndex        =   39
      TabStop         =   0   'False
      Top             =   480
      Width           =   5685
      Begin VB.Frame fraSample3 
         Caption         =   "Sample 3"
         Height          =   2022
         Left            =   406
         TabIndex        =   40
         Tag             =   "Sample 3"
         Top             =   403
         Width           =   2033
      End
   End
   Begin VB.PictureBox picOptions 
      BorderStyle     =   0  'None
      Height          =   3780
      Index           =   1
      Left            =   -20000
      ScaleHeight     =   3840.968
      ScaleMode       =   0  'User
      ScaleWidth      =   5745.64
      TabIndex        =   36
      TabStop         =   0   'False
      Top             =   480
      Width           =   5685
      Begin VB.Frame fraSample2 
         Caption         =   "Sample 2"
         Height          =   2022
         Left            =   307
         TabIndex        =   38
         Tag             =   "Sample 2"
         Top             =   305
         Width           =   2033
      End
   End
   Begin MSComDlg.CommonDialog cdBrowse 
      Left            =   6240
      Top             =   4500
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      CancelError     =   -1  'True
      Filter          =   "Folders|Dir"
   End
   Begin MSComDlg.CommonDialog cdColor 
      Left            =   6840
      Top             =   4500
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      CancelError     =   -1  'True
   End
End
Attribute VB_Name = "frmOptions"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim bCombo As Boolean

Sub GetOptions()
    Dim n As Integer
    Dim nCount As Integer
    
    Dim nPos As Long
    
    Dim sVal As String
    Dim sList As String

    'Get work dir
    Call misGetVal(MIS_SEC_COM, MIS_KEY_WORKD, sWDir, MIS_MOD_INI)
    sWDir = TruncStr(sWDir)
    If sWDir = "" Then sWDir = MIS_DIR_WORK
    sWDir = DecodeEnv(sWDir)
        
    'Get data dir
    Call misGetVal(MIS_SEC_COM, MIS_KEY_DATAD, sDDir, MIS_MOD_INI)
    sDDir = TruncStr(sDDir)
    If sDDir = "" Then sDDir = MIS_DIR_DATA
    sDDir = DecodeEnv(sDDir)
    
    'Get object dir
    Call misGetVal(MIS_SEC_COM, MIS_KEY_OBJD, sODir, MIS_MOD_INI)
    sODir = TruncStr(sODir)
    If sODir = "" Then sODir = MIS_DIR_OBJ
    sODir = DecodeEnv(sODir)
    
    'Get toolbar flag
    bToolFlag = MIS_FLAG_TOOL
    Call misGetVal(MIS_SEC_COM, MIS_KEY_TOOLB, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then bToolFlag = Val(sVal)
    
    'Get status bar flag
    bStatFlag = MIS_FLAG_STAT
    Call misGetVal(MIS_SEC_COM, MIS_KEY_STATB, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then bStatFlag = Val(sVal)
    
    'Reset count
    nCount = 0
    
    'Get offset
    Call misGetListByKey(MIS_SEC_GRAPH, MIS_KEY_VOFFSET, sList, nCount, MIS_MOD_INI)
    
    'Check count
    If nCount > 0 Then
        'Truncate list
        sList = TruncStr(sList)

        'Loop thru list
        For n = 0 To 2
            'Get position of | character in string
            nPos = InStr(sList, "|")
        
            'If possible, truncate string at | character
            If nPos > 0 Then
                'Set eye
                aOffset(n) = Val(Left(sList, nPos - 1))
                sList = Mid(sList, nPos + 1, Len(sList))
            Else
                'Set eye
                aOffset(n) = Val(sList)
            End If
        Next n
    End If
    
    'Get default scale
    fViewScale = MIS_REND_VSCALE
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_VSCALE, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then fViewScale = 100 / Val(sVal)

    'Set offset
    aOffset(0) = MIS_REND_VOX * fViewScale
    aOffset(1) = MIS_REND_VOY * fViewScale
    aOffset(2) = MIS_REND_VOZ * fViewScale
    
    'Get default color
    nViewCol = MIS_REND_VCOL
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_VCOL, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then nViewCol = Val(sVal)

    'Get camera flag
    bCamFlag = MIS_CAM_INDEP
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_CINDEP, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then bCamFlag = Val(sVal)
    
    'Set camera eye
    aEye(0) = MIS_CAM_EX
    aEye(1) = MIS_CAM_EY
    aEye(2) = MIS_CAM_EZ
    
    'Set camera focus
    aFocus(0) = MIS_CAM_FX
    aFocus(1) = MIS_CAM_FY
    aFocus(2) = MIS_CAM_FZ
    
    'Reset count
    nCount = 0
    
    'Get eye
    Call misGetListByKey(MIS_SEC_GRAPH, MIS_KEY_CEYE, sList, nCount, MIS_MOD_INI)
    
    'Check count
    If nCount > 0 Then
        'Truncate list
        sList = TruncStr(sList)

        'Loop thru list
        For n = 0 To 2
            'Get position of | character in string
            nPos = InStr(sList, "|")
        
            'If possible, truncate string at | character
            If nPos > 0 Then
                'Set eye
                aEye(n) = Val(Left(sList, nPos - 1))
                sList = Mid(sList, nPos + 1, Len(sList))
            Else
                'Set eye
                aEye(n) = Val(sList)
            End If
        Next n
    End If
    
    'Reset count
    nCount = 0
    
    'Get focus
    Call misGetListByKey(MIS_SEC_GRAPH, MIS_KEY_CFOCUS, sList, nCount, MIS_MOD_INI)
    
    'Check count
    If nCount > 0 Then
        'Truncate list
        sList = TruncStr(sList)

        'Loop thru list
        For n = 0 To 2
            'Get position of | character in string
            nPos = InStr(sList, "|")
        
            'If possible, truncate string at | character
            If nPos > 0 Then
                'Set focus
                aFocus(n) = Val(Left(sList, nPos - 1))
                sList = Mid(sList, nPos + 1, Len(sList))
            Else
                'Set focus
                aFocus(n) = Val(sList)
            End If
        Next n
    End If
    
    'Get camera color
    nCamCol = MIS_CAM_COL
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_CMCOL, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then nCamCol = Val(sVal)

    'Get grid flag
    bGridFlag = MIS_REND_GSNAP
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_GSNAP, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then bGridFlag = Val(sVal)
    
    'Get grid size
    fGridSize = MIS_REND_GSIZE
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_GSIZE, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then fGridSize = Val(sVal)
    
    'Get grid color
    nGridCol = MIS_REND_GCOL
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_GCOL, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then nGridCol = Val(sVal)
    
    'Get rotation flag
    bRotFlag = MIS_REND_RSNAP
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_RSNAP, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then bRotFlag = Val(sVal)
    
    'Get rotation size
    fRotAngle = MIS_REND_RANG
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_RANG, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then fRotAngle = Val(sVal)
    
    'Get rotation color
    nRotCol = MIS_REND_RCOL
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_RCOL, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then nRotCol = Val(sVal)
    
    'Get scaling flag
    bScaleFlag = MIS_REND_SINDEP
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_SINDEP, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then bScaleFlag = Val(sVal)
    
    'Get selection color
    nSelCol = MIS_REND_SCOL
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_SCOL, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then nSelCol = Val(sVal)
    
    'Get band box color
    nBandCol = MIS_REND_BCOL
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_BCOL, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then nBandCol = Val(sVal)

    'Get cursor color
    nCursCol = MIS_REND_CCOL
    Call misGetVal(MIS_SEC_GRAPH, MIS_KEY_CSCOL, sVal, MIS_MOD_INI)
    sVal = TruncStr(sVal)
    If sVal <> "" Then nCursCol = Val(sVal)
End Sub

Sub PutOptions()
    Dim n As Integer
    Dim nCount As Integer
    
    Dim nPos As Long
    Dim nMask As Long
   
    Dim sVal As String
    Dim sList As String

    'Get bit mask
    Call misGetVal(MIS_SEC_COM, MIS_KEY_BITM, sVal, MIS_MOD_CFG)
    sVal = TruncStr(sVal)
    If sVal <> "" Then
        'Set mask
        nMask = Val(sVal)
    
        'Check bit mask
        If (nMask And MIS_BIT_DEV) = MIS_BIT_DEV Then
            'Put work dir
            sWDir = EncodeEnv(sWDir)
            Call misPutVal(MIS_SEC_COM, MIS_KEY_WORKD, sWDir, MIS_MOD_INI)
            
            'Put data dir
            sDDir = EncodeEnv(sDDir)
            Call misPutVal(MIS_SEC_COM, MIS_KEY_DATAD, sDDir, MIS_MOD_INI)
            
            'Put object dir
            sODir = EncodeEnv(sODir)
            Call misPutVal(MIS_SEC_COM, MIS_KEY_OBJD, sODir, MIS_MOD_INI)
        End If
    End If
    
    'Put toolbar flag
    Call misPutVal(MIS_SEC_COM, MIS_KEY_TOOLB, Trim(Str(bToolFlag)), MIS_MOD_INI)
    
    'Put status bar flag
    Call misPutVal(MIS_SEC_COM, MIS_KEY_STATB, Trim(Str(bStatFlag)), MIS_MOD_INI)
    
    'Reset list
    sList = ""
    For n = 0 To 2
        'Append list
        sList = sList + "|" + Format(aOffset(n) / fViewScale, "0.0;-0.0")
    Next n
    
    'Put offset
    Call misPutListByKey(MIS_SEC_GRAPH, MIS_KEY_VOFFSET, sList, MIS_MOD_INI)
    
    'Put default scale
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_VSCALE, Trim(Str(100 / fViewScale)), MIS_MOD_INI)

    'Put default color
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_VCOL, Trim(Str(nViewCol)), MIS_MOD_INI)

    'Put camera flag
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_CINDEP, Trim(Str(bCamFlag)), MIS_MOD_INI)
    
    'Reset list
    sList = ""
    For n = 0 To 2
        'Append list
        sList = sList + "|" + Format(aEye(n), "0.0;-0.0")
    Next n
    
    'Put eye
    Call misPutListByKey(MIS_SEC_GRAPH, MIS_KEY_CEYE, sList, MIS_MOD_INI)
    
    'Reset list
    sList = ""
    For n = 0 To 2
        'Append list
        sList = sList + "|" + Format(aFocus(n), "0.0;-0.0")
    Next n
    
    'Put focus
    Call misPutListByKey(MIS_SEC_GRAPH, MIS_KEY_CFOCUS, sList, MIS_MOD_INI)
    
    'Put camera color
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_CMCOL, Trim(Str(nCamCol)), MIS_MOD_INI)

    'Put grid flag
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_GSNAP, Trim(Str(bGridFlag)), MIS_MOD_INI)
    
    'Put grid size
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_GSIZE, Format(fGridSize, "0.0;-0.0"), MIS_MOD_INI)
    
    'Put grid color
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_GCOL, Trim(Str(nGridCol)), MIS_MOD_INI)
    
    'Put rotation flag
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_RSNAP, Trim(Str(bRotFlag)), MIS_MOD_INI)
    
    'Put rotation size
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_RANG, Format(fRotAngle, "0.0;-0.0"), MIS_MOD_INI)
    
    'Put rotation color
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_RCOL, Trim(Str(nRotCol)), MIS_MOD_INI)
    
    'Put scaling flag
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_SINDEP, Trim(Str(bScaleFlag)), MIS_MOD_INI)
    
    'Put selection color
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_SCOL, Trim(Str(nSelCol)), MIS_MOD_INI)
    
    'Put band box color
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_BCOL, Trim(Str(nBandCol)), MIS_MOD_INI)

    'Put cursor color
    Call misPutVal(MIS_SEC_GRAPH, MIS_KEY_CSCOL, Trim(Str(nCursCol)), MIS_MOD_INI)
    
    'Clear options flag
    bOptFlag = False
End Sub

Sub ShowOptions()
    'Get work dir
    txtWork.Text = sWDir
    
    'Get data dir
    txtData.Text = sDDir
    
    'Get object dir
    txtObjects.Text = sODir
    
    'Get toolbar flag
    chkTool.Value = bToolFlag
    
    'Get status bar flag
    chkStat.Value = bStatFlag
    
    'Get offset
    txtOffsetX.Text = Format(aOffset(0) / fViewScale, "0;-#")
    txtOffsetY.Text = Format(aOffset(1) / fViewScale, "0;-#")
    txtOffsetZ.Text = Format(aOffset(2) / fViewScale, "0;-#")
    
    'Get default scale
    txtScale.Text = Format(100 / fViewScale, "0;-#")

    'Get default color
    Call SelCol(0, nViewCol)

    'Get camera flag
    chkCam.Value = bCamFlag
    
    'Get camera eye
    txtEyeX.Text = Format(aEye(0), "0;-#")
    txtEyeY.Text = Format(aEye(1), "0;-#")
    txtEyeZ.Text = Format(aEye(2), "0;-#")
    
    'Get camera focus
    txtFocusX.Text = Format(aFocus(0), "0;-#")
    txtFocusY.Text = Format(aFocus(1), "0;-#")
    txtFocusZ.Text = Format(aFocus(2), "0;-#")
    
    'Get camera color
    Call SelCol(2, nCamCol)

    'Get grid flag
    chkGrid.Value = bGridFlag
    
    'Get grid size
    txtSize.Text = Format(fGridSize, "0;-#")
    
    'Get grid color
    Call SelCol(1, nGridCol)
    
    'Get rotation flag
    chkAngle.Value = bRotFlag
    
    'Get rotation size
    txtAngle.Text = Format(fRotAngle, "0;-#")
    
    'Get rotation color
    Call SelCol(3, nRotCol)
    
    'Get scaling flag
    chkScale.Value = bScaleFlag
    
    'Get selection color
    Call SelCol(4, nSelCol)
    
    'Get band box color
    Call SelCol(5, nBandCol)
    
    'Get cursor color
    Call SelCol(6, nCursCol)
End Sub

Sub UpdateOptions()
    Dim nCount As Integer
    
    Dim nCol As Long
    Dim nObj As Long
    
    Dim sVal As String
    Dim sList As String

    'Get work dir
    sVal = txtWork.Text
    If sVal <> "" Then sWDir = sVal
    If Mid(sWDir, Len(sWDir)) <> "\" Then sWDir = sWDir + "\"
    
    'Get data dir
    sVal = txtData.Text
    If sVal <> "" Then sDDir = sVal
    If Mid(sDDir, Len(sDDir)) <> "\" Then sDDir = sDDir + "\"
    
    'Get object dir
    sVal = txtObjects.Text
    If sVal <> "" Then sODir = sVal
    If Mid(sODir, Len(sODir)) <> "\" Then sODir = sODir + "\"
    
    'Get toolbar flag
    bToolFlag = chkTool.Value
    
    'Get status bar flag
    bStatFlag = chkStat.Value
    
    'Get default scale
    fViewScale = 100 / Val(txtScale.Text)

    'Get offset
    aOffset(0) = Val(txtOffsetX.Text) * fViewScale
    aOffset(1) = Val(txtOffsetY.Text) * fViewScale
    aOffset(2) = Val(txtOffsetZ.Text) * fViewScale
   
    'Get default color
    nCol = nViewCol
    nViewCol = FindCol(0)
    
    'Check default color
    If nCol <> nSelCol Then
        'Get selection
        Call rendGetSel("o", nCount, sList)
        
        'Re-initialize renderer
        rendClean
        rendInit
        frmObjects.GetObjects
        
        'Check count
        If nCount > 0 Then
            'Truncate list
            sList = TruncStr(sList)
            
            'Get selection
            Call rendSetSel("o", sList)
        End If
    End If

    'Get camera flag
    bCamFlag = chkCam.Value
    
    'Get camera eye
    aEye(0) = Val(txtEyeX.Text)
    aEye(1) = Val(txtEyeY.Text)
    aEye(2) = Val(txtEyeZ.Text)
    
    'Get camera focus
    aFocus(0) = Val(txtFocusX.Text)
    aFocus(1) = Val(txtFocusY.Text)
    aFocus(2) = Val(txtFocusZ.Text)
    
    'Get camera color
    nCamCol = FindCol(2)

    'Get grid flag
    bGridFlag = chkGrid.Value
    
    'Get grid size
    fGridSize = Val(txtSize.Text)
    
    'Get grid color
    nGridCol = FindCol(1)
    
    'Get rotation flag
    bRotFlag = chkAngle.Value
    
    'Get rotation size
    fRotAngle = Val(txtAngle.Text)
    
    'Get rotation color
    nRotCol = FindCol(3)
    
    'Get scaling flag
    bScaleFlag = chkScale.Value
    
    'Get selection color
    nSelCol = FindCol(4)
    
    'Get band box color
    nBandCol = FindCol(5)
       
    'Get cursor color
    nCursCol = FindCol(6)
       
    'Get flags
    fMainForm.GetFlags
    
    'Set options flag
    bOptFlag = True

    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Refresh front view
    frmFront.SetSel (False)
    frmFront.SetGrid (False)
    frmFront.SetCursor (False)
    frmFront.SetScale (False)
    frmFront.SetView (False)
    frmFront.SetCamera (True)
    
    'Refresh side view
    frmSide.SetSel (False)
    frmSide.SetGrid (False)
    frmSide.SetCursor (False)
    frmSide.SetScale (False)
    frmSide.SetView (False)
    frmSide.SetCamera (True)
    
    'Refresh top view
    frmTop.SetSel (False)
    frmTop.SetGrid (False)
    frmTop.SetCursor (False)
    frmTop.SetScale (False)
    frmTop.SetView (False)
    frmTop.SetCamera (True)
    
    'Refresh camera view
    frmCamera.SetSel (False)
    frmCamera.SetCamera (True)
End Sub

Sub SelCol(ByVal nIndex As Integer, ByVal nCol As Long)
    'Set flag
    bCombo = True
    
    'Check color
    If nCol < 0 Then
        'Set color
        cmbCol(nIndex).ListIndex = 0
        cmbCol(nIndex).BackColor = vbWhite
        Exit Sub
    End If
    
    'Set color
    cmbCol(nIndex).ListIndex = cmbCol(nIndex).ListCount - 1
    cmbCol(nIndex).BackColor = nCol

    'Clear flag
    bCombo = False
End Sub

Function FindCol(ByVal nIndex As Integer) As Long
    Dim n As Integer
    
    'Check combo
    If cmbCol(nIndex).ListCount > 1 Then
        If cmbCol(nIndex).ListIndex = 0 Then
            'Set color
            FindCol = cmbCol(nIndex).ItemData(0)
            Exit Function
        End If
    End If
        
    'Set color
    FindCol = cmbCol(nIndex).BackColor
End Function

Private Sub cmbCol_Click(Index As Integer)
    If bCombo = True Then Exit Sub
    
    'Check combo
    If cmbCol(Index).ListCount > 1 Then
        'Check combo
        If cmbCol(Index).ListIndex = 0 Then
            cmbCol(Index).BackColor = vbWhite
            Exit Sub
        End If
    End If
    
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show open common dialog
    cdColor.Color = cmbCol(Index).BackColor
    cdColor.Flags = &H1
    cdColor.ShowColor
    
    'Set color
    cmbCol(Index).BackColor = cdColor.Color
    cmdApply.SetFocus
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub cmdApply_Click()
    'Update options
    UpdateOptions
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdData_Click()
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show save common dialog
    cdBrowse.DialogTitle = "Select Data Directory"
    cdBrowse.FileName = "Select Data Directory"
    cdBrowse.ShowSave
    txtData.Text = Left(cdBrowse.FileName, InStr(1, cdBrowse.FileName, cdBrowse.FileTitle, vbTextCompare) - 1)
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub cmdObjects_Click()
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show save common dialog
    cdBrowse.DialogTitle = "Select Object Directory"
    cdBrowse.FileName = "Select Object Directory"
    cdBrowse.ShowSave
    txtObjects.Text = Left(cdBrowse.FileName, InStr(1, cdBrowse.FileName, cdBrowse.FileTitle, vbTextCompare) - 1)
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub cmdOK_Click()
    'Update options
    UpdateOptions
    
    Unload Me
End Sub

Private Sub cmdReload_Click()
    'Get options
    GetOptions
    
    'Show options
    ShowOptions
End Sub

Private Sub cmdSave_Click()
    'Update options
    UpdateOptions
    
    'Put options
    PutOptions
End Sub

Private Sub cmdWork_Click()
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show save common dialog
    cdBrowse.DialogTitle = "Select Work Directory"
    cdBrowse.FileName = "Select Work Directory"
    cdBrowse.ShowSave
    txtWork.Text = Left(cdBrowse.FileName, InStr(1, cdBrowse.FileName, cdBrowse.FileTitle, vbTextCompare) - 1)
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub Form_Load()
    Dim nMask As Long

    Dim sVal As String

    'Disable directory selection
    cmdWork.Enabled = False
    txtWork.Enabled = False
    cmdData.Enabled = False
    txtData.Enabled = False
    cmdObjects.Enabled = False
    txtObjects.Enabled = False
    
    'Get bit mask
    Call misGetVal(MIS_SEC_COM, MIS_KEY_BITM, sVal, MIS_MOD_CFG)
    sVal = TruncStr(sVal)
    If sVal <> "" Then
        'Set mask
        nMask = Val(sVal)
    
        'Check bit mask
        If (nMask And MIS_BIT_DEV) = MIS_BIT_DEV Then
            'Enable directory selection
            cmdWork.Enabled = True
            txtWork.Enabled = True
            cmdData.Enabled = True
            txtData.Enabled = True
            cmdObjects.Enabled = True
            txtObjects.Enabled = True
        End If
    End If
    
    'Clear flag
    bCombo = False
    
    'Show options
    ShowOptions
End Sub

