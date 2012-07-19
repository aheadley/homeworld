# Microsoft Developer Studio Project File - Name="FEMan" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=FEMan - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FEMan.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FEMan.mak" CFG="FEMan - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FEMan - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "FEMan - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/tools/win32/FEMan", KBMAAAAA"
# PROP Scc_LocalPath "."
CPP=xicl5.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FEMan - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /Zp4 /MT /W3 /GX /O2 /I "d:\vc42\mfc\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink5.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "d:\vc42\mfc\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink5.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386

!ENDIF 

# Begin Target

# Name "FEMan - Win32 Release"
# Name "FEMan - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\bmp.cpp
# End Source File
# Begin Source File

SOURCE=.\cclist.cpp
# End Source File
# Begin Source File

SOURCE=.\FEMan.cpp
# End Source File
# Begin Source File

SOURCE=.\FEMan.rc

!IF  "$(CFG)" == "FEMan - Win32 Release"

!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FEManDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\FEManView.cpp
# End Source File
# Begin Source File

SOURCE=.\FlowTreeView.cpp
# End Source File
# Begin Source File

SOURCE=.\GridOptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\hwexport.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\Screen.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEditView.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenLinkPropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenObjectBitmapPropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenObjectRectPropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenPropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StringObjectStringPropertiesDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\bmp.h
# End Source File
# Begin Source File

SOURCE=.\ccList.h
# End Source File
# Begin Source File

SOURCE=.\FEMan.h
# End Source File
# Begin Source File

SOURCE=.\FEManDoc.h
# End Source File
# Begin Source File

SOURCE=.\FEManView.h
# End Source File
# Begin Source File

SOURCE=.\FlowTreeView.h
# End Source File
# Begin Source File

SOURCE=.\GridOptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\hwexport.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\screen.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEditView.h
# End Source File
# Begin Source File

SOURCE=.\ScreenLinkPropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\ScreenObjectBitmapPropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\ScreenObjectRectPropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\ScreenPropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StringObjectStringPropertiesDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\FEMan.ico
# End Source File
# Begin Source File

SOURCE=.\res\FEMan.rc2
# End Source File
# Begin Source File

SOURCE=.\res\FEManDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# End Target
# End Project
