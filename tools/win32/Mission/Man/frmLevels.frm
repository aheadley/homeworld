VERSION 5.00
Begin VB.Form frmLevels 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Level"
   ClientHeight    =   1575
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4650
   Icon            =   "frmLevels.frx":0000
   MaxButton       =   0   'False
   MDIChild        =   -1  'True
   MinButton       =   0   'False
   ScaleHeight     =   1575
   ScaleWidth      =   4650
   Begin VB.TextBox txtName 
      Height          =   285
      Left            =   1020
      TabIndex        =   0
      Top             =   120
      Width           =   3495
   End
   Begin VB.TextBox txtInfo 
      Height          =   285
      Left            =   1020
      TabIndex        =   1
      Top             =   600
      Width           =   3495
   End
   Begin VB.CommandButton cmdApply 
      Caption         =   "&Apply"
      Height          =   375
      Left            =   3420
      TabIndex        =   4
      Tag             =   "Apply"
      Top             =   1080
      Width           =   1095
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "&Cancel"
      Height          =   375
      Left            =   2220
      TabIndex        =   3
      Tag             =   "Cancel"
      Top             =   1080
      Width           =   1095
   End
   Begin VB.CommandButton cmdOK 
      Caption         =   "&OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   1020
      TabIndex        =   2
      Tag             =   "OK"
      Top             =   1080
      Width           =   1095
   End
   Begin VB.Label lblInfo 
      Caption         =   "Info:"
      Height          =   195
      Left            =   120
      TabIndex        =   6
      Top             =   660
      Width           =   795
   End
   Begin VB.Label lblName 
      Caption         =   "Name:"
      Height          =   195
      Left            =   120
      TabIndex        =   5
      Top             =   180
      Width           =   795
   End
End
Attribute VB_Name = "frmLevels"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim bSel As Boolean

Dim aPos(4) As Single

Dim sParK As String
Dim sCurK As String
Dim sListK As String

Sub GetLevel(ByVal sPKey As String, ByVal sLKey As String)
    Dim nInd As Integer

    Dim nPos As Long
    
    Dim sCKey As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Reset color
    txtName.BackColor = vbWindowBackground
    txtInfo.BackColor = vbWindowBackground
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Set local keys
    sParK = sPKey
    sListK = sLKey
    sCurK = ""
    
    'Check keys
    If sParK = "" Then
        'Set selection flag
        bSel = True
    Else
        'Clear slection flag
        bSel = False
    End If
    
    'Check key
    If sListK = "" Then
        'Put default data in controls
        txtName.Text = MIS_NAM_LEV
        txtInfo.Text = ""
        Exit Sub
    End If
    
    'Check recordset
    If rsLevels.BOF = True Then Exit Sub
    
    'Reset index
    nInd = 0
    
    'Loop thru types
    Do
        'Get position of space character in string
        nPos = InStr(sLKey, " ")
        
        'If possible, truncate string at space character
        If nPos > 0 Then
            'Set key
            sCKey = Left(sLKey, nPos - 1)
            sLKey = Mid(sLKey, nPos + 1, Len(sLKey))
        Else
            'Set key
            sCKey = sLKey
        End If
        
        'Check key
        If Left(sCKey, 1) = "l" Then
            'Set query
            sQuery = "SELECT * FROM Levels WHERE Key = " + Mid(sCKey, 2)
            
            'Open temporary recordset by query
            If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
                
            'Get data from recordset
            rsTemp.MoveFirst
            
            'Check index
            If nInd = 0 Then
                'Set current key
                sCurK = sCKey
                
                'Put data in controls
                txtName.Text = rsTemp!Name
                txtInfo.Text = rsTemp!Info
            Else
                'Compare data and set colors
                If txtName.Text <> rsTemp!Name Then txtName.BackColor = vbButtonFace
                If txtInfo.Text <> rsTemp!Info Then txtInfo.BackColor = vbButtonFace
            End If
    
            'Close temporary recordset
            rsTemp.Close
            
            'Increment index
            nInd = nInd + 1
        End If
            
        'Check position
        If nPos = 0 Then Exit Do
    Loop
