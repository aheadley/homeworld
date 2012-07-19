VERSION 5.00
Begin VB.Form FrmInfo 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "VolTool"
   ClientHeight    =   384
   ClientLeft      =   48
   ClientTop       =   336
   ClientWidth     =   2544
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   384
   ScaleWidth      =   2544
   ShowInTaskbar   =   0   'False
   StartUpPosition =   2  'CenterScreen
   Begin VB.Label LblInfo 
      Caption         =   "Saving Data! Please Wait..."
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.6
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   252
      Left            =   60
      TabIndex        =   0
      Top             =   60
      Width           =   2412
   End
End
Attribute VB_Name = "FrmInfo"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

