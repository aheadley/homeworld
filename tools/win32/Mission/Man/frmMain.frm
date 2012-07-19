VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Object = "{6B7E6392-850A-101B-AFC0-4210102A8DA7}#1.3#0"; "Comctl32.ocx"
Begin VB.MDIForm frmMain 
   AutoShowChildren=   0   'False
   BackColor       =   &H8000000C&
   Caption         =   "MissionMan"
   ClientHeight    =   3225
   ClientLeft      =   165
   ClientTop       =   735
   ClientWidth     =   4680
   Icon            =   "frmMain.frx":0000
   ScrollBars      =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin ComctlLib.Toolbar tbToolBar 
      Align           =   1  'Align Top
      Height          =   420
      Left            =   0
      TabIndex        =   1
      Top             =   0
      Width           =   4680
      _ExtentX        =   8255
      _ExtentY        =   741
      ButtonWidth     =   635
      ButtonHeight    =   572
      Appearance      =   1
      ImageList       =   "ilToolBar"
      _Version        =   327682
      BeginProperty Buttons {0713E452-850A-101B-AFC0-4210102A8DA7} 
         NumButtons      =   6
         BeginProperty Button1 {0713F354-850A-101B-AFC0-4210102A8DA7} 
            Key             =   "Open"
            Object.ToolTipText     =   "Open"
            Object.Tag             =   ""
            ImageIndex      =   1
         EndProperty
         BeginProperty Button2 {0713F354-850A-101B-AFC0-4210102A8DA7} 
            Key             =   "Generate"
            Object.ToolTipText     =   "Generate"
            Object.Tag             =   ""
            ImageIndex      =   2
         EndProperty
         BeginProperty Button3 {0713F354-850A-101B-AFC0-4210102A8DA7} 
            Object.Tag             =   ""
            Style           =   3
            MixedState      =   -1  'True
         EndProperty
         BeginProperty Button4 {0713F354-850A-101B-AFC0-4210102A8DA7} 
            Key             =   "Cut"
            Object.ToolTipText     =   "Cut"
            Object.Tag             =   ""
            ImageIndex      =   3
         EndProperty
         BeginProperty Button5 {0713F354-850A-101B-AFC0-4210102A8DA7} 
            Key             =   "Copy"
            Object.ToolTipText     =   "Copy"
            Object.Tag             =   ""
            ImageIndex      =   4
         EndProperty
         BeginProperty Button6 {0713F354-850A-101B-AFC0-4210102A8DA7} 
            Key             =   "Paste"
            Object.ToolTipText     =   "Paste"
            Object.Tag             =   ""
            ImageIndex      =   5
         EndProperty
      EndProperty
   End
   Begin ComctlLib.StatusBar sbStatusBar 
      Align           =   2  'Align Bottom
      Height          =   270
      Left            =   0
      Negotiate       =   -1  'True
      TabIndex        =   0
      Top             =   2955
      Width           =   4680
      _ExtentX        =   8255
      _ExtentY        =   476
      SimpleText      =   ""
      _Version        =   327682
      BeginProperty Panels {0713E89E-850A-101B-AFC0-4210102A8DA7} 
         NumPanels       =   4
         BeginProperty Panel1 {0713E89F-850A-101B-AFC0-4210102A8DA7} 
            AutoSize        =   1
            Text            =   "Status"
            TextSave        =   "Status"
            Object.Tag             =   ""
         EndProperty
         BeginProperty Panel2 {0713E89F-850A-101B-AFC0-4210102A8DA7} 
            AutoSize        =   2
            Text            =   "Toggles"
            TextSave        =   "Toggles"
            Object.Tag             =   ""
         EndProperty
         BeginProperty Panel3 {0713E89F-850A-101B-AFC0-4210102A8DA7} 
            Style           =   6
            AutoSize        =   2
            TextSave        =   "08/31/1999"
            Object.Tag             =   ""
         EndProperty
         BeginProperty Panel4 {0713E89F-850A-101B-AFC0-4210102A8DA7} 
            Style           =   5
            AutoSize        =   2
            TextSave        =   "3:49 PM"
            Object.Tag             =   ""
         EndProperty
      EndProperty
   End
   Begin MSComDlg.CommonDialog cdMission 
      Left            =   720
      Top             =   2340
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      CancelError     =   -1  'True
   End
   Begin ComctlLib.ImageList ilToolBar 
      Left            =   60
      Top             =   2280
      _ExtentX        =   1005
      _ExtentY        =   1005
      BackColor       =   -2147483643
      ImageWidth      =   16
      ImageHeight     =   15
      MaskColor       =   12632256
      _Version        =   327682
      BeginProperty Images {0713E8C2-850A-101B-AFC0-4210102A8DA7} 
         NumListImages   =   5
         BeginProperty ListImage1 {0713E8C3-850A-101B-AFC0-4210102A8DA7} 
            Picture         =   "frmMain.frx":030A
            Key             =   ""
         EndProperty
         BeginProperty ListImage2 {0713E8C3-850A-101B-AFC0-4210102A8DA7} 
            Picture         =   "frmMain.frx":084C
            Key             =   ""
         EndProperty
         BeginProperty ListImage3 {0713E8C3-850A-101B-AFC0-4210102A8DA7} 
            Picture         =   "frmMain.frx":0D8E
            Key             =   ""
         EndProperty
         BeginProperty ListImage4 {0713E8C3-850A-101B-AFC0-4210102A8DA7} 
            Picture         =   "frmMain.frx":12D0
            Key             =   ""
         EndProperty
         BeginProperty ListImage5 {0713E8C3-850A-101B-AFC0-4210102A8DA7} 
            Picture         =   "frmMain.frx":1812
            Key             =   ""
         EndProperty
      EndProperty
   End
   Begin VB.Menu mnuFile 
      Caption         =   "&File"
      Begin VB.Menu mnuFileCreate 
         Caption         =   "C&reate..."
         Shortcut        =   ^T
      End
      Begin VB.Menu mnuFileOpen 
         Caption         =   "&Open..."
         Shortcut        =   ^O
      End
      Begin VB.Menu mnuFileClose 
         Caption         =   "&Close"
      End
      Begin VB.Menu mnuFileBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileGenerate 
         Caption         =   "&Generate..."
         Shortcut        =   ^G
      End
      Begin VB.Menu mnuFileBar2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileMRU 
         Caption         =   ""
         Index           =   0
         Visible         =   0   'False
      End
      Begin VB.Menu mnuFileMRU 
         Caption         =   ""
         Index           =   1
         Visible         =   0   'False
      End
      Begin VB.Menu mnuFileMRU 
         Caption         =   ""
         Index           =   2
         Visible         =   0   'False
      End
      Begin VB.Menu mnuFileMRU 
         Caption         =   ""
         Index           =   3
         Visible         =   0   'False
      End
      Begin VB.Menu mnuFileMRU 
         Caption         =   ""
         Index           =   4
         Visible         =   0   'False
      End
      Begin VB.Menu mnuFileBar3 
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileExit 
         Caption         =   "E&xit"
      End
   End
   Begin VB.Menu mnuEdit 
      Caption         =   "&Edit"
      Begin VB.Menu mnuEditUndo 
         Caption         =   "&Undo"
         Shortcut        =   %{BKSP}
      End
      Begin VB.Menu mnuEditBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuEditProp 
         Caption         =   "P&roperties... [Enter]"
      End
      Begin VB.Menu mnuEditFind 
         Caption         =   "&Find"
         Shortcut        =   ^F
      End
      Begin VB.Menu mnuEditBar2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuEditNew 
         Caption         =   "&New..."
         Shortcut        =   ^N
      End
      Begin VB.Menu mnuEditDup 
         Caption         =   "D&uplicate"
         Shortcut        =   ^D
      End
      Begin VB.Menu mnuEditDel 
         Caption         =   "&Delete [Del]"
      End
      Begin VB.Menu mnuEditBar3 
         Caption         =   "-"
      End
      Begin VB.Menu mnuEditCut 
         Caption         =   "Cu&t"
         Shortcut        =   ^X
      End
      Begin VB.Menu mnuEditCopy 
         Caption         =   "&Copy"
         Shortcut        =   ^C
      End
      Begin VB.Menu mnuEditPaste 
         Caption         =   "&Paste"
         Shortcut        =   ^V
      End
   End
   Begin VB.Menu mnuView 
      Caption         =   "&View"
      Begin VB.Menu mnuViewTree 
         Caption         =   "&Tree"
      End
      Begin VB.Menu mnuViewList 
         Caption         =   "&List"
      End
      Begin VB.Menu mnuViewBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuViewGraph 
         Caption         =   "&Graphics"
         Begin VB.Menu mnuViewGraphFront 
            Caption         =   "&Front View"
         End
         Begin VB.Menu mnuViewGraphTop 
            Caption         =   "&Top View"
         End
         Begin VB.Menu mnuViewGraphSide 
            Caption         =   "&Side View"
         End
         Begin VB.Menu mnuViewGraphCamera 
            Caption         =   "&Camera View"
         End
      End
      Begin VB.Menu mnuViewDB 
         Caption         =   "T&ables"
         Begin VB.Menu mnuViewTabMission 
            Caption         =   "&Mission"
         End
         Begin VB.Menu mnuViewTabLevel 
            Caption         =   "&Level"
         End
         Begin VB.Menu mnuViewTabObject 
            Caption         =   "&Object"
         End
         Begin VB.Menu mnuViewTabAttrib 
            Caption         =   "&Attribute"
         End
         Begin VB.Menu mnuViewTabLayer 
            Caption         =   "Lay&ers"
            Shortcut        =   ^L
         End
      End
      Begin VB.Menu mnuViewBar2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuViewDock 
         Caption         =   "&Dock"
         Visible         =   0   'False
      End
      Begin VB.Menu mnuViewRefresh 
         Caption         =   "&Refresh"
         Shortcut        =   {F5}
      End
   End
   Begin VB.Menu mnuTools 
      Caption         =   "&Tools"
      Begin VB.Menu mnuToolsToolbar 
         Caption         =   "&Toolbar"
         Checked         =   -1  'True
      End
      Begin VB.Menu mnuToolsStatusBar 
         Caption         =   "Status &Bar"
         Checked         =   -1  'True
      End
      Begin VB.Menu mnuToolsBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuToolsGrid 
         Caption         =   "Snap &Grid"
         Shortcut        =   +{F1}
      End
      Begin VB.Menu mnuToolsAngle 
         Caption         =   "Snap &Angle"
         Shortcut        =   +{F2}
      End
      Begin VB.Menu mnuToolsScale 
         Caption         =   "Free Object &Scale"
         Shortcut        =   +{F3}
      End
      Begin VB.Menu mnuToolsViewScale 
         Caption         =   "Free &View Scale"
         Shortcut        =   +{F4}
         Visible         =   0   'False
      End
      Begin VB.Menu mnuToolsCamera 
         Caption         =   "Free &Camera"
         Shortcut        =   +{F5}
      End
      Begin VB.Menu mnuToolsBar2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuToolsFind1 
         Caption         =   "Find C&ursor"
      End
      Begin VB.Menu mnuToolsFind2 
         Caption         =   "Find Origi&n"
      End
      Begin VB.Menu mnuToolsBar3 
         Caption         =   "-"
      End
      Begin VB.Menu mnuToolsOptions 
         Caption         =   "&Options..."
         Shortcut        =   ^P
      End
   End
   Begin VB.Menu mnuWindow 
      Caption         =   "&Window"
      WindowList      =   -1  'True
      Begin VB.Menu mnuWindowCascade 
         Caption         =   "&Cascade"
      End
      Begin VB.Menu mnuWindowTileHorizontal 
         Caption         =   "Tile &Horizontal"
      End
      Begin VB.Menu mnuWindowTileVertical 
         Caption         =   "Tile &Vertical"
      End
      Begin VB.Menu mnuWindowArrangeIcons 
         Caption         =   "&Arrange Icons"
      End
      Begin VB.Menu mnuWindowBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuWindowDefaultLayout 
         Caption         =   "&Default Layout"
      End
   End
   Begin VB.Menu mnuHelp 
      Caption         =   "&Help"
      Begin VB.Menu mnuHelpHelp 
         Caption         =   "&Help..."
         Shortcut        =   {F1}
      End
      Begin VB.Menu mnuHelpBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuHelpAbout 
         Caption         =   "&About..."
      End
   End
   Begin VB.Menu mnuPUTreeMis 
      Caption         =   "Popup menu for mission item in tree window"
      Visible         =   0   'False
      Begin VB.Menu mnuPUTreeMisProp 
         Caption         =   "Mission P&roperties..."
      End
      Begin VB.Menu mnuPUTreeMisGen 
         Caption         =   "&Generate Mission..."
      End
      Begin VB.Menu mnuPUTreeMisBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeMisNew 
         Caption         =   "New &Level..."
      End
      Begin VB.Menu mnuPUTreeMisBar2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeMisPaste 
         Caption         =   "&Paste Level"
      End
   End
   Begin VB.Menu mnuPUTreeLev 
      Caption         =   "Popup menu for level item in tree window"
      Visible         =   0   'False
      Begin VB.Menu mnuPUTreeLevProp 
         Caption         =   "Level P&roperties..."
      End
      Begin VB.Menu mnuPUTreeLevList 
         Caption         =   "Attributes &List..."
      End
      Begin VB.Menu mnuPUTreeLevGen 
         Caption         =   "&Generate Level..."
      End
      Begin VB.Menu mnuPUTreeLevBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeLevNew1 
         Caption         =   "New &Object..."
      End
      Begin VB.Menu mnuPUTreeLevNew2 
         Caption         =   "New &Attribute..."
      End
      Begin VB.Menu mnuPUTreeLevDel 
         Caption         =   "&Delete Level"
      End
      Begin VB.Menu mnuPUTreeLevBar3 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeLevCut 
         Caption         =   "Cu&t Level"
      End
      Begin VB.Menu mnuPUTreeLevCopy 
         Caption         =   "&Copy Level"
      End
      Begin VB.Menu mnuPUTreeLevPaste 
         Caption         =   "&Paste Objects/Attribute"
      End
   End
   Begin VB.Menu mnuPUTreeObj 
      Caption         =   "Popup menu for object item in tree window"
      Visible         =   0   'False
      Begin VB.Menu mnuPUTreeObjProp 
         Caption         =   "Object P&roperties..."
      End
      Begin VB.Menu mnuPUTreeObjFind 
         Caption         =   "&Find Object"
      End
      Begin VB.Menu mnuPUTreeObjList 
         Caption         =   "Attributes &List..."
      End
      Begin VB.Menu mnuPUTreeObjGen 
         Caption         =   "&Generate Object..."
      End
      Begin VB.Menu mnuPUTreeObjBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeObjSel1 
         Caption         =   "Select Glo&bal"
      End
      Begin VB.Menu mnuPUTreeObjSel2 
         Caption         =   "&Select Objects"
      End
      Begin VB.Menu mnuPUTreeObjSel3 
         Caption         =   "Select Pat&h"
      End
      Begin VB.Menu mnuPUTreeObjBar2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeObjIns 
         Caption         =   "&Insert Object..."
      End
      Begin VB.Menu mnuPUTreeObjNew1 
         Caption         =   "New &Object..."
      End
      Begin VB.Menu mnuPUTreeObjNew2 
         Caption         =   "New &Attribute..."
      End
      Begin VB.Menu mnuPUTreeObjDel 
         Caption         =   "&Delete Object"
      End
      Begin VB.Menu mnuPUTreeObjBar3 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeObjCut 
         Caption         =   "Cu&t Object"
      End
      Begin VB.Menu mnuPUTreeObjCopy 
         Caption         =   "&Copy Object"
      End
      Begin VB.Menu mnuPUTreeObjPaste 
         Caption         =   "&Paste Objects/Attribute"
      End
   End
   Begin VB.Menu mnuPUTreeAttrib 
      Caption         =   "Popup menu for attribute item in tree window"
      Visible         =   0   'False
      Begin VB.Menu mnuPUTreeAttribProp 
         Caption         =   "Attribute P&roperties..."
      End
      Begin VB.Menu mnuPUTreeAttribBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeAttribDel 
         Caption         =   "&Delete Attribute"
      End
      Begin VB.Menu mnuPUTreeAttribBar2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeAttribCut 
         Caption         =   "Cu&t Attribute"
      End
      Begin VB.Menu mnuPUTreeAttribCopy 
         Caption         =   "&Copy Attribute"
      End
   End
   Begin VB.Menu mnuPUTreeSel 
      Caption         =   "Popup menu for selection of tree window"
      Visible         =   0   'False
      Begin VB.Menu mnuPUTreeSelCollapse 
         Caption         =   "Coll&apse selection [-]"
      End
      Begin VB.Menu mnuPUTreeSelExpand 
         Caption         =   "&Expand sellection [*]"
      End
      Begin VB.Menu mnuPUTreeSelDesel 
         Caption         =   "De&select"
      End
      Begin VB.Menu mnuPUTreeSelClear 
         Caption         =   "Cl&ear Selection"
      End
      Begin VB.Menu mnuPUTreeSelBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeSelProp 
         Caption         =   "Selection P&roperties"
      End
      Begin VB.Menu mnuPUTreeSelBar2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeSelDel 
         Caption         =   "&Delete selection"
      End
      Begin VB.Menu mnuPUTreeSelBar3 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUTreeSelCut 
         Caption         =   "Cu&t selection"
      End
      Begin VB.Menu mnuPUTreeSelCopy 
         Caption         =   "&Copy selection"
      End
   End
   Begin VB.Menu mnuPUList 
      Caption         =   "Popup menu for item in the list window"
      Visible         =   0   'False
      Begin VB.Menu mnuPUListProp 
         Caption         =   "Attribute P&roperties..."
      End
      Begin VB.Menu mnuPUListBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUListNew 
         Caption         =   "New &Attribute..."
      End
      Begin VB.Menu mnuPUListDel 
         Caption         =   "&Delete Attribute"
      End
      Begin VB.Menu mnuPUListBar2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUListCut 
         Caption         =   "Cu&t Attribute"
      End
      Begin VB.Menu mnuPUListCopy 
         Caption         =   "&Copy Attribute"
      End
      Begin VB.Menu mnuPUListPaste 
         Caption         =   "&Paste Attribute"
      End
   End
   Begin VB.Menu mnuPUGraphObj 
      Caption         =   "Popup menu for object item in graphic windows"
      Visible         =   0   'False
      Begin VB.Menu mnuPUGraphObjProp 
         Caption         =   "Object P&roperties..."
      End
      Begin VB.Menu mnuPUGraphObjFind 
         Caption         =   "&Find Object"
      End
      Begin VB.Menu mnuPUGraphObjList 
         Caption         =   "Attributes &List..."
      End
      Begin VB.Menu mnuPUGraphObjBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUGraphObjSel1 
         Caption         =   "Select Glo&bal"
      End
      Begin VB.Menu mnuPUGraphObjSel2 
         Caption         =   "&Select Objects"
      End
      Begin VB.Menu mnuPUGraphObjSel3 
         Caption         =   "Select Pat&h"
      End
      Begin VB.Menu mnuPUGraphObjBar2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUGraphObjNew 
         Caption         =   "New &Attribute..."
      End
      Begin VB.Menu mnuPUGraphObjDup 
         Caption         =   "D&uplicate Object Here"
      End
      Begin VB.Menu mnuPUGraphObjDel 
         Caption         =   "&Delete Object"
      End
      Begin VB.Menu mnuPUGraphObjBar3 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUGraphObjCut 
         Caption         =   "Cu&t Object"
      End
      Begin VB.Menu mnuPUGraphObjCopy 
         Caption         =   "&Copy Object"
      End
      Begin VB.Menu mnuPUGraphObjPaste 
         Caption         =   "&Paste Attribute"
      End
   End
   Begin VB.Menu mnuPUGraphSel 
      Caption         =   "Popup menu for selection of graphic windows"
      Visible         =   0   'False
      Begin VB.Menu mnuPUGraphSelProp 
         Caption         =   "Selection P&roperties"
      End
      Begin VB.Menu mnuPUGraphSelBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUGraphSelDel 
         Caption         =   "&Delete selection"
      End
      Begin VB.Menu mnuPUGraphSelBar2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUGraphSelCut 
         Caption         =   "Cu&t selection"
      End
      Begin VB.Menu mnuPUGraphSelCopy 
         Caption         =   "&Copy selection"
      End
   End
   Begin VB.Menu mnuPUGraphDef 
      Caption         =   "Popup menu for default in graphic windows"
      Visible         =   0   'False
      Begin VB.Menu mnuPUGraphDefNew 
         Caption         =   "New &Object"
      End
      Begin VB.Menu mnuPUGraphDefBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUGraphDefPaste 
         Caption         =   "&Paste Object"
      End
   End
   Begin VB.Menu mnuPUAttribVal 
      Caption         =   "Popup menu for value in attrib window"
      Visible         =   0   'False
      Begin VB.Menu mnuPUAttribValBrowse 
         Caption         =   "&Browse..."
      End
      Begin VB.Menu mnuPUAttribValBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUAttribValOpen 
         Caption         =   "Shell &Open"
      End
   End
   Begin VB.Menu mnuPUObjFile 
      Caption         =   "Popup menu for file in object window"
      Visible         =   0   'False
      Begin VB.Menu mnuPUObjFileBrowse 
         Caption         =   "&Browse"
      End
      Begin VB.Menu mnuPUObjFileBar1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPUObjFileOpen 
         Caption         =   "Shell &Open"
      End
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Sub ShowStatus(ByVal sKey As String)
    Dim nType As Integer
    
    Dim sStat As String
    Dim sTxt As String
    Dim sVal As String

    'Set default
    sStat = ""
    
    ' Check key
    If Left(sKey, 1) = "m" Then
        'Set status
        sStat = "Mission " + frmMission.GetName + ": " + frmMission.GetType
    End If
        
    ' Check key
    If Left(sKey, 1) = "l" Then
        'Set status
        sStat = "Level " + frmLevels.GetName(Val(Mid(sKey, 2)))
    End If
        
    ' Check key
    If Left(sKey, 1) = "o" Then
        'Get name
        sTxt = frmObjects.GetName(Val(Mid(sKey, 2)))
    
        'Get type
        nType = frmObjects.GetType(Val(Mid(sKey, 2)))
        Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_TYPE, sVal, MIS_MOD_CFG)
        sVal = TruncStr(sVal)
        If sVal <> "" Then sTxt = sVal + ": " + sTxt
                
        'Set status
        sStat = "Object " + sTxt
    End If
        
    ' Check key
    If Left(sKey, 1) = "a" Then
        'Get name
        sTxt = frmAttribs.GetName(Val(Mid(sKey, 2)))
    
        'Get value
        sVal = frmAttribs.GetValue(Val(Mid(sKey, 2)))
        
        'Check type
        If sVal <> "" Then sTxt = sTxt + ": " + sVal
                
        'Set status
        sStat = "Attribute " + sTxt
    End If
    
    'Show status
    sbStatusBar.Panels.Item(1).Text = sStat
    
    'Set status
    sStat = " "
    If bGridFlag = 1 Then sStat = sStat + "Grd "
    If bRotFlag = 1 Then sStat = sStat + "Ang "
    If bScaleFlag = 1 Then sStat = sStat + "Obj "
    If bCamFlag = 1 Then sStat = sStat + "Cam "
    
    'Show status
    sbStatusBar.Panels.Item(2).Text = sStat