End Sub

Function GetName(ByVal nKey As Long) As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Set default
    GetName = ""
    
    'Check recordset
    If rsLevels.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Levels WHERE Key = " + Trim(Str(nKey))
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    GetName = rsTemp!Name
    
    'Close temporary recordset
    rsTemp.Close
End Function

Function GetFile(ByVal sKey As String, ByVal nInd As Integer) As String
    Dim nType As Integer
    
    Dim nKey As Long
    Dim nPos As Long
    
    Dim sExt As String
    Dim sVal As String
    
    'Set default
    GetFile = ""
    
    'Check DB
    If bDBFlag = False Then Exit Function
    
    'Check key
    If sKey = "" Then Exit Function
    If Left(sKey, 1) <> "l" Then Exit Function
    
    'Get extension
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_LEV, MIS_KEY_EXT, sExt, MIS_MOD_CFG)
    sExt = TruncStr(sExt)
    If sExt = "" Then Exit Function
    
    'Set value
    sVal = GetName(Mid(sKey, 2))
    
    'Check value
    nPos = InStr(sVal, ",")
    If nPos > 0 Then
        'Truncate value at comma
        sVal = Mid(sVal, nPos + 1)
    End If
        
    'Set File
    GetFile = sDataDir + sVal + sExt
End Function

Sub PutLevel()
    Dim nPos As Long

    Dim sCKey As String
    Dim sLKey As String
    Dim sQuery As String
    
    Dim rsTemp As Recordset
            
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check key
    If sListK = "" Then
        'Add level
        AddLevel
        Exit Sub
    End If
    
    'Check recordset
    If rsLevels.BOF = True Then Exit Sub
    
    'Set list
    sLKey = sListK
    
    'Loop thru types
    Do
        'Get position of space character in string
        nPos = InStr(sLKey, " ")
        
        'If possible, truncate string at space character
        If nPos > 0 Then
            'Set key
            sCKey = Left(sLKey, nPos - 1)
            sLKey = Mid(sLKey, nPos + 1, Len(sLKey))
        Else
            'Set key
            sCKey = sLKey
        End If
        
        'Check key
        If Left(sCKey, 1) = "l" Then
            'Set query
            sQuery = "SELECT * FROM Levels WHERE Key = " + Mid(sCKey, 2)
            
            'Open temporary recordset by query
            If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
            
            'Put data in recordset
            rsTemp.MoveFirst
            rsTemp.Edit
            rsTemp!NumObjs = CountObjects(Val(Mid(sCKey, 2)))
            rsTemp!NumAttribs = CountAttribs(Val(Mid(sCKey, 2)))
            If txtName.BackColor = vbWindowBackground Then rsTemp!Name = Trim(txtName.Text)
            If txtInfo.BackColor = vbWindowBackground Then rsTemp!Info = Trim(txtInfo.Text)
            rsTemp.Update
            
            'Close temporary recordset
            rsTemp.Close
        
            'Edit and select in tree
            frmTree.EditTree (sCKey)
        End If
        
        'Check position
        If nPos = 0 Then Exit Do
    Loop
            
    'Select in tree
    frmTree.SelTree ("")
End Sub

Sub AddLevel()
    'Add data to recordset
    rsLevels.AddNew
    sCurK = "l" + Trim(Str(rsLevels!Key))
    sListK = sCurK
    rsLevels!NumObjs = 0
    rsLevels!NumAttribs = 0
    rsLevels!Name = Trim(txtName.Text)
    rsLevels!Info = Trim(txtInfo.Text)
    rsLevels.Update

    'Add to tree
    Call frmTree.AddTree(sParK, sCurK)
    
    'Add attribs
    AddAttribs
    
    'Select in tree
    Call frmTree.SelTree(sCurK)
End Sub

