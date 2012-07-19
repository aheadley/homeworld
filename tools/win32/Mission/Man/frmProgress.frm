VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmProgress 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "MissionMan Progress"
   ClientHeight    =   1020
   ClientLeft      =   30
   ClientTop       =   330
   ClientWidth     =   3750
   ControlBox      =   0   'False
   Icon            =   "frmProgress.frx":0000
   MaxButton       =   0   'False
   ScaleHeight     =   1020
   ScaleWidth      =   3750
   ShowInTaskbar   =   0   'False
   Begin MSComctlLib.ProgressBar pbProgressBar 
      Height          =   252
      Left            =   120
      TabIndex        =   1
      Top             =   660
      Width           =   3492
      _ExtentX        =   6165
      _ExtentY        =   450
      _Version        =   393216
      Appearance      =   1
   End
   Begin VB.Label lblProgress 
      Caption         =   "Progress"
      Height          =   372
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   3492
      WordWrap        =   -1  'True
   End
End
Attribute VB_Name = "frmProgress"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Sub Init(ByVal sCap As String, ByVal nMax As Integer)
    'Set caption
    lblProgress.Caption = sCap
    
    'Set min and max
    pbProgressBar.Min = 0
    pbProgressBar.Max = nMax
    
    'Check focus flag
    If bFocusFlag = True Then
        'Set mouse pointer
        fMainForm.MousePointer = vbHourglass
        
        'Show form
        Call Me.Move((fMainForm.ScaleWidth - Me.Width) / 2, (fMainForm.ScaleHeight - Me.Height) / 2)
        Me.Show
    End If
End Sub

Sub Update(ByVal nVal As Integer)
    'Set value
    pbProgressBar.Value = nVal
    
    'Check focus flag
    If bFocusFlag = True Then
        'Set mouse pointer
        fMainForm.MousePointer = vbHourglass
    
        'Refresh form
        Me.Refresh
    End If
End Sub

Sub Clean()
    'Hide form
    Me.Hide
    
    'Set mouse pointer
    fMainForm.MousePointer = vbDefault
End Sub
