VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmObjects 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Object"
   ClientHeight    =   5010
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4665
   Icon            =   "frmObjects.frx":0000
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   ScaleHeight     =   5010
   ScaleWidth      =   4665
   Begin VB.CheckBox chkLayer 
      Caption         =   "Check1"
      Height          =   192
      Left            =   720
      TabIndex        =   35
      ToolTipText     =   "Select by Layer"
      Top             =   1560
      Width           =   132
   End
   Begin VB.CheckBox chkName 
      Caption         =   "Check1"
      Height          =   192
      Left            =   720
      TabIndex        =   34
      ToolTipText     =   "Select By Name"
      Top             =   660
      Width           =   132
   End
   Begin VB.CheckBox chkType 
      Caption         =   "Check1"
      Height          =   192
      Left            =   720
      TabIndex        =   33
      ToolTipText     =   "Select By Type"
      Top             =   180
      Width           =   132
   End
   Begin VB.TextBox txtSizeX 
      Height          =   285
      Left            =   1020
      TabIndex        =   15
      Top             =   4020
      Width           =   1095
   End
   Begin VB.TextBox txtSizeY 
      Height          =   285
      Left            =   2220
      TabIndex        =   16
      Top             =   4020
      Width           =   1095
   End
   Begin VB.TextBox txtSizeZ 
      Height          =   285
      Left            =   3420
      TabIndex        =   17
      Top             =   4020
      Width           =   1095
   End
   Begin VB.TextBox txtScaleX 
      Height          =   285
      Left            =   1020
      TabIndex        =   12
      Top             =   3600
      Width           =   1095
   End
   Begin VB.TextBox txtScaleY 
      Height          =   285
      Left            =   2220
      TabIndex        =   13
      Top             =   3600
      Width           =   1095
   End
   Begin VB.TextBox txtScaleZ 
      Height          =   285
      Left            =   3420
      TabIndex        =   14
      Top             =   3600
      Width           =   1095
   End
   Begin VB.ComboBox cmbLayer 
      Height          =   315
      Left            =   1020
      Sorted          =   -1  'True
      Style           =   2  'Dropdown List
      TabIndex        =   3
      Top             =   1500
      Width           =   3495
   End
   Begin VB.TextBox txtFile 
      Height          =   285
      Left            =   1200
      OLEDropMode     =   1  'Manual
      TabIndex        =   5
      Top             =   1980
      Width           =   3315
   End
   Begin VB.CommandButton cmdBrowse 
      Caption         =   "4"
      BeginProperty Font 
         Name            =   "Webdings"
         Size            =   8.25
         Charset         =   2
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   1020
      TabIndex        =   4
      Tag             =   "Apply"
      ToolTipText     =   "Browse"
      Top             =   1980
      Width           =   195
   End
   Begin VB.TextBox txtPosX 
      Height          =   285
      Left            =   1020
      TabIndex        =   6
      Top             =   2760
      Width           =   1095
   End
   Begin VB.TextBox txtPosY 
      Height          =   285
      Left            =   2220
      TabIndex        =   7
      Top             =   2760
      Width           =   1095
   End
   Begin VB.TextBox txtPosZ 
      Height          =   285
      Left            =   3420
      TabIndex        =   8
      Top             =   2760
      Width           =   1095
   End
   Begin VB.TextBox txtRotZ 
      Height          =   285
      Left            =   3420
      TabIndex        =   11
      Top             =   3180
      Width           =   1095
   End
   Begin VB.TextBox txtRotY 
      Height          =   285
      Left            =   2220
      TabIndex        =   10
      Top             =   3180
      Width           =   1095
   End
   Begin VB.TextBox txtRotX 
      Height          =   285
      Left            =   1020
      TabIndex        =   9
      Top             =   3180
      Width           =   1095
   End
   Begin VB.ComboBox cmbName 
      Height          =   315
      Left            =   1020
      Sorted          =   -1  'True
      TabIndex        =   1
      Top             =   600
      Width           =   3495
   End
   Begin MSComDlg.CommonDialog cdBrowse 
      Left            =   120
      Top             =   4440
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      CancelError     =   -1  'True
      DialogTitle     =   "Open File"
      Filter          =   "All Files|*.*"
   End
   Begin VB.ComboBox cmbType 
      Height          =   315
      Left            =   1020
      Sorted          =   -1  'True
      Style           =   2  'Dropdown List
      TabIndex        =   0
      Top             =   120
      Width           =   3495
   End
   Begin VB.TextBox txtInfo 
      Height          =   285
      Left            =   1020
      TabIndex        =   2
      Top             =   1080
      Width           =   3495
   End
   Begin VB.CommandButton cmdApply 
      Caption         =   "&Apply"
      Height          =   375
      Left            =   3420
      TabIndex        =   20
      Tag             =   "Apply"
      Top             =   4500
      Width           =   1095
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   2220
      TabIndex        =   19
      Tag             =   "Cancel"
      Top             =   4500
      Width           =   1095
   End
   Begin VB.CommandButton cmdOK 
      Caption         =   "&OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   1020
      TabIndex        =   18
      Tag             =   "OK"
      Top             =   4500
      Width           =   1095
   End
   Begin VB.Label lblSize 
      Caption         =   "Size:"
      Height          =   195
      Left            =   120
      TabIndex        =   32
      Top             =   4080
      Width           =   825
   End
   Begin VB.Label lblScale 
      Caption         =   "Scale:"
      Height          =   195
      Left            =   120
      TabIndex        =   31
      Top             =   3660
      Width           =   795
   End
   Begin VB.Label lblLayer 
      Caption         =   "Layer:"
      Height          =   192
      Left            =   120
      TabIndex        =   30
      Top             =   1560
      Width           =   552
   End
   Begin VB.Label lblFile 
      Caption         =   "File:"
      Height          =   195
      Left            =   120
      TabIndex        =   29
      Top             =   2040
      Width           =   795
   End
   Begin VB.Label lblPos 
      Caption         =   "Position:"
      Height          =   195
      Left            =   120
      TabIndex        =   28
      Top             =   2820
      Width           =   795
   End
   Begin VB.Label lblRot 
      Caption         =   "Rotation:"
      Height          =   195
      Left            =   120
      TabIndex        =   27
      Top             =   3240
      Width           =   795
   End
   Begin VB.Label lblX 
      Alignment       =   2  'Center
      Caption         =   "x"
      Height          =   195
      Left            =   1020
      TabIndex        =   26
      Top             =   2460
      Width           =   1095
   End
   Begin VB.Label lblY 
      Alignment       =   2  'Center
      Caption         =   "y"
      Height          =   195
      Left            =   2220
      TabIndex        =   25
      Top             =   2460
      Width           =   1095
   End
   Begin VB.Label lblZ 
      Alignment       =   2  'Center
      Caption         =   "z"
      Height          =   195
      Left            =   3420
      TabIndex        =   24
      Top             =   2460
      Width           =   1095
   End
   Begin VB.Label lplType 
      Caption         =   "Type:"
      Height          =   192
      Left            =   120
      TabIndex        =   23
      Top             =   180
      Width           =   552
   End
   Begin VB.Label lblInfo 
      Caption         =   "Info:"
      Height          =   195
      Left            =   120
      TabIndex        =   22
      Top             =   1140
      Width           =   795
   End
   Begin VB.Label lblName 
      Caption         =   "Name:"
      Height          =   192
      Left            =   120
      TabIndex        =   21
      Top             =   660
      Width           =   552
   End
End
Attribute VB_Name = "frmObjects"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim bSel As Boolean

Dim bSize As Boolean
Dim bScale As Boolean
Dim aPos(4) As Single

Dim sParK As String
Dim sCurK As String
Dim sRefK As String
Dim sListK As String

Sub InsObject(ByVal sCKey As String, ByVal sRKey As String, ByVal X As Single, ByVal Y As Single, ByVal Z As Single)
    'Get object
    Call GetObject(sCKey, "", X, Y, Z)
        
    'Set reference key
    sRefK = sRKey
