VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmAttribs 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Attribute"
   ClientHeight    =   2124
   ClientLeft      =   48
   ClientTop       =   336
   ClientWidth     =   4644
   Icon            =   "frmAttribs.frx":0000
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   ScaleHeight     =   2124
   ScaleWidth      =   4644
   Begin VB.ComboBox cmbValue 
      Height          =   288
      Left            =   1200
      Sorted          =   -1  'True
      TabIndex        =   1
      Top             =   120
      Width           =   3312
   End
   Begin VB.ComboBox cmbName 
      Height          =   315
      Left            =   1020
      Sorted          =   -1  'True
      TabIndex        =   2
      Top             =   600
      Width           =   3495
   End
   Begin VB.CommandButton cmdBrowse 
      Caption         =   "4"
      BeginProperty Font 
         Name            =   "Webdings"
         Size            =   8.4
         Charset         =   2
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   300
      Left            =   1020
      TabIndex        =   0
      Tag             =   "Apply"
      ToolTipText     =   "Browse"
      Top             =   120
      Width           =   192
   End
   Begin VB.TextBox txtInfo 
      Height          =   285
      Left            =   1020
      TabIndex        =   3
      Top             =   1080
      Width           =   3495
   End
   Begin VB.CommandButton cmdApply 
      Caption         =   "&Apply"
      Height          =   375
      Left            =   3420
      TabIndex        =   6
      Tag             =   "Apply"
      Top             =   1620
      Width           =   1095
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   2220
      TabIndex        =   5
      Tag             =   "Cancel"
      Top             =   1620
      Width           =   1095
   End
   Begin VB.CommandButton cmdOK 
      Caption         =   "&OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   1020
      TabIndex        =   4
      Tag             =   "OK"
      Top             =   1620
      Width           =   1095
   End
   Begin MSComDlg.CommonDialog cdBrowse 
      Left            =   120
      Top             =   1560
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      CancelError     =   -1  'True
      DialogTitle     =   "Open File"
      Filter          =   "All Files|*.*"
   End
   Begin VB.Label lblValue 
      Caption         =   "Value:"
      Height          =   195
      Left            =   120
      TabIndex        =   9
      Top             =   180
      Width           =   795
   End
   Begin VB.Label lblInfo 
      Caption         =   "Info:"
      Height          =   195
      Left            =   120
      TabIndex        =   8
      Top             =   1140
      Width           =   795
   End
   Begin VB.Label lblName 
      Caption         =   "Name:"
      Height          =   195
      Left            =   120
      TabIndex        =   7
      Top             =   660
      Width           =   795
   End
End
Attribute VB_Name = "frmAttribs"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim bSel As Boolean

Dim aPos(4) As Single

Dim sParK As String
Dim sCurK As String
Dim sListK As String

Sub GetAttrib(ByVal sPKey As String, ByVal sLKey As String)
    Dim nType As Integer
    Dim nInd As Integer
    
    Dim nPos As Long
    
    Dim sCKey As String
    Dim sQuery As String
    Dim sTxt As String
    Dim sVal As String
    
    Dim rsTemp As Recordset
    
    'Reset caption
    Me.Caption = "Attribute"
    
    'Reset color
    cmbValue.BackColor = vbWindowBackground
    cmbName.BackColor = vbWindowBackground
    txtInfo.BackColor = vbWindowBackground

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
        Me.Caption = "Attribute selection"
    Else
        'Clear slection flag
        bSel = False
            
        'Check parent
        If Left(sParK, 1) = "l" Then
            'Set caption
            Me.Caption = "Attribute of " + frmLevels.GetName(Val(Mid(sParK, 2)))
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
            Me.Caption = "Attribute of " + sTxt
        End If
    End If
    
    'Check key
    If sListK = "" Then
        'Put default data in controls
        cmbName.Text = MIS_NAM_ATTRIB
        cmbValue.Text = ""
        txtInfo.Text = ""
    
        'Get names
        GetNames (cmbName.Text)
    
        'Get values
        GetValues (cmbValue.Text)
        Exit Sub
    End If
    
    'Check recordset
    If rsAttribs.BOF = True Then Exit Sub
    
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
        If Left(sCKey, 1) = "a" Then
            'Set query
            sQuery = "SELECT * FROM Attrib WHERE Key = " + Mid(sCKey, 2)
            
            'Open temporary recordset by query
            If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
                
            'Get data from recordset
            rsTemp.MoveFirst
            
            'Check index
            If nInd = 0 Then
                'Set current key
                sCurK = sCKey
            
                'Put data in controls
                cmbName.Text = rsTemp!Name
                cmbValue.Text = rsTemp!Value
                txtInfo.Text = rsTemp!Info
                
                'Get names
                GetNames (cmbName.Text)
                
                'Get values
                GetValues (cmbValue.Text)
            Else
                'Compare data at set colors
                If cmbName.Text <> rsTemp!Name Then cmbName.BackColor = vbButtonFace
                If cmbValue.Text <> rsTemp!Value Then cmbValue.BackColor = vbButtonFace
                If txtInfo.Text <> rsTemp!Info Then txtInfo.BackColor = vbButtonFace
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