Sub AddAttribs()
    Dim n As Integer

    Dim nCount As Integer
    
    Dim nPos As Long
    
    Dim sList As String
    Dim sVal As String
    
    'Reset count
    nCount = 0
    
    'Get attribs
    Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_LEV, MIS_KEY_ATTRIB, sList, nCount, MIS_MOD_CFG)
    
    'Check count
    If nCount = 0 Then Exit Sub
    
    'Truncate attribs
    sList = TruncStr(sList)

    'Loop thru names
    For n = 0 To nCount - 1
        'Get position of | character in string
        nPos = InStr(sList, "|")
        
        'Get default
        Call misGetVal(frmMission.GetPrefix + MIS_SEC_LEV, MIS_KEY_DEF + Trim(Str(n)), sVal, MIS_MOD_CFG)
        
        'Truncate default
        sVal = TruncStr(sVal)

        'If possible, truncate string at | character
        If nPos > 0 Then
            'Create new attrib
            Call frmAttribs.NewAttrib(sCurK, Left(sList, nPos - 1), sVal, "")
            sList = Mid(sList, nPos + 1, Len(sList))
        Else
            'Create new attrib
            Call frmAttribs.NewAttrib(sCurK, sList, sVal, "")
        End If
    Next n
End Sub

Sub DelLevel(ByVal sPKey As String, ByVal sCKey As String, ByVal sFile As String)
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check file
    If sFile = "" Then
        'Check DB
        If bDBFlag = False Then Exit Sub
    End If
    
    'Check key
    If sCKey = "" Then Exit Sub
    If Left(sCKey, 1) <> "l" Then Exit Sub
    
    'Check parent
    If sPKey <> "" And sFile = "" Then
        'Prompt user
        If MsgBox("Delete level " + GetName(Val(Mid(sCKey, 2))) + "?", vbYesNo Or vbQuestion, "MissionMan") = vbNo Then Exit Sub
    End If
    
    'Del attribs
    Call frmAttribs.DelAttribs(sCKey, sFile)
    
    'Del objects
    Call frmObjects.DelObjects(sCKey, sFile)
    
    'Check file
    If sFile = "" Then
        'Check recordset
        If rsLevels.BOF = True Then Exit Sub
    End If
    
    'Set query
    sQuery = "SELECT * FROM Levels WHERE Key = " + Mid(sCKey, 2)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, sFile) = False Then Exit Sub
        
    'Delete data in recordset
    rsTemp.MoveFirst
            
    'Delete data
    rsTemp.Delete
    
    'Close temporary recordset
    Call CloseRecordSetByQuery(rsTemp, sFile)
    
    'Check file
    If sFile <> "" Then Exit Sub
    
    'Delete from tree
    Call frmTree.DelTree(sCKey)
    
    'Check parent
    If sPKey = "" Then Exit Sub
    
    'Select in tree
    Call frmTree.SelTree(sPKey)
    
    'Refresh graphics
    frmFront.Render
    frmTop.Render
    frmSide.Render
    frmCamera.Render
End Sub

Sub CopyLevel(ByVal sSrcKey As String, ByVal sDstKey As String, ByVal bFlag As Boolean, ByVal sSrcFile As String)
    Dim nKey As Long
    
    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check source
    If sSrcKey = "" Then Exit Sub
    If Left(sSrcKey, 1) <> "l" Then Exit Sub
    
    'Check destination
    If sDstKey = "" Then Exit Sub
    If Left(sDstKey, 1) <> "m" Then Exit Sub
    
    'Check source file
    If sSrcFile = "" Then
        'Check recordset
        If rsLevels.BOF = True Then Exit Sub
    End If
    
    'Set query
    sQuery = "SELECT * FROM Levels WHERE Key = " + Mid(sSrcKey, 2)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, sSrcFile) = False Then Exit Sub
        
    'Get data from recordset
    rsTemp.MoveFirst

    'Add data to recordset
    rsLevels.AddNew
    nKey = rsLevels!Key
    
    'Copy data
    rsLevels!NumObjs = rsTemp!NumObjs
    rsLevels!NumAttribs = rsTemp!NumAttribs
    rsLevels!Name = rsTemp!Name
    rsLevels!Info = rsTemp!Info
    rsLevels.Update
    
    'Close temporary recordset
    Call CloseRecordSetByQuery(rsTemp, sSrcFile)
    
    'Add to tree
    Call frmTree.AddTree(sDstKey, "l" + Trim(Str(nKey)))
    
    'Copy attrib
    Call frmAttribs.CopyAttribs(sSrcKey, "l" + Trim(Str(nKey)), sSrcFile)
   
    'Copy levels
    Call frmObjects.CopyObjects(sSrcKey, "l" + Trim(Str(nKey)), sSrcFile)
    
    'Check flag
    If bFlag = True Then Call DelLevel("", sSrcKey, sSrcFile)
    
    'Select in tree
    Call frmTree.SelTree("l" + Trim(Str(nKey)))