End Sub
Sub GetObject(ByVal sPKey As String, ByVal sLKey As String, ByVal X As Single, ByVal Y As Single, ByVal Z As Single)
    Dim nType As Integer
    Dim nInd As Integer
    
    Dim nPos As Long
    
    Dim sCKey As String
    Dim sQuery As String
    Dim sTxt As String
    Dim sVal As String
    
    Dim rsTemp As Recordset
         
    'Reset caption
    Me.Caption = "Object"
    
    'Show size controls
    lblSize.Visible = True
    txtSizeX.Visible = True
    txtSizeY.Visible = True
    txtSizeZ.Visible = True
    
    'Reset color
    txtPosX.BackColor = vbWindowBackground
    txtPosY.BackColor = vbWindowBackground
    txtPosZ.BackColor = vbWindowBackground
    txtScaleX.BackColor = vbWindowBackground
    txtScaleY.BackColor = vbWindowBackground
    txtScaleZ.BackColor = vbWindowBackground
    txtRotX.BackColor = vbWindowBackground
    txtRotY.BackColor = vbWindowBackground
    txtRotZ.BackColor = vbWindowBackground
    cmbType.BackColor = vbWindowBackground
    cmbLayer.BackColor = vbWindowBackground
    cmbName.BackColor = vbWindowBackground
    txtInfo.BackColor = vbWindowBackground
    txtFile.BackColor = vbWindowBackground
            
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Set local keys
    sParK = sPKey
    sListK = sLKey
    sCurK = ""
    
    'Check keys
    If sParK = "" Then
        'Set selection flag
        bSel = True
        
        'Set caption
        Me.Caption = "Object selection"
        
        'Hide size controls
        lblSize.Visible = False
        txtSizeX.Visible = False
        txtSizeY.Visible = False
        txtSizeZ.Visible = False
    Else
        'Clear slection flag
        bSel = False
            
        'Check parent
        If Left(sParK, 1) = "l" Then
            'Set caption
            Me.Caption = "Object of " + frmLevels.GetName(Val(Mid(sParK, 2)))
        End If
        
        'Check parent
        If Left(sParK, 1) = "o" Then
            'Get name
            sTxt = frmObjects.GetName(Val(Mid(sParK, 2)))
        
            'Get type
            nType = frmObjects.GetType(Val(Mid(sParK, 2)))
            Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_TYPE, sVal, MIS_MOD_CFG)
            
            'Truncate type
            sVal = TruncStr(sVal)
            
            'Check type
            If sVal <> "" Then sTxt = sVal + ": " + sTxt
                    
            'Set caption
            Me.Caption = "Object of " + sTxt
        End If
    End If
    
    'Get types
    GetTypes

    'Get layers
    GetLayers
    
    'Check key
    If sListK = "" Then
        If bGridFlag = 1 Then
            'Snap x translation to grid
            If Abs(X) Mod fGridSize < fGridSize / 2 Then
                X = -(X Mod fGridSize) + X
            Else
                X = Sgn(X) * fGridSize - (X Mod fGridSize) + X
            End If
        
            'Snap y translation to grid
            If Abs(Y) Mod fGridSize < fGridSize / 2 Then
                Y = -(Y Mod fGridSize) + Y
            Else
                Y = Sgn(Y) * fGridSize - (Y Mod fGridSize) + Y
            End If
        
            'Snap y translation to grid
            If Abs(Z) Mod fGridSize < fGridSize / 2 Then
                Z = -(Z Mod fGridSize) + Z
            Else
                Z = Sgn(Z) * fGridSize - (Z Mod fGridSize) + Z
            End If
        End If
    
        'Put default data in controls
        txtPosX.Text = Format(X, "0;-#")
        txtPosY.Text = Format(Y, "0;-#")
        txtPosZ.Text = Format(Z, "0;-#")
        txtScaleX.Text = "100"
        txtScaleY.Text = "100"
        txtScaleZ.Text = "100"
        txtRotX.Text = "0"
        txtRotY.Text = "0"
        txtRotZ.Text = "0"
        SelType (0)
        SelLayer (0)
        cmbName.Text = MIS_NAM_OBJ
        txtInfo.Text = ""
        txtFile.Text = ""
    
        'Get Names
        Call GetNames(cmbName.Text, cmbType.ItemData(cmbType.ListIndex))
        Exit Sub
    End If
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Sub
    
    'Reset index
    nInd = 0
    
    'Loop thru types
    Do
        'Get position of space character in string
        nPos = InStr(sLKey, " ")
        
        'If possible, truncate string at space character
        If nPos > 0 Then
            'Set key
            sCKey = Left(sLKey, nPos - 1)
            sLKey = Mid(sLKey, nPos + 1, Len(sLKey))
        Else
            'Set key
            sCKey = sLKey
        End If
        
        'Check key
        If Left(sCKey, 1) = "o" Then
            'Set query
            sQuery = "SELECT * FROM Objects WHERE Key = " + Mid(sCKey, 2)
            
            'Open temporary recordset by query
            If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
                
            'Get data from recordset
            rsTemp.MoveFirst
            
            'Check index
            If nInd = 0 Then
                'Set current key
                sCurK = sCKey
                
                'Put data in controls
                txtPosX.Text = Format(rsTemp!PosX, "0;-#")
                txtPosY.Text = Format(rsTemp!PosY, "0;-#")
                txtPosZ.Text = Format(rsTemp!PosZ, "0;-#")
                txtScaleX.Text = Format(rsTemp!ScaleX * 100, "0;-#")
                txtScaleY.Text = Format(rsTemp!ScaleY * 100, "0;-#")
                txtScaleZ.Text = Format(rsTemp!ScaleZ * 100, "0;-#")
                txtRotX.Text = Format(rsTemp!RotX, "0;-#")
                txtRotY.Text = Format(rsTemp!RotY, "0;-#")
                txtRotZ.Text = Format(rsTemp!RotZ, "0;-#")
                SelType (rsTemp!Type)
                SelLayer (rsTemp!Layer)
                cmbName.Text = rsTemp!Name
                txtInfo.Text = rsTemp!Info
                txtFile.Text = rsTemp!File
        
                'Get Names
                Call GetNames(cmbName.Text, cmbType.ItemData(cmbType.ListIndex))
            Else
                'Compare data and set colors
                If txtPosX.Text <> Format(rsTemp!PosX, "0;-#") Then txtPosX.BackColor = vbButtonFace
                If txtPosY.Text <> Format(rsTemp!PosY, "0;-#") Then txtPosY.BackColor = vbButtonFace
                If txtPosZ.Text <> Format(rsTemp!PosZ, "0;-#") Then txtPosZ.BackColor = vbButtonFace
                If txtScaleX.Text <> Format(rsTemp!ScaleX * 100, "0;-#") Then txtScaleX.BackColor = vbButtonFace
                If txtScaleY.Text <> Format(rsTemp!ScaleY * 100, "0;-#") Then txtScaleY.BackColor = vbButtonFace
                If txtScaleZ.Text <> Format(rsTemp!ScaleZ * 100, "0;-#") Then txtScaleZ.BackColor = vbButtonFace
                If txtRotX.Text <> Format(rsTemp!RotX, "0;-#") Then txtRotX.BackColor = vbButtonFace
                If txtRotY.Text <> Format(rsTemp!RotY, "0;-#") Then txtRotY.BackColor = vbButtonFace
                If txtRotZ.Text <> Format(rsTemp!RotZ, "0;-#") Then txtRotZ.BackColor = vbButtonFace
                If cmbType.ItemData(cmbType.ListIndex) <> rsTemp!Type Then cmbType.BackColor = vbButtonFace
                If cmbLayer.ItemData(cmbLayer.ListIndex) <> rsTemp!Layer Then cmbLayer.BackColor = vbButtonFace
                If cmbName.Text <> rsTemp!Name Then cmbName.BackColor = vbButtonFace
                If txtInfo.Text <> rsTemp!Info Then txtInfo.BackColor = vbButtonFace
                If txtFile.Text <> rsTemp!File Then txtFile.BackColor = vbButtonFace
            End If
            
            'Close temporary recordset
            rsTemp.Close
            
            'Increment index
            nInd = nInd + 1
        End If
            
        'Check position
        If nPos = 0 Then Exit Do
    Loop
End Sub

Sub GetObjects()
    Dim nObj As Long
    Dim nLink As Long
    Dim nMode As Long
    Dim nCol As Long

    Dim sTxt As String
    Dim sVal As String
    Dim sFile As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Objects"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Set progress bar
    Call frmProgress.Init("Loading graphics...", 100)
    
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Update progress bar
        Call frmProgress.Update(rsTemp.PercentPosition)
    
        'Get name
        sTxt = rsTemp!Name
        
        'Get type
        Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(rsTemp!Type - 1)), MIS_KEY_TYPE, sVal, MIS_MOD_CFG)
        sVal = TruncStr(sVal)
        If sVal <> "" Then sTxt = sVal + ": " + sTxt
        
        'Get full filename
        sFile = rsTemp!File
        
        'Check filename
        If sFile <> "" Then
            If InStr(sFile, ":") = 0 Then
                'Validate and hash file
                If Dir(sObjDir + sFile) = "" Then sFile = Trim(Str(misGetHash(rsTemp!File))) + MIS_EXT_OBJ
                
                'Re=validate and update file according to config
                If Dir(sObjDir + sFile) = "" Then
                    sFile = CheckFile(rsTemp!Type, rsTemp!Name, rsTemp!File)
                
                    'Compare file
                    If (sFile <> rsTemp!File) Then
                        'Update data
                        rsTemp.Edit
                        rsTemp!File = sFile
                        rsTemp.Update
                    End If
                    
                    'Validate and hash file
                    If Dir(sObjDir + sFile) = "" Then sFile = Trim(Str(misGetHash(rsTemp!File))) + MIS_EXT_OBJ
                End If
                
                'Append filename to object dir
                sFile = sObjDir + sFile
            End If
        End If
            
        'Create object
        If rendNewObj(nObj, rsTemp!Key, sFile) Then Call MsgBox("DLL error: Unable to create object!", vbOKOnly Or vbExclamation, "MissionMan")
    
        'Set mode and color
        Call frmLayers.GetColor(rsTemp!Layer, nMode, nCol)
        Call rendSetObjMode(nObj, nMode)
        Call rendSetObjCol(nObj, nCol)

        'Position object
        Call rendRotObj(nObj, rsTemp!RotX, rsTemp!RotY, rsTemp!RotZ)
        Call rendScaleObj(nObj, rsTemp!ScaleX, rsTemp!ScaleY, rsTemp!ScaleZ)
        Call rendTransObj(nObj, rsTemp!PosX, rsTemp!PosY, rsTemp!PosZ)
    
        'Set link
        Call rendFindObj(nLink, GetLink(rsTemp!Key, rsTemp!Level, rsTemp!Object, rsTemp!Type))
        Call rendSetObjLink(nObj, nLink)
        
        'Continue
        rsTemp.MoveNext
    Loop
    
    'Reset progress bar
    Call frmProgress.Clean
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Function GetLink(ByVal nKey As Long, ByVal nLev As Long, ByVal nObj As Long, ByVal nType As Integer) As Long
    Dim nRKey As Long
    
    Dim sVal As String
    Dim sName As String
    Dim sRVal As String
    Dim sInfo As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Set default
    GetLink = 0
    
    'Get link
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(GetType(nKey) - 1)), MIS_KEY_LINK, sVal, MIS_MOD_CFG)
    sVal = TruncStr(sVal)
    If sVal = "" Then Exit Function
    sName = sVal
    
    'Get value
    Call frmAttribs.GetAll("o" + Trim(Str(nKey)), sName, nRKey, sVal, sInfo)
    If sVal = "" Then Exit Function
    sRVal = sVal
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Key < " + Str(nKey) + " AND Level = " + Str(nLev) + " AND Object = " + Str(nObj) + " AND Type = " + Str(nType) + " ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Reset key
        nRKey = 0
    
        'Get value
        Call frmAttribs.GetAll("o" + Trim(Str(rsTemp!Key)), sName, nRKey, sVal, sInfo)
        If sVal = sRVal Then GetLink = rsTemp!Key
        
        'Continue
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Function

Sub GetAll(ByVal nKey As Long, nLev As Long, nObj As Long, nType As Integer)
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Reset all
    nLev = 0
    nObj = 0
    nType = 0
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Key = " + Trim(Str(nKey))
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Get data from recordset
    rsTemp.MoveFirst
    nLev = rsTemp!Level
    nObj = rsTemp!Object
    nType = rsTemp!Type
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Function GetType(ByVal nKey As Long) As Integer
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Set default
    GetType = 0
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Key = " + Trim(Str(nKey))
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    GetType = rsTemp!Type
    
    'Close temporary recordset
    rsTemp.Close
End Function

Sub GetTypes()
    Dim n As Integer

    Dim nCount As Integer
    
    Dim nPos As Long
    
    Dim sList As String
    
    'Clear combo
    cmbType.Clear
    
    'Set blank name and key
    cmbType.AddItem ("(None)")
    cmbType.ItemData(cmbType.NewIndex) = 0
    
    'Reset count
    nCount = 0
    
    'Get types
    Call misGetListBySection(frmMission.GetPrefix + MIS_SEC_OBJ, MIS_KEY_TYPE, sList, nCount, MIS_MOD_CFG)
    
    'Check count
    If nCount = 0 Then Exit Sub
    
    'Truncate types
    sList = TruncStr(sList)

    'Loop thru types
    For n = 0 To nCount - 1
        'Get position of | character in string
        nPos = InStr(sList, "|")
        
        'If possible, truncate string at | character
        If nPos > 0 Then
            'Add type to combo
            cmbType.AddItem (Left(sList, nPos - 1))
            cmbType.ItemData(cmbType.NewIndex) = n + 1
            sList = Mid(sList, nPos + 1, Len(sList))
        Else
            'Add type to combo
            cmbType.AddItem (sList)
            cmbType.ItemData(cmbType.NewIndex) = n + 1
        End If
    Next n
    
    'Reset combo index
    If cmbType.ListCount > 0 Then cmbType.ListIndex = 0
End Sub

Sub GetLayers()
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Clear combo
    cmbLayer.Clear
    
    'Set blank name and key
    cmbLayer.AddItem ("(None)")
    cmbLayer.ItemData(cmbLayer.NewIndex) = 0
    
    'Check recordset
    If rsLayers.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Layers"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Find data in recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Get layer data
        cmbLayer.AddItem (rsTemp!Name)
        cmbLayer.ItemData(cmbLayer.NewIndex) = rsTemp!Key
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
    
    'Reset combo index
    If cmbLayer.ListCount > 0 Then cmbLayer.ListIndex = 0
End Sub

Function GetName(ByVal nKey As Long) As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Set default
    GetName = ""
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Key = " + Trim(Str(nKey))
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    GetName = rsTemp!Name
    
    'Close temporary recordset
    rsTemp.Close
End Function

