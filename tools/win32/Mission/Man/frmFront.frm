VERSION 5.00
Object = "{6B7E6392-850A-101B-AFC0-4210102A8DA7}#1.3#0"; "Comctl32.ocx"
Begin VB.Form frmFront 
   BackColor       =   &H80000008&
   Caption         =   "Front View"
   ClientHeight    =   3210
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4710
   FillColor       =   &H80000008&
   FontTransparent =   0   'False
   Icon            =   "frmFront.frx":0000
   KeyPreview      =   -1  'True
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   ScaleHeight     =   3210
   ScaleWidth      =   4710
   Begin VB.PictureBox pbViewPort 
      BackColor       =   &H80000007&
      BorderStyle     =   0  'None
      ClipControls    =   0   'False
      HasDC           =   0   'False
      Height          =   2775
      Left            =   60
      MousePointer    =   2  'Cross
      ScaleHeight     =   2775
      ScaleWidth      =   4575
      TabIndex        =   1
      Top             =   60
      Width           =   4575
   End
   Begin ComctlLib.StatusBar sbStatusBar 
      Align           =   2  'Align Bottom
      Height          =   270
      Left            =   0
      TabIndex        =   0
      Top             =   2940
      Width           =   4710
      _ExtentX        =   8308
      _ExtentY        =   476
      SimpleText      =   ""
      _Version        =   327682
      BeginProperty Panels {0713E89E-850A-101B-AFC0-4210102A8DA7} 
         NumPanels       =   1
         BeginProperty Panel1 {0713E89F-850A-101B-AFC0-4210102A8DA7} 
            AutoSize        =   1
            Object.Width           =   7885
            Text            =   "Cursor: 0 0 0 m"
            TextSave        =   "Cursor: 0 0 0 m"
            Key             =   ""
            Object.Tag             =   ""
         EndProperty
      EndProperty
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
   End
End
Attribute VB_Name = "frmFront"
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

Dim fRx As Single
Dim fRy As Single

Dim fAng As Single

Dim aPos(4) As Single

Dim nContext As Long
Dim nSel As Long

Sub Render()
    'Check flag
    If fMainForm.mnuViewGraphFront.Checked Then rendUpdateCont (nContext)
End Sub

Sub SetDetail(ByVal nDetail As Integer)
    'Set level-of-detail
    Call rendSetContDetail(nContext, nDetail)
End Sub

Sub SetSel(ByVal bFlag As Boolean)
    'Check flag
    If Not fMainForm.mnuViewGraphFront.Checked Then Exit Sub
    
    'Set selection color
    Call rendSetContSel(nContext, nSelCol)
    
    'Refresh
    If bFlag = True Then Render
End Sub
    
Sub SetGrid(ByVal bFlag As Boolean)
    'Check flag
    If Not fMainForm.mnuViewGraphFront.Checked Then Exit Sub
    
    'Set grid size and color
    Call rendSetContGrid(nContext, fGridSize, nGridCol)
    
    'Refresh
    If bFlag = True Then Render
End Sub

Sub SetCursor(ByVal bFlag As Boolean)
    'Check flag
    If Not fMainForm.mnuViewGraphFront.Checked Then Exit Sub
    
    'Set cursor and color
    Call rendSetContCursor(nContext, aCursor(0), nCursCol)
    
    'Refresh
    If bFlag = True Then Render
End Sub

Sub SetScale(ByVal bFlag As Boolean)
    'Check flag
    If Not fMainForm.mnuViewGraphFront.Checked Then Exit Sub
    
    'Set scale
    Call rendSetContScale(nContext, fViewScale)
    
    'Refresh
    If bFlag = True Then Render
End Sub
    
Sub SetView(ByVal bFlag As Boolean)
    'Check flag
    If Not fMainForm.mnuViewGraphFront.Checked Then Exit Sub
    
    'Set origin
    Call rendSetContView(nContext, (-pbViewPort.Width / (fConvScale * 2)) + aOffset(2), (-pbViewPort.Height / (fConvScale * 2)) + aOffset(1), pbViewPort.Width / fConvScale, pbViewPort.Height / fConvScale)
    
    'Refresh
    If bFlag = True Then Render
