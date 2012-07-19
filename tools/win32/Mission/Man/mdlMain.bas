Attribute VB_Name = "mdlMain"
Option Explicit

Public Declare Function misSetFile Lib "mission.dll" (ByVal szFile As String, ByVal nMode As Long) As Long

Public Declare Function misGetVal Lib "mission.dll" (ByVal szSection As String, ByVal szKey As String, szVal As String, ByVal nMode As Long) As Long
Public Declare Function misGetListByKey Lib "mission.dll" (ByVal szSection As String, ByVal szPrefix As String, szList As String, nCount As Integer, ByVal nMode As Long) As Long
Public Declare Function misGetListBySection Lib "mission.dll" (ByVal szPrefix As String, ByVal szKey As String, szList As String, nCount As Integer, ByVal nMode As Long) As Long

Public Declare Function misPutVal Lib "mission.dll" (ByVal szSection As String, ByVal szKey As String, ByVal szVal As String, ByVal nMode As Long) As Long
Public Declare Function misPutListByKey Lib "mission.dll" (ByVal szSection As String, ByVal szPrefix As String, ByVal szList As String, ByVal nMode As Long) As Long

Public Declare Function misOpen Lib "mission.dll" (pStream As Long, ByVal szFile As String) As Long
Public Declare Function misWriteAttrib Lib "mission.dll" (ByVal pStream As Long, ByVal szName As String, ByVal szVal As String) As Long
Public Declare Function misWriteInfo Lib "mission.dll" (ByVal pStream As Long, ByVal szInfo As String) As Long
Public Declare Function misWriteNew Lib "mission.dll" (ByVal pStream As Long) As Long
Public Declare Function misWriteVal Lib "mission.dll" (ByVal pStream As Long, ByVal szFormat As String, ByVal szVal As String) As Long
Public Declare Function misClose Lib "mission.dll" (ByVal pStream As Long) As Long

Public Declare Function misShellExec Lib "mission.dll" (ByVal szFile As String) As Long

Public Declare Function misGetHash Lib "mission.dll" (ByVal szText As String) As Long
Public Declare Function misGetVer Lib "mission.dll" (szVer As String) As Long
Public Declare Function misGetErr Lib "mission.dll" (ByVal nErr As Long, szMsg As String) As Long

Public Declare Function rendInit Lib "mission.dll" () As Long
Public Declare Function rendClean Lib "mission.dll" () As Long

Public Declare Function rendAngCheck Lib "mission.dll" (X As Single, Y As Single, Z As Single) As Long
Public Declare Function rendAngConv Lib "mission.dll" (fAngH As Single, fAngV As Single, ByVal X As Single, ByVal Y As Single, ByVal Z As Single) As Long

Public Declare Function rendNewObj Lib "mission.dll" (pObj As Long, ByVal nKey As Long, ByVal szUID As String) As Long
Public Declare Function rendFindObj Lib "mission.dll" (pObj As Long, ByVal nKey As Long) As Long
Public Declare Function rendDelObj Lib "mission.dll" (ByVal pObj As Long) As Long
Public Declare Function rendResetObj Lib "mission.dll" (ByVal pObj As Long) As Long
Public Declare Function rendSetObjLink Lib "mission.dll" (ByVal pObj As Long, ByVal pLink As Long) As Long
Public Declare Function rendGetObjTrans Lib "mission.dll" (ByVal pObj As Long, X As Single, Y As Single, Z As Single) As Long
Public Declare Function rendGetObjSize Lib "mission.dll" (ByVal pObj As Long, X As Single, Y As Single, Z As Single) As Long
Public Declare Function rendGetObjScale Lib "mission.dll" (ByVal pObj As Long, X As Single, Y As Single, Z As Single) As Long
Public Declare Function rendGetObjRot Lib "mission.dll" (ByVal pObj As Long, X As Single, Y As Single, Z As Single) As Long
Public Declare Function rendTransObj Lib "mission.dll" (ByVal pObj As Long, ByVal X As Single, ByVal Y As Single, ByVal Z As Single) As Long
Public Declare Function rendScaleObj Lib "mission.dll" (ByVal pObj As Long, ByVal X As Single, ByVal Y As Single, ByVal Z As Single) As Long
Public Declare Function rendRotObj Lib "mission.dll" (ByVal pObj As Long, ByVal X As Single, ByVal Y As Single, ByVal Z As Single) As Long
Public Declare Function rendGetObjMode Lib "mission.dll" (ByVal pObj As Long, nMode As Long) As Long
Public Declare Function rendSetObjMode Lib "mission.dll" (ByVal pObj As Long, ByVal nMode As Long) As Long
Public Declare Function rendSetObjCol Lib "mission.dll" (ByVal pObj As Long, ByVal nCol As Long) As Long