Sub GetNames(ByVal sName As String, ByVal nType As Integer)
    Dim n As Integer

    Dim nCount As Integer
    
    Dim nPos As Long
    
    Dim sList As String
    
    'Clear combo
    cmbName.Clear
    cmbName.Text = sName
        
    'Reset count
    nCount = 0
    
    'Get Names
    Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_NAME, sList, nCount, MIS_MOD_CFG)
    
    'Check count
    If nCount = 0 Then Exit Sub
    
    'Truncate names
    sList = TruncStr(sList)

    'Loop thru names
    For n = 0 To nCount - 1
        'Get position of | character in string
        nPos = InStr(sList, "|")
        
        'If possible, truncate string at | character
        If nPos > 0 Then
            'Add name to combo
            cmbName.AddItem (Left(sList, nPos - 1))
            cmbName.ItemData(cmbName.NewIndex) = n + 1
            sList = Mid(sList, nPos + 1, Len(sList))
        Else
            'Add name to combo
            cmbName.AddItem (sList)
            cmbName.ItemData(cmbName.NewIndex) = n + 1
        End If
    Next n
End Sub

Function GetFile(ByVal sKey As String, ByVal nInd As Integer) As String
    Dim nType As Integer
    Dim nKey As Long
    Dim nPos As Long
    
    Dim sExt As String
    Dim sName As String
    Dim sVal As String
    Dim sInfo As String
    
    'Set default
    GetFile = ""
    
    'Check DB
    If bDBFlag = False Then Exit Function
    
    'Check key
    If sKey = "" Then Exit Function
    If Left(sKey, 1) <> "o" Then Exit Function
    
    'Get type
    nType = frmObjects.GetType(Val(Mid(sKey, 2)))
    
    'Get extension
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_EXT, sExt, MIS_MOD_CFG)
    sExt = TruncStr(sExt)
    If sExt = "" Then Exit Function
    
    'Get name
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_REF, sName, MIS_MOD_CFG)
    sName = TruncStr(sName)
    
    'Reset key
    nKey = 0

    'Get key
    Call frmAttribs.GetAll(sKey, sName, nKey, sVal, sInfo)
        
    'Check value
    If sVal <> "" Then
        'Check path
        If InStr(sVal, ":") = 0 Then
            'Update path
            sVal = sDataDir + sVal
        End If
    Else
        'Set value
        sVal = GetName(Mid(sKey, 2))
    
        'Check value
        nPos = InStr(sVal, ",")
        If nPos > 0 Then
            'Truncate value at comma
            sVal = Mid(sVal, nPos + 1)
        End If
        
        'Check index
        If nInd >= 0 Then sVal = sVal + "_" + Trim(Str(nInd))
        
        'Set File
        GetFile = sDataDir + sVal + sExt
        Exit Function
    End If
        
    'Set file
    GetFile = sVal
End Function

Function CheckFile(ByVal nType As Integer, ByVal sName As String, ByVal sFile As String)
    Dim n As Integer

    Dim nInd As Integer
    Dim nCount As Integer
    
    Dim nPos As Long
    
    Dim sVal As String
    Dim sList As String
    
    'Set default
    CheckFile = sFile

    'Reset count
    nCount = 0
    
    'Get Names
    Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_NAME, sList, nCount, MIS_MOD_CFG)
    
    'Check count
    If nCount = 0 Then Exit Function
    
    'Truncate names
    sList = TruncStr(sList)

    'Reset index
    nInd = 0
    
    'Loop thru names
    For n = 0 To nCount - 1
        'Get position of | character in string
        nPos = InStr(sList, "|")
        
        'If possible, truncate string at | character
        If nPos > 0 Then
            'Check name
            If (sName = Left(sList, nPos - 1)) Then
                'Set index
                nInd = n + 1
                Exit For
            End If
            sList = Mid(sList, nPos + 1, Len(sList))
        Else
            'Check name
            If (sName = sList) Then
                'Set index
                nInd = n + 1
                Exit For
            End If
        End If
    Next n
    
    'Check index
    If (nInd = 0) Then Exit Function
    
    'Get file
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_FILE + Trim(Str(nInd - 1)), sVal, MIS_MOD_CFG)
    sVal = TruncStr(sVal)
    If sVal <> "" Then CheckFile = sVal
End Function

Sub PutFile(ByVal sKey As String, ByVal sFile As String)
    Dim nType As Integer
    Dim nKey As Long
    
    Dim sName As String
    Dim sVal As String
    Dim sInfo As String
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check key
    If sKey = "" Then Exit Sub
    If Left(sKey, 1) <> "o" Then Exit Sub
    
    'Get type
    nType = frmObjects.GetType(Val(Mid(sKey, 2)))
    
    'Get name
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_REF, sName, MIS_MOD_CFG)
    sName = TruncStr(sName)
        
    'Reset Key
    nKey = 0
    
    'Get key
    Call frmAttribs.GetAll(sKey, sName, nKey, sVal, sInfo)
        
    'Check value
    If sVal = "" Then
        'Put file
        sVal = sFile
        
        'Find data directory in value
        If InStr(1, sVal, sDataDir, vbTextCompare) > 0 Then
            'Remove data directory
            sVal = Mid(sVal, Len(sDataDir) + 1, Len(sVal))
        End If
        
        Call frmAttribs.EditAttrib(nKey, sName, sVal, sInfo)
    End If
End Sub

Sub PutObject()
    Dim nPos As Long
    Dim nObj As Long
    Dim nLink As Long
    Dim nMode As Long
    Dim nCol As Long
    
    Dim fRotX As Single
    Dim fRotY As Single
    Dim fRotZ As Single
    
    Dim sCKey As String
    Dim sLKey As String
    Dim sFile As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check list
    If sListK = "" Then
        'Add object
        AddObject
        Exit Sub
    End If
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Sub
    
    'Set list
    sLKey = sListK
    
    'Loop thru types
    Do
        'Get position of space character in string
        nPos = InStr(sLKey, " ")
        
        'If possible, truncate string at space character
        If nPos > 0 Then
            'Set key
            sCKey = Left(sLKey, nPos - 1)
            sLKey = Mid(sLKey, nPos + 1, Len(sLKey))
        Else
            'Set key
            sCKey = sLKey
        End If
        
        'Check key
        If Left(sCKey, 1) = "o" Then
            'Set query
            sQuery = "SELECT * FROM Objects WHERE Key = " + Mid(sCKey, 2)
            
            'Open temporary recordset by query
            If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
                
            'Put data in recordset
            rsTemp.MoveFirst
            rsTemp.Edit
            
            'Get file
            sFile = rsTemp!File
            
            'Get data from controls
            rsTemp!NumObjs = CountObjects(Val(Mid(sCKey, 2)))
            rsTemp!NumAttribs = CountAttribs(Val(Mid(sCKey, 2)))
            If txtPosX.BackColor = vbWindowBackground Then rsTemp!PosX = Val(txtPosX.Text)
            If txtPosY.BackColor = vbWindowBackground Then rsTemp!PosY = Val(txtPosY.Text)
            If txtPosZ.BackColor = vbWindowBackground Then rsTemp!PosZ = Val(txtPosZ.Text)
            If txtScaleX.BackColor = vbWindowBackground Then rsTemp!ScaleX = Val(txtScaleX.Text) / 100
            If txtScaleY.BackColor = vbWindowBackground Then rsTemp!ScaleY = Val(txtScaleY.Text) / 100
            If txtScaleZ.BackColor = vbWindowBackground Then rsTemp!ScaleZ = Val(txtScaleZ.Text) / 100
            fRotX = Val(txtRotX.Text)
            fRotY = Val(txtRotY.Text)
            fRotZ = Val(txtRotZ.Text)
            Call rendAngCheck(fRotX, fRotY, fRotZ)
            If txtRotX.BackColor = vbWindowBackground Then rsTemp!RotX = fRotX
            If txtRotY.BackColor = vbWindowBackground Then rsTemp!RotY = fRotY
            If txtRotZ.BackColor = vbWindowBackground Then rsTemp!RotZ = fRotZ
            If cmbType.BackColor = vbWindowBackground Then rsTemp!Type = cmbType.ItemData(cmbType.ListIndex)
            If cmbLayer.BackColor = vbWindowBackground Then rsTemp!Layer = cmbLayer.ItemData(cmbLayer.ListIndex)
            If cmbName.BackColor = vbWindowBackground Then rsTemp!Name = Trim(cmbName.Text)
            If txtInfo.BackColor = vbWindowBackground Then rsTemp!Info = Trim(txtInfo.Text)
            If txtFile.BackColor = vbWindowBackground Then rsTemp!File = Trim(txtFile.Text)
            
            'Find object
            Call rendFindObj(nObj, rsTemp!Key)
            
            'Check file
            If sFile <> rsTemp!File Then
                'Delete object
                Call rendDelObj(nObj)
           
                'Get full filename
                sFile = rsTemp!File
                
                'Check filename
                If sFile <> "" Then
                    If InStr(sFile, ":") = 0 Then
                        'Validate and hash file
                        If Dir(sObjDir + sFile) = "" Then sFile = Trim(Str(misGetHash(rsTemp!File))) + MIS_EXT_OBJ
                        
                        'Append filename to object dir
                        sFile = sObjDir + sFile
                    End If
                End If
                    
                'Recreate object
                If rendNewObj(nObj, rsTemp!Key, sFile) Then Call MsgBox("DLL error: Unable to create object!", vbOKOnly Or vbExclamation, "MissionMan")
                
                'Set link
                Call rendFindObj(nLink, GetLink(rsTemp!Key, rsTemp!Level, rsTemp!Object, rsTemp!Type))
                Call rendSetObjLink(nObj, nLink)
            Else
                'Reset object
                Call rendResetObj(nObj)
            End If
        
            'Get mode
            Call frmLayers.GetColor(rsTemp!Layer, nMode, nCol)
            
            'Set mode and color
            Call rendSetObjMode(nObj, 2)
            Call rendSetObjCol(nObj, nCol)
            Call frmLayers.SetLayer(rsTemp!Layer, 1)
            
            'Position object
            Call rendRotObj(nObj, rsTemp!RotX, rsTemp!RotY, rsTemp!RotZ)
            Call rendScaleObj(nObj, rsTemp!ScaleX, rsTemp!ScaleY, rsTemp!ScaleZ)
            Call rendTransObj(nObj, rsTemp!PosX, rsTemp!PosY, rsTemp!PosZ)
        
            'Update data
            rsTemp.Update
            
            'Close temporary recordset
            rsTemp.Close
            
            'Edit in tree
            frmTree.EditTree (sCKey)
        End If
        
        'Check position
        If nPos = 0 Then Exit Do
    Loop
            
    'Select in tree
    frmTree.SelTree ("")
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub PutColor(ByVal nKey As Long, ByVal nMode As Long, ByVal nCol As Long)
    Dim nCount As Integer

    Dim nObj As Long
       
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Layer = " + Str(nKey) + " ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Find data in recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Find object
        Call rendFindObj(nObj, rsTemp!Key)
        
        'Get mode
        If nMode > 0 Then
            Call rendGetObjMode(nObj, nMode)
            If nMode = 0 Then nMode = 1
        End If
        
        'Set mode and color
        Call rendSetObjMode(nObj, nMode)
        Call rendSetObjCol(nObj, nCol)
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Sub AddObject()
    Dim nType As Integer

    Dim nObj As Long
    Dim nLink As Long
    Dim nMode As Long
    Dim nCol As Long
    Dim nRLev As Long
    Dim nRObj As Long
    
    Dim fRotX As Single
    Dim fRotY As Single
    Dim fRotZ As Single
    
    Dim sFile As String
    
    'Add data to recordset
    rsObjects.AddNew
    
    'Get key
    sCurK = "o" + Trim(Str(rsObjects!Key))
    sListK = sCurK
    
    'Check parent
    If Left(sParK, 1) = "l" Then
        rsObjects!Level = Val(Mid(sParK, 2))
    Else
        rsObjects!Level = 0
    End If
    
    'Check parent
    If Left(sParK, 1) = "o" Then
        rsObjects!Object = Val(Mid(sParK, 2))
    Else
        rsObjects!Object = 0
    End If
    
    'Get data from controls
    rsObjects!NumObjs = 0
    rsObjects!NumAttribs = 0
    rsObjects!PosX = Val(txtPosX.Text)
    rsObjects!PosY = Val(txtPosY.Text)
    rsObjects!PosZ = Val(txtPosZ.Text)
    rsObjects!ScaleX = Val(txtScaleX.Text) / 100
    rsObjects!ScaleY = Val(txtScaleY.Text) / 100
    rsObjects!ScaleZ = Val(txtScaleZ.Text) / 100
    fRotX = Val(txtRotX.Text)
    fRotY = Val(txtRotY.Text)
    fRotZ = Val(txtRotZ.Text)
    Call rendAngCheck(fRotX, fRotY, fRotZ)
    rsObjects!RotX = fRotX
    rsObjects!RotY = fRotY
    rsObjects!RotZ = fRotZ
    rsObjects!Type = cmbType.ItemData(cmbType.ListIndex)
    rsObjects!Layer = cmbLayer.ItemData(cmbLayer.ListIndex)
    rsObjects!Name = Trim(cmbName.Text)
    rsObjects!Info = Trim(txtInfo.Text)
    rsObjects!File = Trim(txtFile.Text)
    
    'Get full filename
    sFile = rsObjects!File
    
    'Check filename
    If sFile <> "" Then
        If InStr(sFile, ":") = 0 Then
            'Validate and hash file
            If Dir(sObjDir + sFile) = "" Then sFile = Trim(Str(misGetHash(rsObjects!File))) + MIS_EXT_OBJ
            
            'Append filename to object dir
            sFile = sObjDir + sFile
        End If
    End If
        
    'Create object
    If rendNewObj(nObj, rsObjects!Key, sFile) Then Call MsgBox("DLL error: Unable to create object!", vbOKOnly Or vbExclamation, "MissionMan")
        
    'Set mode and color
    Call frmLayers.GetColor(rsObjects!Layer, nMode, nCol)
    Call rendSetObjMode(nObj, nMode)
    Call rendSetObjCol(nObj, nCol)
    
    'Set selection
    Call frmLayers.SetLayer(rsObjects!Layer, 1)
    Call rendSetSel("o", sCurK)
    
    'Position object
    Call rendRotObj(nObj, rsObjects!RotX, rsObjects!RotY, rsObjects!RotZ)
    Call rendScaleObj(nObj, rsObjects!ScaleX, rsObjects!ScaleY, rsObjects!ScaleZ)
    Call rendTransObj(nObj, rsObjects!PosX, rsObjects!PosY, rsObjects!PosZ)
        
    'Update data
    rsObjects.Update
    
    'Add to tree
    Call frmTree.AddTree(sParK, sCurK)
    
    'Add attribs
    AddAttribs
    
    'Set link
    Call GetAll(Val(Mid(sCurK, 2)), nRLev, nRObj, nType)
    Call rendFindObj(nLink, GetLink(Val(Mid(sCurK, 2)), nRLev, nRObj, nType))
    Call rendSetObjLink(nObj, nLink)
    
    'Check reference key
    If sRefK <> "" Then
        'Move objects
        Call MoveObjects(sCurK, sRefK, sParK)
        
        'Reset refernce key
        sRefK = ""
        
        'Show tree
        frmTree.ShowTree (sCurK)
        Exit Sub
    End If
    
    'Select in tree
    Call frmTree.SelTree(sCurK)
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub AddAttribs()
    Dim n As Integer

    Dim nCount As Integer
    
    Dim nPos As Long
    
    Dim sList As String
    Dim sVal As String
    
    'Reset count
    nCount = 0
    
    'Get attribs
    Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(cmbType.ItemData(cmbType.ListIndex) - 1)), MIS_KEY_ATTRIB, sList, nCount, MIS_MOD_CFG)
    
    'Check count
    If nCount = 0 Then Exit Sub
    
    'Truncate attribs
    sList = TruncStr(sList)

    'Loop thru attribs
    For n = 0 To nCount - 1
        'Get position of | character in string
        nPos = InStr(sList, "|")
        
        'Get default
        Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(cmbType.ItemData(cmbType.ListIndex) - 1)), MIS_KEY_DEF + Trim(Str(n)), sVal, MIS_MOD_CFG)
        
        'Truncate default
        sVal = TruncStr(sVal)

        'If possible, truncate string at | character
        If nPos > 0 Then
            'Create new attrib
            Call frmAttribs.NewAttrib(sCurK, Left(sList, nPos - 1), sVal, "")
            sList = Mid(sList, nPos + 1, Len(sList))
        Else
            'Create new attrib
            Call frmAttribs.NewAttrib(sCurK, sList, sVal, "")
        End If
    Next n
