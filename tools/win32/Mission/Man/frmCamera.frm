VERSION 5.00
Object = "{6B7E6392-850A-101B-AFC0-4210102A8DA7}#1.3#0"; "Comctl32.ocx"
Begin VB.Form frmCamera 
   BackColor       =   &H80000008&
   Caption         =   "Camera View"
   ClientHeight    =   3204
   ClientLeft      =   60
   ClientTop       =   348
   ClientWidth     =   4716
   FillColor       =   &H80000008&
   FontTransparent =   0   'False
   Icon            =   "frmCamera.frx":0000
   KeyPreview      =   -1  'True
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   ScaleHeight     =   3204
   ScaleWidth      =   4716
   Begin VB.PictureBox pbViewPort 
      BackColor       =   &H80000007&
      BorderStyle     =   0  'None
      ClipControls    =   0   'False
      HasDC           =   0   'False
      Height          =   2775
      Left            =   60
      MousePointer    =   2  'Cross
      ScaleHeight     =   2772
      ScaleWidth      =   4572
      TabIndex        =   1
      Top             =   60
      Width           =   4575
   End
   Begin ComctlLib.StatusBar sbStatusBar 
      Align           =   2  'Align Bottom
      Height          =   276
      Left            =   0
      TabIndex        =   0
      Top             =   2928
      Width           =   4716
      _ExtentX        =   8319
      _ExtentY        =   487
      SimpleText      =   ""
      _Version        =   327682
      BeginProperty Panels {0713E89E-850A-101B-AFC0-4210102A8DA7} 
         NumPanels       =   2
         BeginProperty Panel1 {0713E89F-850A-101B-AFC0-4210102A8DA7} 
            AutoSize        =   1
            Object.Width           =   3937
            Text            =   "Camera Eye: 0 0 0 m"
            TextSave        =   "Camera Eye: 0 0 0 m"
            Object.Tag             =   ""
         EndProperty
         BeginProperty Panel2 {0713E89F-850A-101B-AFC0-4210102A8DA7} 
            AutoSize        =   1
            Object.Width           =   3937
            Text            =   "Camera Focus: 0 0 0 m"
            TextSave        =   "Camera Focus: 0 0 0 m"
            Object.Tag             =   ""
         EndProperty
      EndProperty
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   7.8
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
   End
End
Attribute VB_Name = "frmCamera"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim bClick As Boolean
Dim bPress As Boolean

Dim nButton As Integer
Dim nShift As Integer

Dim fMx As Single
Dim fMy As Single

Dim aPos(4) As Single

Dim nContext As Long

Sub Render()
    'Check flag
    If fMainForm.mnuViewGraphCamera.Checked Then Call rendUpdateCont(nContext)
End Sub

Sub SetDetail(ByVal nDetail As Integer)
    'Set level-of-detail
    Call rendSetContDetail(nContext, nDetail)
End Sub

Sub SetSel(ByVal bFlag As Boolean)
    'Check flag
    If Not fMainForm.mnuViewGraphCamera.Checked Then Exit Sub
    
    'Set selection color
    Call rendSetContSel(nContext, nSelCol)
    
    'Refresh
    If bFlag = True Then Render
End Sub
    
Sub SetCamera(ByVal bFlag As Boolean)
    'Check flag
    If Not fMainForm.mnuViewGraphCamera.Checked Then Exit Sub
    
    'Set camera eye, focus and color
    Call rendSetContCamera(nContext, aEye(0), aFocus(0), nCamCol)
    Call rendGetContCamera(nContext, aEye(0), aFocus(0))
    
    'Refresh
    If bFlag = True Then Render
    
    'Update status bar
    sbStatusBar.Panels.Item(1).Text = "Camera Eye: x" + Format(aEye(0), " 0; -#") + " y" + Format(aEye(1), " 0; -#") + " z" + Format(aEye(2), " 0; -#") + " m"
    sbStatusBar.Panels.Item(2).Text = "Camera Focus: x" + Format(aFocus(0), " 0; -#") + " y" + Format(aFocus(1), " 0; -#") + " z" + Format(aFocus(2), " 0; -#") + " m"
End Sub
    