End Sub
    
Sub SetCamera(ByVal bFlag As Boolean)
    'Check flag
    If Not fMainForm.mnuViewGraphFront.Checked Then Exit Sub
    
    'Set camera eye, focus and color
    Call rendSetContCamera(nContext, aEye(0), aFocus(0), nCamCol)
    Call rendGetContCamera(nContext, aEye(0), aFocus(0))
    
    'Refresh
    If bFlag = True Then Render
End Sub
    
Sub Reset()
    Dim aP(4) As Single
    
    'Set front view position and size
    aP(0) = fMainForm.ScaleWidth / 4
    aP(1) = fMainForm.ScaleHeight / 2
    aP(2) = 3 * fMainForm.ScaleWidth / 8
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
    
    'Clear key press flag
    bPress = False

    'Create new context
    If rendNewCont(nContext, pbViewPort.hWnd, 1) < 0 Then Call MsgBox("DLL error: Unable to create context!", vbOKOnly Or vbExclamation, "MissionMan")
    
    'Set selection color
    Call rendSetContSel(nContext, nSelCol)
    
    'Set grid size and color
    Call rendSetContGrid(nContext, fGridSize, nGridCol)
    
    'Set cursor and color
    Call rendSetContCursor(nContext, aCursor(0), nCursCol)
    
    'Set scale
    Call rendSetContScale(nContext, fViewScale)
    
    'Set camera eye, focus and color
    Call rendSetContCamera(nContext, aEye(0), aFocus(0), nCamCol)
    Call rendGetContCamera(nContext, aEye(0), aFocus(0))
    
    'Set front view position and size
    aPos(0) = fMainForm.ScaleWidth / 4
    aPos(1) = fMainForm.ScaleHeight / 2
    aPos(2) = 3 * fMainForm.ScaleWidth / 8
    aPos(3) = fMainForm.ScaleHeight / 2
    
    'Reset count
    nCount = 0
    
    'Get window
    Call misGetListByKey(MIS_SEC_COM, MIS_KEY_FRONTV, sList, nCount, MIS_MOD_INI)
    
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
    fMainForm.mnuViewGraphFront.Checked = True
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim n As Integer
    
    Dim sList As String
    
    'Delete context
    Call rendDelCont(nContext)
    
    'Cleanup form
    fMainForm.mnuViewGraphFront.Checked = False
    
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
    Call misPutListByKey(MIS_SEC_COM, MIS_KEY_FRONTV, sList, MIS_MOD_INI)
End Sub

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    Dim nCount As Integer
    Dim aBand(4) As Single
    Dim nList As String
    
    If KeyCode = vbKeyReturn Then
        'Check copy key
        If InStr(sListKey, " ") > 0 Then
            'Show selection property
            fMainForm.GetListProp
            Exit Sub
        End If
        
        ' Check node key
        If Left(sCurKey, 1) = "o" Then
            'Show form
            frmObjects.Show
            Call frmObjects.GetObject(sParKey, sCurKey, 0, 0, 0)
            frmObjects.SetFocus
            Exit Sub
        End If
    End If
    
    If KeyCode = vbKeyDelete Then
        'Commit
        Call CommitDB("Delete")
            
        'Delete item(s)
        Call fMainForm.DelList(sParKey, sListKey, "")
        Exit Sub
    End If
    
    If KeyCode = vbKeyEscape Then
        If bClick = True And nButton = 1 And (nShift = 0 Or nShift = 2) Then
            'Reset mouse down flag
            bClick = False
        
            'Reset band box coordinates
            aBand(0) = 0
            aBand(1) = 0
            aBand(2) = 0
            aBand(3) = 0
            
            'Set band box
            Call rendSetContBand(nContext, aBand(0), nBandCol)
            
            'Refresh
            Render
        End If
        Exit Sub
    End If

    If KeyCode = vbKeyUp Then
        'Set offset
        aOffset(1) = aOffset(1) + 10
    End If
        
    If KeyCode = vbKeyDown Then
        'Set offset
        aOffset(1) = aOffset(1) - 10
    End If
        
    If KeyCode = vbKeyLeft Then
        'Set offset
        aOffset(2) = aOffset(2) + 10
    End If
        
    If KeyCode = vbKeyRight Then
        'Set offset
        aOffset(2) = aOffset(2) - 10
    End If
        
    If KeyCode = vbKeyUp Or KeyCode = vbKeyDown Or KeyCode = vbKeyLeft Or KeyCode = vbKeyRight Then
        'Set key press flag
        bPress = True
    
        'Set level-of-detail to low
        SetDetail (1)
        
        'Update status bar
        sbStatusBar.Panels.Item(1).Text = "View Origin: x" + Format(-aOffset(0) / fViewScale, " 0; -#") + " y" + Format(-aOffset(1) / fViewScale, " 0; -#") + " z" + Format(-aOffset(2) / fViewScale, " 0; -#") + " m"
        
        'Set view
        SetView (True)
    End If