End Sub

Sub DelObject(ByVal sPKey As String, ByVal sCKey As String, ByVal sFile As String)
    Dim nType As Integer
    
    Dim nObj As Long
        
    Dim sTxt As String
    Dim sVal As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check file
    If sFile = "" Then
        'Check DB
        If bDBFlag = False Then Exit Sub
    End If
    
    'Check key
    If sCKey = "" Then Exit Sub
    If Left(sCKey, 1) <> "o" Then Exit Sub
    
    'Check parent
    If sPKey <> "" And sFile = "" Then
        'Get name
        sTxt = frmObjects.GetName(Val(Mid(sCKey, 2)))
    
        'Get type
        nType = frmObjects.GetType(Val(Mid(sCKey, 2)))
        Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_TYPE, sVal, MIS_MOD_CFG)
        sVal = TruncStr(sVal)
        If sVal <> "" Then sTxt = sVal + ": " + sTxt
                
        'Prompt user
        If MsgBox("Delete object " + sTxt + "?", vbYesNo Or vbQuestion, "MissionMan") = vbNo Then Exit Sub
    End If
    
    'Del attrib
    Call frmAttribs.DelAttribs(sCKey, sFile)
   
    'Del objects
    Call DelObjects(sCKey, sFile)
    
    'Check file
    If sFile = "" Then
        'Check recordset
        If rsObjects.BOF = True Then Exit Sub
    End If
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Key = " + Mid(sCKey, 2)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, sFile) = False Then Exit Sub
        
    'Delete data in recordset
    rsTemp.MoveFirst
    
    'Find object
    Call rendFindObj(nObj, rsTemp!Key)
    
    'Delete object
    Call rendDelObj(nObj)
    
    'Delete data
    rsTemp.Delete
    
    'Close temporary recordset
    Call CloseRecordSetByQuery(rsTemp, sFile)
    
    'Check file
    If sFile <> "" Then Exit Sub
    
    'Delete from tree
    Call frmTree.DelTree(sCKey)
    
    'Check parent
    If sPKey = "" Then Exit Sub
    
    'Select in tree
    Call frmTree.SelTree(sPKey)
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub DelObjects(ByVal sKey As String, ByVal sFile As String)
    Dim nObj As Long
       
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check file
    If sFile = "" Then
        'Check DB
        If bDBFlag = False Then Exit Sub
    End If
    
    'Check key
    If sKey = "" Then Exit Sub
    If Left(sKey, 1) = "m" Then Exit Sub
    If Left(sKey, 1) = "a" Then Exit Sub
    
    'Check file
    If sFile = "" Then
        'Check recordset
        If rsObjects.BOF = True Then Exit Sub
    End If
        
    'Check key
    If Left(sKey, 1) = "l" Then
        'Set query
        sQuery = "SELECT * FROM Objects WHERE Level = " + Mid(sKey, 2) + " AND Object = 0 ORDER BY Key"
    End If
    
    'Check key
    If Left(sKey, 1) = "o" Then
        'Set query
        sQuery = "SELECT * FROM Objects WHERE Level = 0 AND Object = " + Mid(sKey, 2) + " ORDER BY Key"
    End If
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, sFile) = False Then Exit Sub
        
    'Find data in recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Del attribs
        Call frmAttribs.DelAttribs("o" + Trim(Str(rsTemp!Key)), sFile)
        
        'Del objects
        Call DelObjects("o" + Trim(Str(rsTemp!Key)), sFile)
        
        'Find object
        Call rendFindObj(nObj, rsTemp!Key)

        'Delete object
        Call rendDelObj(nObj)

        'Delete data
        rsTemp.Delete
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    Call CloseRecordSetByQuery(rsTemp, sFile)
End Sub

Sub DelLayer(ByVal nKey As Long)
    Dim nObj As Long
    Dim nMode As Long
       
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Layer = " + Str(nKey) + " ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Find data in recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Find object
        Call rendFindObj(nObj, rsTemp!Key)
        
        'Get mode
        Call rendGetObjMode(nObj, nMode)
        If nMode = 0 Then nMode = 1
        
        'Set mode and color
        Call rendSetObjMode(nObj, nMode)
        Call rendSetObjCol(nObj, nViewCol)
        
        'Edit data in recordset
        rsTemp.Edit
        rsTemp!Layer = 0
        rsTemp.Update
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Sub CopyObject(ByVal sSrcKey As String, ByVal sDstKey As String, ByVal bFlag As Boolean, ByVal sSrcFile As String)
    Dim nType As Integer
    
    Dim nKey As Long
    Dim nObj As Long
    Dim nLink As Long
    Dim nMode As Long
    Dim nCol As Long
    Dim nRLev As Long
    Dim nRObj As Long
    
    Dim sQuery As String
    Dim sFile As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check source
    If sSrcKey = "" Then Exit Sub
    If Left(sSrcKey, 1) <> "o" Then Exit Sub
    
    'Check destination
    If sDstKey = "" Then Exit Sub
    If Left(sDstKey, 1) = "m" Then Exit Sub
    If Left(sDstKey, 1) = "a" Then Exit Sub
    
    'Check source file
    If sSrcFile = "" Then
        'Check recordset
        If rsObjects.BOF = True Then Exit Sub
    End If
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Key = " + Mid(sSrcKey, 2)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, sSrcFile) = False Then Exit Sub
        
    'Get data from recordset
    rsTemp.MoveFirst

    'Add data to recordset
    rsObjects.AddNew
    nKey = rsObjects!Key
    
    'Check destination
    If Left(sDstKey, 1) = "l" Then
        rsObjects!Level = Val(Mid(sDstKey, 2))
    Else
        rsObjects!Level = 0
    End If
    
    'Check destination
    If Left(sDstKey, 1) = "o" Then
        rsObjects!Object = Val(Mid(sDstKey, 2))
    Else
        rsObjects!Object = 0
    End If
    
    'Copy data
    rsObjects!NumObjs = rsTemp!NumObjs
    rsObjects!NumAttribs = rsTemp!NumAttribs
    rsObjects!PosX = rsTemp!PosX
    rsObjects!PosY = rsTemp!PosY
    rsObjects!PosZ = rsTemp!PosZ
    rsObjects!ScaleX = rsTemp!ScaleX
    rsObjects!ScaleY = rsTemp!ScaleY
    rsObjects!ScaleZ = rsTemp!ScaleZ
    rsObjects!RotX = rsTemp!RotX
    rsObjects!RotY = rsTemp!RotY
    rsObjects!RotZ = rsTemp!RotZ
    rsObjects!Type = rsTemp!Type
    rsObjects!Layer = rsTemp!Layer
    rsObjects!Name = rsTemp!Name
    rsObjects!Info = rsTemp!Info
    rsObjects!File = rsTemp!File
        
    'Close temporary recordset
    Call CloseRecordSetByQuery(rsTemp, sSrcFile)
    
    'Get full filename
    sFile = rsObjects!File
    
    'Check filename
    If sFile <> "" Then
        If InStr(sFile, ":") = 0 Then
            'Validate and hash file
            If Dir(sObjDir + sFile) = "" Then sFile = Trim(Str(misGetHash(rsObjects!File))) + MIS_EXT_OBJ
            
            'Append filename to object dir
            sFile = sObjDir + sFile
        End If
    End If
        
    'Create object
    If rendNewObj(nObj, nKey, sFile) Then Call MsgBox("DLL error: Unable to create object!", vbOKOnly Or vbExclamation, "MissionMan")
        
    'Set mode and color
    Call frmLayers.GetColor(rsObjects!Layer, nMode, nCol)
    Call rendSetObjMode(nObj, nMode)
    Call rendSetObjCol(nObj, nCol)
    
    'Set selection
    Call frmLayers.SetLayer(rsObjects!Layer, 1)
    Call rendSetSel("o", "o" + Trim(Str(nKey)))
    
    'Position object
    Call rendRotObj(nObj, rsObjects!RotX, rsObjects!RotY, rsObjects!RotZ)
    Call rendScaleObj(nObj, rsObjects!ScaleX, rsObjects!ScaleY, rsObjects!ScaleZ)
    Call rendTransObj(nObj, rsObjects!PosX, rsObjects!PosY, rsObjects!PosZ)
    
    'Update data
    rsObjects.Update
    
    'Add to tree
    Call frmTree.AddTree(sDstKey, "o" + Trim(Str(nKey)))
    
    'Copy attrib
    Call frmAttribs.CopyAttribs(sSrcKey, "o" + Trim(Str(nKey)), sSrcFile)
   
    'Set link
    Call GetAll(nKey, nRLev, nRObj, nType)
    Call rendFindObj(nLink, GetLink(nKey, nRLev, nRObj, nType))
    Call rendSetObjLink(nObj, nLink)
    
    'Copy objects
    Call CopyObjects(sSrcKey, "o" + Trim(Str(nKey)), sSrcFile)
    
    'Check flag
    If bFlag = True Then Call DelObject("", sSrcKey, sSrcFile)
    
    'Select in tree
    Call frmTree.SelTree("o" + Trim(Str(nKey)))