Public Declare Function rendNewCont Lib "mission.dll" (pContext As Long, ByVal hWnd As Long, ByVal nMode As Long) As Long
Public Declare Function rendDelCont Lib "mission.dll" (ByVal pContext As Long) As Long
Public Declare Function rendSetContDetail Lib "mission.dll" (ByVal pContext As Long, ByVal nDetail As Long) As Long
Public Declare Function rendSetContView Lib "mission.dll" (ByVal pContext As Long, ByVal X As Single, ByVal Y As Single, ByVal w As Single, ByVal h As Single) As Long
Public Declare Function rendSetContScale Lib "mission.dll" (ByVal pContext As Long, ByVal fViewScale As Single) As Long
Public Declare Function rendSetContBand Lib "mission.dll" (ByVal pContext As Long, aPos As Single, ByVal nCol As Long) As Long
Public Declare Function rendSetContGrid Lib "mission.dll" (ByVal pContext As Long, ByVal fSize As Single, ByVal nCol As Long) As Long
Public Declare Function rendSetContRot Lib "mission.dll" (ByVal pContext As Long, ByVal pObj As Long, ByVal nCol As Long) As Long
Public Declare Function rendSetContCursor Lib "mission.dll" (ByVal pContext As Long, aCursor As Single, ByVal nCol As Long) As Long
Public Declare Function rendSetContSel Lib "mission.dll" (ByVal pContext As Long, ByVal nCol As Long) As Long
Public Declare Function rendCheckContSel Lib "mission.dll" (ByVal pContext As Long, ByVal nMode As Long, nKey As Long) As Long
Public Declare Function rendCheckContCamera Lib "mission.dll" (ByVal pContext As Long, ByVal X As Single, ByVal Y As Single, nSel As Long) As Long
Public Declare Function rendGetContCamera Lib "mission.dll" (ByVal pContext As Long, aEye As Single, aFocus As Single) As Long
Public Declare Function rendSetContCamera Lib "mission.dll" (ByVal pContext As Long, aEye As Single, aFocus As Single, ByVal nCol As Long) As Long
Public Declare Function rendTransContCamera Lib "mission.dll" (ByVal pContext As Long, ByVal X As Single, ByVal Y As Single) As Long
Public Declare Function rendScaleContCamera Lib "mission.dll" (ByVal pContext As Long, ByVal fViewScale As Single) As Long
Public Declare Function rendRotContCamera Lib "mission.dll" (ByVal pContext As Long, ByVal fAngH As Single, ByVal fAngV As Single) As Long

Public Declare Function rendUpdateCont Lib "mission.dll" (ByVal pContext As Long) As Long
Public Declare Function rendResizeCont Lib "mission.dll" (ByVal pContext As Long) As Long
Public Declare Function rendPaintCont Lib "mission.dll" (ByVal pContext As Long) As Long

Public Declare Function rendGetSel Lib "mission.dll" (ByVal szPrefix As String, nCount As Integer, szKeys As String) As Long
Public Declare Function rendSetSel Lib "mission.dll" (ByVal szPrefix As String, ByVal szKeys As String) As Long
Public Declare Function rendTransSel Lib "mission.dll" (ByVal X As Single, ByVal Y As Single, ByVal Z As Single) As Long
Public Declare Function rendScaleSel Lib "mission.dll" (ByVal X As Single, ByVal Y As Single, ByVal Z As Single) As Long
Public Declare Function rendRotSel Lib "mission.dll" (ByVal pContext As Long, ByVal X As Single, ByVal Y As Single, ByVal Z As Single) As Long

'General constants
Public Const MIS_VER_NUM = "1.13B"
Public Const MIS_ENV_VAR = "HW_"
Public Const MIS_REF_ID = "MMAN"
Public Const MIS_MRU_COUNT = 5

'Directory constants
Public Const MIS_DIR_WORK = "$HW_Data\Multiplayer"
Public Const MIS_DIR_DATA = ""
Public Const MIS_DIR_OBJ = "~\Objects"