End Sub

Sub GetFlags()
    'Check and set toolbar flag
    If bToolFlag = 1 Then
        mnuToolsToolbar.Checked = True
        tbToolBar.Visible = True
    Else
        mnuToolsToolbar.Checked = False
        tbToolBar.Visible = False
    End If

    'Check and set statusbar flag
    If bStatFlag = 1 Then
        mnuToolsStatusBar.Checked = True
        sbStatusBar.Visible = True
    Else
        mnuToolsStatusBar.Checked = False
        sbStatusBar.Visible = False
    End If

    'Check and set grid flag
    If bGridFlag = 1 Then
        mnuToolsGrid.Checked = True
    Else
        mnuToolsGrid.Checked = False
    End If

    'Check and set rotation flag
    If bRotFlag = 1 Then
        mnuToolsAngle.Checked = True
    Else
        mnuToolsAngle.Checked = False
    End If

    'Check and set scale flag
    If bScaleFlag = 1 Then
        mnuToolsScale.Checked = True
    Else
        mnuToolsScale.Checked = False
    End If

    'Check and set camera flag
    If bCamFlag = 1 Then
        mnuToolsCamera.Checked = True
    Else
        mnuToolsCamera.Checked = False
    End If
    
    'Show status
    ShowStatus (sCurKey)