End Sub

Sub CopyObjects(ByVal sSrcKey As String, ByVal sDstKey As String, ByVal sSrcFile As String)
    Dim nType As Integer
    
    Dim nDstKey As Long
    Dim nSrcKey As Long
    Dim nObj As Long
    Dim nLink As Long
    Dim nMode As Long
    Dim nCol As Long
    Dim nRLev As Long
    Dim nRObj As Long
    
    Dim sQuery As String
    Dim sFile As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check destination
    If sDstKey = "" Then Exit Sub
    If Left(sDstKey, 1) = "m" Then Exit Sub
    If Left(sDstKey, 1) = "a" Then Exit Sub
    
    'Check source file
    If sSrcFile = "" Then
        'Check recordset
        If rsObjects.BOF = True Then Exit Sub
    End If
    
    'Reset Query
    sQuery = ""
    
    'Check source
    If Left(sSrcKey, 1) = "l" Then
        'Set query
        sQuery = "SELECT * FROM Objects WHERE Level = " + Mid(sSrcKey, 2) + " AND Object = 0 ORDER BY Key"
    End If
    
    'Check source
    If Left(sSrcKey, 1) = "o" Then
        'Set query
        sQuery = "SELECT * FROM Objects WHERE Level = 0 AND Object = " + Mid(sSrcKey, 2) + " ORDER BY Key"
    End If
    
    'Check query
    If sQuery = "" Then Exit Sub
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, sSrcFile) = False Then Exit Sub
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        nSrcKey = rsTemp!Key
        
        'Add data to recordset
        rsObjects.AddNew
        nDstKey = rsObjects!Key
    
        'Check destination
        If Left(sDstKey, 1) = "l" Then
            rsObjects!Level = Val(Mid(sDstKey, 2))
        Else
            rsObjects!Level = 0
        End If
    
        'Check destination
        If Left(sDstKey, 1) = "o" Then
            rsObjects!Object = Val(Mid(sDstKey, 2))
        Else
            rsObjects!Object = 0
        End If
    
        'Copy data
        rsObjects!NumObjs = rsTemp!NumObjs
        rsObjects!NumAttribs = rsTemp!NumAttribs
        rsObjects!PosX = rsTemp!PosX
        rsObjects!PosY = rsTemp!PosY
        rsObjects!PosZ = rsTemp!PosZ
        rsObjects!ScaleX = rsTemp!ScaleX
        rsObjects!ScaleY = rsTemp!ScaleY
        rsObjects!ScaleZ = rsTemp!ScaleZ
        rsObjects!RotX = rsTemp!RotX
        rsObjects!RotY = rsTemp!RotY
        rsObjects!RotZ = rsTemp!RotZ
        rsObjects!Type = rsTemp!Type
        rsObjects!Layer = rsTemp!Layer
        rsObjects!Name = rsTemp!Name
        rsObjects!Info = rsTemp!Info
        rsObjects!File = rsTemp!File
    
        'Get full filename
        sFile = rsObjects!File
        
        'Check filename
        If sFile <> "" Then
            If InStr(sFile, ":") = 0 Then
                'Validate and hash file
                If Dir(sObjDir + sFile) = "" Then sFile = Trim(Str(misGetHash(rsObjects!File))) + MIS_EXT_OBJ
                
                'Append filename to object dir
                sFile = sObjDir + sFile
            End If
        End If
            
        'Create object
        If rendNewObj(nObj, rsObjects!Key, sFile) Then Call MsgBox("DLL error: Unable to create object!", vbOKOnly Or vbExclamation, "MissionMan")
        
        'Set mode and color
        Call frmLayers.GetColor(rsObjects!Layer, nMode, nCol)
        Call rendSetObjMode(nObj, nMode)
        Call rendSetObjCol(nObj, nCol)
    
        'Position object
        Call rendRotObj(nObj, rsObjects!RotX, rsObjects!RotY, rsObjects!RotZ)
        Call rendScaleObj(nObj, rsObjects!ScaleX, rsObjects!ScaleY, rsObjects!ScaleZ)
        Call rendTransObj(nObj, rsObjects!PosX, rsObjects!PosY, rsObjects!PosZ)
        
        'Update data
        rsObjects.Update
        
        'Add to tree
        Call frmTree.AddTree(sDstKey, "o" + Trim(Str(nDstKey)))
    
        'Copy attrib
        Call frmAttribs.CopyAttribs("o" + Trim(Str(nSrcKey)), "o" + Trim(Str(nDstKey)), sSrcFile)
   
        'Set link
        Call GetAll(nDstKey, nRLev, nRObj, nType)
        Call rendFindObj(nLink, GetLink(nDstKey, nRLev, nRObj, nType))
        Call rendSetObjLink(nObj, nLink)
        
        'Copy objects
        Call CopyObjects("o" + Trim(Str(nSrcKey)), "o" + Trim(Str(nDstKey)), sSrcFile)
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    Call CloseRecordSetByQuery(rsTemp, sSrcFile)
End Sub

Sub DupObjects(ByVal X As Single, ByVal Y As Single, ByVal Z As Single)
    Dim nType As Integer
    
    Dim nLev As Long
    Dim nObj As Long
    Dim nPos As Long
    
    Dim fPosX As Single
    Dim fPosY As Single
    Dim fPosZ As Single
    
    Dim sKey As String
    Dim sInList As String
    Dim sOutList As String
    
    ' Check current key
    If Left(sCurKey, 1) <> "o" Then Exit Sub
    
    'Get object position
    Call rendFindObj(nObj, Val(Mid(sCurKey, 2)))
    Call rendGetObjTrans(nObj, fPosX, fPosY, fPosZ)
    
    'Translate object
    X = X - fPosX
    Y = Y - fPosY
    Z = Z - fPosZ
    
    'Check grid flag
    If bGridFlag = 1 Then
        'Snap x translation to grid
        If Abs(X) Mod fGridSize < fGridSize / 2 Then
            X = -(X Mod fGridSize) + X
        Else
            X = Sgn(X) * fGridSize - (X Mod fGridSize) + X
        End If
    
        'Snap y translation to grid
        If Abs(Y) Mod fGridSize < fGridSize / 2 Then
            Y = -(Y Mod fGridSize) + Y
        Else
            Y = Sgn(Y) * fGridSize - (Y Mod fGridSize) + Y
        End If
    
        'Snap z translation to grid
        If Abs(Z) Mod fGridSize < fGridSize / 2 Then
            Z = -(Z Mod fGridSize) + Z
        Else
            Z = Sgn(Z) * fGridSize - (Z Mod fGridSize) + Z
        End If
    End If
    
    'Initialize Lists
    sInList = sListKey
    sOutList = ""
    
    'Loop thru types
    Do
        'Get position of space character in string
        nPos = InStr(sInList, " ")
        
        'If possible, truncate string at space character
        If nPos > 0 Then
            'Set key
            sKey = Left(sInList, nPos - 1)
            sInList = Mid(sInList, nPos + 1, Len(sInList))
        Else
            'Set key
            sKey = sInList
        End If
        
        'Check key
        If Left(sKey, 1) = "o" Then
            'Get parent
            Call GetAll(Val(Mid(sKey, 2)), nLev, nObj, nType)
        
            'Duplicate object
            If nLev > 0 And nObj = 0 Then Call CopyObject(sKey, "l" + Trim(Str(nLev)), False, "")
            If nLev = 0 And nObj > 0 Then Call CopyObject(sKey, "o" + Trim(Str(nObj)), False, "")
            
            'Append list
            If sOutList <> "" Then sOutList = sOutList + " "
            sOutList = sOutList + sCurKey
            
            'Get object
            Call rendFindObj(nObj, Val(Mid(sCurKey, 2)))
            
            'Translate object
            Call rendTransObj(nObj, X, Y, Z)
            
            'Set object position
            Call EditObject(Val(Mid(sCurKey, 2)))
        End If
        
        'Check position
        If nPos = 0 Then Exit Do
    Loop
    
    'Select in tree
    Call frmTree.ShowTree(sOutList)
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub EditObject(ByVal nKey As Long)
    Dim nObj As Long
    
    Dim fPosX As Single
    Dim fPosY As Single
    Dim fPosZ As Single
    Dim fScaleX As Single
    Dim fScaleY As Single
    Dim fScaleZ As Single
    Dim fRotX As Single
    Dim fRotY As Single
    Dim fRotZ As Single

    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Key = " + Trim(Str((nKey)))
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Get object
    Call rendFindObj(nObj, nKey)
        
    'Get position
    Call rendGetObjTrans(nObj, fPosX, fPosY, fPosZ)
    Call rendGetObjScale(nObj, fScaleX, fScaleY, fScaleZ)
    Call rendGetObjRot(nObj, fRotX, fRotY, fRotZ)
    Call rendAngCheck(fRotX, fRotY, fRotZ)
                    
    'Put data in recordset
    rsTemp.MoveFirst
    
    'Set data
    rsTemp.Edit
    rsTemp!PosX = fPosX
    rsTemp!PosY = fPosY
    rsTemp!PosZ = fPosZ
    rsTemp!ScaleX = fScaleX
    rsTemp!ScaleY = fScaleY
    rsTemp!ScaleZ = fScaleZ
    rsTemp!RotX = fRotX
    rsTemp!RotY = fRotY
    rsTemp!RotZ = fRotZ
    rsTemp.Update

    'Close temporary recordset
    rsTemp.Close
    
    'Update form
    If fMainForm.mnuViewTabObject.Checked = True And Val(Mid(sCurK, 2)) = nKey Then
        'Put default data in controls
        txtPosX.Text = Format(fPosX, "0;-#")
        txtPosY.Text = Format(fPosY, "0;-#")
        txtPosZ.Text = Format(fPosZ, "0;-#")
        txtScaleX.Text = Format(fScaleX * 100, "0;-#")
        txtScaleY.Text = Format(fScaleY * 100, "0;-#")
        txtScaleZ.Text = Format(fScaleZ * 100, "0;-#")
        txtRotX.Text = Format(fRotX, "0;-#")
        txtRotY.Text = Format(fRotY, "0;-#")
        txtRotZ.Text = Format(fRotZ, "0;-#")
    End If
