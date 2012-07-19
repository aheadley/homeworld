VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Object = "{5E9E78A0-531B-11CF-91F6-C2863C385E30}#1.0#0"; "MSFLXGRD.OCX"
Object = "{6B7E6392-850A-101B-AFC0-4210102A8DA7}#1.3#0"; "comctl32.ocx"
Begin VB.Form FrmLookup 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "VolTool"
   ClientHeight    =   8244
   ClientLeft      =   36
   ClientTop       =   336
   ClientWidth     =   10164
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   8244
   ScaleWidth      =   10164
   StartUpPosition =   2  'CenterScreen
   Begin VB.Frame FmeFreq 
      Caption         =   "Frequency (3)"
      Height          =   2355
      Index           =   2
      Left            =   180
      TabIndex        =   58
      Top             =   5220
      Visible         =   0   'False
      Width           =   5475
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   16
         Left            =   600
         TabIndex        =   59
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -100
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   17
         Left            =   1140
         TabIndex        =   60
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   18
         Left            =   1680
         TabIndex        =   61
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   19
         Left            =   2220
         TabIndex        =   62
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   20
         Left            =   2760
         TabIndex        =   63
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   21
         Left            =   3300
         TabIndex        =   64
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   22
         Left            =   3840
         TabIndex        =   65
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   23
         Left            =   4380
         TabIndex        =   66
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin VB.Label LblMax 
         Caption         =   "100%"
         Height          =   195
         Index           =   5
         Left            =   4920
         TabIndex        =   82
         Top             =   240
         Width           =   495
      End
      Begin VB.Label LblMax 
         Alignment       =   1  'Right Justify
         Caption         =   "100%"
         Height          =   195
         Index           =   4
         Left            =   60
         TabIndex        =   81
         Top             =   240
         Width           =   495
      End
      Begin VB.Label LblMid 
         Caption         =   "50%"
         Height          =   195
         Index           =   5
         Left            =   4920
         TabIndex        =   80
         Top             =   1020
         Width           =   495
      End
      Begin VB.Label LblMid 
         Alignment       =   1  'Right Justify
         Caption         =   "50%"
         Height          =   195
         Index           =   4
         Left            =   60
         TabIndex        =   79
         Top             =   1020
         Width           =   495
      End
      Begin VB.Label LblMin 
         Caption         =   "0%"
         Height          =   195
         Index           =   5
         Left            =   4920
         TabIndex        =   78
         Top             =   1740
         Width           =   495
      End
      Begin VB.Label LblMin 
         Alignment       =   1  'Right Justify
         Caption         =   "0%"
         Height          =   195
         Index           =   4
         Left            =   60
         TabIndex        =   77
         Top             =   1740
         Width           =   495
      End
      Begin VB.Label LblHz 
         Caption         =   "Hz"
         Height          =   195
         Index           =   5
         Left            =   4920
         TabIndex        =   76
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblHz 
         Alignment       =   1  'Right Justify
         Caption         =   "Hz"
         Height          =   195
         Index           =   4
         Left            =   60
         TabIndex        =   75
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "11 k"
         Height          =   195
         Index           =   23
         Left            =   4380
         TabIndex        =   74
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "5.5 k"
         Height          =   195
         Index           =   22
         Left            =   3840
         TabIndex        =   73
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "2.7 k"
         Height          =   195
         Index           =   21
         Left            =   3300
         TabIndex        =   72
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "1.3 k"
         Height          =   195
         Index           =   20
         Left            =   2760
         TabIndex        =   71
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "689"
         Height          =   195
         Index           =   19
         Left            =   2220
         TabIndex        =   70
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "344"
         Height          =   195
         Index           =   18
         Left            =   1680
         TabIndex        =   69
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "172"
         Height          =   195
         Index           =   17
         Left            =   1140
         TabIndex        =   68
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "86"
         Height          =   195
         Index           =   16
         Left            =   600
         TabIndex        =   67
         Top             =   2040
         Width           =   495
      End
   End
   Begin VB.Frame FmeFreq 
      Caption         =   "Frequency (2)"
      Height          =   2355
      Index           =   1
      Left            =   180
      TabIndex        =   33
      Top             =   2820
      Width           =   5475
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   8
         Left            =   600
         TabIndex        =   34
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -100
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   9
         Left            =   1140
         TabIndex        =   35
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   10
         Left            =   1680
         TabIndex        =   36
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   11
         Left            =   2220
         TabIndex        =   37
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   12
         Left            =   2760
         TabIndex        =   38
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   13
         Left            =   3300
         TabIndex        =   39
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   14
         Left            =   3840
         TabIndex        =   40
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -100
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   15
         Left            =   4380
         TabIndex        =   41
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin VB.Label LblMax 
         Caption         =   "100%"
         Height          =   195
         Index           =   3
         Left            =   4920
         TabIndex        =   57
         Top             =   240
         Width           =   495
      End
      Begin VB.Label LblMax 
         Alignment       =   1  'Right Justify
         Caption         =   "100%"
         Height          =   195
         Index           =   2
         Left            =   60
         TabIndex        =   56
         Top             =   240
         Width           =   495
      End
      Begin VB.Label LblMid 
         Caption         =   "50%"
         Height          =   195
         Index           =   3
         Left            =   4920
         TabIndex        =   55
         Top             =   1020
         Width           =   495
      End
      Begin VB.Label LblMid 
         Alignment       =   1  'Right Justify
         Caption         =   "50%"
         Height          =   195
         Index           =   2
         Left            =   60
         TabIndex        =   54
         Top             =   1020
         Width           =   495
      End
      Begin VB.Label LblMin 
         Caption         =   "0%"
         Height          =   195
         Index           =   3
         Left            =   4920
         TabIndex        =   53
         Top             =   1740
         Width           =   495
      End
      Begin VB.Label LblMin 
         Alignment       =   1  'Right Justify
         Caption         =   "0%"
         Height          =   195
         Index           =   2
         Left            =   60
         TabIndex        =   52
         Top             =   1740
         Width           =   495
      End
      Begin VB.Label LblHz 
         Caption         =   "Hz"
         Height          =   195
         Index           =   3
         Left            =   4920
         TabIndex        =   51
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblHz 
         Alignment       =   1  'Right Justify
         Caption         =   "Hz"
         Height          =   195
         Index           =   2
         Left            =   60
         TabIndex        =   50
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "11 k"
         Height          =   195
         Index           =   15
         Left            =   4380
         TabIndex        =   49
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "5.5 k"
         Height          =   195
         Index           =   14
         Left            =   3840
         TabIndex        =   48
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "2.7 k"
         Height          =   195
         Index           =   13
         Left            =   3300
         TabIndex        =   47
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "1.3 k"
         Height          =   195
         Index           =   12
         Left            =   2760
         TabIndex        =   46
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "689"
         Height          =   195
         Index           =   11
         Left            =   2220
         TabIndex        =   45
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "344"
         Height          =   195
         Index           =   10
         Left            =   1680
         TabIndex        =   44
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "172"
         Height          =   195
         Index           =   9
         Left            =   1140
         TabIndex        =   43
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "86"
         Height          =   195
         Index           =   8
         Left            =   600
         TabIndex        =   42
         Top             =   2040
         Width           =   495
      End
   End
   Begin VB.CommandButton CmdReset 
      Caption         =   "&Reset"
      Height          =   372
      Left            =   8880
      TabIndex        =   32
      ToolTipText     =   "Reset Curve"
      Top             =   7200
      Visible         =   0   'False
      Width           =   1092
   End
   Begin VB.Frame FmeFreq 
      Caption         =   "Frequency (1)"
      Height          =   2355
      Index           =   0
      Left            =   180
      TabIndex        =   7
      Top             =   420
      Visible         =   0   'False
      Width           =   5475
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   0
         Left            =   600
         TabIndex        =   8
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -100
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   1
         Left            =   1140
         TabIndex        =   9
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   2
         Left            =   1680
         TabIndex        =   10
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   3
         Left            =   2220
         TabIndex        =   11
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   4
         Left            =   2760
         TabIndex        =   12
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   5
         Left            =   3300
         TabIndex        =   13
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   6
         Left            =   3840
         TabIndex        =   14
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin ComctlLib.Slider SldLev 
         Height          =   1815
         Index           =   7
         Left            =   4380
         TabIndex        =   15
         Top             =   180
         Width           =   510
         _ExtentX        =   889
         _ExtentY        =   3196
         _Version        =   327682
         Orientation     =   1
         Min             =   -100
         Max             =   0
         SelStart        =   -50
         TickStyle       =   2
         TickFrequency   =   10
         Value           =   -100
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "86"
         Height          =   195
         Index           =   0
         Left            =   600
         TabIndex        =   31
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "172"
         Height          =   195
         Index           =   1
         Left            =   1140
         TabIndex        =   30
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "344"
         Height          =   195
         Index           =   2
         Left            =   1680
         TabIndex        =   29
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "689"
         Height          =   195
         Index           =   3
         Left            =   2220
         TabIndex        =   28
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "1.3 k"
         Height          =   195
         Index           =   4
         Left            =   2760
         TabIndex        =   27
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "2.7 k"
         Height          =   195
         Index           =   5
         Left            =   3300
         TabIndex        =   26
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "5.5 k"
         Height          =   195
         Index           =   6
         Left            =   3840
         TabIndex        =   25
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblFreq 
         Alignment       =   2  'Center
         Caption         =   "11 k"
         Height          =   195
         Index           =   7
         Left            =   4380
         TabIndex        =   24
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblHz 
         Alignment       =   1  'Right Justify
         Caption         =   "Hz"
         Height          =   195
         Index           =   0
         Left            =   60
         TabIndex        =   23
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblHz 
         Caption         =   "Hz"
         Height          =   195
         Index           =   1
         Left            =   4920
         TabIndex        =   22
         Top             =   2040
         Width           =   495
      End
      Begin VB.Label LblMin 
         Alignment       =   1  'Right Justify
         Caption         =   "0%"
         Height          =   195
         Index           =   0
         Left            =   60
         TabIndex        =   21
         Top             =   1740
         Width           =   495
      End
      Begin VB.Label LblMin 
         Caption         =   "0%"
         Height          =   195
         Index           =   1
         Left            =   4920
         TabIndex        =   20
         Top             =   1740
         Width           =   495
      End
      Begin VB.Label LblMid 
         Alignment       =   1  'Right Justify
         Caption         =   "50%"
         Height          =   195
         Index           =   0
         Left            =   60
         TabIndex        =   19
         Top             =   1020
         Width           =   495
      End
      Begin VB.Label LblMid 
         Caption         =   "50%"
         Height          =   195
         Index           =   1
         Left            =   4920
         TabIndex        =   18
         Top             =   1020
         Width           =   495
      End
      Begin VB.Label LblMax 
         Alignment       =   1  'Right Justify
         Caption         =   "100%"
         Height          =   195
         Index           =   0
         Left            =   60
         TabIndex        =   17
         Top             =   240
         Width           =   495
      End
      Begin VB.Label LblMax 
         Caption         =   "100%"
         Height          =   195
         Index           =   1
         Left            =   4920
         TabIndex        =   16
         Top             =   240
         Width           =   495
      End
   End
   Begin VB.CommandButton CmdSave 
      Caption         =   "&Save"
      Height          =   372
      Left            =   1260
      TabIndex        =   4
      Top             =   7800
      Width           =   1095
   End
   Begin VB.TextBox TxtLookup 
      BorderStyle     =   0  'None
      Height          =   375
      Left            =   8460
      TabIndex        =   3
      Top             =   7800
      Visible         =   0   'False
      Width           =   435
   End
   Begin VB.CommandButton CmdOPen 
      Caption         =   "&Open..."
      Height          =   372
      Left            =   60
      TabIndex        =   2
      Top             =   7800
      Width           =   1095
   End
   Begin VB.CommandButton CmdSaveAs 
      Caption         =   "Save &As..."
      Height          =   372
      Left            =   2460
      TabIndex        =   0
      Top             =   7800
      Width           =   1095
   End
   Begin VB.CommandButton CmdClose 
      Cancel          =   -1  'True
      Caption         =   "&Close"
      Height          =   372
      Left            =   9000
      TabIndex        =   1
      Top             =   7800
      Width           =   1092
   End
   Begin MSComDlg.CommonDialog CDLookup 
      Left            =   3660
      Top             =   7800
      _ExtentX        =   699
      _ExtentY        =   699
      _Version        =   393216
      CancelError     =   -1  'True
      Filter          =   "Lookup Tables (*.lut)|*.lut|All Files|*.*"
      Flags           =   2
   End
   Begin MSFlexGridLib.MSFlexGrid FGLookup 
      Height          =   7215
      Left            =   120
      TabIndex        =   6
      Top             =   420
      Visible         =   0   'False
      Width           =   9915
      _ExtentX        =   17484
      _ExtentY        =   12721
      _Version        =   393216
      Rows            =   50
      Cols            =   50
      FillStyle       =   1
      AllowUserResizing=   3
   End
   Begin ComctlLib.TabStrip TSLookup 
      Height          =   7635
      Left            =   60
      TabIndex        =   5
      Top             =   60
      Width           =   10035
      _ExtentX        =   17695
      _ExtentY        =   13462
      _Version        =   327682
      BeginProperty Tabs {0713E432-850A-101B-AFC0-4210102A8DA7} 
         NumTabs         =   2
         BeginProperty Tab1 {0713F341-850A-101B-AFC0-4210102A8DA7} 
            Caption         =   "Volume && Range"
            Object.Tag             =   ""
            ImageVarType    =   2
         EndProperty
         BeginProperty Tab2 {0713F341-850A-101B-AFC0-4210102A8DA7} 
            Caption         =   "Frequency"
            Object.Tag             =   ""
            ImageVarType    =   2
         EndProperty
      EndProperty
   End