Function GetName(ByVal nKey As Long) As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Set default
    GetName = ""
    
    'Check recordset
    If rsAttribs.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Attrib WHERE Key = " + Trim(Str(nKey))
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    GetName = rsTemp!Name
    
    'Close temporary recordset
    rsTemp.Close
End Function

Function GetValue(ByVal nKey As Long) As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Set default
    GetValue = ""
    
    'Check recordset
    If rsAttribs.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Attrib WHERE Key = " + Trim(Str(nKey))
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    GetValue = rsTemp!Value
    
    'Close temporary recordset
    rsTemp.Close
End Function

Sub GetNames(ByVal sName As String)
    Dim n As Integer

    Dim nCount As Integer
    
    Dim nPos As Long
    Dim nType As Integer
    
    Dim sList As String
    
    'Clear combo
    cmbName.Clear
    cmbName.Text = sName
        
    'Reset count
    nCount = 0
    
    'Check parent
    If Left(sParK, 1) = "l" Then
        'Get attribs
        Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_LEV, MIS_KEY_ATTRIB, sList, nCount, MIS_MOD_CFG)
    End If
    
    'Check parent
    If Left(sParK, 1) = "o" Then
        'Get type reference
        nType = frmObjects.GetType(Val(Mid(sParK, 2)))
    
        'Get attribs
        Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_ATTRIB, sList, nCount, MIS_MOD_CFG)
    End If
    
    'Check count
    If nCount = 0 Then Exit Sub
    
    'Truncate attribs
    sList = TruncStr(sList)

    'Loop thru attribs
    For n = 0 To nCount - 1
        'Get position of | character in string
        nPos = InStr(sList, "|")
        
        'If possible, truncate string at | character
        If nPos > 0 Then
            'Add attrib to combo
            cmbName.AddItem (Left(sList, nPos - 1))
            sList = Mid(sList, nPos + 1, Len(sList))
        Else
            'Add attrib to combo
            cmbName.AddItem (sList)
        End If
    Next n
End Sub

Sub GetValues(ByVal sVal As String)
    Dim n As Integer

    Dim nCount As Integer
    
    Dim nPos As Long
    Dim nType As Integer
    
    Dim sList As String
    
    'Clear combo
    cmbValue.Clear
    cmbValue.Text = sVal
        
    'Reset count
    nCount = 0
    
    'Check parent
    If Left(sParK, 1) = "l" Then
        'Get constants
        Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_LEV, MIS_KEY_CONST, sList, nCount, MIS_MOD_CFG)
    End If
    
    'Check parent
    If Left(sParK, 1) = "o" Then
        'Get type
        nType = frmObjects.GetType(Val(Mid(sParK, 2)))
    
        'Get constants
        Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(nType - 1)), MIS_KEY_CONST, sList, nCount, MIS_MOD_CFG)
    End If
    
    'Check count
    If nCount = 0 Then Exit Sub
    
    'Truncate constants
    sList = TruncStr(sList)

    'Loop thru constants
    For n = 0 To nCount - 1
        'Get position of | character in string
        nPos = InStr(sList, "|")
        
        'If possible, truncate string at | character
        If nPos > 0 Then
            'Add constant to combo
            cmbValue.AddItem (Left(sList, nPos - 1))
            sList = Mid(sList, nPos + 1, Len(sList))
        Else
            'Add constant to combo
            cmbValue.AddItem (sList)
        End If
    Next n
End Sub

