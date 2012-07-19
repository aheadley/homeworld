VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmLayers 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Layers"
   ClientHeight    =   6084
   ClientLeft      =   48
   ClientTop       =   336
   ClientWidth     =   4644
   Icon            =   "frmLayers.frx":0000
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   ScaleHeight     =   6084
   ScaleWidth      =   4644
   Begin VB.CheckBox chkAll 
      Caption         =   "All"
      Height          =   195
      Left            =   1020
      TabIndex        =   1
      Top             =   3780
      Width           =   1035
   End
   Begin VB.PictureBox picColor 
      Height          =   315
      Left            =   1200
      ScaleHeight     =   264
      ScaleWidth      =   3264
      TabIndex        =   14
      Top             =   4680
      Width           =   3315
   End
   Begin VB.CommandButton cmdColor 
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
      Height          =   315
      Left            =   1020
      TabIndex        =   5
      Tag             =   "Apply"
      ToolTipText     =   "Browse"
      Top             =   4680
      Width           =   195
   End
   Begin MSComDlg.CommonDialog cdColor 
      Left            =   120
      Top             =   3060
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      CancelError     =   -1  'True
   End
   Begin VB.TextBox txtInfo 
      Height          =   285
      Left            =   1020
      TabIndex        =   6
      Top             =   5160
      Width           =   3495
   End
   Begin VB.TextBox txtName 
      Height          =   285
      Left            =   1020
      TabIndex        =   4
      Top             =   4260
      Width           =   3495
   End
   Begin VB.CommandButton cmdDelete 
      Caption         =   "&Delete"
      Height          =   375
      Left            =   3420
      TabIndex        =   3
      Tag             =   "Delete"
      Top             =   3720
      Width           =   1095
   End
   Begin VB.CommandButton cmdNew 
      Caption         =   "&New"
      Height          =   375
      Left            =   2220
      TabIndex        =   2
      Tag             =   "OK"
      Top             =   3720
      Width           =   1095
   End
   Begin VB.CommandButton cmdApply 
      Caption         =   "&Apply"
      Height          =   375
      Left            =   3420
      TabIndex        =   9
      Tag             =   "Apply"
      Top             =   5580
      Width           =   1095
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   2220
      TabIndex        =   8
      Tag             =   "Cancel"
      Top             =   5580
      Width           =   1095
   End
   Begin VB.CommandButton cmdOK 
      Caption         =   "&OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   1020
      TabIndex        =   7
      Tag             =   "OK"
      Top             =   5580
      Width           =   1095
   End
   Begin VB.ListBox lstList 
      Height          =   3288
      Left            =   1020
      Sorted          =   -1  'True
      Style           =   1  'Checkbox
      TabIndex        =   0
      Top             =   120
      Width           =   3495
   End
   Begin VB.Label lblList 
      Caption         =   "List:"
      Height          =   195
      Left            =   120
      TabIndex        =   13
      Top             =   180
      Width           =   795
   End
   Begin VB.Label lblInfo 
      Caption         =   "Info:"
      Height          =   195
      Left            =   120
      TabIndex        =   12
      Top             =   5220
      Width           =   795
   End
   Begin VB.Label lblCol0 
      Caption         =   "Color:"
      Height          =   195
      Left            =   120
      TabIndex        =   11
      Top             =   4740
      Width           =   795
   End
   Begin VB.Label lblName 
      Caption         =   "Name:"
      Height          =   195
      Left            =   120
      TabIndex        =   10
      Top             =   4320
      Width           =   795
   End
End
Attribute VB_Name = "frmLayers"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim bList As Boolean

Dim aPos(4) As Single

Sub GetLayers(ByVal nKey As Long)
    Dim n As Integer
    
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check flag
    If Not fMainForm.mnuViewTabLayer.Checked Then Exit Sub
    
    'Clear list
    lstList.Clear
    
    'Reset data
    txtName.Text = ""
    picColor.BackColor = vbWhite
    txtInfo.Text = ""
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsLayers.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Layers"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
    
    'Set flag
    bList = True
        
    'Find data in recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Get layer data
        lstList.AddItem (rsTemp!Name)
        lstList.ItemData(lstList.NewIndex) = rsTemp!Key
        If rsTemp!Color < 0 Then lstList.Selected(lstList.NewIndex) = False
        If rsTemp!Color > 0 Then lstList.Selected(lstList.NewIndex) = True
        rsTemp.MoveNext
    Loop
    
    'Clear flag
    bList = False
        
    'Close temporary recordset
    rsTemp.Close
    
    'Check list  index
    If lstList.ListCount > 0 Then
        'Loop thru list
        For n = 0 To lstList.ListCount - 1
            'Check list items
            If lstList.ItemData(n) = nKey Then
                lstList.ListIndex = n
                Exit For
            End If
        Next n
    
        'Reset list index
        If lstList.ListIndex < 0 Then lstList.ListIndex = 0
        
        'Get layer
        Call GetLayer(lstList.ItemData(lstList.ListIndex))
    End If