End
Attribute VB_Name = "FrmLookup"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Sub ShowTab()
   'Check tab index
   If TSLookup.SelectedItem.Index = 1 Then
        'Set controls
        FmeFreq(0).Visible = False
        FmeFreq(1).Visible = False
        FmeFreq(2).Visible = False
        CmdReset.Visible = False
        FGLookup.Visible = True
        Exit Sub
   End If
   
   'Check tab index
   If TSLookup.SelectedItem.Index = 2 Then
        'Set controls
        FGLookup.Visible = False
        FmeFreq(0).Visible = True
        FmeFreq(1).Visible = True
        FmeFreq(2).Visible = True
        CmdReset.Visible = True
   End If
End Sub

Sub ShowLabels()
    Dim n, m As Integer
    Dim nPos As Integer
    Dim sData As String
    Dim sNum As String
    
    'Clear grid
    FGLookup.Clear
    
    'Set grid size
    FGLookup.Cols = nVRCount * 2 + 1
    FGLookup.Rows = nHeadCount + 1
    
    'Set row
    FGLookup.Row = 0
    
    'Loop thru volume/range labels
    For n = 0 To nVRCount * 2 - 1
        'Set column
        FGLookup.Col = n + 1
    
        'Set volume/range label
        If n Mod 2 = 0 Then FGLookup.Text = "Volume" + " (" + Str(n / 2 + 1) + ")"
        If n Mod 2 = 1 Then FGLookup.Text = "Range" + " (" + Str((n - 1) / 2 + 1) + ")"
    Next n
    
    'Set column
    FGLookup.Col = 0
    
    'Loop thru header labels
    sData = sHeadLabels
    For n = 0 To nHeadCount - 1
        'Set column
        FGLookup.Row = n + 1
    
        'Get position of space character in string
        nPos = InStr(sData, " ")
    
        'Set number
        sNum = " (" + Str(n) + ")"
    
        'If possible, truncate string at space character
        If nPos > 0 Then
            'Set header label
            FGLookup.Text = Left(sData, nPos - 1) + sNum
            sData = Mid(sData, nPos + 1, Len(sData))
        Else
            'Set header label
            FGLookup.Text = sData + sNum
        End If
    Next n
    
    'Set row and column
    FGLookup.Col = 1
    FGLookup.Row = 1
