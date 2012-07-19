VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form FrmVol 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "VolTool"
   ClientHeight    =   1332
   ClientLeft      =   48
   ClientTop       =   336
   ClientWidth     =   5112
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   1332
   ScaleWidth      =   5112
   StartUpPosition =   2  'CenterScreen
   Begin VB.CommandButton CmdBrowse 
      Caption         =   "&Browse..."
      Height          =   372
      Left            =   3900
      TabIndex        =   3
      Top             =   300
      Width           =   1092
   End
   Begin VB.TextBox TxtHead 
      Height          =   252
      Left            =   120
      OLEDropMode     =   1  'Manual
      TabIndex        =   2
      Top             =   360
      Width           =   3612
   End
   Begin VB.CommandButton CmdOpen 
      Caption         =   "&Open Header..."
      Default         =   -1  'True
      Height          =   372
      Left            =   120
      TabIndex        =   1
      Top             =   840
      Width           =   1632
   End
   Begin VB.CommandButton CmdExit 
      Cancel          =   -1  'True
      Caption         =   "E&xit"
      Height          =   372
      Left            =   3900
      TabIndex        =   0
      Top             =   840
      Width           =   1092
   End
   Begin MSComDlg.CommonDialog CDHead 
      Left            =   4260
      Top             =   240
      _ExtentX        =   699
      _ExtentY        =   699
      _Version        =   393216
      CancelError     =   -1  'True
      DialogTitle     =   "Open Header"
      Filter          =   "Headers (*.h)|*.h|All Files|*.*"
   End
   Begin VB.Label LblHead 
      Caption         =   "Header:"
      Height          =   195
      Left            =   120
      TabIndex        =   4
      Top             =   120
      Width           =   3615
   End
End
Attribute VB_Name = "FrmVol"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub CmdBrowse_Click()
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show open common dialog
    CDHead.ShowOpen
    TxtHead.Text = CDHead.FileName
    
    'Set header file
    sHeadFile = TxtHead.Text
    
    'Get header labels
    GetHeadLabels
    If nHeadCount = 0 Then Exit Sub
    
    'Show lookup form
    Hide
    FrmLookup.Show
    Exit Sub
    
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub CmdExit_Click()
    'End program
    AntiMain
End Sub

Private Sub CmdOpen_Click()
    'Set header file
    sHeadFile = TxtHead.Text
    
    'Get header labels
    GetHeadLabels
    If nHeadCount = 0 Then Exit Sub
    
    'Show lookup form
    Hide
    FrmLookup.Show
End Sub

Private Sub Form_Activate()
    'Get header file
    TxtHead.Text = sHeadFile
End Sub

Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
    'Cancel unload
    Cancel = 1
    
    'End program
    AntiMain
End Sub

Private Sub TxtSFX_OLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
    'Check data
    If Data.GetFormat(vbCFFiles) Then
        TxtHead.Text = Data.Files.Item(1)
    End If
End Sub

Private Sub TxtSFX_OLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
    'Check data
    If Data.GetFormat(vbCFFiles) Then
        'Inform the source of the action to be taken
        Effect = vbDropEffectCopy And Effect
        Exit Sub
    End If
    
    'No drop
    Effect = vbDropEffectNone
End Sub

Sub GetHeadLabels()
    Dim nErr As Long
    Dim sText As String
    Dim sLabels As String
    
    'Reset header count
    nHeadCount = 0
    
    'Get header labels
    nErr = volGetLabels(sHeadFile, sLabels, nHeadCount)
    If nErr < 0 Then
        'Inform User
        Call volGetErr(nErr, sText)
        Call MsgBox("Error: " + TruncStr(sText) + " (header)!", vbOKOnly Or vbExclamation, "VolTool")
        Exit Sub
    End If
    
    'Set header labels
    sHeadLabels = TruncStr(sLabels)
End Sub