End Sub

Private Sub Form_KeyPress(KeyAscii As Integer)
    Dim fScale As Single
    
    If KeyAscii = 13 Then KeyAscii = 0
    
    'Set scale
    fScale = fViewScale
    
    'Check key state
    If KeyAscii = Asc("-") Then
        fViewScale = fViewScale * (1 - 0.05)
        If fViewScale < 0.0001 Then fViewScale = 0.0001
    End If
        
    'Check key state
    If KeyAscii = Asc("+") Then
        fViewScale = fViewScale * (1 + 0.05)
    End If
        
    'Check key state
    If (KeyAscii = Asc("-")) Or (KeyAscii = Asc("+")) Then
        'Set key press flag
        bPress = True
    
        'Set level-of-detail to low
        SetDetail (1)
        
        'Remove scale effect from offset
        aOffset(2) = aOffset(2) * fViewScale / fScale
        aOffset(1) = aOffset(1) * fViewScale / fScale
    
        'Update status bar
        sbStatusBar.Panels.Item(1).Text = "View Scale:" + Format(1 / fViewScale, " 0%; -#%")
                
        'Set view and scale
        SetView (False)
        SetScale (True)
    End If
End Sub

Private Sub Form_KeyUp(KeyCode As Integer, Shift As Integer)
    If bPress = True Then
        'Set level-of-detail to high
        SetDetail (0)
        
        'Refresh
        Render
        
        'Set view and scale
        frmTop.SetView (False)
        frmTop.SetScale (True)
        frmSide.SetView (False)
        frmSide.SetScale (True)
        
        'Show options
        frmOptions.ShowOptions
        
        'Clear key press flag
        bPress = False
    End If
End Sub