'File constants
Public Const MIS_FILE_INI = "mission.ini"
Public Const MIS_FILE_CFG = "mission.cfg"
Public Const MIS_FILE_HLP = "mission.doc"
Public Const MIS_FILE_DEF = "mission.mdr"

'Extension constants
Public Const MIS_EXT_DB = ".md?"
Public Const MIS_EXT_OBJ = ".mor"

'Mode constants
Public Const MIS_MOD_INI = 0
Public Const MIS_MOD_CFG = 1
Public Const MIS_MOD_REG = 2

'Bit mask
Public Const MIS_BIT_DEV = 1

'Section constants
Public Const MIS_SEC_SING = "S_"
Public Const MIS_SEC_MULTI = "M_"
Public Const MIS_SEC_LEV = "Level"
Public Const MIS_SEC_OBJ = "Object_"

Public Const MIS_SEC_COM = "Common"
Public Const MIS_SEC_GRAPH = "Graphics"

'Key constants
Public Const MIS_KEY_TYPE = "Type"
Public Const MIS_KEY_EXT = "Ext"
Public Const MIS_KEY_NAME = "Name_"
Public Const MIS_KEY_FILE = "File_"
Public Const MIS_KEY_VAR = "Var_"
Public Const MIS_KEY_FORM = "Format_"
Public Const MIS_KEY_REF = "Reference"
Public Const MIS_KEY_LINK = "Link"
Public Const MIS_KEY_ATTRIB = "Attrib_"
Public Const MIS_KEY_DEF = "Def_"
Public Const MIS_KEY_CONST = "Const_"
Public Const MIS_KEY_ALIAS = "Alias_"

Public Const MIS_KEY_VER = "Version"
Public Const MIS_KEY_REGK = "Reg_Key"
Public Const MIS_KEY_BITM = "Bit_Mask"
Public Const MIS_KEY_MRU = "Recent_File_"

Public Const MIS_KEY_TOOLB = "Tool_Bar"
Public Const MIS_KEY_STATB = "Status_Bar"

Public Const MIS_KEY_WORKD = "Work_Dir"
Public Const MIS_KEY_DATAD = "Data_Dir"
Public Const MIS_KEY_OBJD = "Object_Dir"

Public Const MIS_KEY_TREEV = "Tree_View_"
Public Const MIS_KEY_CAMV = "Cam_View_"
Public Const MIS_KEY_FRONTV = "Front_View_"
Public Const MIS_KEY_TOPV = "Top_View_"
Public Const MIS_KEY_SIDEV = "Side_View_"
Public Const MIS_KEY_LISTV = "List_View_"

Public Const MIS_KEY_MIST = "Mis_Table_"
Public Const MIS_KEY_LEVT = "Lev_Table_"
Public Const MIS_KEY_OBJT = "Obj_Table_"
Public Const MIS_KEY_ATTRIBT = "Attrib_Table_"
Public Const MIS_KEY_LAYERT = "Layer_Table_"

Public Const MIS_KEY_VOFFSET = "View_Offset_"
Public Const MIS_KEY_VSCALE = "View_Scale"
Public Const MIS_KEY_VCOL = "View_Color"

Public Const MIS_KEY_CINDEP = "Cam_Indep"
Public Const MIS_KEY_CEYE = "Cam_Eye_"
Public Const MIS_KEY_CFOCUS = "Cam_Focus_"
Public Const MIS_KEY_CSCALE = "Cam_Scale"
Public Const MIS_KEY_CMCOL = "Cam_Color"

Public Const MIS_KEY_GSNAP = "Grid_Snap"
Public Const MIS_KEY_GSIZE = "Grid_Size"
Public Const MIS_KEY_GCOL = "Grid_Color"

Public Const MIS_KEY_RSNAP = "Rot_Snap"
Public Const MIS_KEY_RANG = "Rot_Angle"
Public Const MIS_KEY_RCOL = "Rot_Color"

Public Const MIS_KEY_SINDEP = "Scale_Indep"

Public Const MIS_KEY_BCOL = "Band_Color"
Public Const MIS_KEY_SCOL = "Sel_Color"
Public Const MIS_KEY_CSCOL = "Curs_Color"

'Name constants
Public Const MIS_NAM_MIS = "New Mission"
Public Const MIS_NAM_LEV = "New Level"
Public Const MIS_NAM_OBJ = "New Object"
Public Const MIS_NAM_ATTRIB = "New Attribute"
Public Const MIS_NAM_NULL = "?"