End Sub

Sub GenObject(ByVal sKey As String, ByVal sFile As String)
    Dim bFlag As Boolean

    Dim n As Integer
    Dim nType As Integer
    Dim nErr As Integer
    Dim nCount As Integer
    Dim nNum As Integer
    Dim nInd As Integer
    
    Dim nPos As Long
    Dim nObj As Long
    Dim nKey As Long
    Dim nStream As Long
    
    Dim fAngX As Single
    Dim fAngY As Single
    Dim fAngZ As Single
    Dim fAngH As Single
    Dim fAngV As Single
    Dim fSizeX As Single
    Dim fSizeY As Single
    Dim fSizeZ As Single
        
    Dim sQuery As String
    Dim sName As String
    Dim sAlias As String
    Dim sInfo As String
    Dim sExt As String
    Dim sType As String
    Dim sForm As String
    Dim sMsg As String
    Dim sList As String
    Dim sVal As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check key
    If sKey = "" Then Exit Sub
    If Left(sKey, 1) <> "o" Then Exit Sub
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Key = " + Mid(sKey, 2)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Get data from recordset
    rsTemp.MoveFirst
    nType = rsTemp!Type
    sName = rsTemp!Name
    sInfo = rsTemp!Info
    
    'Close temporary recordset
    rsTemp.Close
    
    'Get extension
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_EXT, sExt, MIS_MOD_CFG)
    sExt = TruncStr(sExt)
    If sExt = "" Then Exit Sub
    
    'Set progress bar
    Call frmProgress.Init("Generating object " + sName + "...", 100)
    
    'Check file
    If sFile = "" Then sFile = sName + sExt
    
    'Open file
    nErr = misOpen(nStream, sFile)
    If nErr < 0 Then
        'Reset progress bar
        Call frmProgress.Clean
        
        'Inform user
        Call misGetErr(nErr, sMsg)
        Call MsgBox("Error: " + TruncStr(sMsg) + " " + sFile + " (Check attributes and directories)!", vbOKOnly Or vbExclamation, "MissionMan")
        Exit Sub
    End If
    
    'Get type
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_TYPE, sType, MIS_MOD_CFG)
    sType = TruncStr(sType)
    
    'Write data
    Call misWriteInfo(nStream, "MissionMan script, " + Format(Date, "dddd, mmm d yyyy") + ", " + Format(Time, "h:mm:ss AMPM"))
    Call misWriteInfo(nStream, "Copyright (c) 1998-99, Relic Entertainment Inc.")
    If sType <> "" Then
        Call misWriteInfo(nStream, sType + " Object: " + sName)
    Else
        Call misWriteInfo(nStream, "Object: " + sName)
    End If
    If sInfo <> "" Then Call misWriteInfo(nStream, "Info: " + sInfo)
    misWriteNew (nStream)
    
    'Reset flag
    bFlag = False
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Level = 0 AND Object = " + Mid(sKey, 2) + " ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = True Then
        'Get data from recordset
        rsTemp.MoveFirst
        Do Until rsTemp.EOF
            'Update status bar
            Call frmProgress.Update(rsTemp.PercentPosition)
            
            'Get size
            Call rendFindObj(nObj, rsTemp!Key)
            Call rendGetObjSize(nObj, fSizeX, fSizeY, fSizeZ)
    
            'Get angles
            fAngX = rsTemp!RotX
            fAngY = rsTemp!RotY
            fAngZ = rsTemp!RotZ
            
            'Check angles
            If (rendAngCheck(fAngX, fAngY, fAngZ) < 0) Then
                'Update database
                rsTemp.Edit
                rsTemp!RotX = fAngX
                rsTemp!RotY = fAngY
                rsTemp!RotZ = fAngZ
                rsTemp.Update
            End If
            
            'Convert angles
            Call rendAngConv(fAngH, fAngV, rsTemp!RotX, rsTemp!RotY, rsTemp!RotZ)
            
            'Get vars
            Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(rsTemp!Type - 1)), MIS_KEY_VAR, sList, nCount, MIS_MOD_CFG)
    
            'Check count
            If nCount > 0 Then
                'Set flag
                bFlag = True
                
                'Truncate vars
                sList = TruncStr(sList)

                'Loop thru vars
                For n = 0 To nCount - 1
            
                    'Reset name
                    sName = ""
                
                    'Get position of | character in string
                    nPos = InStr(sList, "|")
                
                    'If possible, truncate string at | character
                    If nPos > 0 Then
                        'Set name
                        sName = Left(sList, nPos - 1)
                        sList = Mid(sList, nPos + 1, Len(sList))
                    Else
                        'Set name
                        sName = sList
                    End If
                
                    'Check var
                    Select Case Trim(sName)
                        Case "!PosX"
                            sVal = Format(rsTemp!PosX, "0.0;-0.0")
                        Case "!PosY"
                            sVal = Format(rsTemp!PosY, "0.0;-0.0")
                        Case "!PosZ"
                            sVal = Format(rsTemp!PosZ, "0.0;-0.0")
                        Case "!ScaleX"
                            sVal = Format(rsTemp!ScaleX, "0.0;-0.0")
                        Case "!ScaleY"
                            sVal = Format(rsTemp!ScaleY, "0.0;-0.0")
                        Case "!ScaleZ"
                            sVal = Format(rsTemp!ScaleY, "0.0;-0.0")
                        Case "!SizeX"
                            sVal = Format(fSizeX * rsTemp!ScaleX, "0.0;-0.0")
                        Case "!SizeY"
                            sVal = Format(fSizeY * rsTemp!ScaleY, "0.0;-0.0")
                        Case "!SizeZ"
                            sVal = Format(fSizeZ * rsTemp!ScaleZ, "0.0;-0.0")
                        Case "!RotX"
                            sVal = Format(rsTemp!RotX, "0.0;-0.0")
                        Case "!RotY"
                            sVal = Format(rsTemp!RotY, "0.0;-0.0")
                        Case "!RotZ"
                            sVal = Format(rsTemp!RotY, "0.0;-0.0")
                        Case "!RotH"
                            sVal = Format(fAngH, "0.0;-0.0")
                        Case "!RotV"
                            sVal = Format(fAngV, "0.0;-0.0")
                        Case "-!PosX"
                            sVal = Format(-rsTemp!PosX, "0.0;-0.0")
                        Case "-!PosY"
                            sVal = Format(-rsTemp!PosY, "0.0;-0.0")
                        Case "-!PosZ"
                            sVal = Format(-rsTemp!PosZ, "0.0;-0.0")
                        Case "!SizeX/2"
                            sVal = Format(fSizeX * rsTemp!ScaleX / 2, "0.0;-0.0")
                        Case "!SizeY/2"
                            sVal = Format(fSizeY * rsTemp!ScaleY / 2, "0.0;-0.0")
                        Case "!SizeZ/2"
                            sVal = Format(fSizeZ * rsTemp!ScaleZ / 2, "0.0;-0.0")
                        Case "-!RotX"
                            sVal = Format(-rsTemp!RotX, "0.0;-0.0")
                        Case "-!RotY"
                            sVal = Format(-rsTemp!RotY, "0.0;-0.0")
                        Case "-!RotZ"
                            sVal = Format(-rsTemp!RotY, "0.0;-0.0")
                        Case "-!RotH"
                            sVal = Format(-fAngH, "0.0;-0.0")
                        Case "-!RotV"
                            sVal = Format(-fAngV, "0.0;-0.0")
                        Case "!Type"
                            Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(rsTemp!Type - 1)), MIS_KEY_TYPE, sVal, MIS_MOD_CFG)
                            sVal = TruncStr(sVal)
                        Case "!Name"
                            sVal = Trim(rsTemp!Name)
                        Case "!File"
                            sVal = Trim(rsTemp!File)
                        Case "!Count"
                            Call CountLinks(rsTemp!Key, rsTemp!Level, rsTemp!Object, rsTemp!Type, nInd, nNum)
                            sVal = Trim(Str(nInd)) + "/" + Trim(Str(nNum))
                        Case Else
                            'Reset Key
                            nKey = 0
    
                            'Check value
                            If Left(sName, 1) = "-" Then
                                'Get negative value
                                Call frmAttribs.GetAll("o" + Trim(Str(rsTemp!Key)), Mid(sName, 2), nKey, sVal, sInfo)
                                If sVal = "" Then sVal = MIS_NAM_NULL
                                sVal = "-" + sVal
                            Else
                                'Get positive value
                                Call frmAttribs.GetAll("o" + Trim(Str(rsTemp!Key)), sName, nKey, sVal, sInfo)
                                If sVal = "" Then sVal = MIS_NAM_NULL
                            End If
                    End Select
                
                    'Get format
                    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(rsTemp!Type - 1)), MIS_KEY_FORM + Trim(Str(n)), sForm, MIS_MOD_CFG)
        
                    'Truncate format
                    sForm = TruncStr(sForm)

                    'Write data
                    If n = 0 And rsTemp!Info <> "" Then Call misWriteInfo(nStream, rsTemp!Info)
                    Call misWriteVal(nStream, sForm, sVal)
                Next n
            
                'Write newline
                misWriteNew (nStream)
            End If
        
            rsTemp.MoveNext
        Loop
        
        'Close temporary recordset
        rsTemp.Close
        
        'Check flag
        If bFlag = True Then misWriteNew (nStream)
    End If
    
    'Reset progress bar
    Call frmProgress.Clean
        
    'Check flag
    If bFlag = False Then
        'Get attribs
        Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_ATTRIB, sList, nCount, MIS_MOD_CFG)
    
        'Check count
        If nCount > 0 Then
            'Set progress bar
            Call frmProgress.Init("Generating attributes for object " + sName + "...", nCount)
    
            'Truncate attribs
            sList = TruncStr(sList)

            'Loop thru attribs
            For n = 0 To nCount - 1
                'Update status bar
                Call frmProgress.Update(n)
            
                'Reset name
                sName = ""
                
                'Get position of | character in string
                nPos = InStr(sList, "|")
                
                'If possible, truncate string at | character
                If nPos > 0 Then
                    'Set name
                    sName = Left(sList, nPos - 1)
                    sList = Mid(sList, nPos + 1, Len(sList))
                Else
                    'Set name
                    sName = sList
                End If
                
                'Reset key
                nKey = 0
                               
                'Loop
                Do Until nKey < 0
                    'Get value
                    Call frmAttribs.GetAll(sKey, sName, nKey, sVal, sInfo)
                    If sVal <> "" Then
                        'Get alias
                        Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_ALIAS + Trim(Str(n)), sAlias, MIS_MOD_CFG)
                        sAlias = TruncStr(sAlias)
                        If sAlias = "" Then sAlias = sName
        
                        'Write data
                        If sInfo <> "" Then Call misWriteInfo(nStream, sInfo)
                        Call misWriteAttrib(nStream, sAlias, sVal)
                    End If
                Loop
            Next n
            
            'Reset progress bar
            Call frmProgress.Clean
        End If
    End If
        
    'Close event file
    misClose (nStream)