End Sub

Function OpenLookup(ByVal sFileName As String, ByVal nOffset As Integer) As Boolean
    Dim i As Integer, j As Integer, k As Integer
    
    Dim nVolRC As Integer
    Dim nHeaderC As Integer
    
    Dim nErr As Long
    
    Dim aLookup() As Long
    
    Dim sText As String
    
    'Default
    OpenLookup = True
    
    'Set mouse pointer
    MousePointer = vbHourglass
    
    'Dimension data array
    ReDim aLookup(nVRCount)
    
    'Open lookup table
    nErr = volOpenRLookup(sFileName, nVolRC, nHeaderC)
    If nErr < 0 Then
        'Reset mouse pointer
        MousePointer = vbDefault
        
        'Inform User
        Call volGetErr(nErr, sText)
        Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
        
        'Error
        OpenLookup = False
        Exit Function
    End If

    'Check volume/range count
    If nVolRC <> nVRCount Then
        'Reset mouse pointer
        MousePointer = vbDefault
        
        'Inform User
        Call MsgBox("Error: Invalid number of columns in lookup table (" + Str(nVolRC) + ")!", vbOKOnly Or vbExclamation, "VolTool")
        
        'Close lookup table
        Call volCloseRLookup
        
        'Error
        OpenLookup = False
        Exit Function
    End If

    'Check header count
    If nHeaderC <> nHeadCount Then
        'Reset mouse pointer
        MousePointer = vbDefault
        
        'Inform User
        Call MsgBox("Error: Invalid number of rows in lookup table (" + Str(nHeaderC) + ")!", vbOKOnly Or vbExclamation, "VolTool")
        
        'Close lookup table
        Call volCloseRLookup
        
        'Error
        OpenLookup = False
        Exit Function
    End If

    'Loop thru lookup table
    For i = 0 To nHeadCount - 1
        'Read from lookup table
        nErr = volReadLookup(aLookup(0))
        If nErr < 0 Then
            'Set row and column
            FGLookup.Col = 1
            FGLookup.Row = 1
            
            'Reset mouse pointer
            MousePointer = vbDefault
                
            'Inform User
            Call volGetErr(nErr, sText)
            Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
                
            'Close lookup table
            Call volCloseRLookup
            
            'Error
            OpenLookup = False
            Exit Function
        End If
        
        'Set row
        FGLookup.Row = i + 1
    
        For j = 0 To nVRCount * 2 - 1
            'Set column
            FGLookup.Col = j + 1
            
            'Set data
            If j Mod 2 = nOffset Then FGLookup.Text = Str(aLookup((j - nOffset) / 2))
        Next j
    Next i
    
    'Close lookup table
    nErr = volCloseRLookup()
    If nErr < 0 Then
        'Set row and column
        FGLookup.Col = 1
        FGLookup.Row = 1
        
        'Reset mouse pointer
        MousePointer = vbDefault
        
        'Inform User
        Call volGetErr(nErr, sText)
        Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
        
        'Error
        OpenLookup = False
        Exit Function
    End If
        
    'Set row and column
    FGLookup.Col = 1
    FGLookup.Row = 1
    
    'Reset mouse pointer
    MousePointer = vbDefault
