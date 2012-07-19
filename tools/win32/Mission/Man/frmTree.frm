VERSION 5.00
Object = "{6B7E6392-850A-101B-AFC0-4210102A8DA7}#1.3#0"; "Comctl32.ocx"
Begin VB.Form frmTree 
   Caption         =   "Tree"
   ClientHeight    =   3210
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4650
   Icon            =   "FrmTree.frx":0000
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   ScaleHeight     =   3210
   ScaleWidth      =   4650
   Begin VB.ListBox lstTree 
      Height          =   1968
      IntegralHeight  =   0   'False
      ItemData        =   "FrmTree.frx":030A
      Left            =   120
      List            =   "FrmTree.frx":030C
      Sorted          =   -1  'True
      TabIndex        =   1
      Top             =   1080
      Width           =   4395
   End
   Begin ComctlLib.TreeView tvTree 
      CausesValidation=   0   'False
      Height          =   855
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   4395
      _ExtentX        =   7752
      _ExtentY        =   1508
      _Version        =   327682
      HideSelection   =   0   'False
      LabelEdit       =   1
      LineStyle       =   1
      Sorted          =   -1  'True
      Style           =   6
      Appearance      =   1
   End
   Begin VB.PictureBox pbTree 
      BorderStyle     =   0  'None
      Height          =   2940
      Left            =   120
      MousePointer    =   7  'Size N S
      ScaleHeight     =   2940
      ScaleWidth      =   4335
      TabIndex        =   2
      Top             =   120
      Width           =   4332
   End
End
Attribute VB_Name = "frmTree"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
    
'Local variables
Dim bClick As Boolean

Dim nShift As Integer

Dim fMx As Single
Dim fMy As Single
Dim fRy As Single
Dim fHeight As Single

Dim aPos(4) As Single

Dim nodTree As Node

Sub GetTree(ByVal sList As String)
    'Check flag
    If Not fMainForm.mnuViewTree.Checked Then Exit Sub
    
    'Hide tree
    tvTree.Visible = False

    'Clear tree
    tvTree.Nodes.Clear
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check flag
    If Not fMainForm.mnuViewTree.Checked Then Exit Sub
    
    'Get mission
    GetMission
    
    'Get levels
    GetLevels
    
    'Get objects
    GetObjects
    
    'Get attribs
    GetAttribs
      
    'Select tree
    SelTree (sList)

    'Show tree
    tvTree.Visible = True
End Sub

Sub SelTree(ByVal sList As String)
    Dim bFlag As Boolean
    
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Clear flag
    bFlag = False
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Set flag
    If sList <> sListKey Then bFlag = True
    
    'Set list
    If sList <> "" Then sListKey = sList
    
    'Set keys
    sCurKey = FirstStr(sListKey, " ")
    sParKey = ""
       
    'Check current key
    If sCurKey = "a" Then
        'Set current key
        sCurKey = "o"
        
        'Check recordset
        If rsAttribs.BOF = False Then
            'Open temporary recordset by query
            sQuery = "SELECT * FROM Attrib"
            If OpenRecordSetByQuery(sQuery, rsTemp, "") = True Then
                'Get data from recordset
                rsTemp.MoveFirst
                sCurKey = "a" + Trim(Str(rsTemp!Key))
                sListKey = sCurKey
                
                'Close temporary recordset
                rsTemp.Close
            End If
        End If
    End If
    
    'Check current key
    If sCurKey = "o" Then
        'Set current key
        sCurKey = "l"
        
        'Check recordset
        If rsObjects.BOF = False Then
            'Open temporary recordset by query
            sQuery = "SELECT * FROM Objects"
            If OpenRecordSetByQuery(sQuery, rsTemp, "") = True Then
                'Get data from recordset
                rsTemp.MoveFirst
                sCurKey = "o" + Trim(Str(rsTemp!Key))
                sListKey = sCurKey
                
                'Close temporary recordset
                rsTemp.Close
            End If
        End If
    End If
    
    'Check current key
    If sCurKey = "l" Then
        'Set current key
        sCurKey = "m"
        
        'Check recordset
        If rsLevels.BOF = False Then
            'Open temporary recordset by query
            sQuery = "SELECT * FROM Levels"
            If OpenRecordSetByQuery(sQuery, rsTemp, "") = True Then
                'Get data from recordset
                rsTemp.MoveFirst
                sCurKey = "l" + Trim(Str(rsTemp!Key))
                sListKey = sCurKey
                
                'Close temporary recordset
                rsTemp.Close
            End If
        End If
    End If
    
    'Set node error handler
    On Error GoTo NodeErr
    
    'Check flag
    If fMainForm.mnuViewTree.Checked Then
        'Set parent key
        If Left(sCurKey, 1) <> "m" Then
            sParKey = tvTree.Nodes.Item(sCurKey).Parent.Key
        End If
    
        'Select node
        tvTree.Nodes.Item(sCurKey).Selected = True
    End If
    
    'Show list
    If bFlag = True Then ShowList (sListKey)
    
    'Get status
    fMainForm.ShowStatus (sCurKey)
    Exit Sub
    
