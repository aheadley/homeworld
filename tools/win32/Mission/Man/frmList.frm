VERSION 5.00
Object = "{6B7E6392-850A-101B-AFC0-4210102A8DA7}#1.3#0"; "Comctl32.ocx"
Begin VB.Form frmList 
   Caption         =   "List"
   ClientHeight    =   3192
   ClientLeft      =   60
   ClientTop       =   348
   ClientWidth     =   4680
   Icon            =   "frmList.frx":0000
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   ScaleHeight     =   3192
   ScaleWidth      =   4680
   Begin ComctlLib.ListView lvList 
      Height          =   2955
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   4395
      _ExtentX        =   7747
      _ExtentY        =   5207
      View            =   3
      LabelEdit       =   1
      Sorted          =   -1  'True
      LabelWrap       =   -1  'True
      HideSelection   =   -1  'True
      _Version        =   327682
      ForeColor       =   -2147483640
      BackColor       =   -2147483643
      BorderStyle     =   1
      Appearance      =   1
      NumItems        =   3
      BeginProperty ColumnHeader(1) {0713E8C7-850A-101B-AFC0-4210102A8DA7} 
         Key             =   ""
         Object.Tag             =   ""
         Text            =   "Name"
         Object.Width           =   2540
      EndProperty
      BeginProperty ColumnHeader(2) {0713E8C7-850A-101B-AFC0-4210102A8DA7} 
         SubItemIndex    =   1
         Key             =   ""
         Object.Tag             =   ""
         Text            =   "Value"
         Object.Width           =   2540
      EndProperty
      BeginProperty ColumnHeader(3) {0713E8C7-850A-101B-AFC0-4210102A8DA7} 
         SubItemIndex    =   2
         Key             =   ""
         Object.Tag             =   ""
         Text            =   "Info"
         Object.Width           =   2540
      EndProperty
   End
End
Attribute VB_Name = "frmList"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
    
Dim aPos(4) As Single

Dim sParK As String
Dim sCurK As String
    
Sub GetList(ByVal sPKey As String, ByVal sCKey As String)
    Dim nType As Integer
    
    Dim sTxt As String
    Dim sVal As String
    
    'Check flag
    If Not fMainForm.mnuViewList.Checked Then Exit Sub
    
    'Clear tree
    lvList.ListItems.Clear
    
    'Set keys
    If sPKey <> "" Then sParK = sPKey
    sCurK = sCKey
    
    'Reset caption
    Me.Caption = "List"
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check parent
    If Left(sParK, 1) = "l" Then
        'Set caption
        Me.Caption = "List - " + frmLevels.GetName(Val(Mid(sParK, 2)))
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
        Me.Caption = "List - " + sTxt
    End If
    
    'Get attribs
    GetAttribs
    
    'Check count
    If lvList.ListItems.Count > 0 Then
        'Check key
        If sCurK = "" Then sCurK = lvList.ListItems.Item(1).Key
    
        'Set item
        lvList.ListItems.Item(sCurK).Selected = True
    
        'Sel in tree
        frmTree.SelTree (sCurK)
    End If
End Sub

Sub GetAttribs()
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    Dim itmCur As ListItem
    
    'Check recordset
    If rsAttribs.BOF = True Then Exit Sub
    
    'Reset Query
    sQuery = ""
    
    'Check key
    If Left(sParK, 1) = "l" Then
        'Set query
        sQuery = "SELECT * FROM Attrib WHERE Level = " + Mid(sParK, 2) + " AND Object = 0 ORDER BY Key"
    End If
    
    'Check key
    If Left(sParK, 1) = "o" Then
        'Set query
        sQuery = "SELECT * FROM Attrib WHERE Level = 0 AND Object = " + Mid(sParK, 2) + " ORDER BY Key"
    End If
    
    'Check query
    If sQuery = "" Then Exit Sub
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        Set itmCur = lvList.ListItems.Add(, "a" + Trim(Str(rsTemp!Key)), rsTemp!Name)
        itmCur.SubItems(1) = rsTemp!Value
        itmCur.SubItems(2) = rsTemp!Info
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Sub Reset()
    Dim aP(4) As Single
    
    'Set list view position and size
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

    'Set list view position and size
    aPos(0) = 0
    aPos(1) = 0
    aPos(2) = fMainForm.ScaleWidth / 4
    aPos(3) = fMainForm.ScaleHeight
    
    'Reset count
    nCount = 0
    
    'Get window
    Call misGetListByKey(MIS_SEC_COM, MIS_KEY_LISTV, sList, nCount, MIS_MOD_INI)
    
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
    fMainForm.mnuViewList.Checked = True
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim n As Integer
    
    Dim sList As String
    
    'Cleanup form
    fMainForm.mnuViewList.Checked = False

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
    Call misPutListByKey(MIS_SEC_COM, MIS_KEY_LISTV, sList, MIS_MOD_INI)
End Sub

Private Sub Form_Resize()
    On Error Resume Next
    lvList.Move 0, 0, Me.ScaleWidth, Me.ScaleHeight
    lvList.ColumnHeaders.Item(3).Width = Me.ScaleWidth - lvList.ColumnHeaders.Item(2).Width - lvList.ColumnHeaders.Item(1).Width
    On Error GoTo 0
End Sub

Private Sub lvList_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = vbKeyReturn Then
        'Show form
        frmAttribs.Show
        Call frmAttribs.GetAttrib(sParK, sCurK)
        Exit Sub
    End If
    
    If KeyCode = vbKeyDelete Then
        'Commit
        Call CommitDB("Delete Attribute")
        
        'Delete item
        Call frmAttribs.DelAttrib(sParK, sCurK, "")
        Exit Sub
    End If
End Sub

Private Sub lvList_KeyPress(KeyAscii As Integer)
    If KeyAscii = 13 Then KeyAscii = 0
End Sub

Private Sub lvList_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim itmCur As ListItem
    
    ' Set error handler for node
    On Error GoTo ItemErr
    
    ' Get and check node
    Set itmCur = lvList.HitTest(X, Y)
        
    'Set keys
    sCurK = itmCur.Key
    
    If Button = 1 Then
        ' check node
        If itmCur.Selected = False Then
            ' Select node
            itmCur.Selected = True
    
            'Sel in tree
            frmTree.SelTree (sCurK)
            Exit Sub
        End If
    
        'Show form
        frmAttribs.Show
        Call frmAttribs.GetAttrib(sParK, sCurK)
        frmAttribs.SetFocus
        Exit Sub
    End If
    
    If Button = 2 Then
        ' Select node
        itmCur.Selected = True
        
        ' Show popup menu
        Call PopupMenu(fMainForm.mnuPUList, 2)
        Exit Sub
    End If
    Exit Sub
    
ItemErr:
    Exit Sub
End Sub