'Flag constants
Public Const MIS_FLAG_TOOL = 1
Public Const MIS_FLAG_STAT = 1

'Camera constants
Public Const MIS_CAM_INDEP = 0
Public Const MIS_CAM_EX = -100000
Public Const MIS_CAM_EY = 0
Public Const MIS_CAM_EZ = 0
Public Const MIS_CAM_FX = 0
Public Const MIS_CAM_FY = 0
Public Const MIS_CAM_FZ = 0
Public Const MIS_CAM_SCALE = 1000
Public Const MIS_CAM_COL = vbWhite

'Renderer constants
Public Const MIS_REND_VOX = 0
Public Const MIS_REND_VOY = 0
Public Const MIS_REND_VOZ = 0
Public Const MIS_REND_VSCALE = 1000
Public Const MIS_REND_VCOL = vbWhite

Public Const MIS_REND_GSNAP = 1
Public Const MIS_REND_GSIZE = 1000
Public Const MIS_REND_GCOL = vbWhite

Public Const MIS_REND_RSNAP = 1
Public Const MIS_REND_RANG = 10
Public Const MIS_REND_RCOL = vbWhite

Public Const MIS_REND_SINDEP = 0

Public Const MIS_REND_BCOL = vbWhite
Public Const MIS_REND_SCOL = vbRed
Public Const MIS_REND_CCOL = vbGreen

'Attributes
Dim nDBAttribs As Integer

'Workspaces
Dim wspCurrent As Workspace

'Databases
Dim dbMission As Database
Dim dbClipboard As Database

'Query definitions
Dim qdfMission As QueryDef
Dim qdfClipboard As QueryDef

'Keys
Public sParKey As String
Public sCurKey As String
Public sCopyKey As String
Public sListKey As String

'Directories
Public sWorkDir As String
Public sDataDir As String
Public sObjDir As String
Public sWDir As String
Public sODir As String
Public sDDir As String

'Files
Public sDBFile As String
Public sCopyFile As String

'Info
Public sUndoInfo As String

'Flags
Public bFocusFlag As Boolean
Public bUndoFlag As Boolean
Public bDBFlag As Boolean
Public bDelFlag As Boolean
Public bOptFlag As Boolean
Public bToolFlag As Integer
Public bStatFlag As Integer
Public bCamFlag As Integer
Public bGridFlag As Integer
Public bRotFlag As Integer
Public bScaleFlag As Integer

'Colors
Public nViewCol As Long
Public nCamCol As Long
Public nGridCol As Long
Public nRotCol As Long
Public nBandCol As Long
Public nSelCol As Long
Public nCursCol As Long

'MRU list
Public nMRUCount As Integer
Public nMRUIndex As Integer

'Scales
Public fConvScale As Single
Public fViewScale As Single

'Sizes
Public fGridSize As Single
Public fRotAngle As Single

'Camera
Public aEye(3) As Single
Public aFocus(3) As Single

'Offset
Public aOffset(3) As Single
Public aCursor(3) As Single
Public aMouse(3) As Single

'Forms
Public fMainForm As frmMain

'Database recordsets
Public rsAttribs As Recordset
Public rsLayers As Recordset
Public rsLevels As Recordset
Public rsMission As Recordset
Public rsObjects As Recordset

Sub Main()
    Dim nAttribs As Integer

    Dim sPath As String
    Dim sFile As String

    'Get Path
    sPath = App.Path
    If Mid(sPath, Len(sPath)) <> "\" Then sPath = sPath + "\"
    
    'Check and change attribs
    sFile = sPath + MIS_FILE_INI
    nAttribs = GetAttr(sFile)
    If (nAttribs And vbReadOnly) <> 0 Then Call SetAttr(sFile, nAttribs And (Not vbReadOnly))

    'Set INI file
    Call misSetFile(sFile, MIS_MOD_INI)
    
    'Set config file
    sFile = sPath + MIS_FILE_CFG
    Call misSetFile(sFile, MIS_MOD_CFG)
    
    'Create MDI form
    Set fMainForm = New frmMain
    
    'Reset files
    sDBFile = ""
    sCopyFile = ""
    
    'Reset dirs
    sWorkDir = ""
    sObjDir = ""
    sDataDir = ""
    
    'Reset keys
    sParKey = ""
    sCurKey = ""
    sCopyKey = ""
    sListKey = ""
    
    'Reset flags
    bFocusFlag = True
    bUndoFlag = False
    bDBFlag = False
    bDelFlag = False
    bOptFlag = False
    
    'Reset cursor
    aCursor(0) = 0
    aCursor(1) = 0
    aCursor(2) = 0
    
    'Get conversion scale
    fConvScale = (Screen.TwipsPerPixelX + Screen.TwipsPerPixelY) / 2
    
    'Initialize renderer
    rendInit
        
    'Get options
    frmOptions.GetOptions
        
    'Show MDI form
    fMainForm.Show