End Function

Function OpenFreq(ByVal sFileName As String) As Boolean
    Dim i As Integer, j As Integer
    
    Dim nVolRC As Integer
    
    Dim nErr As Long
    
    Dim aFreq(8) As Single
    
    Dim sText As String
    
    'Default
    OpenFreq = True
    
    'Set mouse pointer
    MousePointer = vbHourglass
    
    'Open frequency data
    nErr = volOpenRFreq(sFileName, nVolRC)
    If nErr < 0 Then
        'Reset mouse pointer
        MousePointer = vbDefault
        
        'Inform User
        Call volGetErr(nErr, sText)
        Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
        
        'Error
        OpenFreq = False
        Exit Function
    End If

    'Check volume/range count
    If nVolRC <> nVRCount Then
        'Reset mouse pointer
        MousePointer = vbDefault
        
        'Inform User
        Call MsgBox("Error: Invalid number of rows in frequency data (" + Str(nVolRC) + ")!", vbOKOnly Or vbExclamation, "VolTool")
        
        'Close frequency data
        Call volCloseRFreq
        
        'Error
        OpenFreq = False
        Exit Function
    End If

    'Loop thru frequency data
    For i = 0 To nVRCount - 1
        'Read frequency data
        nErr = volReadFreq(aFreq(0))
        If nErr < 0 Then
            'Reset mouse pointer
            MousePointer = vbDefault
                
            'Inform User
            Call volGetErr(nErr, sText)
            Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
                
            'Close frequency data
            Call volCloseRFreq
            
            'Error
            OpenFreq = False
            Exit Function
        End If
        
        'Loop thru level data
        For j = 0 To 7
            SldLev(j + (i * 8)).Value = -aFreq(j) * 100
        Next j
    Next i
    
    'Close frequency data
    nErr = volCloseRFreq()
    If nErr < 0 Then
        'Reset mouse pointer
        MousePointer = vbDefault
        
        'Inform User
        Call volGetErr(nErr, sText)
        Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
        
        'Error
        OpenFreq = False
        Exit Function
    End If
        
    'Reset mouse pointer
    MousePointer = vbDefault