Sub GetAll(ByVal sKey As String, ByVal sName As String, nKey As Long, sVal As String, sInfo As String)
    Dim bFlag As Boolean

    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Reset data
    sVal = ""
    sInfo = ""
    
    'Check recordset
    If rsAttribs.BOF = True Then
        nKey = -1
        Exit Sub
    End If
    
    'Reset Query
    sQuery = ""
    
    'Check source
    If Left(sKey, 1) = "l" Then
        'Set query
        sQuery = "SELECT * FROM Attrib WHERE Key > " + Str(nKey) + " AND Level = " + Mid(sKey, 2) + " AND Object = 0 AND Name = """ + sName + """ ORDER BY Key"
    End If
    
    'Check source
    If Left(sKey, 1) = "o" Then
        'Set query
        sQuery = "SELECT * FROM Attrib WHERE Key > " + Str(nKey) + " AND Level = 0 AND Object = " + Mid(sKey, 2) + " AND Name = """ + sName + """ ORDER BY Key"
    End If
    
    'Check query
    If sQuery = "" Then
        nKey = -1
        Exit Sub
    End If
        
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then
        nKey = -1
        Exit Sub
    End If
        
    'Get data from recordset
    rsTemp.MoveFirst
    nKey = rsTemp!Key
    sVal = rsTemp!Value
    sInfo = rsTemp!Info
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Function GetKey() As Long
    'Default
    GetKey = 0
    
    'Check form
    If fMainForm.mnuViewTabAttrib.Checked = False Then Exit Function
    
    'Return Key
    GetKey = Val(Mid(sCurK, 2))
End Function

Sub PutAttrib()
    Dim nType As Integer
    
    Dim nPos As Long
    Dim nObj As Long
    Dim nLink As Long
    Dim nRLev As Long
    Dim nRObj As Long
    
    Dim sCKey As String
    Dim sLKey As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
            
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check key
    If sListK = "" Then
        'Add object
        AddAttrib
        Exit Sub
    End If
    
    'Check recordset
    If rsAttribs.BOF = True Then Exit Sub
    
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
        If Left(sCKey, 1) = "a" Then
            'Set query
            sQuery = "SELECT * FROM Attrib WHERE Key = " + Mid(sCKey, 2)
            
            'Open temporary recordset by query
            If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
                
            'Put data in recordset
            rsTemp.MoveFirst
                    
            'Get data from controls
            rsTemp.Edit
            If cmbName.BackColor = vbWindowBackground Then rsTemp!Name = Trim(cmbName.Text)
            If cmbValue.BackColor = vbWindowBackground Then rsTemp!Value = Trim(cmbValue.Text)
            If txtInfo.BackColor = vbWindowBackground Then rsTemp!Info = Trim(txtInfo.Text)
            rsTemp.Update
            
            'Close temporary recordset
            rsTemp.Close
        
            'Edit and select tree
            frmTree.EditTree (sCKey)
        End If
        
        'Check position
        If nPos = 0 Then Exit Do
    Loop
    
    'Select in tree
    frmTree.SelTree ("")
    
    'Get list
    Call frmList.GetList("", "")
    
    'Check parent
    If Left(sParK, 1) <> "o" Then Exit Sub
    
    'Set link
    Call rendFindObj(nObj, Val(Mid(sParK, 2)))
    Call frmObjects.GetAll(Val(Mid(sParK, 2)), nRLev, nRObj, nType)
    Call rendFindObj(nLink, frmObjects.GetLink(Val(Mid(sParK, 2)), nRLev, nRObj, nType))
    Call rendSetObjLink(nObj, nLink)
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub AddAttrib()
    Dim nType As Integer
    
    Dim nObj As Long
    Dim nLink As Long
    Dim nRLev As Long
    Dim nRObj As Long
    
    'Add data to recordset
    rsAttribs.AddNew
    
    'Set key
    sCurK = "a" + Trim(Str(rsAttribs!Key))
    sListK = sCurK

    'Check parent
    If Left(sParK, 1) = "l" Then
        rsAttribs!Level = Val(Mid(sParK, 2))
    Else
        rsAttribs!Level = 0
    End If
    
    'Check parent
    If Left(sParK, 1) = "o" Then
        rsAttribs!Object = Val(Mid(sParK, 2))
    Else
        rsAttribs!Object = 0
    End If
    
    'Get data from controls
    rsAttribs!Name = Trim(cmbName.Text)
    rsAttribs!Value = Trim(cmbValue.Text)
    rsAttribs!Info = Trim(txtInfo.Text)
    rsAttribs.Update
    
    'Add to tree
    Call frmTree.AddTree(sParK, sCurK)
    
    'Select in tree
    Call frmTree.SelTree(sCurK)
    
    'Get list
    Call frmList.GetList(sParK, sCurK)

    'Check parent
    If Left(sParK, 1) <> "o" Then Exit Sub
    
    'Set link
    Call rendFindObj(nObj, Val(Mid(sParK, 2)))
    Call frmObjects.GetAll(Val(Mid(sParK, 2)), nRLev, nRObj, nType)
    Call rendFindObj(nLink, frmObjects.GetLink(Val(Mid(sParK, 2)), nRLev, nRObj, nType))
    Call rendSetObjLink(nObj, nLink)
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub NewAttrib(ByVal sKey As String, ByVal sName As String, ByVal sVal As String, ByVal sInfo As String)
    Dim nKey As Long
    Dim nObj As Long
    Dim nLink As Long

    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check key
    If sKey = "" Then Exit Sub
    If Left(sKey, 1) = "m" Then Exit Sub
    If Left(sKey, 1) = "a" Then Exit Sub
    
    'Add data to recordset
    rsAttribs.AddNew
    nKey = rsAttribs!Key
    
    'Check parent
    If Left(sKey, 1) = "l" Then
        rsAttribs!Level = Val(Mid(sKey, 2))
    Else
        rsAttribs!Level = 0
    End If
    
    'Check parent
    If Left(sKey, 1) = "o" Then
        rsAttribs!Object = Val(Mid(sKey, 2))
    Else
        rsAttribs!Object = 0
    End If
    
    'Set data
    rsAttribs!Name = Trim(sName)
    rsAttribs!Value = Trim(sVal)
    rsAttribs!Info = Trim(sInfo)
    rsAttribs.Update
        
    'Add to tree
    Call frmTree.AddTree(sKey, "a" + Trim(Str(nKey)))
End Sub

Sub EditAttrib(ByVal nKey As Long, ByVal sName As String, ByVal sVal As String, ByVal sInfo As String)
    Dim nType As Integer
    
    Dim nRKey As Long
    Dim nObj As Long
    Dim nLink As Long
    Dim nRLev As Long
    Dim nRObj As Long
    
    Dim sQuery As String
    
    Dim rsTemp As Recordset
            
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsAttribs.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Attrib WHERE Key = " + Str(nKey)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Put data in recordset
    rsTemp.MoveFirst
    nRKey = rsTemp!Object
        
    'Set data
    rsTemp.Edit
    rsTemp!Name = Trim(sName)
    rsTemp!Value = Trim(sVal)
    rsTemp!Info = Trim(sInfo)
    rsTemp.Update
    
    'Close temporary recordset
    rsTemp.Close
    
    'Update form
    If fMainForm.mnuViewTabAttrib.Checked = True And Val(Mid(sCurK, 2)) = nKey Then
        'Update data in controls
        cmbName.Text = Trim(sName)
        cmbValue.Text = Trim(sVal)
        txtInfo.Text = Trim(sInfo)
    End If
    
    'Edit and select in tree
    Call frmTree.EditTree("a" + Trim(Str(nKey)))
    
    'Get list
    Call frmList.GetList(sParK, sCurK)
    
    'Check parent
    If nRKey = 0 Then Exit Sub
    
    'Set link
    Call rendFindObj(nObj, nRKey)
    Call frmObjects.GetAll(nRKey, nRLev, nRObj, nType)
    Call rendFindObj(nLink, frmObjects.GetLink(nRKey, nRLev, nRObj, nType))
    Call rendSetObjLink(nObj, nLink)
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub DelAttrib(ByVal sPKey As String, ByVal sCKey As String, ByVal sFile As String)
    Dim nKey As Long
    Dim nObj As Long
    
    Dim sQuery As String
    
    Dim rsTemp As Recordset
            
    'Check file
    If sFile = "" Then
        'Check DB
        If bDBFlag = False Then Exit Sub
    End If
    
    'Check key
    If sCKey = "" Then Exit Sub
    If Left(sCKey, 1) <> "a" Then Exit Sub
    
    'Check parent
    If sPKey <> "" And sFile = "" Then
        'Prompt user
        If MsgBox("Delete attribute " + GetName(Val(Mid(sCKey, 2))) + "?", vbYesNo Or vbQuestion, "MissionMan") = vbNo Then Exit Sub
    End If
    
    'Check file
    If sFile = "" Then
        'Check recordset
        If rsAttribs.BOF = True Then Exit Sub
    End If
    
    'Set query
    sQuery = "SELECT * FROM Attrib WHERE Key = " + Mid(sCKey, 2)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, sFile) = False Then Exit Sub
        
    'Delete data in recordset
    rsTemp.MoveFirst
    nKey = rsTemp!Object
            
    'Delete data
    rsTemp.Delete
    
    'Close temporary recordset
    Call CloseRecordSetByQuery(rsTemp, sFile)
    
    'Delete from tree
    Call frmTree.DelTree(sCKey)
    
    'Check parent
    If nKey > 0 Then
         'Reset link
         Call rendFindObj(nObj, nKey)
         Call rendSetObjLink(nObj, 0)
    End If
    
    'Check parent
    If sPKey = "" Or sFile <> "" Then Exit Sub
    
    'Select in tree
    Call frmTree.SelTree(sPKey)
    
    'Get list
    Call frmList.GetList(sPKey, "")
    
    'Check parent
    If nKey > 0 And nObj > 0 Then
        'Refresh graphics
        frmFront.Render
        frmTop.Render
        frmSide.Render
        frmCamera.Render
    End If
End Sub

Sub DelAttribs(ByVal sKey As String, ByVal sFile As String)
    Dim nKey As Long
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
        If rsAttribs.BOF = True Then Exit Sub
    End If
    
    'Check key
    If Left(sKey, 1) = "l" Then
        'Set query
        sQuery = "SELECT * FROM Attrib WHERE Level = " + Mid(sKey, 2) + " AND Object = 0 ORDER BY Key"
    End If
    
    'Check key
    If Left(sKey, 1) = "o" Then
        'Set query
        sQuery = "SELECT * FROM Attrib WHERE Level = 0 AND Object = " + Mid(sKey, 2) + " ORDER BY Key"
    End If
        
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, sFile) = False Then Exit Sub
        
    'Delete data in recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Get key
        nKey = rsTemp!Key
    
        'Delete attrib
        rsTemp.Delete
        
        'Delete from tree
        Call frmTree.DelTree("a" + Trim(Str(nKey)))
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    Call CloseRecordSetByQuery(rsTemp, sFile)
End Sub

Sub CopyAttrib(ByVal sSrcKey As String, ByVal sDstKey As String, ByVal bFlag As Boolean, ByVal sSrcFile As String)
    Dim nKey As Long
    
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check source
    If sSrcKey = "" Then Exit Sub
    If Left(sSrcKey, 1) <> "a" Then Exit Sub
    
    'Check destination
    If sDstKey = "" Then Exit Sub
    If Left(sDstKey, 1) = "m" Then Exit Sub
    If Left(sDstKey, 1) = "a" Then Exit Sub
       
    'Check source file
    If sSrcFile = "" Then
        'Check recordset
        If rsAttribs.BOF = True Then Exit Sub
    End If
    
    'Set query
    sQuery = "SELECT * FROM Attrib WHERE Key = " + Mid(sSrcKey, 2)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, sSrcFile) = False Then Exit Sub
        
    'Get data from recordset
    rsTemp.MoveFirst

    'Add data to recordset
    rsAttribs.AddNew
    nKey = rsAttribs!Key
    
    'Check destination
    If Left(sDstKey, 1) = "l" Then
        rsAttribs!Level = Val(Mid(sDstKey, 2))
    Else
        rsAttribs!Level = 0
    End If
    
    'Check destination
    If Left(sDstKey, 1) = "o" Then
        rsAttribs!Object = Val(Mid(sDstKey, 2))
    Else
        rsAttribs!Object = 0
    End If
    
    'Copy data
    rsAttribs!Name = rsTemp!Name
    rsAttribs!Value = rsTemp!Value
    rsAttribs!Info = rsTemp!Info
    rsAttribs.Update
    
    'Add to tree
    Call frmTree.AddTree(sDstKey, "a" + Trim(Str(nKey)))
    
    'Close temporary recordset
    Call CloseRecordSetByQuery(rsTemp, sSrcFile)
    
    'Check flag
    If bFlag = True Then Call DelAttrib("", sSrcKey, sSrcFile)
    
    'Select in tree
    Call frmTree.SelTree("a" + Trim(Str(nKey)))
End Sub

Sub CopyAttribs(ByVal sSrcKey As String, ByVal sDstKey As String, ByVal sSrcFile As String)
    Dim nKey As Long

    Dim sQuery As String
    
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
        If rsAttribs.BOF = True Then Exit Sub
    End If
    
    'Reset Query
    sQuery = ""
    
    'Check source
    If Left(sSrcKey, 1) = "l" Then
        'Set query
        sQuery = "SELECT * FROM Attrib WHERE Level = " + Mid(sSrcKey, 2) + " AND Object = 0 ORDER BY Key"
    End If
    
    'Check source
    If Left(sSrcKey, 1) = "o" Then
        'Set query
        sQuery = "SELECT * FROM Attrib WHERE Level = 0 AND Object = " + Mid(sSrcKey, 2) + " ORDER BY Key"
    End If
    
    'Check query
    If sQuery = "" Then Exit Sub
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, sSrcFile) = False Then Exit Sub
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Add data to recordset
        rsAttribs.AddNew
        nKey = rsAttribs!Key
    
        'Check destination
        If Left(sDstKey, 1) = "l" Then
            rsAttribs!Level = Val(Mid(sDstKey, 2))
        Else
            rsAttribs!Level = 0
        End If
    
        'Check destination
        If Left(sDstKey, 1) = "o" Then
            rsAttribs!Object = Val(Mid(sDstKey, 2))
        Else
            rsAttribs!Object = 0
        End If
    
        'Copy data
        rsAttribs!Name = rsTemp!Name
        rsAttribs!Value = rsTemp!Value
        rsAttribs!Info = rsTemp!Info
        rsAttribs.Update
    
        'Add to tree
        Call frmTree.AddTree(sDstKey, "a" + Trim(Str(nKey)))
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    Call CloseRecordSetByQuery(rsTemp, sSrcFile)
End Sub

Sub BrowseAttrib()
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show open common dialog
    cdBrowse.InitDir = sDataDir
    cdBrowse.ShowOpen
    
    'Find data directory in full filename
    If InStr(1, cdBrowse.FileName, sDataDir, vbTextCompare) > 0 Then
        'Remove data directory
        cmbValue.Text = Mid(cdBrowse.FileName, Len(sDataDir) + 1, Len(cdBrowse.FileName))
    Else
        'Keep full filename
        cmbValue.Text = cdBrowse.FileName
    End If
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub cmbName_Change()
    'Reset color
    cmbName.BackColor = vbWindowBackground
End Sub

Private Sub cmbName_Click()
    'Reset color
    cmbName.BackColor = vbWindowBackground
End Sub
    
Private Sub cmbValue_Change()
    'Reset color
    cmbValue.BackColor = vbWindowBackground
End Sub

Private Sub cmbValue_Click()
    'Reset color
    cmbValue.BackColor = vbWindowBackground
End Sub

Private Sub cmdApply_Click()
    'Commit
    Call CommitDB("Edit Attribute")
    
    'Put attrib
    PutAttrib
    
    'Get names
    If cmbName.BackColor = vbWindowBackground Then GetNames (cmbName.Text)
    
    'Get values
    If cmbValue.BackColor = vbWindowBackground Then GetValues (cmbValue.Text)
End Sub

Private Sub cmdBrowse_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    'Check mouse button
    If Button = 2 Then
        'Show menu
        Call PopupMenu(fMainForm.mnuPUAttribVal, 2)
        Exit Sub
    End If
    
    'Browse
    BrowseAttrib
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdOK_Click()
    'Commit
    Call CommitDB("Edit Attribute")
    
    'Put attrib
    PutAttrib
    
    Unload Me
End Sub

Private Sub Form_Load()
    Dim n As Integer
    Dim nCount As Integer
    
    Dim nPos As Long
        
    Dim sList As String

    'Set tree view position
    aPos(0) = fMainForm.ScaleWidth / 4
    aPos(1) = fMainForm.ScaleHeight / 4
    
    'Reset count
    nCount = 0
    
    'Get window
    Call misGetListByKey(MIS_SEC_COM, MIS_KEY_ATTRIBT, sList, nCount, MIS_MOD_INI)
    
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
    fMainForm.mnuViewTabAttrib.Checked = True
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim n As Integer
    
    Dim sList As String
    
    'Cleanup form
    fMainForm.mnuViewTabAttrib.Checked = False

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
    Call misPutListByKey(MIS_SEC_COM, MIS_KEY_ATTRIBT, sList, MIS_MOD_INI)
End Sub

Private Sub cmbValue_OLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
    'Check data
    If Data.GetFormat(vbCFFiles) Then
        cmbValue.Text = Data.Files.Item(1)
    End If
End Sub

Private Sub cmbValue_OLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
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