Private Sub pbViewPort_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim nObj As Long
    
    Dim fX As Single
    Dim fY As Single
    Dim fZ As Single
    Dim aBand(4) As Single
       
    'Set mouse down flag
    bClick = True
    
    'Set mouse and key
    nButton = Button
    nShift = Shift
    
    'Set mouse coordinates
    fMx = X
    fMy = Y
    fRx = X
    fRy = Y
    
    'Reset angle
    fAng = 0
    
    'Check mouse and key state
    If nButton = 1 And (nShift = 0 Or nShift = 2) Then
        'Get band box coordinates
        aBand(0) = X / fConvScale
        aBand(1) = Y / fConvScale
        aBand(2) = X / fConvScale
        aBand(3) = Y / fConvScale
            
        'Set band box
        Call rendSetContBand(nContext, aBand(0), nBandCol)
        Exit Sub
    End If
    
    'Check mouse and key state
    If nButton = 1 And nShift = 1 Then
        'Update status bar
        sbStatusBar.Panels.Item(1).Text = "View Origin: x" + Format(-aOffset(0) / fViewScale, " 0; -#") + " y" + Format(-aOffset(1) / fViewScale, " 0; -#") + " z" + Format(-aOffset(2) / fViewScale, " 0; -#") + " m"
        Exit Sub
    End If
    
    'Check mouse and key state
    If nButton = 2 And nShift = 2 Then
        'Get selection
        Call rendCheckContCamera(nContext, X / fConvScale, Y / fConvScale, nSel)
        Exit Sub
    End If
            
    'Check mouse and key state
    If nButton = 1 And nShift = 4 Then
        'Find object
        Call rendFindObj(nObj, Val(Mid(sCurKey, 2)))
        Call rendGetObjTrans(nObj, fX, fY, fZ)
        
        'Update status bar
        sbStatusBar.Panels.Item(1).Text = "Object Position: x" + Format(fX, " 0; -#") + " y" + Format(fY, " 0; -#") + " z" + Format(fZ, " 0; -#") + " m"
        Exit Sub
    End If
    
    'Check mouse and key state
    If nButton = 2 And nShift = 4 Then
        'Set level-of-detail to low
        SetDetail (1)
        
        'Find object
        Call rendFindObj(nObj, Val(Mid(sCurKey, 2)))
        Call rendGetObjRot(nObj, fX, fY, fZ)
        
        'Check key
        If InStr(sListKey, " ") = 0 Then
            'Set context rotation
            Call rendSetContRot(nContext, nObj, nRotCol)
        End If
           
        'Update status bar
        sbStatusBar.Panels.Item(1).Text = "Object Rotation: x" + Format(fX, " 0; -#") + " y" + Format(fY, " 0; -#") + " z" + Format(fZ, " 0; -#") + " deg"
       
        'Refresh
        Render
        
        'Set context rotation
        Call rendSetContRot(nContext, 0, nRotCol)
        
        'Set level-of-detail to high
        SetDetail (0)
    End If
End Sub