NodeErr:
    'Select tree
    SelTree (Left(sCurKey, 1))
End Sub

Sub AddTree(ByVal sPKey As String, ByVal sCKey As String)
    Dim sTxt As String
    Dim sVal As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check flag
    If Not fMainForm.mnuViewTree.Checked Then Exit Sub
    
    ' Check key
    If Left(sCKey, 1) = "l" Then
        'Check recordset
        If rsLevels.BOF = True Then Exit Sub
    
        'Open temporary recordset by query
        sQuery = "SELECT * FROM Levels WHERE Key = " + Mid(sCKey, 2)
        If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
        'Get data from recordset
        rsTemp.MoveFirst
        sTxt = rsTemp!Name
        
        'Close temporary recordset
        rsTemp.Close
        
        'Add node
        Set nodTree = tvTree.Nodes.Add(sPKey, tvwChild, sCKey, sTxt)
        nodTree.Expanded = True
        Exit Sub
    End If
    
    ' Check key
    If Left(sCKey, 1) = "o" Then
        'Check recordset
        If rsObjects.BOF = True Then Exit Sub
    
        'Open temporary recordset by query
        sQuery = "SELECT * FROM Objects WHERE Key = " + Mid(sCKey, 2)
        If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
        'Get data from recordset
        rsTemp.MoveFirst
        sTxt = rsTemp!Name
        
        'Get type
        Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(rsTemp!Type - 1)), MIS_KEY_TYPE, sVal, MIS_MOD_CFG)
        sVal = TruncStr(sVal)
        If sVal <> "" Then sTxt = sVal + ": " + sTxt
        
        'Close temporary recordset
        rsTemp.Close
        
        'Add node
        Set nodTree = tvTree.Nodes.Add(sPKey, tvwChild, sCKey, sTxt)
        If Left(sPKey, 1) = "l" Then
            nodTree.Expanded = True
        Else
            nodTree.Expanded = False
        End If
        Exit Sub
    End If
    
    ' Check key
    If Left(sCKey, 1) = "a" Then
        'Check recordset
        If rsAttribs.BOF = True Then Exit Sub
    
        'Open temporary recordset by query
        sQuery = "SELECT * FROM Attrib WHERE Key = " + Mid(sCKey, 2)
        If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
        'Get data from recordset
        rsTemp.MoveFirst
        sTxt = rsTemp!Name
        If rsTemp!Value <> "" Then sTxt = sTxt + ": " + rsTemp!Value
        
        'Close temporary recordset
        rsTemp.Close
        
        'Add node
        Set nodTree = tvTree.Nodes.Add(sPKey, tvwChild, sCKey, sTxt)
        nodTree.Expanded = False
        Exit Sub
    End If
End Sub