End Sub

Sub DelList(ByVal sPKey As String, ByVal sLKey As String, ByVal sFile As String)
    Dim nPos As Long
    Dim sCurK As String
    Dim sParK As String
    Dim sListK As String
    
    'Set list
    sListK = sLKey
    
    'Set parent key
    sParK = sPKey
    
    'Loop thru types
    Do
        'Get position of space character in string
        nPos = InStr(sListK, " ")
        
        'If possible, truncate string at space character
        If nPos > 0 Then
            'Check parent key
            If sListK = sLKey Then
                'Inform user
                If MsgBox("Delete selection?", vbYesNo Or vbQuestion, "MissionMan") = vbNo Then Exit Sub
            End If
            
            'Reset parent key
            sParK = ""
                
            'Set key
            sCurK = Left(sListK, nPos - 1)
            sListK = Mid(sListK, nPos + 1, Len(sListK))
        Else
            'Set key
            sCurK = sListK
        End If
        
        'Check key
        If Left(sCurK, 1) = "l" Then
            'Delete item
            Call frmLevels.DelLevel(sParK, sCurK, sFile)
        End If
        
        'Check key
        If Left(sCurK, 1) = "o" Then
            'Delete item
            Call frmObjects.DelObject(sParK, sCurK, sFile)
        End If
        
        'Check key
        If Left(sCurK, 1) = "a" Then
            'Delete item
            Call frmAttribs.DelAttrib(sParK, sCurK, sFile)
        End If
        
        'Check position
        If nPos = 0 Then Exit Do
    Loop
    
    'Check list
    If sListK = sLKey Then Exit Sub
      
    'Select in tree
    Call frmTree.SelTree(sPKey)
    
    'Refresh
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub CopyList(ByVal sLKey As String, ByVal sCKey As String, ByVal sPKey As String, ByVal bFlag As Boolean, ByVal sFile As String)
    Dim nPos As Long
    Dim sCopyK As String
    Dim sListK As String
    
    'Check list
    If InStr(sLKey + " ", sCKey + " ") > 0 Then sCKey = sPKey
    
    'Set list
    sListK = sLKey
    
    'Loop thru types
    Do
        'Get position of space character in string
        nPos = InStr(sListK, " ")
        
        'If possible, truncate string at space character
        If nPos > 0 Then
            'Check list key
            If sListK = sLKey Then
                'Inform user
                If MsgBox("Paste selection?", vbYesNo Or vbQuestion, "MissionMan") = vbNo Then Exit Sub
            End If
            
            'Set key
            sCopyK = Left(sListK, nPos - 1)
            sListK = Mid(sListK, nPos + 1, Len(sListK))
        Else
            'Set key
            sCopyK = sListK
        End If
        
        'Check key
        If Left(sCopyK, 1) = "l" Then
            'Copy item
            Call frmLevels.CopyLevel(sCopyK, sCKey, bFlag, sFile)
        End If
        
        'Check key
        If Left(sCopyK, 1) = "o" Then
            'Copy item
            Call frmObjects.CopyObject(sCopyK, sCKey, bFlag, sFile)
        End If
        
        'Check key
        If Left(sCopyK, 1) = "a" Then
            'Copy item
            Call frmAttribs.CopyAttrib(sCopyK, sCKey, bFlag, sFile)
        End If
        
        'Check position
        If nPos = 0 Then Exit Do
    Loop
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub GetListProp()
    'Check list key
    If sListKey = sCurKey Then
        'Check key
        If Left(sCurKey, 1) = "m" Then
            'Show form
            frmMission.Show
            frmMission.GetMission
            frmMission.SetFocus
            Exit Sub
        End If
            
        'Check key
        If Left(sCurKey, 1) = "l" Then
            'Show form
            frmLevels.Show
            Call frmLevels.GetLevel(sParKey, sCurKey)
            frmLevels.SetFocus
            Exit Sub
        End If
            
        'Check key
        If Left(sCurKey, 1) = "o" Then
            'Show form
            frmObjects.Show
            Call frmObjects.GetObject(sParKey, sCurKey, 0, 0, 0)
            frmObjects.SetFocus
            Exit Sub
        End If
            
        'Check key
        If Left(sCurKey, 1) = "a" Then
            'Show form
            frmAttribs.Show
            Call frmAttribs.GetAttrib(sParKey, sCurKey)
            frmAttribs.SetFocus
            Exit Sub
        End If
    End If
    
    'Check key
    If InStr(sListKey, "m") > 0 Then
        'Show table
        frmMission.Show
        frmMission.GetMission
        frmMission.SetFocus
    End If
        
    'Check key
    If InStr(sListKey, "l") > 0 Then
        'Show table
        frmLevels.Show
        Call frmLevels.GetLevel("", sListKey)
        frmLevels.SetFocus
    End If
    
    'Check key
    If InStr(sListKey, "o") > 0 Then
        'Show table
        frmObjects.Show
        Call frmObjects.GetObject("", sListKey, 0, 0, 0)
        frmObjects.SetFocus
    End If
    
    'Check key
    If InStr(sListKey, "a") > 0 Then
        'Show table
        frmAttribs.Show
        Call frmAttribs.GetAttrib("", sListKey)
        frmAttribs.SetFocus
    End If