End Sub

Sub AntiMain()
    'Put MRU list
    fMainForm.PutMRUList
        
    'Check options flag
    If bOptFlag = True Then
        'Prompt user
        If MsgBox("Save options?", vbYesNo Or vbQuestion, "MissionMan") = vbYes Then frmOptions.PutOptions
    End If
    
    'Prompt user
    If MsgBox("Exit MissionMan?", vbYesNo Or vbQuestion, "MissionMan") = vbNo Then Exit Sub
    
    'Hide views
    Unload frmCamera
    Unload frmSide
    Unload frmTop
    Unload frmFront
    Unload frmTree
    
    'Hide other views
    Unload frmList
    Unload frmLayers
    Unload frmMission
    Unload frmLevels
    Unload frmObjects
    Unload frmAttribs
    
    'Cleanup renderer
    rendClean
    
    'Close databases
    CloseDB
            
    'End program
    End
End Sub

Sub OpenDB()
    'Close database
    CloseDB
    
    'Get attributes
    nDBAttribs = GetAttr(sDBFile)
    
    'Check attrib
    If (nDBAttribs And vbReadOnly) <> 0 Then
        'Inform user
        If MsgBox("File " + sDBFile + " is read-only. Open mission database anyway?", vbYesNo Or vbQuestion, "MissionMan") = vbNo Then Exit Sub
        
        'Change attributes
        Call SetAttr(sDBFile, nDBAttribs And (Not vbReadOnly))
    End If

    'Set error handler for database
    On Error GoTo DBErr
      
    'Clear flags
    bUndoFlag = False
    
    'Open mission database
    Set wspCurrent = DBEngine.Workspaces(0)
    Set dbMission = wspCurrent.OpenDatabase(sDBFile)
    
    'Set error handler for recordsets
    On Error GoTo RSErr
    
    'Open record sets
    Set rsAttribs = dbMission.OpenRecordset("Attrib")
    Set rsLayers = dbMission.OpenRecordset("Layers")
    Set rsLevels = dbMission.OpenRecordset("Levels")
    Set rsMission = dbMission.OpenRecordset("Mission")
    Set rsObjects = dbMission.OpenRecordset("Objects")
    
    'Create temporary query definition
    Set qdfMission = dbMission.CreateQueryDef("")
    
    'Memorize changes
    wspCurrent.BeginTrans
    
    'Set database flag
    bDBFlag = True
    Exit Sub
    
    'Error handler for database
DBErr:
    'Inform user
    Call MsgBox("Error: Unable to open mission database (Check file " + sDBFile + ")!", vbOKOnly Or vbExclamation, "MissionMan")
    Exit Sub
    
    'Error handler for recordsets
RSErr:
    'Inform user
    Call MsgBox("Error: Invalid mission database (Check tables)!", vbOKOnly Or vbExclamation, "MissionMan")
    Exit Sub
End Sub

Sub CloseDB()
    'Check database flag
    If bDBFlag = True Then
        'Commit change
        wspCurrent.CommitTrans
    
        'Close recordsets
        rsAttribs.Close
        rsLayers.Close
        rsLevels.Close
        rsMission.Close
        rsObjects.Close
        
        'Close query definitions
        qdfMission.Close
        
        'Close database
        dbMission.Close
        
        'Check and set lear read-only attribute
        If (nDBAttribs And vbReadOnly) <> 0 Then Call SetAttr(sDBFile, nDBAttribs And vbReadOnly)
    End If
    
    'Clear database flag
    bDBFlag = False
End Sub

Sub RollbackDB()
    'Check database flag
    If bDBFlag = False Then Exit Sub
    
    'Rollback changes
    wspCurrent.Rollback
    wspCurrent.BeginTrans
    
    'Clear flags
    bUndoFlag = False
End Sub

Sub CommitDB(ByVal sInfo As String)
    'Check database flag
    If bDBFlag = False Then Exit Sub
    
    'Set undo info
    sUndoInfo = sInfo

    'Commit changes
    wspCurrent.CommitTrans
    wspCurrent.BeginTrans

    'Set flags
    bUndoFlag = True