Sub EditTree(ByVal sKey As String)
    Dim sTxt As String
    Dim sVal As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check flag
    If Not fMainForm.mnuViewTree.Checked Then Exit Sub
    
    ' Check key
    If Left(sKey, 1) = "m" Then
        rsMission.MoveFirst
        sTxt = rsMission!Name + ": " + frmMission.GetType
    End If
    
    ' Check key
    If Left(sKey, 1) = "l" Then
        'Check recordset
        If rsLevels.BOF = True Then Exit Sub
    
        'Open temporary recordset by query
        sQuery = "SELECT * FROM Levels WHERE Key = " + Mid(sKey, 2)
        If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
        'Get data from recordset
        rsTemp.MoveFirst
        sTxt = rsTemp!Name
        
        'Close temporary recordset
        rsTemp.Close
    End If
    
    ' Check key
    If Left(sKey, 1) = "o" Then
        'Check recordset
        If rsObjects.BOF = True Then Exit Sub
    
        'Open temporary recordset by query
        sQuery = "SELECT * FROM Objects WHERE Key = " + Mid(sKey, 2)
        If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
        'Get data from recordset
        rsTemp.MoveFirst
        sTxt = rsTemp!Name
        
        'Get type
        Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(rsTemp!Type - 1)), MIS_KEY_TYPE, sVal, MIS_MOD_CFG)
        sVal = TruncStr(sVal)
        If sVal <> "" Then sTxt = sVal + ": " + sTxt
        
        'Close temporary recordset
        rsTemp.Close
    End If
    
    ' Check key
    If Left(sKey, 1) = "a" Then
        'Check recordset
        If rsAttribs.BOF = True Then Exit Sub
    
        'Open temporary recordset by query
        sQuery = "SELECT * FROM Attrib WHERE Key = " + Mid(sKey, 2)
        If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
        'Get data from recordset
        rsTemp.MoveFirst
        sTxt = rsTemp!Name
        If rsTemp!Value <> "" Then sTxt = sTxt + ": " + rsTemp!Value
        
        'Close temporary recordset
        rsTemp.Close
    End If
    
    'Edit node
    tvTree.Nodes.Item(sKey).Text = sTxt
End Sub

Sub ShowTree(ByVal sList As String)
    'Set list
    sListKey = sList

    'Set current key
    sCurKey = FirstStr(sListKey, " ")
    
    'Set parent key
    If Left(sCurKey, 1) = "m" Then
        sParKey = ""
    Else
        sParKey = tvTree.Nodes.Item(sCurKey).Parent.Key
    End If
    
    'Get status
    fMainForm.ShowStatus (sCurKey)
        
    ' Check node key
    If Left(sCurKey, 1) = "m" Then
        'Select objects
        Call rendSetSel("o", "")
    End If
    
    ' Check node key
    If Left(sCurKey, 1) = "l" Then
        'Select objects
        Call rendSetSel("o", "")
    End If
    
    ' Check node key
    If Left(sCurKey, 1) = "o" Then
        'Select objects
        Call rendSetSel("o", sListKey)
    End If
    
    ' Check node key
    If Left(sCurKey, 1) = "a" Then
        'Select objects
        Call rendSetSel("o", "")
    End If
    
    'Show list
    ShowList (sListKey)
        
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
    Exit Sub
End Sub

Sub ExpandTree(ByVal bFlag As Boolean)
    Dim nPos As Long
    Dim sList As String
    Dim sKey As String
    
    'Set list
    sList = sListKey
    
    'Loop thru types
    Do
        'Get position of space character in string
        nPos = InStr(sList, " ")
        
        'If possible, truncate string at space character
        If nPos > 0 Then
            'Set key
            sKey = Left(sList, nPos - 1)
            sList = Mid(sList, nPos + 1, Len(sList))
        Else
            'Set key
            sKey = sList
        End If
        
        'Collapse or expand
        tvTree.Nodes.Item(sKey).Expanded = bFlag
        
        'Check position
        If nPos = 0 Then Exit Do
    Loop