Private Sub pbViewPort_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim nObj As Long
    
    Dim fS As Single
    Dim fX As Single
    Dim fY As Single
    Dim fZ As Single
    Dim fA As Single
    Dim fDist As Single
    Dim aBand(4) As Single
    
    'Check mouse down flag
    If bClick = True Then
        If nButton <> Button Or nShift <> Shift Then
            Call pbViewPort_MouseUp(nButton, nShift, X, Y)
            Call pbViewPort_MouseDown(Button, Shift, X, Y)
            Exit Sub
        End If
    
        'Set level-of-detail to low
        SetDetail (1)
        frmCamera.SetDetail (1)
        frmSide.SetDetail (1)
        frmTop.SetDetail (1)
        
        'Check mouse and key state
        If nButton = 1 And (nShift = 0 Or nShift = 2) Then
            'Get band box coordinates
            aBand(0) = fRx / fConvScale
            aBand(1) = fRy / fConvScale
            aBand(2) = X / fConvScale
            aBand(3) = Y / fConvScale
            
            'Set band box
            Call rendSetContBand(nContext, aBand(0), nBandCol)
            
            'Calculate distance
            fDist = Sqr((aBand(0) - aBand(2)) * (aBand(0) - aBand(2)) + (aBand(1) - aBand(3)) * (aBand(1) - aBand(3)))
            
            'Update status bar
            sbStatusBar.Panels.Item(1).Text = "Distance:" + Format(fDist / fViewScale, " 0; -#") + " m"
           
           'Refresh form
            Render
        End If
        
        'Check mouse and key state
        If nButton = 1 And nShift = 1 Then
            'Set offset
            aOffset(2) = aOffset(2) + (fMx - X) / fConvScale
            aOffset(1) = aOffset(1) + (fMy - Y) / fConvScale
            
            'Update status bar
            sbStatusBar.Panels.Item(1).Text = "View Origin: x" + Format(-aOffset(0) / fViewScale, " 0; -#") + " y" + Format(-aOffset(1) / fViewScale, " 0; -#") + " z" + Format(-aOffset(2) / fViewScale, " 0; -#") + " m"
            
            'Set view
            SetView (True)
        End If
        
        'Check mouse and key state
        If nButton = 3 And nShift = 1 Then
            'Remove scale effect from offset
            aOffset(2) = aOffset(2) / fViewScale
            aOffset(1) = aOffset(1) / fViewScale
        
            'Set scale
            If Abs(fMx - X) > Abs(fMy - Y) Then
                fViewScale = fViewScale * (1 + (fMx - X) / (fConvScale * 200))
            Else
                fViewScale = fViewScale * (1 + (fMy - Y) / (fConvScale * 200))
            End If
            If fViewScale < 0.0001 Then fViewScale = 0.0001
        
            'Add scale effect to offset
            aOffset(2) = aOffset(2) * fViewScale
            aOffset(1) = aOffset(1) * fViewScale
            
            'Update status bar
            sbStatusBar.Panels.Item(1).Text = "View Scale:" + Format(1 / fViewScale, " 0%; -#%")
                    
            'Set view and scale
            SetView (False)
            SetScale (True)
        End If
        
        'Check mouse and key state
        If nButton = 2 And nShift = 2 Then
            'Check camera flag
            If bCamFlag = 1 Then
                'Check selection
                If nSel = 1 Then
                    aEye(1) = aEye(1) + (fMy - Y) / (fConvScale * fViewScale)
                    aEye(2) = aEye(2) + (fMx - X) / (fConvScale * fViewScale)
                End If
                
                'Check selection
                If nSel = 2 Then
                    aFocus(1) = aFocus(1) + (fMy - Y) / (fConvScale * fViewScale)
                    aFocus(2) = aFocus(2) + (fMx - X) / (fConvScale * fViewScale)
                End If
            Else
                'Check selection
                If nSel <> 0 Then
                    aEye(1) = aEye(1) + (fMy - Y) / (fConvScale * fViewScale)
                    aEye(2) = aEye(2) + (fMx - X) / (fConvScale * fViewScale)
                    aFocus(1) = aFocus(1) + (fMy - Y) / (fConvScale * fViewScale)
                    aFocus(2) = aFocus(2) + (fMx - X) / (fConvScale * fViewScale)
                End If
            End If
            
            'Refresh
            SetCamera (True)
            frmCamera.SetCamera (False)
        End If
        
        'Check mouse and key state
        If nButton = 1 And nShift = 4 Then
            'Translate object
            Call rendTransSel(0, (fMy - Y) / (fConvScale * fViewScale), (fMx - X) / (fConvScale * fViewScale))
            
            'Find object
            Call rendFindObj(nObj, Val(Mid(sCurKey, 2)))
            Call rendGetObjTrans(nObj, fX, fY, fZ)
            
            'Update status bar
            sbStatusBar.Panels.Item(1).Text = "Object Position: x" + Format(fX, " 0; -#") + " y" + Format(fY, " 0; -#") + " z" + Format(fZ, " 0; -#") + " m"
           
            'Refresh
            Render
            frmCamera.Render
            frmSide.Render
            frmTop.Render
        End If
        
        'Check mouse and key state
        If nButton = 2 And nShift = 4 Then
            'Rotate object
            If Abs(fMx - X) > Abs(fMy - Y) Then
                'Calc angle
                fA = (X - fMx) / fConvScale
            Else
                'Calc angle
                fA = (fMy - Y) / fConvScale
            End If
                      
            'Get absolute angle
            fAng = fAng + fA
            
            'Check key
            If InStr(sListKey, " ") = 0 Then
                'Rotate selection
                Call rendRotSel(0, fA, 0, 0)
                
                'Find object
                Call rendFindObj(nObj, Val(Mid(sCurKey, 2)))
                
                'Set context rotation
                Call rendSetContRot(nContext, nObj, nRotCol)
            Else
                'Rotate selection
                Call rendRotSel(nContext, fA, 0, 0)
            End If
                
            'Update status bar
            sbStatusBar.Panels.Item(1).Text = "Object Rotation: x" + Format(fAng, " 0; -#") + " y" + " 0" + " z" + " 0" + " deg"
                  
            'Refresh
            Render
            frmCamera.Render
            frmSide.Render
            frmTop.Render
            
            'Set context rotation
            Call rendSetContRot(nContext, 0, nRotCol)
        End If
        
        'Check mouse and key state
        If nButton = 3 And nShift = 4 Then
            'Check mouse coordinates
            If Abs(fMx - X) > Abs(fMy - Y) Then
                'Calc scale
                fS = 1 + (X - fMx) / (fConvScale * 200)
            Else
                'Calc scale
                fS = 1 + (fMy - Y) / (fConvScale * 200)
            End If
           
            'Scale selection
            If bScaleFlag = 1 Then
                Call rendScaleSel(1, fS, 1)
            Else
                Call rendScaleSel(fS, fS, fS)
            End If
            
            'Find object
            Call rendFindObj(nObj, Val(Mid(sCurKey, 2)))
            Call rendGetObjScale(nObj, fX, fY, fZ)
            
            'Update status bar
            sbStatusBar.Panels.Item(1).Text = "Object Scale: x" + Format(fX * 100, " 0; -#") + " y" + Format(fY * 100, " 0; -#") + " z" + Format(fZ * 100, " 0; -#") + " %"
           
            'Refresh
            Render
            frmCamera.Render
            frmSide.Render
            frmTop.Render
        End If
        
        'Set level-of-detail to high
        SetDetail (0)
        frmCamera.SetDetail (0)
        frmSide.SetDetail (0)
        frmTop.SetDetail (0)
    End If
    
    'Get cursor
    aMouse(0) = aCursor(0)
    aMouse(1) = -(((Y - pbViewPort.Height / 2) / fConvScale) + aOffset(1)) / fViewScale
    aMouse(2) = -(((X - pbViewPort.Width / 2) / fConvScale) + aOffset(2)) / fViewScale
        
    'Check mouse down flag
    If bClick = False Then
        'Update status bar
        sbStatusBar.Panels.Item(1).Text = "Cursor: x" + Format(aMouse(0), " 0; -#") + " y" + Format(aMouse(1), " 0; -#") + " z" + Format(aMouse(2), " 0; -#") + " m"
        If fMainForm.mnuViewGraphSide.Checked Then frmSide.sbStatusBar.Panels.Item(1).Text = "Cursor: x" + Format(aMouse(0), " 0; -#") + " y" + Format(aMouse(1), " 0; -#") + " z" + Format(aMouse(2), " 0; -#") + " m"
        If fMainForm.mnuViewGraphTop.Checked Then frmTop.sbStatusBar.Panels.Item(1).Text = "Cursor: x" + Format(aMouse(0), " 0; -#") + " y" + Format(aMouse(1), " 0; -#") + " z" + Format(aMouse(2), " 0; -#") + " m"
    End If
    
    'Set mouse coordinates
    fMx = X
    fMy = Y