Sub Reset()
    Dim aP(4) As Single

    'Set camera view position and size
    aP(0) = (fMainForm.ScaleWidth / 4) + (3 * fMainForm.ScaleWidth / 8)
    aP(1) = 0
    aP(2) = (3 * fMainForm.ScaleWidth / 8)
    aP(3) = fMainForm.ScaleHeight / 2
    
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
        
    Dim fSize As Single
    
    Dim sList As String
    
    'Reset mouse coordinates
    fMx = 0
    fMy = 0
    
    'Clear key press flag
    bPress = False

    'Create new context
    If rendNewCont(nContext, pbViewPort.hWnd, 0) < 0 Then Call MsgBox("DLL error: Unable to create context!", vbOKOnly Or vbExclamation, "MissionMan")
    
    'Set selection color
    Call rendSetContSel(nContext, nSelCol)
    
    'Set scale
    Call rendSetContScale(nContext, MIS_CAM_SCALE)
    
    'Set camera eye, focus and color
    Call rendSetContCamera(nContext, aEye(0), aFocus(0), nCamCol)
    Call rendGetContCamera(nContext, aEye(0), aFocus(0))
    
    'Update status bar
    sbStatusBar.Panels.Item(1).Text = "Camera Eye: x" + Format(aEye(0), " 0; -#") + " y" + Format(aEye(1), " 0; -#") + " z" + Format(aEye(2), " 0; -#") + " m"
    sbStatusBar.Panels.Item(2).Text = "Camera Focus: x" + Format(aFocus(0), " 0; -#") + " y" + Format(aFocus(1), " 0; -#") + " z" + Format(aFocus(2), " 0; -#") + " m"
            
    'Set camera view position and size
    aPos(0) = (fMainForm.ScaleWidth / 4) + (3 * fMainForm.ScaleWidth / 8)
    aPos(1) = 0
    aPos(2) = (3 * fMainForm.ScaleWidth / 8)
    aPos(3) = fMainForm.ScaleHeight / 2
    
    'Reset count
    nCount = 0
    
    'Get window
    Call misGetListByKey(MIS_SEC_COM, MIS_KEY_CAMV, sList, nCount, MIS_MOD_INI)
    
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
    fMainForm.mnuViewGraphCamera.Checked = True
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim n As Integer
    
    Dim sList As String
    
    'Delete context
    Call rendDelCont(nContext)
    
    'Cleanup form
    fMainForm.mnuViewGraphCamera.Checked = False
    
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
    Call misPutListByKey(MIS_SEC_COM, MIS_KEY_CAMV, sList, MIS_MOD_INI)
End Sub

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    Dim fDist As Single
              
    If KeyCode = vbKeyUp Or KeyCode = vbKeyDown Or KeyCode = vbKeyLeft Or KeyCode = vbKeyRight Then
        'Calculate distance
        fDist = Sqr((aEye(0) - aFocus(0)) * (aEye(0) - aFocus(0)) + (aEye(1) - aFocus(1)) * (aEye(1) - aFocus(1)) + (aEye(2) - aFocus(2)) * (aEye(2) - aFocus(2)))
    End If
    
    If KeyCode = vbKeyUp And Shift > 0 Then
        'Translate camera
        Call rendTransContCamera(nContext, 0, -fDist / (MIS_CAM_SCALE * 0.1))
    End If
        
    If KeyCode = vbKeyDown And Shift > 0 Then
        'Translate camera
        Call rendTransContCamera(nContext, 0, fDist / (MIS_CAM_SCALE * 0.1))
    End If
        
    If KeyCode = vbKeyLeft And Shift > 0 Then
        'Translate camera
        Call rendTransContCamera(nContext, -fDist / (MIS_CAM_SCALE * 0.1), 0)
    End If
        
    If KeyCode = vbKeyRight And Shift > 0 Then
        'Translate camera
        Call rendTransContCamera(nContext, fDist / (MIS_CAM_SCALE * 0.1), 0)
    End If
                   
    If KeyCode = vbKeyUp And Shift = 0 Then
        'Rotate camera
        Call rendRotContCamera(nContext, 0, 2.5)
    End If
        
    If KeyCode = vbKeyDown And Shift = 0 Then
        'Rotate camera
        Call rendRotContCamera(nContext, 0, -2.5)
    End If
        
    If KeyCode = vbKeyLeft And Shift = 0 Then
        'Rotate camera
        Call rendRotContCamera(nContext, 2.5, 0)
    End If
        
    If KeyCode = vbKeyRight And Shift = 0 Then
        'Rotate camera
        Call rendRotContCamera(nContext, -2.5, 0)
    End If
                       
    If KeyCode = vbKeyUp Or KeyCode = vbKeyDown Or KeyCode = vbKeyLeft Or KeyCode = vbKeyRight Then
        'Set key press flag
        bPress = True
        
        'Set level-of-detail to low
        SetDetail (1)
        
        'Get camera eye and focus
        Call rendGetContCamera(nContext, aEye(0), aFocus(0))

        'Update status bar
        sbStatusBar.Panels.Item(1).Text = "Camera Eye: x" + Format(aEye(0), " 0; -#") + " y" + Format(aEye(1), " 0; -#") + " z" + Format(aEye(2), " 0; -#") + " m"
        sbStatusBar.Panels.Item(2).Text = "Camera Focus: x" + Format(aFocus(0), " 0; -#") + " y" + Format(aFocus(1), " 0; -#") + " z" + Format(aFocus(2), " 0; -#") + " m"
        
        'Refresh
        Render
    End If