End Sub

Sub ShowList(ByVal sList As String)
    Dim nType As Integer
    Dim nIndex As Integer
    Dim nSel As Integer
    
    Dim nPos As Long
    
    Dim sKey As String
    Dim sItem As String
    Dim sTxt As String
    Dim sVal As String

    'Check flag
    If Not fMainForm.mnuViewTree.Checked Then Exit Sub
    
    'Set selection
    nSel = lstTree.ListIndex
        
    'Clear list
    lstTree.Clear
    
    'Initialize index
    nIndex = 0
    
    'Loop thru types
    Do
        'Get position of space character in string
        nPos = InStr(sList, " ")
        
        'If possible, truncate string at space character
        If nPos > 0 Then
            'Set key
            sKey = Left(sList, nPos - 1)
            sList = Mid(sList, nPos + 1, Len(sList))
        Else
            'Set key
            sKey = sList
        End If
        
        'Reset item
        sItem = ""
        
        ' Check key
        If Left(sKey, 1) = "m" Then
            'Set item
            sItem = "Mission " + frmMission.GetName + ": " + frmMission.GetType
        End If
            
        ' Check key
        If Left(sKey, 1) = "l" Then
            'Set item
            sItem = "Level " + frmLevels.GetName(Val(Mid(sKey, 2)))
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
                    
            'Set item
            sItem = "Object " + sTxt
        End If
            
        ' Check key
        If Left(sKey, 1) = "a" Then
            'Get name
            sTxt = frmAttribs.GetName(Val(Mid(sKey, 2)))
        
            'Get value
            sVal = frmAttribs.GetValue(Val(Mid(sKey, 2)))
            
            'Check type
            If sVal <> "" Then sTxt = sTxt + ": " + sVal
                    
            'Set item
            sItem = "Attribute " + sTxt
        End If
        
        'Check item
        If sItem <> "" Then
            'Add item to list
            lstTree.AddItem (sItem)
            lstTree.ItemData(lstTree.NewIndex) = nIndex
            
            'Increment index
            nIndex = nIndex + 1
        End If
        
        'Check position
        If nPos = 0 Then Exit Do
    Loop
    
    'Check selection
    If nSel < 0 Then nSel = 0
    If nSel > lstTree.ListCount - 1 Then nSel = lstTree.ListCount - 1
        
    'Set list index
    lstTree.ListIndex = nSel
End Sub

Function EditList(ByVal sKey As String, ByVal sList As String) As String
    Dim nPos As Long
    
    'Set default
    EditList = ""
    
    'Check for key in list
    nPos = InStr(sList + " ", sKey + " ")
    If nPos = 0 Then Exit Function
    
    'Check position
    If nPos > 2 Then
        'Set beginning
        EditList = Left(sList, nPos - 2)
    Else
        'Set beginning
        EditList = ""
    End If
    
    'Check for space in list
    nPos = InStr(nPos, sList, " ")
    If nPos = 0 Then Exit Function
    
    'Check list
    If EditList = "" Then
        'Set list
        EditList = Mid(sList, nPos + 1)
    Else
        'Set list
        EditList = EditList + " " + Mid(sList, nPos + 1)
    End If
End Function

Sub DelTree(ByVal sCKey As String)
    'Check flag
    If Not fMainForm.mnuViewTree.Checked Then Exit Sub
    
    'Remove node
    tvTree.Nodes.Remove (sCKey)
End Sub

Sub GetMission()
    Dim sTxt As String
    
    'Check recordset
    If rsMission.BOF = True Then
        'Add mission
        frmMission.AddMission
    End If
        
    'Get data from recordset
    rsMission.MoveFirst
    Set nodTree = tvTree.Nodes.Add(, , "m", rsMission!Name + ": " + frmMission.GetType)
    nodTree.Expanded = True
End Sub