End Sub

Private Sub OpenFile(ByVal sFile As String)
    Dim nPos As Long

    Dim sVal As String
    Dim sName As String
    Dim sExt As String
    
    'Close file
    Call CloseFile
    
    'Set database file
    sDBFile = sFile
    
    'Set work directory
    sName = Dir(sDBFile)
    If sName = "" Then
        'Inform user
        Call MsgBox("Error: Unable to open mission database (Check path " + sDBFile + ")!", vbOKOnly Or vbExclamation, "MissionMan")
        Exit Sub
    End If
    
    'Remove name from DB path
    nPos = InStr(sDBFile, sName)
    If nPos > 0 Then
        sWorkDir = Left(sDBFile, nPos - 1)
    Else
        sWorkDir = sWDir
    End If
    
    'Set data directory
    sDataDir = sDDir
    If sDataDir = "" Then
        'Set data dir to work dir if data dir not set
        sDataDir = sWorkDir
    Else
        'Check work dir
        If InStr(1, sWorkDir, sWDir, vbTextCompare) > 0 Then
            'Remove extension from DB name
            nPos = InStr(sName, ".")
            If nPos > 0 Then sExt = Left(sName, nPos - 1)
            
            'Set data dir to partial mirror work dir plus mission DB name
            sDataDir = sDataDir + Mid(sWorkDir, Len(sWDir) + 1) + sExt + "\"
        Else
            'Set data dir to full work dir if unable to mirror partial work dir
            sDataDir = sWorkDir
        End If
    End If
    
    'Set objects directory
    sObjDir = sODir
    If sObjDir = "" Then sObjDir = sWorkDir
    
    'Set caption
    Me.Caption = "MissionMan - " + sDBFile
    
    'Open database
    OpenDB
    If bDBFlag = False Then Exit Sub
    
    'Check type
    If frmMission.CheckType = False Then
        'Inform user
        Call MsgBox("Error: Invalid mission database (Check type)!", vbOKOnly Or vbExclamation, "MissionMan")
        Exit Sub
    End If
    
    'Append MRU list
    Call AppendMRUList(sDBFile)
    
    'Get tree
    frmTree.Hide
    frmTree.GetTree ("m")
    
    'Get objects
    frmObjects.GetObjects
    
    'Show tree view
    frmTree.Show
        
    'Show views
    frmCamera.Show
    frmFront.Show
    frmTop.Show
    frmSide.Show
    
    'Set focus
    frmTree.SetFocus
End Sub

Private Sub MDIForm_Load()
    Dim sCommand As String

    'Maximize form
    Me.WindowState = vbMaximized
    
    'Get MRU list
    GetMRUList
    
    'Get flags
    GetFlags
    
    'Show form
    Me.Show
    
    'Check command line and open file
    sCommand = Command
    If sCommand <> "" Then Call OpenFile(sCommand)
End Sub

Sub CloseFile()
    'Hide views
    Unload frmTree
    Unload frmCamera
    Unload frmSide
    Unload frmTop
    Unload frmFront
    
    'Hide other views
    Unload frmList
    Unload frmLayers
    Unload frmMission
    Unload frmLevels
    Unload frmObjects
    Unload frmAttribs
    
    'Re-initialize renderer
    rendClean
    rendInit
    
    'Close database
    CloseDB
    
    'Reset caption
    Me.Caption = "MissionMan"
    
    'Reset keys
    sParKey = ""
    sCurKey = ""
    sCopyKey = ""
    sListKey = ""
    
    'Get status
    ShowStatus (sCurKey)
End Sub

Sub GetMRUList()
    Dim n As Integer
    Dim nPos As Long
        
    Dim sList As String
    
    'Reset count
    nMRUCount = 0
    
    'Get window
    Call misGetListByKey(MIS_SEC_COM, MIS_KEY_MRU, sList, nMRUCount, MIS_MOD_INI)
    
    'Check count
    If nMRUCount > 0 Then
        'Truncate list
        sList = TruncStr(sList)
        
        'Loop thru list
        For n = 0 To nMRUCount - 1
            'Get position of | character in string
            nPos = InStr(sList, "|")
        
            'If possible, truncate string at | character
            If nPos > 0 Then
                'Set position
                mnuFileMRU(n).Caption = Left(sList, nPos - 1)
                mnuFileMRU(n).Visible = True
                sList = Mid(sList, nPos + 1, Len(sList))
            Else
                'Set position
                mnuFileMRU(n).Caption = sList
                mnuFileMRU(n).Visible = True
            End If
        Next n
        
        'Set MRU index
        nMRUIndex = nMRUCount
        If nMRUIndex >= MIS_MRU_COUNT Then nMRUIndex = 0
    End If
End Sub

Sub AppendMRUList(ByVal sFile As String)
    Dim n As Integer

    'Check for existing file
    For n = 0 To nMRUCount - 1
        If mnuFileMRU(n).Caption = sFile Then Exit Sub
    Next n

    'Append file
    mnuFileMRU(nMRUIndex).Caption = sDBFile
    mnuFileMRU(nMRUIndex).Visible = True
    
    'Increment index
    nMRUIndex = nMRUIndex + 1
    If nMRUCount < nMRUIndex Then nMRUCount = nMRUIndex
    If nMRUIndex >= MIS_MRU_COUNT Then nMRUIndex = 0
End Sub