End Sub

Private Sub Form_KeyPress(KeyAscii As Integer)
    If KeyAscii = 13 Then KeyAscii = 0
    
    'Check key state
    If KeyAscii = Asc("-") Then
        'Scale camera
        Call rendScaleContCamera(nContext, 1 + 0.05)
    End If
        
    'Check key state
    If KeyAscii = Asc("+") Then
        'Scale camera
        Call rendScaleContCamera(nContext, 1 - 0.05)
    End If
        
    'Check key state
    If (KeyAscii = Asc("-")) Or (KeyAscii = Asc("+")) Then
        'Set key press flag
        bPress = True
    
        'Set level-of-detail to low
        SetDetail (1)
        
        'Get camera eye and focus
        Call rendGetContCamera(nContext, aEye(0), aFocus(0))

        'Update status bar
        sbStatusBar.Panels.Item(1).Text = "Camera Eye: x" + Format(aEye(0), " 0; -#") + " y" + Format(aEye(1), " 0; -#") + " z" + Format(aEye(2), " 0; -#") + " m"
        sbStatusBar.Panels.Item(2).Text = "Camera Focus: x" + Format(aFocus(0), " 0; -#") + " y" + Format(aFocus(1), " 0; -#") + " z" + Format(aFocus(2), " 0; -#") + " m"
        
        'Refresh
        Render
    End If
End Sub

Private Sub Form_KeyUp(KeyCode As Integer, Shift As Integer)
    If bPress = True Then
        'Set level-of-detail to high
        SetDetail (0)
        
        'Refresh
        Render
        
        'Set camera
        frmFront.SetCamera (True)
        frmTop.SetCamera (True)
        frmSide.SetCamera (True)
        
        'Show options
        frmOptions.ShowOptions
        
        'Clear key press flag
        bPress = False
    End If
End Sub

Private Sub pbViewPort_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    'Set mouse down flag
    bClick = True
    
    'Set mouse and key
    nButton = Button
    nShift = Shift
End Sub