Sub GetLevels()
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check recordset
    If rsLevels.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Levels"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Set progress bar
    Call frmProgress.Init("Loading levels...", 100)
    
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        Call frmProgress.Update(rsTemp.PercentPosition)
        Set nodTree = tvTree.Nodes.Add("m", tvwChild, "l" + Trim(Str(rsTemp!Key)), rsTemp!Name)
        nodTree.Expanded = True
        rsTemp.MoveNext
    Loop
    
    'Reset progress bar
    Call frmProgress.Clean
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Sub GetObjects()
    Dim sTxt As String
    Dim sVal As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Objects"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Set progress bar
    Call frmProgress.Init("Loading objects...", 100)
    
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Update progress bar
        Call frmProgress.Update(rsTemp.PercentPosition)
        
        'Get name
        sTxt = rsTemp!Name
        
        'Get type
        Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(rsTemp!Type - 1)), MIS_KEY_TYPE, sVal, MIS_MOD_CFG)
        
        'Truncate type
        sVal = TruncStr(sVal)
        
        'Check type
        If sVal <> "" Then sTxt = sVal + ": " + sTxt
                
        'Set node
        If rsTemp!Level > 0 Then
            Set nodTree = tvTree.Nodes.Add("l" + Trim(Str(rsTemp!Level)), tvwChild, "o" + Trim(Str(rsTemp!Key)), sTxt)
            nodTree.Expanded = True
        End If
        If rsTemp!Object > 0 Then
            Set nodTree = tvTree.Nodes.Add("o" + Trim(Str(rsTemp!Object)), tvwChild, "o" + Trim(Str(rsTemp!Key)), sTxt)
            nodTree.Expanded = False
        End If

        'Continue
        rsTemp.MoveNext
    Loop
    
    'Reset progress bar
    Call frmProgress.Clean
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Sub GetAttribs()
    Dim sTxt As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check recordset
    If rsAttribs.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Attrib"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Set progress bar
    Call frmProgress.Init("Loading attributes...", 100)
    
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Update progress bar
        Call frmProgress.Update(rsTemp.PercentPosition)
        
        sTxt = rsTemp!Name
        If rsTemp!Value <> "" Then sTxt = sTxt + ": " + rsTemp!Value
        If rsTemp!Level > 0 Then Set nodTree = tvTree.Nodes.Add("l" + Trim(Str(rsTemp!Level)), tvwChild, "a" + Trim(Str(rsTemp!Key)), sTxt)
        If rsTemp!Object > 0 Then Set nodTree = tvTree.Nodes.Add("o" + Trim(Str(rsTemp!Object)), tvwChild, "a" + Trim(Str(rsTemp!Key)), sTxt)
        nodTree.Expanded = False
        rsTemp.MoveNext
    Loop
    
    'Reset progress bar
    Call frmProgress.Clean
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Sub DelKey()
    Dim sKey As String
    Dim sList As String

    'Get key
    sKey = GetKey(lstTree.ItemData(lstTree.ListIndex), sListKey)
    If sKey = "" Then Exit Sub
    
    'Check keys
    If sListKey = sKey Then Exit Sub
    
    'Set key list
    sList = EditList(sKey, sListKey)
                
    'Check list and show tree
    If sList <> "" Then ShowTree (sList)
End Sub

Function GetKey(ByVal nIndex As Integer, ByVal sList As String) As String
    Dim n As Integer
    
    Dim nStart As Long
    Dim nStop As Long
    
    'Set default
    GetKey = ""
    
    'Initialize end
    nStop = 1
    
    ' Loop through indexes
    For n = 0 To nIndex
        'Set start
        nStart = nStop
        
        'Set and check end
        nStop = InStr(nStop, sList, " ")
        If nStop = 0 Then Exit For
        nStop = nStop + 1
    Next
    
    'Adjust end
    If nStop = 0 Then nStop = Len(sList) + 2
    
    'Set key
    GetKey = Mid(sList, nStart, nStop - (nStart + 1))