End Sub

Sub GenLevel(ByVal sKey As String, ByVal sFile As String)
    Dim bFlag As Boolean
    
    Dim n As Integer
    Dim nErr As Integer
    Dim nCount As Integer
    Dim nNum As Integer
    Dim nInd As Integer
    
    Dim nPos As Long
    Dim nObj As Long
    Dim nKey As Long
    Dim nStream As Long
    
    Dim fAngX As Single
    Dim fAngY As Single
    Dim fAngZ As Single
    Dim fAngH As Single
    Dim fAngV As Single
    Dim fSizeX As Single
    Dim fSizeY As Single
    Dim fSizeZ As Single
    
    Dim sQuery As String
    Dim sName As String
    Dim sAlias As String
    Dim sInfo As String
    Dim sExt As String
    Dim sForm As String
    Dim sMsg As String
    Dim sList As String
    Dim sVal As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub

    'Check key
    If sKey = "" Then Exit Sub
    If Left(sKey, 1) <> "l" Then Exit Sub
    
    'Check recordset
    If rsLevels.BOF = True Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Levels WHERE Key = " + Mid(sKey, 2)
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Get data from recordset
    rsTemp.MoveFirst
    sName = rsTemp!Name
    sInfo = rsTemp!Info
    
    'Close temporary recordset
    rsTemp.Close
    
    'Get extension
    Call misGetVal(frmMission.GetPrefix + MIS_SEC_LEV, MIS_KEY_EXT, sExt, MIS_MOD_CFG)
    sExt = TruncStr(sExt)
    If sExt = "" Then Exit Sub
    
    'Set progress bar
    Call frmProgress.Init("Generating level " + sName + "...", 100)
    
    'Check file
    If sFile = "" Then sFile = sName + sExt
    
    'Open file
    nErr = misOpen(nStream, sFile)
    If nErr < 0 Then
        'Reset progress bar
        Call frmProgress.Clean
        
        'Inform user
        Call misGetErr(nErr, sMsg)
        Call MsgBox("Error: " + TruncStr(sMsg) + " " + sFile + " (Check attributes and directories)!", vbOKOnly Or vbExclamation, "MissionMan")
        Exit Sub
    End If
    
    'Write data
    Call misWriteInfo(nStream, "MissionMan script, " + Format(Date, "dddd, mmm d yyyy") + ", " + Format(Time, "h:mm:ss AMPM"))
    Call misWriteInfo(nStream, "Copyright (c) 1998-99, Relic Entertainment Inc.")
    Call misWriteInfo(nStream, frmMission.GetType + " Level: " + sName)
    If sInfo <> "" Then Call misWriteInfo(nStream, "Info: " + sInfo)
    misWriteNew (nStream)
    
    'Write data
    Call misWriteVal(nStream, "[%s]", sInfo)
    misWriteNew (nStream)
    misWriteNew (nStream)
    
    'Reset flag
    bFlag = False
    
    'Check recordset
    If rsObjects.BOF = False Then
        'Set query
        sQuery = "SELECT * FROM Objects WHERE Level = " + Mid(sKey, 2) + " AND Object = 0 ORDER BY Key"
    
        'Open temporary recordset by query
        If OpenRecordSetByQuery(sQuery, rsTemp, "") = True Then
            'Get data from recordset
            rsTemp.MoveFirst
            Do Until rsTemp.EOF
                'Update status bar
                Call frmProgress.Update(rsTemp.PercentPosition)
                
                'Get size
                Call rendFindObj(nObj, rsTemp!Key)
                Call rendGetObjSize(nObj, fSizeX, fSizeY, fSizeZ)
    
                'Get angles
                fAngX = rsTemp!RotX
                fAngY = rsTemp!RotY
                fAngZ = rsTemp!RotZ
                
                'Check angles
                If (rendAngCheck(fAngX, fAngY, fAngZ) < 0) Then
                    'Update database
                    rsTemp.Edit
                    rsTemp!RotX = fAngX
                    rsTemp!RotY = fAngY
                    rsTemp!RotZ = fAngZ
                    rsTemp.Update
                End If
                
                'Convert angles
                Call rendAngConv(fAngH, fAngV, rsTemp!RotX, rsTemp!RotY, rsTemp!RotZ)
               
                'Get vars
                Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(rsTemp!Type - 1)), MIS_KEY_VAR, sList, nCount, MIS_MOD_CFG)
    
                'Check count
                If nCount > 0 Then
                    'Set flag
                    bFlag = True
                
                    'Truncate vars
                    sList = TruncStr(sList)

                    'Loop thru vars
                    For n = 0 To nCount - 1
                        'Reset name
                        sName = ""
                
                        'Get position of | character in string
                        nPos = InStr(sList, "|")
                
                        'If possible, truncate string at | character
                        If nPos > 0 Then
                            'Set name
                            sName = Left(sList, nPos - 1)
                            sList = Mid(sList, nPos + 1, Len(sList))
                        Else
                            'Set name
                            sName = sList
                        End If
                
                        'Check var
                        Select Case Trim(sName)
                            Case "!PosX"
                                sVal = Format(rsTemp!PosX, "0.0;-0.0")
                            Case "!PosY"
                                sVal = Format(rsTemp!PosY, "0.0;-0.0")
                            Case "!PosZ"
                                sVal = Format(rsTemp!PosZ, "0.0;-0.0")
                            Case "!ScaleX"
                                sVal = Format(rsTemp!ScaleX, "0.0;-0.0")
                            Case "!ScaleY"
                                sVal = Format(rsTemp!ScaleY, "0.0;-0.0")
                            Case "!ScaleZ"
                                sVal = Format(rsTemp!ScaleY, "0.0;-0.0")
                            Case "!SizeX"
                                sVal = Format(fSizeX * rsTemp!ScaleX, "0.0;-0.0")
                            Case "!SizeY"
                                sVal = Format(fSizeY * rsTemp!ScaleY, "0.0;-0.0")
                            Case "!SizeZ"
                                sVal = Format(fSizeZ * rsTemp!ScaleZ, "0.0;-0.0")
                            Case "!RotX"
                                sVal = Format(rsTemp!RotX, "0.0;-0.0")
                            Case "!RotY"
                                sVal = Format(rsTemp!RotY, "0.0;-0.0")
                            Case "!RotZ"
                                sVal = Format(rsTemp!RotY, "0.0;-0.0")
                            Case "!RotH"
                                sVal = Format(fAngH, "0.0;-0.0")
                            Case "!RotV"
                                sVal = Format(fAngV, "0.0;-0.0")
                            Case "-!PosX"
                                sVal = Format(-rsTemp!PosX, "0.0;-0.0")
                            Case "-!PosY"
                                sVal = Format(-rsTemp!PosY, "0.0;-0.0")
                            Case "-!PosZ"
                                sVal = Format(-rsTemp!PosZ, "0.0;-0.0")
                            Case "!SizeX/2"
                                sVal = Format(fSizeX * rsTemp!ScaleX / 2, "0.0;-0.0")
                            Case "!SizeY/2"
                                sVal = Format(fSizeY * rsTemp!ScaleY / 2, "0.0;-0.0")
                            Case "!SizeZ/2"
                                sVal = Format(fSizeZ * rsTemp!ScaleZ / 2, "0.0;-0.0")
                            Case "-!RotX"
                                sVal = Format(-rsTemp!RotX, "0.0;-0.0")
                            Case "-!RotY"
                                sVal = Format(-rsTemp!RotY, "0.0;-0.0")
                            Case "-!RotZ"
                                sVal = Format(-rsTemp!RotY, "0.0;-0.0")
                            Case "-!RotH"
                                sVal = Format(-fAngH, "0.0;-0.0")
                            Case "-!RotV"
                                sVal = Format(-fAngV, "0.0;-0.0")
                            Case "!Type"
                                Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(rsTemp!Type - 1)), MIS_KEY_TYPE, sVal, MIS_MOD_CFG)
                                sVal = TruncStr(sVal)
                            Case "!Name"
                                sVal = Trim(rsTemp!Name)
                            Case "!File"
                                sVal = Trim(rsTemp!File)
                            Case "!Count"
                                Call frmObjects.CountLinks(rsTemp!Key, rsTemp!Level, rsTemp!Object, rsTemp!Type, nInd, nNum)
                                sVal = Trim(Str(nInd)) + "/" + Trim(Str(nNum))
                            Case Else
                                'Reset key
                                nKey = 0
                                
                                'Check value
                                If Left(sName, 1) = "-" Then
                                    'Get negative value
                                    Call frmAttribs.GetAll("o" + Trim(Str(rsTemp!Key)), Mid(sName, 2), nKey, sVal, sInfo)
                                    If sVal = "" Then sVal = MIS_NAM_NULL
                                    sVal = "-" + sVal
                                Else
                                    'Get positive value
                                    Call frmAttribs.GetAll("o" + Trim(Str(rsTemp!Key)), sName, nKey, sVal, sInfo)
                                    If sVal = "" Then sVal = MIS_NAM_NULL
                                End If
                        End Select
                
                        'Get format
                        Call misGetVal(frmMission.GetPrefix + MIS_SEC_OBJ + Trim(Str(rsTemp!Type - 1)), MIS_KEY_FORM + Trim(Str(n)), sForm, MIS_MOD_CFG)
        
                        'Truncate format
                        sForm = TruncStr(sForm)

                        'Write data
                        If n = 0 And rsTemp!Info <> "" Then Call misWriteInfo(nStream, rsTemp!Info)
                        Call misWriteVal(nStream, sForm, sVal)
                    Next n
            
                    'Write newline
                    misWriteNew (nStream)
                End If
        
                rsTemp.MoveNext
            Loop
            
            'Close temporary recordset
            rsTemp.Close
    
        'Check flag
        If bFlag = True Then misWriteNew (nStream)
        End If
    End If
    
    'Reset progress bar
    Call frmProgress.Clean
    
    'Get attribs
    Call misGetListByKey(frmMission.GetPrefix + MIS_SEC_LEV, MIS_KEY_ATTRIB, sList, nCount, MIS_MOD_CFG)
    
    'Check count
    If nCount > 0 Then
        'Set progress bar
        Call frmProgress.Init("Generating attributes for level " + sName + "...", nCount)
    
        'Truncate attribs
        sList = TruncStr(sList)

        'Loop thru attribs
        For n = 0 To nCount - 1
            'Update status bar
            Call frmProgress.Update(n)
            
            'Reset name
            sName = ""
                
            'Get position of | character in string
            nPos = InStr(sList, "|")
                
            'If possible, truncate string at | character
            If nPos > 0 Then
                'Set name
                sName = Left(sList, nPos - 1)
                sList = Mid(sList, nPos + 1, Len(sList))
            Else
                'Set name
                sName = sList
            End If
            
            'Reset key
            nKey = 0
                           
            'Loop
            Do Until nKey < 0
                'Get values
                Call frmAttribs.GetAll(sKey, sName, nKey, sVal, sInfo)
                If sVal <> "" Then
                    'Get alias
                    Call misGetVal(frmMission.GetPrefix + MIS_SEC_LEV, MIS_KEY_ALIAS + Trim(Str(n)), sAlias, MIS_MOD_CFG)
                    sAlias = TruncStr(sAlias)
                    If sAlias = "" Then sAlias = sName
        
                    'Write data
                    If sInfo <> "" Then Call misWriteInfo(nStream, sInfo)
                    Call misWriteAttrib(nStream, sAlias, sVal)
                End If
            Loop
        Next n
        
        'Reset progress bar
        Call frmProgress.Clean
    End If
        
    'Close event file
    misClose (nStream)