End Sub

Function OpenRecordSetByQuery(ByVal sQStr As String, rsQRecSet As Recordset, ByVal sDBStr As String)
    Dim sDBFileName As String
    
    'Default
    OpenRecordSetByQuery = False

    'Check database
    If sDBStr = "" Then
        'Set error handler for database
        On Error GoTo QDFErr
        
        'Set query string
        qdfMission.SQL = sQStr
    
        ' Create recordset
        Set rsQRecSet = qdfMission.OpenRecordset()
    Else
        'Set error handler for database
        On Error GoTo DBErr
          
        'Open database
        Set dbClipboard = wspCurrent.OpenDatabase(sDBStr)
        
        'Create query definition
        Set qdfClipboard = dbClipboard.CreateQueryDef("")
        
        'Set error handler for database
        On Error GoTo QDFErr
        
        'Set query string
        qdfClipboard.SQL = sQStr
    
        ' Create recordset
        Set rsQRecSet = qdfClipboard.OpenRecordset()
    End If
        
    'Check recordset
    If rsQRecSet.BOF = True Then
        'Close temporary recordset
        Call CloseRecordSetByQuery(rsQRecSet, sDBFileName)
        Exit Function
    End If
    
    'Success
    OpenRecordSetByQuery = True
    Exit Function
  
DBErr:
    'Inform user
    Call MsgBox("Error: Unable to open database " + sDBStr + "!", vbOKOnly Or vbExclamation, "MissionMan")
    Exit Function

QDFErr:
    'Inform user
    Call MsgBox("Error: Unable to perform SQL query (" + sQStr + ")!", vbOKOnly Or vbExclamation, "MissionMan")
    Exit Function
End Function

Sub CloseRecordSetByQuery(rsQRecSet As Recordset, ByVal sDBStr As String)
    'Close recordset
    rsQRecSet.Close
    
    'Check database
    If sDBStr = "" Then Exit Sub
    
    'Close query definitions
    qdfClipboard.Close
    
    'Close database
    dbClipboard.Close
End Sub

Function FileStr(ByVal sData As String) As String
    Dim nPos As Long
    
    'Set default
    FileStr = ""
    
    'Get filename
    sData = Dir(sData)
    
    'Get position of period character in string
    nPos = InStr(sData, ".")
    
    'If possible, truncate string at period character
    If nPos > 0 Then FileStr = Left(sData, nPos - 1)
End Function

Function TruncStr(ByVal sData As String) As String
    Dim nPos As Long
    
    'Set default
    TruncStr = ""
    
    'Get position of null character in C string
    nPos = InStr(sData, Chr(0))
    
    'If possible, truncate C string at null character
    If nPos > 0 Then TruncStr = Left(sData, nPos - 1)
End Function

Function FirstStr(ByVal sData As String, ByVal sSep As String) As String
    Dim nPos As Long
    
    'Set default
    FirstStr = ""
    
    'Get position of separator character in string
    nPos = InStr(sData, sSep)
        
    'If possible, truncate string at separator character
    If nPos > 0 Then
        'Get string
        FirstStr = Left(sData, nPos - 1)
        sData = Mid(sData, nPos + 1, Len(sData))
    Else
        'Get string
        FirstStr = sData
    End If
End Function

Function NextStr(ByVal sData As String, ByVal sSub As String, ByVal sSep As String) As String
    Dim nPos As Long
    
    'Set default
    NextStr = ""
    
    'Get position of sub-string in string
    nPos = InStr(sData + " ", sSub + " ")
        
    'If possible, truncate string at sub-string
    If nPos > 0 Then
        'Get string
        NextStr = FirstStr(Mid(sData, nPos + Len(sSub) + 1), sSep)
    End If
        
    'Check string
    If NextStr = "" Then NextStr = FirstStr(sData, sSep)
End Function