End Sub

Private Sub pbViewPort_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim n As Integer
    Dim nCount As Integer
    Dim nPos As Long
    Dim nObj As Long
    Dim nKey As Long
    Dim fX As Single
    Dim fY As Single
    Dim fZ As Single
    Dim fS As Single
    Dim sText As String
    Dim sKey As String
    Dim sList As String
    
    'Check mouse down flag
    If bClick = False Then Exit Sub
    
    'Check mouse for special case (both buttons)
    If Button <> 0 And Button <> 3 And nButton = 3 Then Exit Sub
    
    'Reset mouse down flag
    bClick = False
    
    'Check mouse and key state
    If nButton = 1 And nShift = 1 Then
        'Show options
        frmOptions.ShowOptions
        
        'Set scale
        SetView (True)
        frmTop.SetView (True)
        frmSide.SetView (True)
        Exit Sub
    End If
    
    'Check mouse and key state
    If nButton = 3 And nShift = 1 Then
        'Show options
        frmOptions.ShowOptions
        
        'Set view and scale
        SetView (False)
        SetScale (True)
        frmTop.SetView (False)
        frmTop.SetScale (True)
        frmSide.SetView (False)
        frmSide.SetScale (True)
        Exit Sub
    End If
    
    'Check mouse and key state
    If nButton = 1 And nShift = 4 And bGridFlag = 1 Then
        'Get absolute translation
        fX = (fRx - X) / (fConvScale * fViewScale)
        fY = (fRy - Y) / (fConvScale * fViewScale)
        
        'Snap x translation to grid
        If Abs(fX) Mod fGridSize < fGridSize / 2 Then
            fX = -(fX Mod fGridSize)
        Else
            fX = Sgn(fX) * fGridSize - (fX Mod fGridSize)
        End If

        'Snap y translation to grid
        If Abs(fY) Mod fGridSize < fGridSize / 2 Then
            fY = -(fY Mod fGridSize)
        Else
            fY = Sgn(fY) * fGridSize - (fY Mod fGridSize)
        End If
    
        'Translate object
        Call rendTransSel(0, fY, fX)
           
        'Refresh
        Render
        frmTop.Render
        frmSide.Render
        frmCamera.Render
    End If

    'Check mouse and key state
    If nButton = 2 And nShift = 4 Then
        'Check rotation flag
        If bRotFlag = 1 Then
            'Snap rotation
            If Abs(fAng) Mod fRotAngle < fRotAngle / 2 Then
                fAng = -(fAng Mod fRotAngle)
            Else
                fAng = Sgn(fAng) * fRotAngle - (fAng Mod fRotAngle)
            End If
        
            'Check key
            If InStr(sListKey, " ") = 0 Then
                'Rotate selection
                Call rendRotSel(0, fAng, 0, 0)
            Else
                'Rotate selection
                Call rendRotSel(nContext, fAng, 0, 0)
            End If
        End If
        
        'Set context rotation
        Call rendSetContRot(nContext, 0, nRotCol)
        
        'Refresh
        Render
        frmTop.Render
        frmSide.Render
        frmCamera.Render
    End If
        
    'Check mouse and key state
    If (nButton = 1 Or nButton = 2 Or nButton = 3) And nShift = 4 Then
        'Get selection
        Call rendGetSel("o", nCount, sList)
                    
        'Check count
        If nCount > 0 Then
            'Commit
            Call CommitDB("Edit Object")
            
            'Loop
            For n = 0 To nCount - 1
                'Get position of space character in string
                nPos = InStr(sList, " ")
            
                'If possible, truncate string at space character
                If nPos > 0 Then
                    'Edit object
                    sKey = Left(sList, nPos - 1)
                    frmObjects.EditObject (Val(Mid(sKey, 2)))
                    sList = Mid(sList, nPos + 1, Len(sList))
                Else
                    'Edit object
                    frmObjects.EditObject (Val(Mid(sList, 2)))
                End If
            Next n
        End If
        
        'Refresh
        Render
        frmTop.Render
        frmSide.Render
        frmCamera.Render
        Exit Sub
    End If
    
    'Check mouse and key state
    If nButton = 1 And nShift = 0 Then
        'Set object mode
        Call rendCheckContSel(nContext, 2, nKey)
        
        'Get selection
        Call rendGetSel("o", nCount, sList)
        
        'Check count
        If nCount > 0 Then
            'Truncate list
            sList = TruncStr(sList)
           
            'Check mouse
            If Abs(X - fRx) = 0 And Abs(Y - fRy) = 0 Then
                'Get next item
                sList = NextStr(sList, sCurKey, " ")
                
                'Select objects
                Call rendSetSel("o", sList)
            End If
                   
            'Select in tree
            frmTree.SelTree (sList)
        Else
            'Select in tree
            If Left(sCurKey, 1) = "o" Then
                frmTree.SelTree (sParKey)
            Else
                frmTree.SelTree ("l")
            End If
        End If
        
        'Refresh
        Render
        frmTop.Render
        frmSide.Render
        frmCamera.Render
        Exit Sub
    End If
    
    'Check mouse and key state
    If nButton = 1 And nShift = 2 Then
        'Toggle object mode
        Call rendCheckContSel(nContext, 3, nKey)
        
        'Get selection
        Call rendGetSel("o", nCount, sList)
        
        'Check count
        If nCount > 0 Then
            'Truncate list
            sList = TruncStr(sList)

            'Select in tree
            frmTree.SelTree (sList)
        Else
            'Select in tree
            If Left(sCurKey, 1) = "o" Then
                frmTree.SelTree (sParKey)
            Else
                frmTree.SelTree ("l")
            End If
        End If
               
        'Refresh form
        Render
        frmTop.Render
        frmSide.Render
        frmCamera.Render
        Exit Sub
    End If
    
    'Check mouse and key state
    If nButton = 2 And nShift = 0 Then
        'Get selection
        Call rendGetSel("o", nCount, sList)
        
        'Check count
        If nCount > 1 Then
            'Truncate list
            sList = TruncStr(sList)

            'Select in tree
            frmTree.SelTree (sList)
            
            ' Show popup menu
            Call PopupMenu(fMainForm.mnuPUGraphSel, 2)
            Exit Sub
        End If
        
        'Check count
        If nCount > 0 Then
            'Truncate list
            sList = TruncStr(sList)

            'Select in tree
            frmTree.SelTree (sList)
            
            ' Show popup menu
            Call PopupMenu(fMainForm.mnuPUGraphObj, 2)
            Exit Sub
        End If
        
        ' Show popup menu
        Call PopupMenu(fMainForm.mnuPUGraphDef, 2)
        Exit Sub
    End If
    
    'Check mouse and key state
    If nButton = 1 And nShift = 3 Then
        'Get cursor
        aCursor(1) = -(((Y - pbViewPort.Height / 2) / fConvScale) + aOffset(1)) / fViewScale
        aCursor(2) = -(((X - pbViewPort.Width / 2) / fConvScale) + aOffset(2)) / fViewScale
        
        'Check grid flag
        If bGridFlag = 1 Then
            'Snap cursor y to grid
            If Abs(aCursor(1)) Mod fGridSize < fGridSize / 2 Then
                aCursor(1) = -(aCursor(1) Mod fGridSize) + aCursor(1)
            Else
                aCursor(1) = Sgn(aCursor(1)) * fGridSize - (aCursor(1) Mod fGridSize) + aCursor(1)
            End If
        
            'Snap cursor z to grid
            If Abs(aCursor(2)) Mod fGridSize < fGridSize / 2 Then
                aCursor(2) = -(aCursor(2) Mod fGridSize) + aCursor(2)
            Else
                aCursor(2) = Sgn(aCursor(2)) * fGridSize - (aCursor(2) Mod fGridSize) + aCursor(2)
            End If
        End If
    
        'Set view
        SetCursor (True)
        frmTop.SetCursor (True)
        frmSide.SetCursor (True)
        Exit Sub
    End If
    
    'Check mouse and key state
    If nButton = 2 And nShift = 2 Then
        'Show options
        frmOptions.ShowOptions
        
        'Set camera
        SetCamera (True)
        frmTop.SetCamera (True)
        frmSide.SetCamera (True)
        frmCamera.SetCamera (True)
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
    
    'Set view
    Call rendSetContView(nContext, (-pbViewPort.Width / (fConvScale * 2)) + aOffset(2), (-pbViewPort.Height / (fConvScale * 2)) + aOffset(1), pbViewPort.Width / fConvScale, pbViewPort.Height / fConvScale)
    
    'Render
    Call rendResizeCont(nContext)
End Sub