End Sub

Sub GenLevels()
    Dim nInd As Integer
    
    Dim sQuery As String
    Dim sFile As String
    
    Dim rsTemp As Recordset
    
    'Check DB
    If bDBFlag = False Then Exit Sub
    
    'Check recordset
    If rsLevels.BOF = True Then Exit Sub
    
    'Prompt user
    If MsgBox("Generate all level and object files?", vbYesNo Or vbQuestion, "MissionMan") = vbNo Then Exit Sub
    
    'Set query
    sQuery = "SELECT * FROM Levels"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Sub
        
    'Reset index
    nInd = 0
    
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        'Generate objects
        frmObjects.GenObjects ("l" + Trim(Str(rsTemp!Key)))
        
        'Get file
        sFile = GetFile("l" + Trim(Str(rsTemp!Key)), nInd)

        'Generate level
        Call GenLevel("l" + Trim(Str(rsTemp!Key)), sFile)

        'Increment index
        nInd = nInd + 1
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Sub

Function CountObjects(ByVal nKey As Long) As Integer
    Dim nCount As Integer

    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Reset count
    CountObjects = 0
    
    'Check recordset
    If rsObjects.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Objects WHERE Level = " + Str(nKey) + " AND Object = 0 ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        CountObjects = CountObjects + 1
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Function