Sub FromClipboard()
    Dim nPos As Long
    
    Dim sClipboard As String
    Dim sRef As String
    Dim sFlag As String

    'Set defaults
    sCopyKey = ""
    bDelFlag = False
    sCopyFile = ""
    
    'Get data from clipboard
    sClipboard = Clipboard.GetText
    
    'Check data
    If sClipboard = "" Then Exit Sub
    
    'Get position of reference in data
    nPos = InStr(sClipboard, "|")
    If nPos > 0 Then
        'Get reference
        sRef = Left(sClipboard, nPos - 1)
        sClipboard = Mid(sClipboard, nPos + 1, Len(sClipboard))
    Else
        'Get reference
        sRef = sClipboard
        Exit Sub
    End If
    
    'Check reference
    If sRef <> MIS_REF_ID Then Exit Sub
    
    'Get position of list in data
    nPos = InStr(sClipboard, "|")
    If nPos > 0 Then
        'Get list
        sCopyKey = Left(sClipboard, nPos - 1)
        sClipboard = Mid(sClipboard, nPos + 1, Len(sClipboard))
    Else
        'Get list
        sCopyKey = sClipboard
        Exit Sub
    End If
    
    'Get position of flag in data
    nPos = InStr(sClipboard, "|")
    If nPos > 0 Then
        'Get flag
        sFlag = Left(sClipboard, nPos - 1)
        If Val(sFlag) > 0 Then bDelFlag = True
        If Val(sFlag) <= 0 Then bDelFlag = False
        sClipboard = Mid(sClipboard, nPos + 1, Len(sClipboard))
    Else
        'Get flag
        sFlag = sClipboard
        If Val(sFlag) > 0 Then bDelFlag = True
        If Val(sFlag) <= 0 Then bDelFlag = False
        Exit Sub
    End If
    
    'Get position of file in data
    nPos = InStr(sClipboard, "|")
    If nPos > 0 Then
        'Get file
        sCopyFile = Left(sClipboard, nPos - 1)
        If sCopyFile = sDBFile Then
            sCopyFile = ""
        Else
            bDelFlag = False
        End If
        sClipboard = Mid(sClipboard, nPos + 1, Len(sClipboard))
    Else
        'Get file
        sCopyFile = sClipboard
        If sCopyFile = sDBFile Then
            sCopyFile = ""
        Else
            bDelFlag = False
        End If
    End If
End Sub

Sub ToClipboard()
    Dim sFlag As String
    
    'Get flag
    If bDelFlag = True Then sFlag = "1"
    If bDelFlag = False Then sFlag = "0"
    
    'Put data in clipboard
    Clipboard.Clear
    Clipboard.SetText (MIS_REF_ID + "|" + sCopyKey + "|" + sFlag + "|" + sDBFile)
End Sub