End Function

Sub Reset()
    Dim aP(4) As Single
    
    'Set tree view position and size
    aP(0) = 0
    aP(1) = 0
    aP(2) = fMainForm.ScaleWidth / 4
    aP(3) = fMainForm.ScaleHeight
    
    'Move form
    On Error Resume Next
    Me.WindowState = vbNormal
    Call Me.Move(aP(0), aP(1), aP(2), aP(3))
    On Error GoTo 0
End Sub
    
Private Sub Form_Load()
    Dim n As Integer
    Dim nCount As Integer
    
    Dim nPos As Long
        
    Dim sList As String
      
    'Clear mouse down flag
    bClick = False
    
    'Set tree view position and size
    aPos(0) = 0
    aPos(1) = 0
    aPos(2) = fMainForm.ScaleWidth / 4
    aPos(3) = fMainForm.ScaleHeight
    
    'Reset count
    nCount = 0
    
    'Get window
    Call misGetListByKey(MIS_SEC_COM, MIS_KEY_TREEV, sList, nCount, MIS_MOD_INI)
    
    'Check count
    If nCount > 0 Then
        'Truncate list
        sList = TruncStr(sList)

        'Loop thru list
        For n = 0 To 3
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
    Call Me.Move(aPos(0), aPos(1), aPos(2), aPos(3))
    On Error GoTo 0
    fMainForm.mnuViewTree.Checked = True
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim n As Integer
    
    Dim sList As String
    
    'Cleanup form
    fMainForm.mnuViewTree.Checked = False

    'Check position
    If aPos(0) = Me.Left And aPos(1) = Me.Top And aPos(2) = Me.Width And aPos(3) = Me.Height Then Exit Sub
    
    'Set position
    aPos(0) = Me.Left
    aPos(1) = Me.Top
    aPos(2) = Me.Width
    aPos(3) = Me.Height
    
    'Reset list
    sList = ""
    For n = 0 To 3
        'Append list
        sList = sList + "|" + Format(aPos(n) / fConvScale, "0.0;-0.0")
    Next n
    
    'Put window
    Call misPutListByKey(MIS_SEC_COM, MIS_KEY_TREEV, sList, MIS_MOD_INI)
End Sub

Private Sub Form_Resize()
    'Resize form
    On Error Resume Next
    Call pbTree.Move(0, 0, Me.ScaleWidth, Me.ScaleHeight)
    Call tvTree.Move(0, 0, Me.ScaleWidth, Me.ScaleHeight - lstTree.Height + 50)
    Call lstTree.Move(0, tvTree.Height + 50, Me.ScaleWidth, lstTree.Height)
    On Error GoTo 0
End Sub

Private Sub pbTree_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    'Set mouse down flag
    bClick = True
    
    'Set height
    fHeight = lstTree.Height
    
    'Set mouse coordinates
    fRy = Y
End Sub

Private Sub pbTree_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    'Check mouse down flag
    If (bClick = True) And (Y <> fRy) Then
        'Calculate height
        fHeight = fHeight + fRy - Y
        
        'Check height
        If fHeight < 500 Then fHeight = 500
        If fHeight > Me.ScaleHeight - 500 Then fHeight = Me.ScaleHeight - 500
        
        'Resize controls
        On Error Resume Next
        Call tvTree.Move(0, 0, Me.ScaleWidth, Me.ScaleHeight - fHeight + 50)
        Call lstTree.Move(0, tvTree.Height + 50, Me.ScaleWidth, fHeight)
        On Error GoTo 0
    End If
    
    'Set mouse coordinates
    fRy = Y
End Sub

Private Sub pbTree_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    'Clear mouse down flag
    bClick = False
End Sub

Private Sub lstTree_Click()
    If nShift = 2 Then
        'Reset key
        nShift = 0
        
        'Deselect key
        DelKey
    End If
End Sub