Function CountAttribs(ByVal nKey As Long) As Integer
    Dim nCount As Integer

    Dim sQuery As String
    
    Dim rsTemp As Recordset
    
    'Reset count
    CountAttribs = 0
    
    'Check recordset
    If rsAttribs.BOF = True Then Exit Function
    
    'Set query
    sQuery = "SELECT * FROM Attrib WHERE Level = " + Str(nKey) + " AND Object = 0 ORDER BY Key"
    
    'Open temporary recordset by query
    If OpenRecordSetByQuery(sQuery, rsTemp, "") = False Then Exit Function
        
    'Get data from recordset
    rsTemp.MoveFirst
    Do Until rsTemp.EOF
        CountAttribs = CountAttribs + 1
        rsTemp.MoveNext
    Loop
    
    'Close temporary recordset
    rsTemp.Close
End Function

Private Sub cmdApply_Click()
    'Commit
    Call CommitDB("Edit Level")
    
    'Put level
    PutLevel
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdOK_Click()
    'Commit
    Call CommitDB("Edit Level")
    
    'Put level
    PutLevel
    
    Unload Me
End Sub

Private Sub Form_Load()
    Dim n As Integer
    Dim nCount As Integer
    
    Dim nPos As Long
        
    Dim sList As String

    'Set tree view position
    aPos(0) = fMainForm.ScaleWidth / 4
    aPos(1) = fMainForm.ScaleHeight / 4
    
    'Reset count
    nCount = 0
    
    'Get window
    Call misGetListByKey(MIS_SEC_COM, MIS_KEY_LEVT, sList, nCount, MIS_MOD_INI)
    
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
    fMainForm.mnuViewTabLevel.Checked = True
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim n As Integer
    
    Dim sList As String
    
    'Cleanup form
    fMainForm.mnuViewTabLevel.Checked = False

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
    Call misPutListByKey(MIS_SEC_COM, MIS_KEY_LEVT, sList, MIS_MOD_INI)
End Sub

Private Sub txtInfo_Change()
    'Reset color
    txtInfo.BackColor = vbWindowBackground
End Sub

Private Sub txtName_Change()
    'Reset color
    txtName.BackColor = vbWindowBackground
End Sub