End Sub

Sub GetLayer(ByVal nKey As Long)
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check list index
    If lstList.ListCount = 0 Then Exit Sub
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsLayers.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Layers WHERE Key = " + Str(nKey)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Find data in recordset
    rsTemp.MoveFirst
    txtName.Text = rsTemp!Name
    picColor.BackColor = Abs(rsTemp!Color)
    txtInfo.Text = rsTemp!Info
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Sub GetColor(ByVal nKey As Long, nMode As Long, nCol As Long)
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Reset mode and color
    nMode = 1
    nCol = nViewCol
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsLayers.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Layers WHERE Key = " + Str(nKey)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Find data in recordset
    rsTemp.MoveFirst
    If rsTemp!Color < 0 Then nMode = 0
    If rsTemp!Color > 0 Then nMode = 1
    nCol = Abs(rsTemp!Color)
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Function GetName(ByVal nKey As Long) As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Set default
    GetName = ""
    
    'Check recordset
    If rsLayers.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Layers WHERE Key = " + Str(nKey)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    GetName = rsTemp!Name
    
    'Close temporary recordset
    rsTemp.Close
End Function

Sub PutLayer(ByVal nKey As Long)
    Dim n As Integer
    Dim nItem As Integer
    Dim nCount As Integer
    
    Dim sList As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check list index
    If lstList.ListCount = 0 Then Exit Sub
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsLayers.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Layers WHERE Key = " + Str(nKey)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Loop thru list
    For n = 0 To lstList.ListCount - 1
        'Check list items
        If lstList.ItemData(n) = nKey Then
            nItem = n
            Exit For
        End If
    Next n
    
    'Edit data in recordset
    rsTemp.MoveFirst
    rsTemp.Edit
    rsTemp!Name = txtName.Text
    If lstList.Selected(nItem) = False Then rsTemp!Color = -picColor.BackColor
    If lstList.Selected(nItem) = True Then rsTemp!Color = picColor.BackColor
    rsTemp!Info = txtInfo.Text
    rsTemp.Update
    
    'Close temporary recordset
    rsTemp.Close
    
    'Update objects
    If lstList.Selected(nItem) = False Then Call frmObjects.PutColor(nKey, 0, picColor.BackColor)
    If lstList.Selected(nItem) = True Then Call frmObjects.PutColor(nKey, 1, picColor.BackColor)
        
    'Get selection
    Call rendGetSel("o", nCount, sList)
    
    'Check count
    If nCount > 1 Then
        'Truncate list
        sList = TruncStr(sList)

        'Select in tree
        frmTree.SelTree (sList)
    End If
    
    'Get layers
    Call GetLayers(lstList.ItemData(lstList.ListIndex))
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub TogLayer(ByVal nKey As Long)
    Dim n As Integer
    Dim nItem As Integer
    Dim nCount As Integer
    
    Dim nMode As Long
    Dim nCol As Long
    
    Dim sList As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check list index
    If lstList.ListCount = 0 Then Exit Sub
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsLayers.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Layers WHERE Key = " + Str(nKey)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Loop thru list
    For n = 0 To lstList.ListCount - 1
        'Check list items
        If lstList.ItemData(n) = nKey Then
            nItem = n
            Exit For
        End If
    Next n

    'Find data in recordset
    rsTemp.MoveFirst
    nCol = Abs(rsTemp!Color)
    
    'Edit data in recordset
    rsTemp.Edit
    If lstList.Selected(nItem) = False Then rsTemp!Color = -nCol
    If lstList.Selected(nItem) = True Then rsTemp!Color = nCol
    rsTemp.Update
    
    'Close temporary recordset
    rsTemp.Close
    
    'Update objects
    If lstList.Selected(nItem) = False Then Call frmObjects.PutColor(nKey, 0, nCol)
    If lstList.Selected(nItem) = True Then Call frmObjects.PutColor(nKey, 1, nCol)
    
    'Get selection
    Call rendGetSel("o", nCount, sList)
    
    'Check count
    If nCount > 1 Then
        'Truncate list
        sList = TruncStr(sList)

        'Select in tree
        frmTree.SelTree (sList)
    End If
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub AddLayer()
    Dim nKey As Long

    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Add new data to recordset
    rsLayers.AddNew
    nKey = rsLayers!Key
    
    'Default data
    rsLayers!Name = "New Layer"
    rsLayers!Color = nViewCol
    rsLayers!Info = ""
    rsLayers.Update
    
    'Get layers
    Call GetLayers(nKey)
End Sub