Private Sub pbViewPort_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim fDist As Single
    
    'Check mouse down flag
    If bClick = True Then
        If nButton <> Button Or nShift <> Shift Then
            Call pbViewPort_MouseUp(nButton, nShift, X, Y)
            Call pbViewPort_MouseDown(Button, Shift, X, Y)
            Exit Sub
        End If
        
        'Set level-of-detail to low
        SetDetail (1)
        
        'Check mouse and key state
        'If nButton = 1 And nShift = 1 Then
        If nButton = 1 Then
            'Calculate distance
            fDist = Sqr((aEye(0) - aFocus(0)) * (aEye(0) - aFocus(0)) + (aEye(1) - aFocus(1)) * (aEye(1) - aFocus(1)) + (aEye(2) - aFocus(2)) * (aEye(2) - aFocus(2)))
               
            'Translate camera
            Call rendTransContCamera(nContext, (X - fMx) * fDist / (MIS_CAM_SCALE * fConvScale), (Y - fMy) * fDist / (MIS_CAM_SCALE * fConvScale))
            
            'Get camera eye and focus
            Call rendGetContCamera(nContext, aEye(0), aFocus(0))
    
            'Update status bar
            sbStatusBar.Panels.Item(1).Text = "Camera Eye: x" + Format(aEye(0), " 0; -#") + " y" + Format(aEye(1), " 0; -#") + " z" + Format(aEye(2), " 0; -#") + " m"
            sbStatusBar.Panels.Item(2).Text = "Camera Focus: x" + Format(aFocus(0), " 0; -#") + " y" + Format(aFocus(1), " 0; -#") + " z" + Format(aFocus(2), " 0; -#") + " m"
            
            'Refresh
            Render
        End If
        
        'Check mouse and key state
        If nButton = 2 Then
            'Rotate camera
            Call rendRotContCamera(nContext, (X - fMx) / (fConvScale * 2), (fMy - Y) / (fConvScale * 2))
            
            'Get camera eye and focus
            Call rendGetContCamera(nContext, aEye(0), aFocus(0))
    
            'Update status bar
            sbStatusBar.Panels.Item(1).Text = "Camera Eye: x" + Format(aEye(0), " 0; -#") + " y" + Format(aEye(1), " 0; -#") + " z" + Format(aEye(2), " 0; -#") + " m"
            sbStatusBar.Panels.Item(2).Text = "Camera Focus: x" + Format(aFocus(0), " 0; -#") + " y" + Format(aFocus(1), " 0; -#") + " z" + Format(aFocus(2), " 0; -#") + " m"
            
            'Refresh
            Render
        End If
        
        'Check mouse and key state
        If nButton = 3 Then
            'Check mouse coordinates
            If Abs(fMx - X) > Abs(fMy - Y) Then
                'Scale camera
                Call rendScaleContCamera(nContext, 1 + (X - fMx) / (fConvScale * 200))
            Else
                'Scale camera
                Call rendScaleContCamera(nContext, 1 + (Y - fMy) / (fConvScale * 200))
            End If
        
            'Get camera eye and focus
            Call rendGetContCamera(nContext, aEye(0), aFocus(0))
    
            'Update status bar
            sbStatusBar.Panels.Item(1).Text = "Camera Eye: x" + Format(aEye(0), " 0; -#") + " y" + Format(aEye(1), " 0; -#") + " z" + Format(aEye(2), " 0; -#") + " m"
            sbStatusBar.Panels.Item(2).Text = "Camera Focus: x" + Format(aFocus(0), " 0; -#") + " y" + Format(aFocus(1), " 0; -#") + " z" + Format(aFocus(2), " 0; -#") + " m"
            
            'Refresh
            Render
        End If
        
        'Set level-of-detail to high
        SetDetail (0)
    End If
    
    'Set mouse coordinates
    fMx = X
    fMy = Y
End Sub

Private Sub pbViewPort_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim nKey As Long
    Dim nObj As Long
    
    Dim fPosX As Single
    Dim fPosY As Single
    Dim fPosZ As Single
    
    Dim aBand(3) As Single
    
    'Check mouse for special case (both buttons)
    If Button <> 0 And Button <> 3 And nButton = 3 Then Exit Sub
    
    'Reset mouse down flag
    bClick = False
    
    'Check mouse and key state
    'If nButton = 1 And nShift = 0 Then
    If 0 Then
        'Get band box coordinates
        aBand(0) = X / fConvScale
        aBand(1) = Y / fConvScale
        aBand(2) = X / fConvScale
        aBand(3) = Y / fConvScale
            
        'Set band box
        Call rendSetContBand(nContext, aBand(0), nBandCol)
        
        'Set object mode
        Call rendCheckContSel(nContext, 1, nKey)
        
        'Get object position
        Call rendFindObj(nObj, nKey)
        Call rendGetObjTrans(nObj, fPosX, fPosY, fPosZ)

        'Check object
        If nObj = 0 Then Exit Sub
        
        'Set focus
        aFocus(0) = fPosX
        aFocus(1) = fPosY
        aFocus(2) = fPosZ
    
        'Set camera
        SetCamera (True)
        frmFront.SetCamera (True)
        frmTop.SetCamera (True)
        frmSide.SetCamera (True)
        Exit Sub
    End If
    
    'Check mouse and key state
    'If (nButton = 1 And nShift = 1) Or nButton = 2 Or nButton = 3 Then
    If nButton = 1 Or nButton = 2 Or nButton = 3 Then
        'Refresh
        Render
        
        'Set camera
        frmFront.SetCamera (True)
        frmTop.SetCamera (True)
        frmSide.SetCamera (True)
        
        'Show options
        frmOptions.ShowOptions
    End If
End Sub

Private Sub pbViewPort_Paint()
    'Render
    Call rendPaintCont(nContext)
End Sub

Private Sub Form_Resize()
    'Resize form
    On Error Resume Next
    Call pbViewPort.Move(0, 0, Me.ScaleWidth, Me.ScaleHeight - sbStatusBar.Height)
    On Error GoTo 0
    
    'Set origin
    Call rendSetContView(nContext, -pbViewPort.Width / (fConvScale * 2), -pbViewPort.Height / (fConvScale * 2), pbViewPort.Width / fConvScale, pbViewPort.Height / fConvScale)
    
    'Render
    Call rendResizeCont(nContext)
End Sub