End Sub

Sub GenObjects(ByVal sKey As String)
    Dim nInd As Integer
    
    Dim sQuery As String
    Dim sFile As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check key
    If sKey = "" Then Exit Sub
    If Left(sKey, 1) = "m" Then Exit Sub
    If Left(sKey, 1) = "a" Then Exit Sub
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Sub
    
    'Reset Query
    sQuery = ""
    
    'Check source
    If Left(sKey, 1) = "l" Then
        'Set query
        sQuery = "SELECT * FROM Objects WHERE Level = " + Mid(sKey, 2) + " AND Object = 0 ORDER BY Key"
    End If
    
    'Check source
    If Left(sKey, 1) = "o" Then
        'Set query
        sQuery = "SELECT * FROM Objects WHERE Level = 0 AND Object = " + Mid(sKey, 2) + " ORDER BY Key"
    End If
    
    'Check query
    If sQuery = "" Then Exit Sub
        
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
    
    'Reset index
    nInd = 0
    
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Generate sub-objects
        frmObjects.GenObjects ("o" + Trim(Str(rsTemp!Key)))
        
        'Get file
        sFile = GetFile("o" + Trim(Str(rsTemp!Key)), nInd)
        
        'Put file
        Call PutFile("o" + Trim(Str(rsTemp!Key)), sFile)
        
        'Generate object
        Call GenObject("o" + Trim(Str(rsTemp!Key)), sFile)
        
        'Increment index
        nInd = nInd + 1
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Function CountObjects(ByVal nKey As Long) As Integer
    Dim nCount As Integer

    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Reset count
    CountObjects = 0
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Level = 0 AND Object = " + Str(nKey) + " ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        CountObjects = CountObjects + 1
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Function

Function CountAttribs(ByVal nKey As Long) As Integer
    Dim nCount As Integer

    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Reset count
    CountAttribs = 0
    
    'Check recordset
    If rsAttribs.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Attrib WHERE Level = 0 AND Object = " + Str(nKey) + " ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        CountAttribs = CountAttribs + 1
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Function

Sub CountLinks(ByVal nKey As Long, ByVal nLev As Long, ByVal nObj As Long, ByVal nType As Integer, nInd As Integer, nNum As Integer)
    Dim nRKey As Long
    
    Dim sVal As String
    Dim sName As String
    Dim sRVal As String
    Dim sInfo As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Reset count and index
    nNum = 0
    nInd = 0
    
    'Get link
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(GetType(nKey) - 1)), MIS_KEY_LINK, sVal, MIS_MOD_CFG)
    sVal = TruncStr(sVal)
    If sVal = "" Then Exit Sub
    sName = sVal
    
    'Get value
    Call frmAttribs.GetAll("o" + Trim(Str(nKey)), sName, nRKey, sVal, sInfo)
    If sVal = "" Then Exit Sub
    sRVal = sVal
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Level = " + Str(nLev) + " AND Object = " + Str(nObj) + " AND Type = " + Str(nType) + " ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Reset key
        nRKey = 0
    
        'Get value
        Call frmAttribs.GetAll("o" + Trim(Str(rsTemp!Key)), sName, nRKey, sVal, sInfo)
        If sVal = sRVal Then
            'Increment count and index
            nNum = nNum + 1
            If rsTemp!Key <= nKey Then nInd = nInd + 1
        End If
        
        'Continue
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Sub SelType(ByVal nType As Integer)
    Dim n As Integer
    
    'Loop thru combo
    For n = 0 To cmbType.ListCount - 1
        'Check combo items
        If cmbType.ItemData(n) = nType Then
            cmbType.ListIndex = n
            Exit Sub
        End If
    Next n
End Sub

Sub SelLayer(ByVal nLayer As Integer)
    Dim n As Integer
    
    'Loop thru combo
    For n = 0 To cmbLayer.ListCount - 1
        'Check combo items
        If cmbLayer.ItemData(n) = nLayer Then
            cmbLayer.ListIndex = n
            Exit Sub
        End If
    Next n
End Sub