Function DecodeEnv(ByVal sDir As String) As String
    Dim nPos As Long
    
    Dim sVar As String
    Dim sReg As String

    'Set default
    DecodeEnv = ""
    
    'Check directory
    If sDir = "" Then Exit Function
    
    'Append \ character
    If Mid(sDir, Len(sDir)) <> "\" Then sDir = sDir + "\"
    
    'Set default
    DecodeEnv = sDir
    
    'Check for application path
    If Left(sDir, 1) = "~" Then
        'Remove ~ character
        sDir = Mid(sDir, 2)
    
        'Get application path
        sVar = App.Path
        If Mid(sVar, Len(sVar)) <> "\" Then sVar = sVar + "\"
        
        'Truncate at \ character
        nPos = InStr(sDir, "\")
        If nPos > 0 Then
            'Get path
            DecodeEnv = sVar + Mid(sDir, nPos + 1)
            Exit Function
        Else
            'Get path
            DecodeEnv = sVar
            Exit Function
        End If
        Exit Function
    End If
    
    'Check for environment variable
    If Left(sDir, 1) = "%" Then
        'Remove % character
        sDir = Mid(sDir, 2)
    
        'Truncate at \ character
        nPos = InStr(sDir, "\")
        If nPos > 0 Then
            'Get environment variable
            sVar = Environ(Left(sDir, nPos - 1))
            If sVar <> "" Then
                'Append \ character
                If Mid(sVar, Len(sDir)) <> "\" Then sVar = sVar + "\"
                DecodeEnv = sVar + Mid(sDir, nPos + 1)
                Exit Function
            End If
            
            'Set environment variable
            sVar = Left(sDir, nPos - 1)
        Else
            'Get environment variable
            sVar = Environ(sDir)
            If sVar <> "" Then
                DecodeEnv = sVar
                Exit Function
            End If
            
            'Set environment variable
            sVar = sDir
        End If
        
        'Inform user
        Call MsgBox("Error: Unable to find environment variable (" + sVar + ")!", vbOKOnly Or vbExclamation, "MissionMan")
        Exit Function
    End If
    
    'Check for registry entry
    If Left(sDir, 1) = "$" Then
        'Remove $ character
        sDir = Mid(sDir, 2)
    
        'Get registry key
        Call misGetVal(MIS_SEC_COM, MIS_KEY_REGK, sReg, MIS_MOD_CFG)
        sReg = TruncStr(sReg)
        If sReg = "" Then
            'Set registry value
            sVar = sDir
            
            'Inform user
            Call MsgBox("Error: Unable to find registry key (" + sVar + ")!", vbOKOnly Or vbExclamation, "MissionMan")
            Exit Function
        End If
        
        'Truncate at \ character
        nPos = InStr(sDir, "\")
        If nPos > 0 Then
            'Get registry value
            Call misGetVal(sReg, Left(sDir, nPos - 1), sVar, MIS_MOD_REG)
            sVar = TruncStr(sVar)
            If sVar <> "" Then
                'Append \ character
                If Mid(sVar, Len(sDir)) <> "\" Then sVar = sVar + "\"
                DecodeEnv = sVar + Mid(sDir, nPos + 1)
                Exit Function
            End If
            
            'Set registry value
            sVar = Left(sDir, nPos - 1)
        Else
            'Get registry value
            Call misGetVal(sReg, sDir, sVar, MIS_MOD_REG)
            sVar = TruncStr(sVar)
            If sVar <> "" Then
                DecodeEnv = sVar
                Exit Function
            End If
            
            'Set registry value
            sVar = sDir
        End If
        
        'Inform user
        Call MsgBox("Error: Unable to find registry value (" + sVar + ")!", vbOKOnly Or vbExclamation, "MissionMan")
    End If
End Function

Function EncodeEnv(ByVal sDir As String) As String
    Dim nInd As Integer
    Dim nPos As Long
    
    Dim sLine As String
    Dim sVar As String
    Dim sReg As String

    'Set default
    EncodeEnv = sDir
    
    'Check directory
    If sDir = "" Then Exit Function
    
    'Get application path
    sVar = App.Path
    If Mid(sVar, Len(sVar)) <> "\" Then sVar = sVar + "\"
        
    'Set application path
    If InStr(1, sDir, sVar, vbTextCompare) > 0 Then
        EncodeEnv = "~\" + Mid(sDir, Len(sVar) + 1)
        Exit Function
    End If
    
    'Reset index
    nInd = 1
    
    'Loop
    Do
        'Get environment variable
        sLine = Environ(nInd)
        If sLine = "" Then Exit Do
        
        'Check environment variable
        If InStr(1, sLine, MIS_ENV_VAR, vbTextCompare) > 0 Then
            'Truncate at = character
            nPos = InStr(sLine, "=")
            
            'Append \ character
            sVar = Mid(sLine, nPos + 1)
            If Mid(sVar, Len(sVar)) <> "\" Then sVar = sVar + "\"
            
            'Set environment variable
            If InStr(1, sDir, sVar, vbTextCompare) > 0 Then
                EncodeEnv = "%" + Left(sLine, nPos - 1) + "\" + Mid(sDir, Len(sVar) + 1)
                Exit Function
            End If
        End If
        
        'Increment index
        nInd = nInd + 1
    Loop
    
    'Get registry key
    Call misGetVal(MIS_SEC_COM, MIS_KEY_REGK, sReg, MIS_MOD_CFG)
    sReg = TruncStr(sReg)
    If sReg = "" Then Exit Function
    
    'Reset index
    nInd = 0
    
    'Loop
    Do
        'Get registry value
        Call misGetVal(sReg, "#" + Trim(Str(nInd)), sLine, MIS_MOD_REG)
        sLine = TruncStr(sLine)
        If sLine = "" Then Exit Do
        
        'Check registry value
        If InStr(1, sLine, MIS_ENV_VAR, vbTextCompare) > 0 Then
            'Truncate at = character
            nPos = InStr(sLine, "=")
            
            'Append \ character
            sVar = Mid(sLine, nPos + 1)
            If Mid(sVar, Len(sVar)) <> "\" Then sVar = sVar + "\"
            
            'Set environment variable
            If InStr(1, sDir, sVar, vbTextCompare) > 0 Then
                EncodeEnv = "$" + Left(sLine, nPos - 1) + "\" + Mid(sDir, Len(sVar) + 1)
                Exit Function
            End If
        End If
        
        'Increment index
        nInd = nInd + 1
    Loop
End Function