Sub PutMRUList()
    Dim n As Integer
    
    Dim sList As String
    
    'Reset list
    sList = ""
    For n = 0 To nMRUCount - 1
        'Append list
        sList = sList + "|" + mnuFileMRU(n).Caption
    Next n
    
    'Put window
    Call misPutListByKey(MIS_SEC_COM, MIS_KEY_MRU, sList, MIS_MOD_INI)
End Sub

Private Sub MDIForm_QueryUnload(Cancel As Integer, UnloadMode As Integer)
    'Cancel unload
    Cancel = 1
    
    'End program
    AntiMain
End Sub

Private Sub mnuEditFind_Click()
    Dim nObj As Long
    
    Dim X As Single
    Dim Y As Single
    Dim Z As Single
    
    'Check key
    If Left(sCurKey, 1) <> "o" Then Exit Sub

    'Get object position
    Call rendFindObj(nObj, Val(Mid(sCurKey, 2)))
    Call rendGetObjTrans(nObj, X, Y, Z)

    'Set offset
    aOffset(0) = -X * fViewScale
    aOffset(1) = -Y * fViewScale
    aOffset(2) = -Z * fViewScale
    
    'Set focus
    aFocus(0) = X
    aFocus(1) = Y
    aFocus(2) = Z
    
    'Refresh
    frmFront.SetView (False)
    frmFront.SetCamera (True)
    frmTop.SetView (False)
    frmTop.SetCamera (True)
    frmSide.SetView (False)
    frmSide.SetCamera (True)
    frmCamera.SetCamera (True)
End Sub

Private Sub mnuEditUndo_Click()
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'CHeck undo flag
    If bUndoFlag = False Then Exit Sub

    'Promp user
    If MsgBox("Undo " + sUndoInfo + "?", vbYesNo Or vbQuestion, "MissionMan") = vbNo Then Exit Sub
    
    'Undo
    RollbackDB
    
    'Hide list view
    Unload frmList
       
    'Refresh tree
    frmTree.GetTree (sListKey)
    
    'Re-initialize renderer
    rendClean
    rendInit
    frmObjects.GetObjects
    Call rendSetSel("o", sListKey)
       
    'Show views
    frmCamera.Render
    frmFront.Render
    frmTop.Render
    frmSide.Render
    
    'Refresh layers
    frmLayers.GetLayers (0)
    
    'Refresh list
    Call frmList.GetList(sCurKey, "")
    
    'Set focus
    frmTree.SetFocus
End Sub

Private Sub mnuFileCreate_Click()
    Dim nAttribs As Integer

    Dim sFile As String
    Dim sPath As String

    'Get reference file
    sFile = App.Path
    If Mid(sFile, Len(sFile)) <> "\" Then sFile = sFile + "\"
    sFile = sFile + MIS_FILE_DEF
    
    'Check and change attribs
    nAttribs = GetAttr(sFile)
    If (nAttribs And vbReadOnly) <> 0 Then Call SetAttr(sFile, nAttribs And (Not vbReadOnly))
   
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show common dialog
    cdMission.FileName = "New Folder"
    cdMission.InitDir = sWDir
    cdMission.DialogTitle = "Create Mission Database Folder"
    cdMission.Filter = "Folders|Dir"
    cdMission.ShowSave
        
    'Reset handler for Cancel button
    On Error GoTo 0
    
    'Create directory
    sPath = cdMission.FileName
    Call MkDir(sPath)
    
    'Check directory
    If Dir(sPath, vbDirectory) = "" Then
        'Inform user
        Call MsgBox("Error: Unable to create folder " + sPath + "!", vbOKOnly Or vbExclamation, "MissionMan")
        Exit Sub
    End If
    
    'Append file name
    If Mid(sPath, Len(sPath)) <> "\" Then sPath = sPath + "\"
    sPath = sPath + MIS_FILE_DEF
    
    'Copy reference file
    Call FileCopy(sFile, sPath)
    
    'Open file
    Call OpenFile(sPath)
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub mnuFileMRU_Click(Index As Integer)
    'Open file
    Call OpenFile(mnuFileMRU(Index).Caption)
End Sub

Private Sub mnuHelpAbout_Click()
    Dim sDll As String
    Dim sIni As String

    'Get DLL version
    Call misGetVer(sDll)
    sDll = TruncStr(sDll)
    If sDll = "" Then sDll = "N/A"
    
    'Get config version
    Call misGetVal(MIS_SEC_COM, MIS_KEY_VER, sIni, MIS_MOD_CFG)
    sIni = TruncStr(sIni)
    If sIni = "" Then sIni = "N/A"

    'Show version
    Call MsgBox("MissionMan version " + MIS_VER_NUM + Chr(13) + "DLL version " + sDll + Chr(13) + "Config version " + sIni + Chr(13) + Chr(13) + "Copyright (c) 1998-99, Relic Entertainment Inc." + Chr(13) + Chr(13) + "This program is freeware and freely distributable." + Chr(13) + Chr(13) + "There is absolutely NO WARRANTY and" + Chr(13) + "NO SUPPORT for this software, you use it" + Chr(13) + "at your own risk.", vbOKOnly Or vbInformation, "MissionMan")
End Sub

Private Sub mnuHelpHelp_Click()
    Dim sFile As String

    'Set help file
    sFile = App.Path
    If Mid(sFile, Len(sFile)) <> "\" Then sFile = sFile + "\"
    sFile = sFile + MIS_FILE_HLP
    
    'Attempt to open
    Call misShellExec(sFile)
End Sub

Private Sub mnuPUAttribValBrowse_Click()
    'Browse
    frmAttribs.BrowseAttrib
End Sub

Private Sub mnuPUAttribValOpen_Click()
    Dim sValue As String

    'Get path
    sValue = frmAttribs.cmbValue.Text
    
    'Check path
    If InStr(sValue, ":") = 0 Then
        'Update path
        sValue = sDataDir + sValue
    End If
    
    'Attempt to open
    Call misShellExec(sValue)
End Sub

Private Sub mnuPUGraphDefNew_Click()
    'Check current key
    If Left(sCurKey, 1) = "m" Then Exit Sub
    If Left(sCurKey, 1) = "a" Then Exit Sub
    
    'New object
    frmObjects.Show
    Call frmObjects.GetObject(sCurKey, "", aCursor(0), aCursor(1), aCursor(2))
    frmObjects.SetFocus
End Sub

Private Sub mnuPUGraphDefPaste_Click()
    Dim nObj As Long
    
    Dim fPosX As Single
    Dim fPosY As Single
    Dim fPosZ As Single
    
    'Get data from clipboard
    FromClipboard
    
    'Check copy key
    If InStr(sCopyKey, "l") > 0 Then Exit Sub
    If InStr(sCopyKey, "a") > 0 Then Exit Sub
        
    'Commit
    Call CommitDB("Paste")
    
    'Copy item(s)
    Call CopyList(sCopyKey, sCurKey, sParKey, bDelFlag, sCopyFile)
    
    'Check copy key
    If InStr(sCopyKey, " ") > 0 Then Exit Sub
    
    'Get object position
    Call rendFindObj(nObj, Val(Mid(sCurKey, 2)))
    Call rendGetObjTrans(nObj, fPosX, fPosY, fPosZ)
    
    'Translate object
    fPosX = aCursor(0) - fPosX
    fPosY = aCursor(1) - fPosY
    fPosZ = aCursor(2) - fPosZ
    Call rendTransObj(nObj, fPosX, fPosY, fPosZ)
    
    If bGridFlag = 1 Then
        'Snap x translation to grid
        If Abs(fPosX) Mod fGridSize < fGridSize / 2 Then
            fPosX = -(fPosX Mod fGridSize)
        Else
            fPosX = Sgn(fPosX) * fGridSize - (fPosX Mod fGridSize)
        End If
    
        'Snap y translation to grid
        If Abs(fPosY) Mod fGridSize < fGridSize / 2 Then
            fPosY = -(fPosY Mod fGridSize)
        Else
            fPosY = Sgn(fPosY) * fGridSize - (fPosY Mod fGridSize)
        End If
    
        'Snap y translation to grid
        If Abs(fPosZ) Mod fGridSize < fGridSize / 2 Then
            fPosZ = -(fPosZ Mod fGridSize)
        Else
            fPosZ = Sgn(fPosZ) * fGridSize - (fPosZ Mod fGridSize)
        End If
    
        'Translate object
        Call rendTransObj(nObj, fPosX, fPosY, fPosZ)
    End If
    
    'Set object position
    Call frmObjects.EditObject(Val(Mid(sCurKey, 2)))
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Private Sub mnuPUGraphObjCopy_Click()
    'Set copy key and delete flag
    sCopyKey = sCurKey
    bDelFlag = False
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUGraphObjCut_Click()
    'Set copy key and delete flag
    sCopyKey = sCurKey
    bDelFlag = True
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUGraphObjDel_Click()
    'Commit
    Call CommitDB("Delete")
    
    'Delete item(s)
    Call DelList(sParKey, sCurKey, "")