Function SelGlobal(ByVal nKey As Long) As String
    Dim nType As Integer
    
    Dim nLayer As Long
    Dim nSKey As Long
    
    Dim sVal As String
    Dim sName As String
    Dim sSName As String
    Dim sInfo As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Set default
    SelGlobal = ""
    
    'Get name
    nSKey = frmAttribs.GetKey()
    If nSKey > 0 Then
        sSName = frmAttribs.GetName(nSKey)
    Else
        sSName = ""
    End If
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Key = " + Str(nKey)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    nType = rsTemp!Type
    sName = rsTemp!Name
    nLayer = rsTemp!Layer
    
    'Close temporary recordset
    rsTemp.Close
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE 1 = 1 "
    If (nKey = Val(Mid(sCurK, 2))) And (fMainForm.mnuViewTabObject.Checked = True) Then
        If chkType.Value = 1 Then sQuery = sQuery + " AND Type = " + Str(nType)
        If chkName.Value = 1 Then sQuery = sQuery + " AND Name = """ + sName + """"
        If chkLayer.Value = 1 Then sQuery = sQuery + " AND Layer = " + Str(nLayer)
    End If
    sQuery = sQuery + " ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Check selection name
        If sSName <> "" Then
            'Reset key
            nSKey = -1
            
            'Get selection key
            Call frmAttribs.GetAll("o" + Trim(Str(rsTemp!Key)), sSName, nSKey, sVal, sInfo)
                        
            'Check key
            If nSKey > 0 Then
                'Add object
                If SelGlobal <> "" Then SelGlobal = SelGlobal + " "
                SelGlobal = SelGlobal + "o" + Trim(Str(Str(rsTemp!Key)))
        
                'Add attribute
                SelGlobal = SelGlobal + " a" + Trim(Str(nSKey))
            End If
        Else
            'Add object
            If SelGlobal <> "" Then SelGlobal = SelGlobal + " "
            SelGlobal = SelGlobal + "o" + Trim(Str(Str(rsTemp!Key)))
        End If
        
        'Continue
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Function

Function SelObjects(ByVal nKey As Long) As String
    Dim nType As Integer
    
    Dim nLayer As Long
    Dim nLev As Long
    Dim nObj As Long
    Dim nSKey As Long
    
    Dim sVal As String
    Dim sName As String
    Dim sSName As String
    Dim sInfo As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Set default
    SelObjects = ""
    
    'Get name
    nSKey = frmAttribs.GetKey()
    If nSKey > 0 Then
        sSName = frmAttribs.GetName(nSKey)
    Else
        sSName = ""
    End If
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Key = " + Str(nKey)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    nLev = rsTemp!Level
    nObj = rsTemp!Object
    nType = rsTemp!Type
    sName = rsTemp!Name
    nLayer = rsTemp!Layer
    
    'Close temporary recordset
    rsTemp.Close
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Level = " + Str(nLev) + " AND Object = " + Str(nObj)
    If (nKey = Val(Mid(sCurK, 2))) And (fMainForm.mnuViewTabObject.Checked = True) Then
        If chkType.Value = 1 Then sQuery = sQuery + " AND Type = " + Str(nType)
        If chkName.Value = 1 Then sQuery = sQuery + " AND Name = """ + sName + """"
        If chkLayer.Value = 1 Then sQuery = sQuery + " AND Layer = " + Str(nLayer)
    End If
    sQuery = sQuery + " ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Check selection name
        If sSName <> "" Then
            'Reset key
            nSKey = -1
            
            'Get selection key
            Call frmAttribs.GetAll("o" + Trim(Str(rsTemp!Key)), sSName, nSKey, sVal, sInfo)
                        
            'Check key
            If nSKey > 0 Then
                'Add object
                If SelObjects <> "" Then SelObjects = SelObjects + " "
                SelObjects = SelObjects + "o" + Trim(Str(Str(rsTemp!Key)))
            
                'Add attribute
                SelObjects = SelObjects + " a" + Trim(Str(nSKey))
            End If
        Else
            'Add object
            If SelObjects <> "" Then SelObjects = SelObjects + " "
            SelObjects = SelObjects + "o" + Trim(Str(Str(rsTemp!Key)))
        End If
        
        'Continue
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Function

Function SelPath(ByVal nKey As Long) As String
    Dim nType As Integer
    
    Dim nRKey As Long
    Dim nSKey As Long
    Dim nLev As Long
    Dim nObj As Long
    
    Dim sVal As String
    Dim sRVal As String
    Dim sName As String
    Dim sSName As String
    Dim sInfo As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Set default
    SelPath = ""
    
    'Get all
    Call GetAll(Val(Mid(sCurKey, 2)), nLev, nObj, nType)

    'Get link
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(GetType(nKey) - 1)), MIS_KEY_LINK, sVal, MIS_MOD_CFG)
    sVal = TruncStr(sVal)
    If sVal = "" Then Exit Function
    sName = sVal
    
    'Get value
    Call frmAttribs.GetAll("o" + Trim(Str(nKey)), sName, nRKey, sVal, sInfo)
    If sVal = "" Then Exit Function
    sRVal = sVal
    
    'Get name
    nSKey = frmAttribs.GetKey()
    If nSKey > 0 Then
        sSName = frmAttribs.GetName(nSKey)
    Else
        sSName = ""
    End If
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Level = " + Str(nLev) + " AND Object = " + Str(nObj) + " AND Type = " + Str(nType) + " ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Reset key
        nRKey = 0
    
        'Get value
        Call frmAttribs.GetAll("o" + Trim(Str(rsTemp!Key)), sName, nRKey, sVal, sInfo)
        If sVal = sRVal Then
            'Check selection name
            If sSName <> "" Then
                If sSName = sName Then
                    'Set selection key
                    nSKey = nRKey
                Else
                    'Reset key
                    nSKey = -1
                    
                    'Get selection key
                    Call frmAttribs.GetAll("o" + Trim(Str(rsTemp!Key)), sSName, nSKey, sVal, sInfo)
                End If
                            
                'Check key
                If nSKey > 0 Then
                    'Add object
                    If SelPath <> "" Then SelPath = SelPath + " "
                    SelPath = SelPath + "o" + Trim(Str(rsTemp!Key))
            
                    'Add attribute
                    SelPath = SelPath + " a" + Trim(Str(nSKey))
                End If
            Else
                'Add object
                If SelPath <> "" Then SelPath = SelPath + " "
                SelPath = SelPath + "o" + Trim(Str(rsTemp!Key))
            End If
        End If
        
        'Continue
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Function

Sub MoveObjects(ByVal sCKey As String, ByVal sRKey As String, ByVal sPKey As String)
    Dim sQuery As String
    Dim sFile As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check keys
    If Left(sCKey, 1) <> "o" Then Exit Sub
    If Left(sRKey, 1) <> "o" Then Exit Sub
    If Left(sPKey, 1) = "m" Then Exit Sub
    If Left(sPKey, 1) = "a" Then Exit Sub
        
    'Check recordset
    If rsObjects.BOF = True Then Exit Sub
    
    'Reset Query
    sQuery = ""
    
    'Check source
    If Left(sPKey, 1) = "l" Then
        'Set query
        sQuery = "SELECT * FROM Objects WHERE Key >= " + Mid(sRKey, 2) + " AND Key < " + Mid(sCKey, 2) + " AND Level = " + Mid(sPKey, 2) + " AND Object = 0 ORDER BY Key"
    End If
    
    'Check source
    If Left(sPKey, 1) = "o" Then
        'Set query
        sQuery = "SELECT * FROM Objects WHERE Key >= " + Mid(sRKey, 2) + " AND Key < " + Mid(sCKey, 2) + " AND Level = 0 AND Object = " + Mid(sPKey, 2) + " ORDER BY Key"
    End If
    
    'Check query
    If sQuery = "" Then Exit Sub
        
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
    
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Copy item
        Call frmObjects.CopyObject("o" + Trim(Str(rsTemp!Key)), sPKey, True, "")
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Sub BrowseObject()
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show open common dialog
    cdBrowse.InitDir = sObjDir
    cdBrowse.ShowOpen
    
    'Find object directory in full filename
    If InStr(1, cdBrowse.FileName, sObjDir, vbTextCompare) > 0 Then
        'Remove object directory
        txtFile.Text = Mid(cdBrowse.FileName, Len(sObjDir) + 1, Len(cdBrowse.FileName))
    Else
        'Keep full filename
        txtFile.Text = cdBrowse.FileName
    End If
    
    'Set change
    cmdApply_Click
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub cmbLayer_Click()
    'Reset color
    cmbLayer.BackColor = vbWindowBackground
End Sub

Private Sub cmbName_Change()
    'Reset color
    cmbName.BackColor = vbWindowBackground
End Sub

Private Sub cmbName_Click()
    Dim sVal As String
    
    'Reset color
    cmbName.BackColor = vbWindowBackground
    
    'Check combo
    If cmbName.ListIndex = -1 Then Exit Sub
    
    'Get file
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(cmbType.ItemData(cmbType.ListIndex) - 1)), MIS_KEY_FILE + Trim(Str(cmbName.ItemData(cmbName.ListIndex) - 1)), sVal, MIS_MOD_CFG)
    sVal = TruncStr(sVal)
    If sVal <> "" Then txtFile.Text = sVal
End Sub

Private Sub cmbType_Click()
    'Reset color
    cmbType.BackColor = vbWindowBackground
    
    'Get names
    Call GetNames(cmbName.Text, cmbType.ItemData(cmbType.ListIndex))
    
    'Set combo
    If cmbName.ListCount > 0 Then cmbName.ListIndex = 0
End Sub

Private Sub cmdApply_Click()
    'Commit
    Call CommitDB("Edit Objects")
    
    'Put object
    PutObject
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Get Names
    If cmbName.BackColor = vbWindowBackground Or cmbType.BackColor = vbWindowBackground Then Call GetNames(cmbName.Text, cmbType.ItemData(cmbType.ListIndex))
End Sub

Private Sub cmdBrowse_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    'Check mouse button
    If Button = 2 Then
        'Show menu
        Call PopupMenu(fMainForm.mnuPUObjFile, 2)
        Exit Sub
    End If
    
    'Browse
    BrowseObject
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdOK_Click()
    'Commit
    Call CommitDB("Edit Objects")
    
    'Put object
    PutObject
    
    Unload Me
End Sub

Private Sub Form_Load()
    Dim n As Integer
    Dim nCount As Integer
    
    Dim nPos As Long
    Dim nMask As Long
        
    Dim sVal As String
    Dim sList As String

    'Disable file selection
    cmdBrowse.Enabled = False
    txtFile.Enabled = False
    
    'Get bit mask
    Call misGetVal(MIS_SEC_COM, MIS_KEY_BITM, sVal, MIS_MOD_CFG)
    sVal = TruncStr(sVal)
    If sVal <> "" Then
        'Set mask
        nMask = Val(sVal)
    
        'Check bit mask
        If (nMask And MIS_BIT_DEV) = MIS_BIT_DEV Then
            'Enable file selection
            cmdBrowse.Enabled = True
            txtFile.Enabled = True
        End If
    End If
    
    'Reset Flags
    bScale = False
    bSize = False
    
    'Set tree view position
    aPos(0) = fMainForm.ScaleWidth / 4
    aPos(1) = fMainForm.ScaleHeight / 4
    
    'Reset count
    nCount = 0
    
    'Get window
    Call misGetListByKey(MIS_SEC_COM, MIS_KEY_OBJT, sList, nCount, MIS_MOD_INI)
    
    'Check count
    If nCount > 0 Then
        'Truncate list
        sList = TruncStr(sList)

        'Loop thru list
        For n = 0 To 1
            'Get position of | character in string
            nPos = InStr(sList, "|")
        
            'If possible, truncate string at | character
            If nPos > 0 Then
                'Set position
                aPos(n) = Val(Left(sList, nPos - 1)) * fConvScale
                sList = Mid(sList, nPos + 1, Len(sList))
            Else
                'Set position
                aPos(n) = Val(sList) * fConvScale
            End If
        Next n
    End If
    
    'Initialize form
    On Error Resume Next
    Call Me.Move(aPos(0), aPos(1))
    On Error GoTo 0
    fMainForm.mnuViewTabObject.Checked = True
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim n As Integer
    
    Dim sList As String
    
    'Cleanup form
    fMainForm.mnuViewTabObject.Checked = False

    'Check position
    If aPos(0) = Me.Left And aPos(1) = Me.Top Then Exit Sub
    
    'Set position
    aPos(0) = Me.Left
    aPos(1) = Me.Top
    
    'Reset list
    sList = ""
    For n = 0 To 1
        'Append list
        sList = sList + "|" + Format(aPos(n) / fConvScale, "0.0;-0.0")
    Next n
    
    'Put window
    Call misPutListByKey(MIS_SEC_COM, MIS_KEY_OBJT, sList, MIS_MOD_INI)
End Sub

Private Sub txtFile_Change()
    'Reset color
    txtFile.BackColor = vbWindowBackground
End Sub

Private Sub txtFile_OLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
    'Check data
    If Data.GetFormat(vbCFFiles) Then
        txtFile.Text = Data.Files.Item(1)
    End If
End Sub

Private Sub txtFile_OLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
    'Check data
    If Data.GetFormat(vbCFFiles) Then
        'Inform the source of the action to be taken
        Effect = vbDropEffectCopy And Effect
        Exit Sub
    End If
    
    'No drop
    Effect = vbDropEffectNone
End Sub

Private Sub txtInfo_Change()
    'Reset color
    txtInfo.BackColor = vbWindowBackground
End Sub

Private Sub txtPosX_Change()
    'Reset color
    txtPosX.BackColor = vbWindowBackground
End Sub

Private Sub txtPosY_Change()
    'Reset color
    txtPosY.BackColor = vbWindowBackground
End Sub

Private Sub txtPosZ_Change()
    'Reset color
    txtPosZ.BackColor = vbWindowBackground
End Sub

Private Sub txtRotX_Change()
    'Reset color
    txtRotX.BackColor = vbWindowBackground
End Sub

Private Sub txtRotY_Change()
    'Reset color
    txtRotY.BackColor = vbWindowBackground
End Sub

Private Sub txtRotZ_Change()
    'Reset color
    txtRotZ.BackColor = vbWindowBackground
End Sub

Private Sub txtScaleX_Change()
    Dim nObj As Long
    
    Dim fSizeX As Single
    Dim fSizeY As Single
    Dim fSizeZ As Single
    
    'Check flag
    If bSize = True Then Exit Sub
    
    'Reset color
    txtScaleX.BackColor = vbWindowBackground
    
    'Check scale
    If bSel = True Then
        txtSizeX.Text = ""
        Exit Sub
    End If
    
    'Get size
    Call rendFindObj(nObj, Val(Mid(sCurK, 2)))
    Call rendGetObjSize(nObj, fSizeX, fSizeY, fSizeZ)
    
    'Put data in control
    bScale = True
    txtSizeX.Text = Format(fSizeX * Val(txtScaleX.Text) / 100, "0;-#")
    bScale = False
End Sub

Private Sub txtScaleY_Change()
    Dim nObj As Long
    
    Dim fSizeX As Single
    Dim fSizeY As Single
    Dim fSizeZ As Single
    
    'Check flag
    If bSize = True Then Exit Sub
    
    'Reset color
    txtScaleY.BackColor = vbWindowBackground
    
    'Check scale
    If bSel = True Then
        txtSizeY.Text = ""
        Exit Sub
    End If
    
    'Get size
    Call rendFindObj(nObj, Val(Mid(sCurK, 2)))
    Call rendGetObjSize(nObj, fSizeX, fSizeY, fSizeZ)
    
    'Put data in control
    bScale = True
    txtSizeY.Text = Format(fSizeY * Val(txtScaleY.Text) / 100, "0;-#")
    bScale = False
End Sub

Private Sub txtScaleZ_Change()
    Dim nObj As Long
    
    Dim fSizeX As Single
    Dim fSizeY As Single
    Dim fSizeZ As Single
    
    'Check flag
    If bSize = True Then Exit Sub
    
    'Reset color
    txtScaleZ.BackColor = vbWindowBackground
    
    'Check scale
    If bSel = True Then
        txtSizeZ.Text = ""
        Exit Sub
    End If
    
    'Get size
    Call rendFindObj(nObj, Val(Mid(sCurK, 2)))
    Call rendGetObjSize(nObj, fSizeX, fSizeY, fSizeZ)
    
    'Put data in control
    bScale = True
    txtSizeZ.Text = Format(fSizeZ * Val(txtScaleZ.Text) / 100, "0;-#")
    bScale = False
End Sub

Private Sub txtSizeX_Change()
    Dim nObj As Long
    
    Dim fSizeX As Single
    Dim fSizeY As Single
    Dim fSizeZ As Single
    
    'Check flag
    If bScale = True Then Exit Sub
    
    'Check scale
    If bSel = True Then
        txtScaleX.Text = ""
        Exit Sub
    End If
    
    'Get size
    Call rendFindObj(nObj, Val(Mid(sCurK, 2)))
    Call rendGetObjSize(nObj, fSizeX, fSizeY, fSizeZ)
    If fSizeX < 0.1 Then fSizeX = 0.1
    
    'Put data in control
    bSize = True
    txtScaleX.Text = Format(Val(txtSizeX.Text) * 100 / fSizeX, "0;-#")
    bSize = False
End Sub

Private Sub txtSizeY_Change()
    Dim nObj As Long
    
    Dim fSizeX As Single
    Dim fSizeY As Single
    Dim fSizeZ As Single
    
    'Check flag
    If bScale = True Then Exit Sub
    
    'Check scale
    If bSel = True Then
        txtScaleY.Text = ""
        Exit Sub
    End If
    
    'Get size
    Call rendFindObj(nObj, Val(Mid(sCurK, 2)))
    Call rendGetObjSize(nObj, fSizeX, fSizeY, fSizeZ)
    If fSizeY < 0.1 Then fSizeY = 0.1
    
    'Put data in control
    bSize = True
    txtScaleY.Text = Format(Val(txtSizeY.Text) * 100 / fSizeY, "0;-#")
    bSize = False
End Sub

Private Sub txtSizeZ_Change()
    Dim nObj As Long
    
    Dim fSizeX As Single
    Dim fSizeY As Single
    Dim fSizeZ As Single
    
    'Check flag
    If bScale = True Then Exit Sub
    
    'Check scale
    If bSel = True Then
        txtScaleZ.Text = ""
        Exit Sub
    End If
    
    'Get size
    Call rendFindObj(nObj, Val(Mid(sCurK, 2)))
    Call rendGetObjSize(nObj, fSizeX, fSizeY, fSizeZ)
    If fSizeZ < 0.1 Then fSizeZ = 0.1
    
    'Put data in control
    bSize = True
    txtScaleZ.Text = Format(Val(txtSizeZ.Text) * 100 / fSizeZ, "0;-#")
    bSize = False
End Sub