Private Sub lstTree_KeyDown(KeyCode As Integer, Shift As Integer)
    Dim sKey As String
    Dim sList As String

    'Check DB
    If bDBFlag = False Then Exit Sub
    
    If KeyCode = vbKeyReturn Then
        'Show selection property
        fMainForm.GetListProp
        Exit Sub
    End If
    
    If KeyCode = vbKeyDelete Then
        'Commit
        Call CommitDB("Delete")
        
        'Delete item(s)
        Call fMainForm.DelList(sParKey, sListKey, "")
    End If
End Sub

Private Sub lstTree_KeyPress(KeyAscii As Integer)
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    If KeyAscii = 13 Then KeyAscii = 0
    
    'Check key state
    If KeyAscii = Asc("-") Then
        'Collapse tree
        Call ExpandTree(False)
        Exit Sub
    End If
    
    'Check key state
    If KeyAscii = Asc("*") Then
        'Expand tree
        Call ExpandTree(True)
    End If
End Sub

Private Sub lstTree_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Set key
    nShift = Shift
    
    If Button = 2 Then
        ' Show popup menu
        Call PopupMenu(fMainForm.mnuPUTreeSel, 2)
        Exit Sub
    End If
End Sub

Private Sub tvTree_Collapse(ByVal Node As ComctlLib.Node)
    'Show tree
    ShowTree (Node.Key)
End Sub

Private Sub tvTree_DblClick()
    Dim nodItem As Node
    
    'Set error handler for node
    On Error GoTo NodeErr
    
    'Get and check node
    Set nodItem = tvTree.HitTest(fMx, fMy)
        
    'Toggle expand/collapse
    If nodItem.Expanded = True Then
        nodItem.Expanded = False
    Else
        nodItem.Expanded = True
    End If
    
    ' Check node key
    If Left(nodItem.Key, 1) = "m" Then
        'Show form
        frmMission.Show
        frmMission.GetMission
        frmMission.SetFocus
        Exit Sub
    End If
    
    ' Check node key
    If Left(nodItem.Key, 1) = "l" Then
        'Show form
        frmLevels.Show
        Call frmLevels.GetLevel(sParKey, sCurKey)
        frmLevels.SetFocus
        Exit Sub
    End If
    
    ' Check node key
    If Left(nodItem.Key, 1) = "o" Then
        'Select objects
        Call rendSetSel("o", sCurKey)
            
        'Show form
        frmObjects.Show
        Call frmObjects.GetObject(sParKey, sCurKey, 0, 0, 0)
        frmObjects.SetFocus
        Exit Sub
    End If
    
    ' Check node key
    If Left(nodItem.Key, 1) = "a" Then
        'Show form
        frmAttribs.Show
        Call frmAttribs.GetAttrib(sParKey, sCurKey)
        frmAttribs.SetFocus
        Exit Sub
    End If
    Exit Sub

NodeErr:
    Exit Sub
End Sub

Private Sub tvTree_KeyDown(KeyCode As Integer, Shift As Integer)
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    If KeyCode = vbKeyReturn Then
        ' Check node key
        If InStr(sListKey, " ") > 0 Then Exit Sub
      
        'Show selection property
        fMainForm.GetListProp
        Exit Sub
    End If
    
    If KeyCode = vbKeyDelete Then
        ' Check node key
        If InStr(sListKey, " ") > 0 Then
            'Commit
            Call CommitDB("Delete")
            
            'Delete item(s)
            Call fMainForm.DelList(sParKey, sListKey, "")
            Exit Sub
        End If
        
        ' Check node key
        If Left(sCurKey, 1) = "l" Then
            'Commit
            Call CommitDB("Delete Level")
            
            'Delete item
            Call frmLevels.DelLevel(sParKey, sCurKey, "")
            Exit Sub
        End If
        
        ' Check node key
        If Left(sCurKey, 1) = "o" Then
            'Commit
            Call CommitDB("Delete Object")
            
            'Delete item
            Call frmObjects.DelObject(sParKey, sCurKey, "")
            Exit Sub
        End If
        
        ' Check node key
        If Left(sCurKey, 1) = "a" Then
            'Commit
            Call CommitDB("Delete Attribute")
            
            'Delete item
            Call frmAttribs.DelAttrib(sParKey, sCurKey, "")
            Exit Sub
        End If
    End If