Sub DelLayer(ByVal nKey As Long)
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check list index
    If lstList.ListCount = 0 Then Exit Sub
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsLayers.BOF = True Then Exit Sub
    
    'Prompt user
    If MsgBox("Delete layer " + GetName(lstList.ItemData(lstList.ListIndex)) + "?", vbYesNo Or vbQuestion, "MissionMan") = vbNo Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Layers WHERE Key = " + Str(nKey)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Delete data in recordset
    rsTemp.MoveFirst
    rsTemp.Delete
    
    'Close temporary recordset
    rsTemp.Close
        
    'Update object
    Call frmObjects.DelLayer(nKey)
    
    'Get layers
    Call GetLayers(lstList.ItemData(lstList.ListIndex))

    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub SetLayer(ByVal nKey As Long, ByVal nMode As Long)
    Dim n As Integer
    Dim nItem As Integer
    
    Dim nCol As Long
    
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsLayers.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Layers WHERE Key = " + Str(nKey)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Find data in recordset
    rsTemp.MoveFirst
    nCol = Abs(rsTemp!Color)
    
    'Edit data in recordset
    rsTemp.Edit
    If nMode = 0 Then rsTemp!Color = -nCol
    If nMode = 1 Then rsTemp!Color = nCol
    rsTemp.Update
    
    'Close temporary recordset
    rsTemp.Close
    
    'Update objects
    Call frmObjects.PutColor(nKey, nMode, nCol)
    
    'Update form
    If fMainForm.mnuViewTabLayer.Checked = True Then
        'Get layers
        Call GetLayers(lstList.ItemData(lstList.ListIndex))
    End If
End Sub

Private Sub chkAll_Click()
    Dim n As Integer

    'Check checkbox
    If chkAll.Value = 0 Then
        'Loop thru list
        For n = 0 To lstList.ListCount - 1
            'Check list items
            lstList.Selected(n) = False
        Next n
        Exit Sub
    End If
    
    'Check checkbox
    If chkAll.Value = 1 Then
        'Loop thru list
        For n = 0 To lstList.ListCount - 1
            'Check list items
            lstList.Selected(n) = True
        Next n
        Exit Sub
    End If
End Sub

Private Sub cmdApply_Click()
    'Check list
    If lstList.ListCount > 0 Then
        'Commit
        Call CommitDB("Edit Layer")
        
        'Put layer
        Call PutLayer(lstList.ItemData(lstList.ListIndex))
    End If
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdDelete_Click()
    'Check list
    If lstList.ListCount > 0 Then
        'Commit
        Call CommitDB("Delete Layer")
    
        'Delete Layer
        Call DelLayer(lstList.ItemData(lstList.ListIndex))
    End If
End Sub

Private Sub cmdNew_Click()
    'Commit
    Call CommitDB("New Layer")
    
    'Add Layer
    AddLayer
End Sub

Private Sub cmdOK_Click()
    'Check list
    If lstList.ListCount > 0 Then
        'Commit
        Call CommitDB("Edit Layer")
        
        'Put layer
        Call PutLayer(lstList.ItemData(lstList.ListIndex))
    End If
    
    Unload Me
End Sub

Private Sub Form_Load()
    Dim n As Integer
    Dim nCount As Integer
    
    Dim nPos As Long
        
    Dim sList As String

    'Clear flag
    bList = False
        
    'Set tree view position
    aPos(0) = fMainForm.ScaleWidth / 4
    aPos(1) = fMainForm.ScaleHeight / 4
    
    'Reset count
    nCount = 0
    
    'Get window
    Call misGetListByKey(MIS_SEC_COM, MIS_KEY_LAYERT, sList, nCount, MIS_MOD_INI)
    
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
    fMainForm.mnuViewTabLayer.Checked = True
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim n As Integer
    
    Dim sList As String
    
    'Cleanup form
    fMainForm.mnuViewTabLayer.Checked = False

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
    Call misPutListByKey(MIS_SEC_COM, MIS_KEY_LAYERT, sList, MIS_MOD_INI)
End Sub

Private Sub cmdColor_Click()
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show open common dialog
    cdColor.Color = picColor.BackColor
    cdColor.Flags = &H1
    cdColor.ShowColor
    
    'Set color
    picColor.BackColor = cdColor.Color
    
    'Set change
    cmdApply_Click
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub lstList_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = vbKeyDelete Then cmdDelete_Click
End Sub

Private Sub lstlist_KeyPress(KeyAscii As Integer)
    If KeyAscii = 13 Then KeyAscii = 0
End Sub

Private Sub lstList_Click()
    'Check flag
    If bList = True Then Exit Sub
    
    'Get layer
    If lstList.ListCount > 0 Then Call GetLayer(lstList.ItemData(lstList.ListIndex))
End Sub

Private Sub lstList_ItemCheck(Item As Integer)
    'Check flag
    If bList = True Then Exit Sub
        
    'Check list
    If lstList.ListCount > 0 Then
        'Toggle layer
        Call TogLayer(lstList.ItemData(Item))
    End If
End Sub