End Function

Function SaveLookup(ByVal sFileName As String, ByVal sID As String, ByVal nOffset As Integer) As Boolean
    Dim i As Integer, j As Integer, k As Integer
    Dim nRow As Integer, nCol As Integer
    
    Dim nErr As Long
    
    Dim aLookup() As Long
    
    Dim sText As String
    
    'Default
    SaveLookup = True
    
    'Set mouse pointer
    MousePointer = vbHourglass
    
    'Dimension data array
    ReDim aLookup(nVRCount)
    
    'Open lookup table
    nErr = volOpenWLookup(sFileName, sID, nVRCount, nHeadCount)
    If nErr < 0 Then
        'Reset mouse pointer
        MousePointer = vbDefault
        
        'Inform User
        Call volGetErr(nErr, sText)
        Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
        
        'Error
        SaveLookup = False
        Exit Function
    End If

    'Get row and column
    nCol = FGLookup.Col
    nRow = FGLookup.Row
    
    'Loop thru lookup table
    For i = 0 To nHeadCount - 1
        'Set row
        FGLookup.Row = i + 1
    
        For j = 0 To nVRCount * 2 - 1
            'Set column
            FGLookup.Col = j + 1
            
            'Set data
            If j Mod 2 = nOffset Then aLookup((j - nOffset) / 2) = Val(FGLookup.Text)
        Next j
            
        'Write to lookup table
        nErr = volWriteLookup(aLookup(0))
        If nErr < 0 Then
            'Set row and column
            FGLookup.Col = nCol
            FGLookup.Row = nRow
            
            'Reset mouse pointer
            MousePointer = vbDefault
                
            'Inform User
            Call volGetErr(nErr, sText)
            Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
                
            'Close lookup table
            Call volCloseWLookup
            
            'Error
            SaveLookup = False
            Exit Function
        End If
    Next i
    
    'Close lookup table
    nErr = volCloseWLookup()
    If nErr < 0 Then
        'Set row and column
        FGLookup.Col = nCol
        FGLookup.Row = nRow
            
        'Reset mouse pointer
        MousePointer = vbDefault
        
        'Inform User
        Call volGetErr(nErr, sText)
        Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
        
        'Error
        SaveLookup = False
        Exit Function
    End If
    
    'Set row and column
    FGLookup.Col = nCol
    FGLookup.Row = nRow
        
    'Reset mouse pointer
    MousePointer = vbDefault