End Sub

Private Sub mnuPUGraphObjDup_Click()
    'Commit
    Call CommitDB("Duplicate Objects")
    
    Call frmObjects.DupObjects(aCursor(0), aCursor(1), aCursor(2))
End Sub

Private Sub mnuPUGraphObjFind_Click()
    Call mnuEditFind_Click
End Sub

Private Sub mnuPUGraphObjList_Click()
    frmList.Show
    Call frmList.GetList(sCurKey, "")
    frmList.SetFocus
End Sub

Private Sub mnuPUGraphObjNew_Click()
    frmAttribs.Show
    Call frmAttribs.GetAttrib(sCurKey, "")
    frmAttribs.SetFocus
End Sub

Private Sub mnuPUGraphObjPaste_Click()
    'Get data from clipboard
    FromClipboard
    
    ' Check copy key
    If InStr(sCopyKey, "l") > 0 Then Exit Sub
    If InStr(sCopyKey, "o") > 0 Then Exit Sub
    
    'Commit
    Call CommitDB("Paste")
    
    'Copy item(s)
    Call CopyList(sCopyKey, sCurKey, sParKey, bDelFlag, sCopyFile)
End Sub

Private Sub mnuPUGraphObjProp_Click()
    frmObjects.Show
    Call frmObjects.GetObject(sParKey, sCurKey, 0, 0, 0)
    frmObjects.SetFocus
End Sub

Private Sub mnuPUGraphObjSel1_Click()
    Dim sList As String

    'Select links
    sList = frmObjects.SelGlobal(Val(Mid(sCurKey, 2)))
    If sList = "" Then Exit Sub
    sListKey = sList
    
    'Check form
    If mnuViewTabObject.Checked = True Then Call frmObjects.GetObject("", sListKey, 0, 0, 0)
    If mnuViewTabAttrib.Checked = True Then Call frmAttribs.GetAttrib("", sListKey)
    
    'Show list
    frmTree.ShowList (sListKey)
        
    'Set selection
    Call rendSetSel("o", sList)
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Private Sub mnuPUGraphObjSel2_Click()
    Dim sList As String

    'Select links
    sList = frmObjects.SelObjects(Val(Mid(sCurKey, 2)))
    If sList = "" Then Exit Sub
    sListKey = sList
    
    'Check form
    If mnuViewTabObject.Checked = True Then Call frmObjects.GetObject("", sListKey, 0, 0, 0)
    If mnuViewTabAttrib.Checked = True Then Call frmAttribs.GetAttrib("", sListKey)
    
    'Show list
    frmTree.ShowList (sListKey)
        
    'Set selection
    Call rendSetSel("o", sList)
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Private Sub mnuPUGraphObjSel3_Click()
    Dim sList As String
    
    sList = frmObjects.SelPath(Val(Mid(sCurKey, 2)))
    If sList = "" Then Exit Sub
    sListKey = sList
    
    'Check form
    If mnuViewTabObject.Checked = True Then Call frmObjects.GetObject("", sListKey, 0, 0, 0)
    If mnuViewTabAttrib.Checked = True Then Call frmAttribs.GetAttrib("", sListKey)
    
    'Show list
    frmTree.ShowList (sListKey)
        
    'Set selection
    Call rendSetSel("o", sList)
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Private Sub mnuPUGraphSelCopy_Click()
    'Set copy key and delete flag
    sCopyKey = sListKey
    bDelFlag = False
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUGraphSelCut_Click()
    'Set copy key and delete flag
    sCopyKey = sListKey
    bDelFlag = True
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUGraphSelDel_Click()
    'Commit
    Call CommitDB("Delete")
    
    'Delete item(s)
    Call DelList(sParKey, sListKey, "")
End Sub

Private Sub mnuPUGraphSelProp_Click()
    'Get list property
    GetListProp
End Sub

Private Sub mnuPUListCopy_Click()
    'Set copy key and delete flag
    sCopyKey = sCurKey
    bDelFlag = False
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUListCut_Click()
    'Set copy key and delete flag
    sCopyKey = sCurKey
    bDelFlag = True
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUListDel_Click()
    'Commit
    Call CommitDB("Delete Attribute")
    
    'Del attrib
    Call frmAttribs.DelAttrib(sParKey, sCurKey, "")
End Sub

Private Sub mnuPUListNew_Click()
    frmAttribs.Show
    Call frmAttribs.GetAttrib(sCurKey, "")
    frmAttribs.SetFocus
End Sub

Private Sub mnuPUListPaste_Click()
    'Get data from clipboard
    FromClipboard
    
    'Check copy key
    If InStr(sCopyKey, " ") > 0 Then Exit Sub
    
    'Commit
    Call CommitDB("Paste")
    
    'Copy item(s)
    Call CopyList(sCopyKey, sCurKey, sParKey, bDelFlag, sCopyFile)
End Sub

Private Sub mnuPUListProp_Click()
    frmAttribs.Show
    Call frmAttribs.GetAttrib(sParKey, sCurKey)
    frmAttribs.SetFocus
End Sub

Private Sub mnuPUObjFileBrowse_Click()
    'Browse
    frmObjects.BrowseObject
End Sub

Private Sub mnuPUObjFileOpen_Click()
    Dim sFile As String

    'Get path
    sFile = frmObjects.txtFile.Text
    
    'Check path
    If InStr(sFile, ":") = 0 Then
        'Update path
        sFile = sObjDir + sFile
    End If
    
    'Attempt to open
    Call misShellExec(sFile)
End Sub

Private Sub mnuPUTreeAttribCopy_Click()
    'Set copy key and delete flag
    sCopyKey = sCurKey
    bDelFlag = False
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUTreeAttribCut_Click()
    'Set copy key and delete flag
    sCopyKey = sCurKey
    bDelFlag = True
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUTreeAttribDel_Click()
    'Commit
    Call CommitDB("Delete Attribute")
    
    'Del attrib
    Call frmAttribs.DelAttrib(sParKey, sCurKey, "")
End Sub

Private Sub mnuPUTreeAttribProp_Click()
    frmAttribs.Show
    Call frmAttribs.GetAttrib(sParKey, sCurKey)
    frmAttribs.SetFocus
End Sub

Private Sub mnuPUTreeLevCopy_Click()
    'Set copy key and delete flag
    sCopyKey = sCurKey
    bDelFlag = False
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUTreeLevCut_Click()
    'Set copy key and delete flag
    sCopyKey = sCurKey
    bDelFlag = True
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUTreeLevDel_Click()
    'Commit
    Call CommitDB("Delete Level")
    
    'Del level
    Call frmLevels.DelLevel(sParKey, sCurKey, "")
End Sub

Private Sub mnuPUTreeLevGen_Click()
    Dim sFile As String
    Dim sExt As String
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check key
    If sCurKey = "" Then Exit Sub
    If Left(sCurKey, 1) <> "l" Then Exit Sub
    
    'Get extension
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_LEV, MIS_KEY_EXT, sExt, MIS_MOD_CFG)
    sExt = TruncStr(sExt)
    If sExt = "" Then Exit Sub
    
    'Set file
    sFile = frmLevels.GetFile(sCurKey, -1)

    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show common dialog
    cdMission.FileName = sFile
    cdMission.InitDir = sDataDir
    cdMission.DefaultExt = sExt
    cdMission.DialogTitle = "Generate Level File"
    cdMission.Filter = "Level Files (*" + sExt + ")|*" + sExt + "|All Files|*.*"
    cdMission.ShowSave
    sFile = cdMission.FileName
    
    'Reset handler for Cancel button
    On Error GoTo 0

    'Commit
    Call CommitDB("Generate Level")
    
    'Generate level
    Call frmLevels.GenLevel(sCurKey, sFile)
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub mnuPUTreeLevList_Click()
    frmList.Show
    Call frmList.GetList(sCurKey, "")
    frmList.SetFocus
End Sub

Private Sub mnuPUTreeLevNew1_Click()
    frmObjects.Show
    Call frmObjects.GetObject(sCurKey, "", aCursor(0), aCursor(1), aCursor(2))
    frmObjects.SetFocus
End Sub

Private Sub mnuPUTreeLevNew2_Click()
    frmAttribs.Show
    Call frmAttribs.GetAttrib(sCurKey, "")
    frmAttribs.SetFocus
End Sub

Private Sub mnuPUTreeLevPaste_Click()
    'Get data from clipboard
    FromClipboard
    
    ' Check copy key
    If InStr(sCopyKey, "l") > 0 Then Exit Sub
    
    'Commit
    Call CommitDB("Paste")
    
    'Copy item(s)
    Call CopyList(sCopyKey, sCurKey, sParKey, bDelFlag, sCopyFile)
End Sub