End Sub

Private Sub tvTree_KeyPress(KeyAscii As Integer)
    If KeyAscii = 13 Then KeyAscii = 0
End Sub

Private Sub tvTree_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim nodItem As Node
    
    ' Set error handler for node
    On Error GoTo NodeErr
    
    'Set key
    nShift = Shift
    
    'Get and check node
    Set nodItem = tvTree.HitTest(X, Y)
    
    If Button = 2 Then
        ' Select node
        nodItem.Selected = True
    
        'Show tree
        ShowTree (nodItem.Key)
            
        ' Check node key
        If Left(nodItem.Key, 1) = "o" Then
            'Select objects
            Call rendSetSel("o", sCurKey)
            
            'Refresh graphics
            frmFront.Render
            frmTop.Render
            frmSide.Render
            frmCamera.Render
        End If
            
        ' Check node key
        If Left(nodItem.Key, 1) = "m" Then
            ' Show popup menu
            Call PopupMenu(fMainForm.mnuPUTreeMis, 2)
            Exit Sub
        End If
        
        ' Check node key
        If Left(nodItem.Key, 1) = "l" Then
            ' Show popup menu
            Call PopupMenu(fMainForm.mnuPUTreeLev, 2)
            Exit Sub
        End If
        
        ' Check node key
        If Left(nodItem.Key, 1) = "o" Then
            'Select objects
            Call rendSetSel("o", sCurKey)
                
            ' Show popup menu
            Call PopupMenu(fMainForm.mnuPUTreeObj, 2)
            Exit Sub
        End If
        
        ' Check node key
        If Left(nodItem.Key, 1) = "a" Then
            ' Show popup menu
            Call PopupMenu(fMainForm.mnuPUTreeAttrib, 2)
            Exit Sub
        End If
    End If
    Exit Sub
    
NodeErr:
    Exit Sub
End Sub

Private Sub tvTree_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim nodItem As Node
    
    'Set mouse coordinates
    fMx = X
    fMy = Y

    'Set tooltip
    tvTree.ToolTipText = ""
    
    ' Set error handler for node
    On Error GoTo NodeErr
    
    ' Get and check node
    Set nodItem = tvTree.HitTest(X, Y)
        
    ' Check node key
    If Left(nodItem.Key, 1) = "m" Then
        'Set tooltip
        tvTree.ToolTipText = "Mission"
        Exit Sub
    End If
        
    ' Check node key
    If Left(nodItem.Key, 1) = "l" Then
        'Set tooltip
        tvTree.ToolTipText = "Level"
        Exit Sub
    End If
        
    ' Check node key
    If Left(nodItem.Key, 1) = "o" Then
        'Set tooltip
        tvTree.ToolTipText = "Object"
        Exit Sub
    End If
    
    ' Check node key
    If Left(nodItem.Key, 1) = "a" Then
        'Set tooltip
        tvTree.ToolTipText = "Attribute"
        Exit Sub
    End If
    Exit Sub
    
NodeErr:
    Exit Sub
End Sub

Private Sub tvTree_NodeClick(ByVal Node As ComctlLib.Node)
    Dim sList As String

    'Check key
    If nShift = 2 Then
        'Reset key
        nShift = 0
        
        'Check keys
        If sListKey = Node.Key Then
            sList = sListKey
        Else
            'Set key list
            sList = EditList(Node.Key, sListKey)
            If sList = "" Then sList = Node.Key + " " + sListKey
        End If
    Else
        'Set key list
        sList = Node.Key
    End If
    
    'Show tree
    ShowTree (sList)
    Exit Sub
End Sub