End Function

Function SaveFreq(ByVal sFileName As String, ByVal sID As String) As Boolean
    Dim i As Integer, j As Integer
   
    Dim nErr As Long
    
    Dim aFreq(8) As Single
    
    Dim sText As String
    
    'Default
    SaveFreq = True
    
    'Set mouse pointer
    MousePointer = vbHourglass
    
    'Open frequency data
    nErr = volOpenWFreq(sFileName, sID, nVRCount)
    If nErr < 0 Then
        'Reset mouse pointer
        MousePointer = vbDefault
        
        'Inform User
        Call volGetErr(nErr, sText)
        Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
        
        'Error
        SaveFreq = False
        Exit Function
    End If

    'Loop thru frequency data
    For i = 0 To nVRCount - 1
        'Loop thru level data
        For j = 0 To 7
            aFreq(j) = -SldLev(j + (i * 8)).Value / 100
        Next j
        
        'Write frequency data
        nErr = volWriteFreq(aFreq(0))
        If nErr < 0 Then
            'Reset mouse pointer
            MousePointer = vbDefault
                
            'Inform User
            Call volGetErr(nErr, sText)
            Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
                
            'Close frequency data
            Call volCloseWFreq
            
            'Error
            SaveFreq = False
            Exit Function
        End If
    Next i
    
    'Close frequency data
    nErr = volCloseWFreq()
    If nErr < 0 Then
        'Reset mouse pointer
        MousePointer = vbDefault
        
        'Inform User
        Call volGetErr(nErr, sText)
        Call MsgBox("Error: " + TruncStr(sText) + "!", vbOKOnly Or vbExclamation, "VolTool")
        
        'Error
        SaveFreq = False
        Exit Function
    End If
    
    'Reset mouse pointer
    MousePointer = vbDefault
