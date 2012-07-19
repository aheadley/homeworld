VERSION 5.00
Begin VB.Form frmMission 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Mission"
   ClientHeight    =   1932
   ClientLeft      =   48
   ClientTop       =   336
   ClientWidth     =   4644
   Icon            =   "frmMission.frx":0000
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   ScaleHeight     =   1932
   ScaleWidth      =   4644
   Begin VB.TextBox txtInfo 
      Height          =   285
      Left            =   1020
      TabIndex        =   3
      Top             =   960
      Width           =   3495
   End
   Begin VB.OptionButton optMulti 
      Caption         =   "&Multiplayer"
      Enabled         =   0   'False
      Height          =   195
      Left            =   2880
      TabIndex        =   2
      Top             =   600
      Width           =   1635
   End
   Begin VB.OptionButton optSingle 
      Caption         =   "&Single Player"
      Enabled         =   0   'False
      Height          =   195
      Left            =   1080
      TabIndex        =   1
      Top             =   600
      Value           =   -1  'True
      Width           =   1635
   End
   Begin VB.TextBox txtName 
      Height          =   285
      Left            =   1020
      TabIndex        =   0
      Top             =   120
      Width           =   3495
   End
   Begin VB.CommandButton cmdApply 
      Caption         =   "&Apply"
      Height          =   375
      Left            =   3420
      TabIndex        =   6
      Tag             =   "Apply"
      Top             =   1440
      Width           =   1095
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   2220
      TabIndex        =   5
      Tag             =   "Cancel"
      Top             =   1440
      Width           =   1095
   End
   Begin VB.CommandButton cmdOK 
      Caption         =   "&OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   1020
      TabIndex        =   4
      Tag             =   "OK"
      Top             =   1440
      Width           =   1095
   End
   Begin VB.Label lblInfo 
      Caption         =   "Info:"
      Height          =   195
      Left            =   120
      TabIndex        =   9
      Top             =   1020
      Width           =   795
   End
   Begin VB.Label lblType 
      Caption         =   "Type:"
      Height          =   195
      Left            =   120
      TabIndex        =   8
      Top             =   600
      Width           =   795
   End
   Begin VB.Label lblName 
      Caption         =   "Name:"
      Height          =   195
      Left            =   120
      TabIndex        =   7
      Top             =   180
      Width           =   795
   End
End
Attribute VB_Name = "frmMission"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim aPos(4) As Single

Sub GetMission()
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsMission.BOF = True Then
        'Add mission
        AddMission
    End If
    
    'Get data from recordset
    rsMission.MoveFirst
    txtName.Text = rsMission!Name
    If rsMission!Type = 0 Then optSingle.Value = True
    If rsMission!Type > 0 Then optMulti.Value = True
    txtInfo.Text = rsMission!Info
End Sub

Function GetName() As String
    'Set default
    GetName = ""

    'Check recordset
    If rsMission.BOF = True Then Exit Function
    
    'Get data from recordset
    rsMission.MoveFirst
    GetName = rsMission!Name
End Function

Function GetType() As String
    'Set default
    GetType = ""

    'Check recordset
    If rsMission.BOF = True Then Exit Function
    
    'Get data from recordset
    rsMission.MoveFirst
    If rsMission!Type = 0 Then GetType = "Single Player"
    If rsMission!Type > 0 Then GetType = "Multiplayer"
End Function

Function GetPrefix() As String
    'Set default
    GetPrefix = ""

    'Check recordset
    If rsMission.BOF = True Then Exit Function
    
    'Get data from recordset
    rsMission.MoveFirst
    If rsMission!Type = 0 Then GetPrefix = MIS_SEC_SING
    If rsMission!Type > 0 Then GetPrefix = MIS_SEC_MULTI
End Function

Sub PutMission()
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Put data in recordset
    rsMission.MoveFirst
    rsMission.Edit
    rsMission!NumLevels = CountLevels
    If optSingle.Value = True Then rsMission!Type = 0
    If optMulti.Value = True Then rsMission!Type = 1
    rsMission!Name = Trim(txtName.Text)
    rsMission!Info = Trim(txtInfo.Text)
    rsMission.Update
    
    'Edit and select in tree
    frmTree.EditTree ("m")
    frmTree.SelTree ("m")
End Sub

Sub AddMission()
    'Add data to recordset
    rsMission.AddNew
    rsMission!NumLevels = 0
    rsMission!Type = 0
    rsMission!Name = MIS_NAM_MIS
    rsMission!Info = ""
    rsMission.Update
End Sub

Function CheckType() As Boolean
    Dim nMask As Long

    Dim sVal As String

    'Set default
    CheckType = False

    'Check recordset
    If rsMission.BOF = True Then Exit Function
    rsMission.MoveFirst
    If rsMission!Type > 0 Then
        'OK, multiplayer
        CheckType = True
        Exit Function
    End If
    
    'Get bit mask
    Call misGetVal(MIS_SEC_COM, MIS_KEY_BITM, sVal, MIS_MOD_CFG)
    sVal = TruncStr(sVal)
    If sVal <> "" Then
        'Set mask
        nMask = Val(sVal)
    
        'Check bit mask
        If (nMask And MIS_BIT_DEV) = MIS_BIT_DEV Then
            'OK, single player
            CheckType = True
        End If
    End If
End Function

Function CountLevels() As Integer
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Reset count
    CountLevels = 0
    
    'Set query
    sQuery = "SELECT * FROM Levels"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Check recordset
    If rsLevels.BOF = True Then Exit Function
    
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        CountLevels = CountLevels + 1
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Function

Private Sub cmdApply_Click()
    'Commit
    Call CommitDB("Edit Mission")
    
    'Put mission
    PutMission
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdOK_Click()
    'Commit
    Call CommitDB("Edit Mission")
    
    'Put mission
    PutMission
    
    Unload Me
End Sub

Private Sub Form_Activate()
    'Get mission
    GetMission
End Sub

Private Sub Form_Load()
    Dim n As Integer
    Dim nCount As Integer
    
    Dim nPos As Long
    Dim nMask As Long
        
    Dim sVal As String
    Dim sList As String

    'Disable type selection
    optSingle.Enabled = False
    optMulti.Enabled = False
    
    'Get bit mask
    Call misGetVal(MIS_SEC_COM, MIS_KEY_BITM, sVal, MIS_MOD_CFG)
    sVal = TruncStr(sVal)
    If sVal <> "" Then
        'Set mask
        nMask = Val(sVal)
    
        'Check bit mask
        If (nMask And MIS_BIT_DEV) = MIS_BIT_DEV Then
            'Enable type selection
            optSingle.Enabled = True
            optMulti.Enabled = True
        End If
    End If
    
    'Set tree view position
    aPos(0) = fMainForm.ScaleWidth / 4
    aPos(1) = fMainForm.ScaleHeight / 4
    
    'Reset count
    nCount = 0
    
    'Get window
    Call misGetListByKey(MIS_SEC_COM, MIS_KEY_MIST, sList, nCount, MIS_MOD_INI)
    
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
    fMainForm.mnuViewTabMission.Checked = True
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim n As Integer
    
    Dim sList As String
    
    'Cleanup form
    fMainForm.mnuViewTabMission.Checked = False

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
    Call misPutListByKey(MIS_SEC_COM, MIS_KEY_MIST, sList, MIS_MOD_INI)
End Sub