Private Sub mnuPUTreeLevProp_Click()
    frmLevels.Show
    Call frmLevels.GetLevel(sParKey, sCurKey)
    frmLevels.SetFocus
End Sub

Private Sub mnuPUTreeMisGen_Click()
    'Commit
    Call CommitDB("Generate Mission")
    
    'Generate mission
    frmLevels.GenLevels
End Sub

Private Sub mnuPUTreeMisNew_Click()
    frmLevels.Show
    Call frmLevels.GetLevel(sCurKey, "")
    frmLevels.SetFocus
End Sub

Private Sub mnuPUTreeMisPaste_Click()
    'Get data from clipboard
    FromClipboard
    
    ' Check copy key
    If InStr(sCopyKey, " ") > 0 Then Exit Sub
    
    'Commit
    Call CommitDB("Paste")
    
    'Copy item(s)
    Call CopyList(sCopyKey, sCurKey, sParKey, bDelFlag, sCopyFile)
End Sub

Private Sub mnuPUTreeMisProp_Click()
    frmMission.Show
    frmMission.GetMission
    frmMission.SetFocus
End Sub

Private Sub mnuPUTreeObjCopy_Click()
    'Set copy key and delete flag
    sCopyKey = sCurKey
    bDelFlag = False
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUTreeObjCut_Click()
    'Set copy key and delete flag
    sCopyKey = sCurKey
    bDelFlag = True
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUTreeObjDel_Click()
    'Commit
    Call CommitDB("Delete Object")
    
    'Del object
    Call frmObjects.DelObject(sParKey, sCurKey, "")
End Sub

Private Sub mnuPUTreeObjFind_Click()
    Call mnuEditFind_Click
End Sub

Private Sub mnuPUTreeObjGen_Click()
    Dim nType As Integer
    
    Dim nPos As Long

    Dim sFile As String
    Dim sExt As String
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check key
    If sCurKey = "" Then Exit Sub
    If Left(sCurKey, 1) <> "o" Then Exit Sub
    If Left(sParKey, 1) <> "l" Then Exit Sub
    
    'Get extension
    nType = frmObjects.GetType(Val(Mid(sCurKey, 2)))
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_EXT, sExt, MIS_MOD_CFG)
    sExt = TruncStr(sExt)
    If sExt = "" Then Exit Sub
    
    'Get file
    sFile = frmObjects.GetFile(sCurKey, -1)

    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show common dialog
    cdMission.FileName = sFile
    cdMission.InitDir = sDataDir
    cdMission.DefaultExt = sExt
    cdMission.DialogTitle = "Generate Object File"
    cdMission.Filter = "Object Files (*" + sExt + ")|*" + sExt + "|All Files|*.*"
    cdMission.ShowSave
    sFile = cdMission.FileName
    
    'Reset handler for Cancel button
    On Error GoTo 0
    
    'Commit
    Call CommitDB("Generate Object")
    
    'Put file
    Call frmObjects.PutFile(sCurKey, sFile)
    
    'Generate object
    Call frmObjects.GenObject(sCurKey, sFile)
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub mnuPUTreeObjIns_Click()
    frmObjects.Show
    Call frmObjects.InsObject(sParKey, sCurKey, aCursor(0), aCursor(1), aCursor(2))
    frmObjects.SetFocus
End Sub

Private Sub mnuPUTreeObjList_Click()
    frmList.Show
    Call frmList.GetList(sCurKey, "")
    frmList.SetFocus
End Sub

Private Sub mnuPUTreeObjNew1_Click()
    frmObjects.Show
    Call frmObjects.GetObject(sCurKey, "", aCursor(0), aCursor(1), aCursor(2))
    frmObjects.SetFocus
End Sub

Private Sub mnuPUTreeObjNew2_Click()
    frmAttribs.Show
    Call frmAttribs.GetAttrib(sCurKey, "")
    frmAttribs.SetFocus
End Sub

Private Sub mnuPUTreeObjPaste_Click()
    'Get data from clipboard
    FromClipboard
    
    ' Check copy key
    If InStr(sCopyKey, "l") > 0 Then Exit Sub
    
    'Commit
    Call CommitDB("Paste")
    
    'Copy item(s)
    Call CopyList(sCopyKey, sCurKey, sParKey, bDelFlag, sCopyFile)
End Sub

Private Sub mnuPUTreeObjProp_Click()
    frmObjects.Show
    Call frmObjects.GetObject(sParKey, sCurKey, 0, 0, 0)
    frmObjects.SetFocus
End Sub

Private Sub mnuPUTreeObjSel1_Click()
    mnuPUGraphObjSel1_Click
End Sub

Private Sub mnuPUTreeObjSel2_Click()
    mnuPUGraphObjSel2_Click
End Sub

Private Sub mnuPUTreeObjSel3_Click()
    mnuPUGraphObjSel3_Click
End Sub

Private Sub mnuPUTreeSelCollapse_Click()
    'Collapse tree
    Call frmTree.ExpandTree(False)
End Sub

Private Sub mnuPUTreeSelCopy_Click()
    'Set copy key and delete flag
    sCopyKey = sListKey
    bDelFlag = False
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUTreeSelCut_Click()
    'Set copy key and delete flag
    sCopyKey = sListKey
    bDelFlag = True
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuPUTreeSelDel_Click()
    'Commit
    Call CommitDB("Delete")
    
    'Delete item(s)
    Call DelList(sParKey, sListKey, "")
End Sub

Private Sub mnuPUTreeSelDesel_Click()
    'Deselect key
    frmTree.DelKey
End Sub

Private Sub mnuPUTreeSelExpand_Click()
    'Expand tree
    Call frmTree.ExpandTree(True)
End Sub

Private Sub mnuPUTreeSelProp_Click()
    'Get list property
    GetListProp
End Sub

Private Sub mnuPUTreeSelClear_Click()
    'Set key
    frmTree.ShowList (sCurKey)
End Sub

Private Sub mnuToolsAngle_Click()
    If mnuToolsAngle.Checked Then
        bRotFlag = 0
        frmOptions.ShowOptions
        mnuToolsAngle.Checked = False
    Else
        bRotFlag = 1
        frmOptions.ShowOptions
        mnuToolsAngle.Checked = True
    End If
    
    'Show status
    ShowStatus (sCurKey)
End Sub

Private Sub mnuToolsCamera_Click()
    If mnuToolsCamera.Checked Then
        bCamFlag = 0
        frmOptions.ShowOptions
        mnuToolsCamera.Checked = False
    Else
        bCamFlag = 1
        frmOptions.ShowOptions
        mnuToolsCamera.Checked = True
    End If
    
    'Show status
    ShowStatus (sCurKey)
End Sub

Private Sub mnuToolsFind1_Click()
    'Set offset
    aOffset(0) = -aCursor(0) * fViewScale
    aOffset(1) = -aCursor(1) * fViewScale
    aOffset(2) = -aCursor(2) * fViewScale
    
    'Refresh
    frmFront.SetView (True)
    frmTop.SetView (True)
    frmSide.SetView (True)
End Sub

Private Sub mnuToolsFind2_Click()
    'Set offset
    aOffset(0) = 0
    aOffset(1) = 0
    aOffset(2) = 0
    
    'Set focus
    aFocus(0) = 0
    aFocus(1) = 0
    aFocus(2) = 0
    
    'Refresh
    frmFront.SetView (False)
    frmFront.SetCamera (True)
    frmTop.SetView (False)
    frmTop.SetCamera (True)
    frmSide.SetView (False)
    frmSide.SetCamera (True)
    frmCamera.SetCamera (True)
End Sub

Private Sub mnuToolsGrid_Click()
    If mnuToolsGrid.Checked Then
        bGridFlag = 0
        frmOptions.ShowOptions
        mnuToolsGrid.Checked = False
    Else
        bGridFlag = 1
        frmOptions.ShowOptions
        mnuToolsGrid.Checked = True
    End If
    
    'Show status
    ShowStatus (sCurKey)
End Sub

Private Sub mnuToolsScale_Click()
    If mnuToolsScale.Checked Then
        bScaleFlag = 0
        frmOptions.ShowOptions
        mnuToolsScale.Checked = False
    Else
        bScaleFlag = 1
        frmOptions.ShowOptions
        mnuToolsScale.Checked = True
    End If
    
    'Show status
    ShowStatus (sCurKey)
End Sub

Private Sub mnuToolsStatusBar_Click()
    If mnuToolsStatusBar.Checked Then
        bStatFlag = 0
        sbStatusBar.Visible = False
        mnuToolsStatusBar.Checked = False
    Else
        bStatFlag = 1
        sbStatusBar.Visible = True
        mnuToolsStatusBar.Checked = True
    End If
End Sub

Private Sub mnuToolsToolbar_Click()
    If mnuToolsToolbar.Checked Then
        bToolFlag = 0
        tbToolBar.Visible = False
        mnuToolsToolbar.Checked = False
    Else
        bToolFlag = 1
        tbToolBar.Visible = True
        mnuToolsToolbar.Checked = True
    End If