End Function

Private Sub CmdClose_Click()
    'Hide form
    Hide
    FrmVol.Show
End Sub

Private Sub CmdOpen_Click()
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show common dialog
    CDLookup.FileName = sVolFile
    CDLookup.DefaultExt = ""
    CDLookup.DialogTitle = "Load Volume Lookup Table"
    CDLookup.ShowOpen
    sVolFile = CDLookup.FileName
    
    'Reset handler for Cancel button
    On Error GoTo 0
    
    'Open volume lookup table
    If OpenLookup(sVolFile, 0) = False Then Exit Sub
    
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show common dialog
    CDLookup.FileName = sRangeFile
    CDLookup.DefaultExt = ""
    CDLookup.DialogTitle = "Load Range Lookup Table"
    CDLookup.ShowOpen
    sRangeFile = CDLookup.FileName
    
    'Reset handler for Cancel button
    On Error GoTo 0
    
    'Open range lookup table
    If OpenLookup(sRangeFile, 1) = False Then Exit Sub
    
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show common dialog
    CDLookup.FileName = sFreqFile
    CDLookup.DefaultExt = ""
    CDLookup.DialogTitle = "Load Frequency Data"
    CDLookup.ShowOpen
    sFreqFile = CDLookup.FileName
    
    'Reset handler for Cancel button
    On Error GoTo 0
    
    'Open frequency data
    If OpenFreq(sFreqFile) = False Then Exit Sub
    Exit Sub
        
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub CmdReset_Click()
    Dim n As Integer
    
    'Reset all sliders
    For n = 0 To 23
        SldLev(n).Value = -100
    Next n
End Sub

