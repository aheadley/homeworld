Attribute VB_Name = "MdlMain"
Option Explicit

Public Declare Function volOpenWLookup Lib "vol.dll" (ByVal szFileName As String, ByVal szID As String, ByVal nNumVRs As Integer, ByVal nNumHeads As Integer) As Long
Public Declare Function volWriteLookup Lib "vol.dll" (aLookup As Long) As Long
Public Declare Function volCloseWLookup Lib "vol.dll" () As Long
Public Declare Function volOpenRLookup Lib "vol.dll" (ByVal szFileName As String, nNumVRs As Integer, nNumHeads As Integer) As Long
Public Declare Function volReadLookup Lib "vol.dll" (aLookup As Long) As Long
Public Declare Function volCloseRLookup Lib "vol.dll" () As Long
Public Declare Function volOpenWFreq Lib "vol.dll" (ByVal szFileName As String, ByVal szID As String, ByVal nNumVRs As Integer) As Long
Public Declare Function volWriteFreq Lib "vol.dll" (aFreq As Single) As Long
Public Declare Function volCloseWFreq Lib "vol.dll" () As Long
Public Declare Function volOpenRFreq Lib "vol.dll" (ByVal szFileName As String, nNumVRs As Integer) As Long
Public Declare Function volReadFreq Lib "vol.dll" (aFreq As Single) As Long
Public Declare Function volCloseRFreq Lib "vol.dll" () As Long
Public Declare Function volGetLabels Lib "vol.dll" (ByVal szFileName As String, szLabels As String, nCount As Integer) As Long
Public Declare Function volGetErr Lib "vol.dll" (ByVal nErr As Long, szText As String) As Long

'Global variables

'Strings
Public sVolFile As String
Public sRangeFile As String
Public sFreqFile As String
Public sHeadFile As String

Public sVolID As String
Public sRangeID As String
Public sFreqID As String

Public sHeadLabels As String

'Counts
Public nVRCount As Integer
Public nHeadCount As Integer

Sub Main()
    Dim sOutDir As String
    Dim sInDir As String
    
    'Get dirs
    sInDir = DecodeEnv("%HW_ROOT\Src\Game")
    sOutDir = DecodeEnv("%HW_DATA\SoundFX")
    
    'Set default strings
    sVolFile = sOutDir + "Volume.lut"
    sRangeFile = sOutDir + "Range.lut"
    sFreqFile = sOutDir + "Frequency.lut"
    sHeadFile = sInDir + "VolTweakDefs.h"
    
    ' Set default IDs
    sVolID = "VOL1"
    sRangeID = "RNG1"
    sFreqID = "FRQ1"
    
    'Set default counts
    nVRCount = 3
           
    'Show volume form
    FrmVol.Show (1)
End Sub

Sub AntiMain()
    'Prompt user
    If MsgBox("Exit VolTool?", vbYesNo Or vbQuestion, "VolTool") = vbNo Then Exit Sub
            
    'End program
    End
End Sub

Function FileStr(ByVal sData As String) As String
    Dim nPos As Long
    
    'Set default
    FileStr = "(Invalid filename)"
    
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
    TruncStr = "(Invalid data)"
    
    'Get position of null character in C string
    nPos = InStr(sData, Chr(0))
    
    'If possible, truncate C string at null character
    If nPos > 0 Then TruncStr = Left(sData, nPos - 1)
End Function

Sub FlexGridEdit(FlexGrid As Control, Edt As Control, KeyAscii As Integer)
    ' Use the character that was typed.
    Select Case KeyAscii

    ' A space means edit the current text.
    Case 0 To 32
        Edt = FlexGrid
        Edt.SelStart = Len(FlexGrid.Text)

    ' Anything else means replace the current text.
    Case Else
        Edt = Chr(KeyAscii)
        Edt.SelStart = 1
    End Select

    ' Show Edt at the right place.
    Edt.Move FlexGrid.Left + FlexGrid.CellLeft, FlexGrid.Top + FlexGrid.CellTop, FlexGrid.CellWidth, FlexGrid.CellHeight
    Edt.Visible = True

    ' And let it work.
    Edt.SetFocus
End Sub

Sub EditKeyCode(FlexGrid As Control, Edt As Control, KeyCode As Integer, Shift As Integer)
    ' Standard edit control processing.
    Select Case KeyCode

    Case 27 ' ESC: hide, return focus to FlexGrid.
        Edt.Visible = False
        FlexGrid.SetFocus

    Case 13 ' ENTER return focus to FlexGrid.
        FlexGrid.SetFocus

    Case 38     ' Up.
        FlexGrid.SetFocus
        DoEvents
        If FlexGrid.Row > FlexGrid.FixedRows Then
            FlexGrid.Row = FlexGrid.Row - 1
        End If

    Case 40     ' Down.
        FlexGrid.SetFocus
        DoEvents
        If FlexGrid.Row < FlexGrid.Rows - 1 Then
            FlexGrid.Row = FlexGrid.Row + 1
        End If
    End Select
End Sub

Function DecodeEnv(ByVal sDir As String) As String
    Dim nPos As Long
    
    Dim sVar As String

    'Set default
    DecodeEnv = sDir
    
    'Check directory
    If sDir = "" Then Exit Function
    
    'Append \ character
    If Mid(sDir, Len(sDir)) <> "\" Then sDir = sDir + "\"
    
    'Check for environment variables
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
        Call MsgBox("Error: Unable to find environment variable (" + sVar + ")!", vbOKOnly Or vbExclamation, "VolTool")
    End If
End Function