End Sub

Private Sub mnuToolsOptions_Click()
    frmOptions.Show
    frmOptions.SetFocus
End Sub

Private Sub mnuViewDock_Click()
    If mnuViewDock.Checked Then
        mnuViewDock.Checked = False
    Else
        mnuViewDock.Checked = True
    End If
End Sub

Private Sub mnuViewGraphCamera_Click()
    If mnuViewGraphCamera.Checked Then
        Unload frmCamera
    Else
        frmCamera.Show
        frmCamera.SetFocus
    End If
End Sub

Private Sub mnuViewGraphFront_Click()
    If mnuViewGraphFront.Checked Then
        Unload frmFront
    Else
        frmFront.Show
        frmFront.SetFocus
    End If
End Sub

Private Sub mnuViewGraphTop_Click()
    If mnuViewGraphTop.Checked Then
        Unload frmTop
    Else
        frmTop.Show
        frmTop.SetFocus
    End If
End Sub

Private Sub mnuViewGraphSide_Click()
    If mnuViewGraphSide.Checked Then
        Unload frmSide
    Else
        frmSide.Show
        frmSide.SetFocus
    End If
End Sub

Private Sub mnuViewList_Click()
    If mnuViewList.Checked Then
        Unload frmList
    Else
        frmList.Show
        frmList.SetFocus
    End If
End Sub

Private Sub mnuViewTree_Click()
    If mnuViewTree.Checked Then
        Unload frmTree
    Else
        frmTree.Show
        frmTree.GetTree (sListKey)
        frmTree.SetFocus
    End If
End Sub

Private Sub mnuViewRefresh_Click()
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Refresh tree
    frmTree.GetTree (sListKey)
    
    'Re-initialize renderer
    rendClean
    rendInit
    frmObjects.GetObjects
    Call rendSetSel("o", sListKey)
       
    'Show views
    frmCamera.Render
    frmFront.Render
    frmTop.Render
    frmSide.Render
    
    'Refresh layers
    frmLayers.GetLayers (0)
    
    'Refresh list
    Call frmList.GetList(sCurKey, "")
    
    'Set focus
    frmTree.SetFocus
End Sub

Private Sub mnuViewTabAttrib_Click()
    If mnuViewTabAttrib.Checked Then
        Unload frmAttribs
    Else
        frmAttribs.Show
        frmAttribs.SetFocus
    End If
End Sub

Private Sub mnuViewTabLayer_Click()
    If mnuViewTabLayer.Checked Then
        Unload frmLayers
    Else
        frmLayers.Show
        frmLayers.GetLayers (0)
        frmLayers.SetFocus
    End If
End Sub

Private Sub mnuViewTabLevel_Click()
    If mnuViewTabLevel.Checked Then
        Unload frmLevels
    Else
        frmLevels.Show
        frmLevels.SetFocus
    End If
End Sub

Private Sub mnuViewTabMission_Click()
    If mnuViewTabMission.Checked Then
        Unload frmMission
    Else
        frmMission.Show
        frmMission.SetFocus
    End If
End Sub

Private Sub mnuViewTabObject_Click()
    If mnuViewTabObject.Checked Then
        Unload frmObjects
    Else
        frmObjects.Show
        frmObjects.SetFocus
    End If
End Sub

Private Sub mnuWindowDefaultLayout_Click()
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Hide other views
    Unload frmList
    Unload frmLayers
    Unload frmMission
    Unload frmLevels
    Unload frmObjects
    Unload frmAttribs
    
    'Reset and show tree view
    frmTree.Reset
    frmTree.Show
    
    'Reset views
    frmCamera.Reset
    frmFront.Reset
    frmTop.Reset
    frmSide.Reset
    
    'Show views
    frmCamera.Show
    frmFront.Show
    frmTop.Show
    frmSide.Show
    
    'Set focus
    frmTree.SetFocus
End Sub

Private Sub tbToolBar_ButtonClick(ByVal Button As ComctlLib.Button)
    Select Case Button.Key
        Case "Open"
            mnuFileOpen_Click
        Case "Generate"
            mnuFileGenerate_Click
        Case "Cut"
            mnuEditCut_Click
        Case "Copy"
            mnuEditCopy_Click
        Case "Paste"
            mnuEditPaste_Click
    End Select
End Sub

Private Sub mnuWindowArrangeIcons_Click()
    Me.Arrange vbArrangeIcons
End Sub

Private Sub mnuWindowCascade_Click()
    Me.Arrange vbCascade
End Sub

Private Sub mnuWindowTileHorizontal_Click()
    Me.Arrange vbTileHorizontal
End Sub

Private Sub mnuWindowTileVertical_Click()
    Me.Arrange vbTileVertical
End Sub

Private Sub mnuEditProp_Click()
    'Get list property
    GetListProp
End Sub

Private Sub mnuEditNew_Click()
    ' Check key
    If Left(sCurKey, 1) = "a" Then Exit Sub
    
    ' Check key
    If Left(sCurKey, 1) = "m" Then
        'New level
        frmLevels.Show
        Call frmLevels.GetLevel(sCurKey, "")
        frmLevels.SetFocus
        Exit Sub
    End If
    
    ' Check key
    If Left(sCurKey, 1) = "l" Then
        'New object
        frmObjects.Show
        Call frmObjects.GetObject(sCurKey, "", aCursor(0), aCursor(1), aCursor(2))
        frmObjects.SetFocus
        Exit Sub
    End If
        
    ' Check parent key
    If Left(sParKey, 1) = "o" Then
        'New attrib
        frmAttribs.Show
        Call frmAttribs.GetAttrib(sCurKey, "")
        frmAttribs.SetFocus
        Exit Sub
    End If
    
    ' Check key
    If Left(sCurKey, 1) = "o" Then
        'New object
        frmObjects.Show
        Call frmObjects.GetObject(sCurKey, "", aCursor(0), aCursor(1), aCursor(2))
        frmObjects.SetFocus
    End If
End Sub

Private Sub mnuEditDel_Click()
    'Commit
    Call CommitDB("Delete")
    
    'Delete item(s)
    Call DelList(sParKey, sListKey, "")
End Sub

Private Sub mnuEditDup_Click()
    ' Check key
    If Left(sCurKey, 1) = "m" Then Exit Sub
    
    ' Check key
    If Left(sCurKey, 1) = "l" Then
        'Commit
        Call CommitDB("Duplicate Level")
        
        'Duplicate level
        Call frmLevels.CopyLevel(sCurKey, sParKey, False, "")
        Exit Sub
    End If
           
    ' Check key
    If Left(sCurKey, 1) = "a" Then
        'Commit
        Call CommitDB("Duplicate Attribute")
        
        'Duplicate attribute
        Call frmAttribs.CopyAttrib(sCurKey, sParKey, False, "")
        Exit Sub
    End If
        
    'Commit
    Call CommitDB("Duplicate Objects")
    
    'Duplicate object
    Call frmObjects.DupObjects(aMouse(0), aMouse(1), aMouse(2))
End Sub

Private Sub mnuEditCopy_Click()
    'Set copy key and delete flag
    sCopyKey = sListKey
    bDelFlag = False
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuEditCut_Click()
    'Set copy key and delete flag
    sCopyKey = sListKey
    bDelFlag = True
    sCopyFile = ""
    
    'Put data in clipboard
    ToClipboard
End Sub

Private Sub mnuEditPaste_Click()
    'Get data from clipboard
    FromClipboard
    
    'Commit
    Call CommitDB("Paste")
    
    'Copy item(s)
    Call CopyList(sCopyKey, sCurKey, sParKey, bDelFlag, sCopyFile)
End Sub

Private Sub mnuFileOpen_Click()
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show common dialog
    If sDBFile <> "" Then cdMission.FileName = Dir(sDBFile)
    cdMission.InitDir = sWDir
    cdMission.DialogTitle = "Open Mission Database"
    cdMission.Filter = "Mission Database Files (*" + MIS_EXT_DB + ")|*" + MIS_EXT_DB + "|All Files|*.*"
    cdMission.ShowOpen
    
    'Reset handler for Cancel button
    On Error GoTo 0
    
    'Open file
    Call OpenFile(cdMission.FileName)
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub mnuFileClose_Click()
    'Close file
    Call CloseFile
End Sub

Private Sub mnuFileGenerate_Click()
    ' Check key
    If Left(sCurKey, 1) = "m" Then
        mnuPUTreeMisGen_Click
        Exit Sub
    End If
    
    ' Check key
    If Left(sCurKey, 1) = "l" Then
        mnuPUTreeLevGen_Click
        Exit Sub
    End If
        
    ' Check key
    If Left(sCurKey, 1) = "o" Then
        mnuPUTreeObjGen_Click
        Exit Sub
    End If
End Sub

Private Sub mnuFileExit_Click()
    'End program
    Unload Me
End Sub