Private Sub CmdSave_Click()
    'Show info
    FrmInfo.Show
    FrmInfo.Refresh
    
    'Save volume lookup table
    If SaveLookup(sVolFile, sVolID, 0) = False Then GoTo Error
    
    'Save range lookup table
    If SaveLookup(sRangeFile, sRangeID, 1) = False Then GoTo Error
    
    'Save frequency data
    If SaveFreq(sFreqFile, sFreqID) = False Then GoTo Error
    
Error:
    'Hide info
    FrmInfo.Hide
    Exit Sub
End Sub

Private Sub CmdSaveAs_Click()
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show common dialog
    CDLookup.FileName = sVolFile
    CDLookup.DefaultExt = ".lut"
    CDLookup.DialogTitle = "Save Volume Lookup Table"
    CDLookup.ShowSave
    sVolFile = CDLookup.FileName
    
    'Reset handler for Cancel button
    On Error GoTo 0
    
    'Save volume lookup table
    If SaveLookup(sVolFile, sVolID, 0) = False Then Exit Sub
    
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show common dialog
    CDLookup.FileName = sRangeFile
    CDLookup.DefaultExt = ".lut"
    CDLookup.DialogTitle = "Save Range Lookup Table"
    CDLookup.ShowSave
    sRangeFile = CDLookup.FileName
    
    'Reset handler for Cancel button
    On Error GoTo 0
    
    'Save range lookup table
    If SaveLookup(sRangeFile, sRangeID, 1) = False Then Exit Sub
    
    'Set handler for Cancel button
    On Error GoTo Cancel
    
    'Show common dialog
    CDLookup.FileName = sFreqFile
    CDLookup.DefaultExt = ".lut"
    CDLookup.DialogTitle = "Save Frequency Data"
    CDLookup.ShowSave
    sFreqFile = CDLookup.FileName
    
    'Reset handler for Cancel button
    On Error GoTo 0
    
    'Save frequency data
    If SaveFreq(sFreqFile, sFreqID) = False Then Exit Sub
    Exit Sub
        
    'Cancel button handler
Cancel:
    Exit Sub
End Sub

Private Sub FGLookup_DblClick()
    Call FlexGridEdit(FGLookup, TxtLookup, 32)
End Sub

Private Sub FGLookup_GotFocus()
    If TxtLookup.Visible = False Then Exit Sub
    FGLookup = TxtLookup
    TxtLookup.Visible = False
End Sub

Private Sub FGLookup_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = 46 Then FGLookup.Text = ""
End Sub

Private Sub FGLookup_KeyPress(KeyAscii As Integer)
    Call FlexGridEdit(FGLookup, TxtLookup, KeyAscii)
End Sub

Private Sub FGLookup_LeaveCell()
    If TxtLookup.Visible = False Then Exit Sub
    FGLookup = TxtLookup
    TxtLookup.Visible = False
End Sub

Private Sub Form_Load()
    'Set form caption
    Caption = "VolTool - " + sHeadFile
       
    'Reset controls
    TxtLookup.Text = ""
    FGLookup.ColWidth(0) = FGLookup.ColWidth(0) * 2
    
    'Show tab
    ShowTab
    
    'Show labels
    ShowLabels
    
    'Open volume lookup table
    Call OpenLookup(sVolFile, 0)
    
    'Open range lookup table
    Call OpenLookup(sRangeFile, 1)
    
    'Open frequency data
    Call OpenFreq(sFreqFile)
End Sub

Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
    'Cancel unload
    Cancel = 1
    
    'End program
    AntiMain
End Sub

Private Sub TSLookup_Click()
    'Show tab
    ShowTab
End Sub

Private Sub TxtLookup_KeyDown(KeyCode As Integer, Shift As Integer)
    Call EditKeyCode(FGLookup, TxtLookup, KeyCode, Shift)
End Sub

Private Sub TxtLookup_KeyPress(KeyAscii As Integer)
    If KeyAscii = 13 Then KeyAscii = 0
End Sub
