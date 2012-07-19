# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=FEMan - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to FEMan - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "FEMan - Win32 Release" && "$(CFG)" != "FEMan - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "FEMan.mak" CFG="FEMan - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FEMan - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "FEMan - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "FEMan - Win32 Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "FEMan - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\FEMan.exe"

CLEAN : 
	-@erase "$(INTDIR)\bmp.obj"
	-@erase "$(INTDIR)\cclist.obj"
	-@erase "$(INTDIR)\FEMan.obj"
	-@erase "$(INTDIR)\FEMan.pch"
	-@erase "$(INTDIR)\FEMan.res"
	-@erase "$(INTDIR)\FEManDoc.obj"
	-@erase "$(INTDIR)\FEManView.obj"
	-@erase "$(INTDIR)\FlowTreeView.obj"
	-@erase "$(INTDIR)\GridOptionsDlg.obj"
	-@erase "$(INTDIR)\hwexport.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\Screen.obj"
	-@erase "$(INTDIR)\ScreenEditView.obj"
	-@erase "$(INTDIR)\ScreenLinkPropertiesDlg.obj"
	-@erase "$(INTDIR)\ScreenObjectBitmapPropertiesDlg.obj"
	-@erase "$(INTDIR)\ScreenObjectRectPropertiesDlg.obj"
	-@erase "$(INTDIR)\ScreenPropertiesDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StringObjectStringPropertiesDlg.obj"
	-@erase "$(OUTDIR)\FEMan.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /Zp4 /MT /W3 /GX /O2 /I "d:/vc42/mfc/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /Zp4 /MT /W3 /GX /O2 /I "d:/vc42/mfc/include" /D "WIN32" /D\
 "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)/FEMan.pch" /Yu"stdafx.h"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/FEMan.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/FEMan.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/FEMan.pdb" /machine:I386 /out:"$(OUTDIR)/FEMan.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bmp.obj" \
	"$(INTDIR)\cclist.obj" \
	"$(INTDIR)\FEMan.obj" \
	"$(INTDIR)\FEMan.res" \
	"$(INTDIR)\FEManDoc.obj" \
	"$(INTDIR)\FEManView.obj" \
	"$(INTDIR)\FlowTreeView.obj" \
	"$(INTDIR)\GridOptionsDlg.obj" \
	"$(INTDIR)\hwexport.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\Screen.obj" \
	"$(INTDIR)\ScreenEditView.obj" \
	"$(INTDIR)\ScreenLinkPropertiesDlg.obj" \
	"$(INTDIR)\ScreenObjectBitmapPropertiesDlg.obj" \
	"$(INTDIR)\ScreenObjectRectPropertiesDlg.obj" \
	"$(INTDIR)\ScreenPropertiesDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\StringObjectStringPropertiesDlg.obj"

"$(OUTDIR)\FEMan.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\FEMan.exe" "$(OUTDIR)\FEMan.bsc"

CLEAN : 
	-@erase "$(INTDIR)\bmp.obj"
	-@erase "$(INTDIR)\bmp.sbr"
	-@erase "$(INTDIR)\cclist.obj"
	-@erase "$(INTDIR)\cclist.sbr"
	-@erase "$(INTDIR)\FEMan.obj"
	-@erase "$(INTDIR)\FEMan.pch"
	-@erase "$(INTDIR)\FEMan.res"
	-@erase "$(INTDIR)\FEMan.sbr"
	-@erase "$(INTDIR)\FEManDoc.obj"
	-@erase "$(INTDIR)\FEManDoc.sbr"
	-@erase "$(INTDIR)\FEManView.obj"
	-@erase "$(INTDIR)\FEManView.sbr"
	-@erase "$(INTDIR)\FlowTreeView.obj"
	-@erase "$(INTDIR)\FlowTreeView.sbr"
	-@erase "$(INTDIR)\GridOptionsDlg.obj"
	-@erase "$(INTDIR)\GridOptionsDlg.sbr"
	-@erase "$(INTDIR)\hwexport.obj"
	-@erase "$(INTDIR)\hwexport.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\Screen.obj"
	-@erase "$(INTDIR)\Screen.sbr"
	-@erase "$(INTDIR)\ScreenEditView.obj"
	-@erase "$(INTDIR)\ScreenEditView.sbr"
	-@erase "$(INTDIR)\ScreenLinkPropertiesDlg.obj"
	-@erase "$(INTDIR)\ScreenLinkPropertiesDlg.sbr"
	-@erase "$(INTDIR)\ScreenObjectBitmapPropertiesDlg.obj"
	-@erase "$(INTDIR)\ScreenObjectBitmapPropertiesDlg.sbr"
	-@erase "$(INTDIR)\ScreenObjectRectPropertiesDlg.obj"
	-@erase "$(INTDIR)\ScreenObjectRectPropertiesDlg.sbr"
	-@erase "$(INTDIR)\ScreenPropertiesDlg.obj"
	-@erase "$(INTDIR)\ScreenPropertiesDlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\StringObjectStringPropertiesDlg.obj"
	-@erase "$(INTDIR)\StringObjectStringPropertiesDlg.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\FEMan.bsc"
	-@erase "$(OUTDIR)\FEMan.exe"
	-@erase "$(OUTDIR)\FEMan.ilk"
	-@erase "$(OUTDIR)\FEMan.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "d:/vc42/mfc/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /c
CPP_PROJ=/nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "d:/vc42/mfc/include" /D\
 "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/FEMan.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/FEMan.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/FEMan.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\bmp.sbr" \
	"$(INTDIR)\cclist.sbr" \
	"$(INTDIR)\FEMan.sbr" \
	"$(INTDIR)\FEManDoc.sbr" \
	"$(INTDIR)\FEManView.sbr" \
	"$(INTDIR)\FlowTreeView.sbr" \
	"$(INTDIR)\GridOptionsDlg.sbr" \
	"$(INTDIR)\hwexport.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\Screen.sbr" \
	"$(INTDIR)\ScreenEditView.sbr" \
	"$(INTDIR)\ScreenLinkPropertiesDlg.sbr" \
	"$(INTDIR)\ScreenObjectBitmapPropertiesDlg.sbr" \
	"$(INTDIR)\ScreenObjectRectPropertiesDlg.sbr" \
	"$(INTDIR)\ScreenPropertiesDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\StringObjectStringPropertiesDlg.sbr"

"$(OUTDIR)\FEMan.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/FEMan.pdb" /debug /machine:I386 /out:"$(OUTDIR)/FEMan.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bmp.obj" \
	"$(INTDIR)\cclist.obj" \
	"$(INTDIR)\FEMan.obj" \
	"$(INTDIR)\FEMan.res" \
	"$(INTDIR)\FEManDoc.obj" \
	"$(INTDIR)\FEManView.obj" \
	"$(INTDIR)\FlowTreeView.obj" \
	"$(INTDIR)\GridOptionsDlg.obj" \
	"$(INTDIR)\hwexport.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\Screen.obj" \
	"$(INTDIR)\ScreenEditView.obj" \
	"$(INTDIR)\ScreenLinkPropertiesDlg.obj" \
	"$(INTDIR)\ScreenObjectBitmapPropertiesDlg.obj" \
	"$(INTDIR)\ScreenObjectRectPropertiesDlg.obj" \
	"$(INTDIR)\ScreenPropertiesDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\StringObjectStringPropertiesDlg.obj"

"$(OUTDIR)\FEMan.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "FEMan - Win32 Release"
# Name "FEMan - Win32 Debug"

!IF  "$(CFG)" == "FEMan - Win32 Release"

!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "FEMan - Win32 Release"

!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\FEMan.cpp
DEP_CPP_FEMAN=\
	".\bmp.h"\
	".\ccList.h"\
	".\FEMan.h"\
	".\FEManDoc.h"\
	".\FEManView.h"\
	".\MainFrm.h"\
	".\screen.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\FEMan.obj" : $(SOURCE) $(DEP_CPP_FEMAN) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\FEMan.obj" : $(SOURCE) $(DEP_CPP_FEMAN) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\FEMan.sbr" : $(SOURCE) $(DEP_CPP_FEMAN) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /Zp4 /MT /W3 /GX /O2 /I "d:/vc42/mfc/include" /D "WIN32" /D\
 "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)/FEMan.pch" /Yc"stdafx.h"\
 /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\FEMan.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "d:/vc42/mfc/include" /D\
 "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/FEMan.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\FEMan.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainFrm.cpp
DEP_CPP_MAINF=\
	".\bmp.h"\
	".\ccList.h"\
	".\FEMan.h"\
	".\FEManDoc.h"\
	".\FlowTreeView.h"\
	".\MainFrm.h"\
	".\screen.h"\
	".\ScreenEditView.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\FEManDoc.cpp
DEP_CPP_FEMAND=\
	".\bmp.h"\
	".\ccList.h"\
	".\FEMan.h"\
	".\FEManDoc.h"\
	".\FlowTreeView.h"\
	".\hwexport.h"\
	".\screen.h"\
	".\ScreenEditView.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\FEManDoc.obj" : $(SOURCE) $(DEP_CPP_FEMAND) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\FEManDoc.obj" : $(SOURCE) $(DEP_CPP_FEMAND) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\FEManDoc.sbr" : $(SOURCE) $(DEP_CPP_FEMAND) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\FEManView.cpp
DEP_CPP_FEMANV=\
	".\bmp.h"\
	".\ccList.h"\
	".\FEMan.h"\
	".\FEManDoc.h"\
	".\FEManView.h"\
	".\screen.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\FEManView.obj" : $(SOURCE) $(DEP_CPP_FEMANV) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\FEManView.obj" : $(SOURCE) $(DEP_CPP_FEMANV) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\FEManView.sbr" : $(SOURCE) $(DEP_CPP_FEMANV) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\FEMan.rc
DEP_RSC_FEMAN_=\
	".\res\FEMan.ico"\
	".\res\FEMan.rc2"\
	".\res\FEManDoc.ico"\
	".\res\Toolbar.bmp"\
	

"$(INTDIR)\FEMan.res" : $(SOURCE) $(DEP_RSC_FEMAN_) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\StringObjectStringPropertiesDlg.cpp
DEP_CPP_STRIN=\
	".\bmp.h"\
	".\ccList.h"\
	".\FEMan.h"\
	".\screen.h"\
	".\StdAfx.h"\
	".\StringObjectStringPropertiesDlg.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\StringObjectStringPropertiesDlg.obj" : $(SOURCE) $(DEP_CPP_STRIN)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\StringObjectStringPropertiesDlg.obj" : $(SOURCE) $(DEP_CPP_STRIN)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"

"$(INTDIR)\StringObjectStringPropertiesDlg.sbr" : $(SOURCE) $(DEP_CPP_STRIN)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cclist.cpp
DEP_CPP_CCLIS=\
	".\ccList.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\cclist.obj" : $(SOURCE) $(DEP_CPP_CCLIS) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\cclist.obj" : $(SOURCE) $(DEP_CPP_CCLIS) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\cclist.sbr" : $(SOURCE) $(DEP_CPP_CCLIS) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\FlowTreeView.cpp
DEP_CPP_FLOWT=\
	".\bmp.h"\
	".\ccList.h"\
	".\FEMan.h"\
	".\FEManDoc.h"\
	".\FlowTreeView.h"\
	".\screen.h"\
	".\ScreenLinkPropertiesDlg.h"\
	".\ScreenPropertiesDlg.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\FlowTreeView.obj" : $(SOURCE) $(DEP_CPP_FLOWT) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\FlowTreeView.obj" : $(SOURCE) $(DEP_CPP_FLOWT) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\FlowTreeView.sbr" : $(SOURCE) $(DEP_CPP_FLOWT) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GridOptionsDlg.cpp
DEP_CPP_GRIDO=\
	".\FEMan.h"\
	".\GridOptionsDlg.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\GridOptionsDlg.obj" : $(SOURCE) $(DEP_CPP_GRIDO) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\GridOptionsDlg.obj" : $(SOURCE) $(DEP_CPP_GRIDO) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\GridOptionsDlg.sbr" : $(SOURCE) $(DEP_CPP_GRIDO) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Screen.cpp
DEP_CPP_SCREE=\
	".\bmp.h"\
	".\ccList.h"\
	".\FEMan.h"\
	".\FEManDoc.h"\
	".\screen.h"\
	".\ScreenEditView.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\Screen.obj" : $(SOURCE) $(DEP_CPP_SCREE) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\Screen.obj" : $(SOURCE) $(DEP_CPP_SCREE) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\Screen.sbr" : $(SOURCE) $(DEP_CPP_SCREE) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ScreenEditView.cpp
DEP_CPP_SCREEN=\
	".\bmp.h"\
	".\ccList.h"\
	".\FEMan.h"\
	".\FEManDoc.h"\
	".\GridOptionsDlg.h"\
	".\screen.h"\
	".\ScreenEditView.h"\
	".\ScreenObjectBitmapPropertiesDlg.h"\
	".\ScreenObjectRectPropertiesDlg.h"\
	".\StdAfx.h"\
	".\StringObjectStringPropertiesDlg.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\ScreenEditView.obj" : $(SOURCE) $(DEP_CPP_SCREEN) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\ScreenEditView.obj" : $(SOURCE) $(DEP_CPP_SCREEN) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\ScreenEditView.sbr" : $(SOURCE) $(DEP_CPP_SCREEN) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ScreenLinkPropertiesDlg.cpp
DEP_CPP_SCREENL=\
	".\FEMan.h"\
	".\ScreenLinkPropertiesDlg.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\ScreenLinkPropertiesDlg.obj" : $(SOURCE) $(DEP_CPP_SCREENL)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\ScreenLinkPropertiesDlg.obj" : $(SOURCE) $(DEP_CPP_SCREENL)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"

"$(INTDIR)\ScreenLinkPropertiesDlg.sbr" : $(SOURCE) $(DEP_CPP_SCREENL)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ScreenObjectBitmapPropertiesDlg.cpp
DEP_CPP_SCREENO=\
	".\FEMan.h"\
	".\ScreenObjectBitmapPropertiesDlg.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\ScreenObjectBitmapPropertiesDlg.obj" : $(SOURCE) $(DEP_CPP_SCREENO)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\ScreenObjectBitmapPropertiesDlg.obj" : $(SOURCE) $(DEP_CPP_SCREENO)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"

"$(INTDIR)\ScreenObjectBitmapPropertiesDlg.sbr" : $(SOURCE) $(DEP_CPP_SCREENO)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ScreenPropertiesDlg.cpp
DEP_CPP_SCREENP=\
	".\bmp.h"\
	".\ccList.h"\
	".\FEMan.h"\
	".\FEManDoc.h"\
	".\screen.h"\
	".\ScreenPropertiesDlg.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\ScreenPropertiesDlg.obj" : $(SOURCE) $(DEP_CPP_SCREENP) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\ScreenPropertiesDlg.obj" : $(SOURCE) $(DEP_CPP_SCREENP) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\ScreenPropertiesDlg.sbr" : $(SOURCE) $(DEP_CPP_SCREENP) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bmp.cpp
DEP_CPP_BMP_C=\
	".\bmp.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\bmp.obj" : $(SOURCE) $(DEP_CPP_BMP_C) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\bmp.obj" : $(SOURCE) $(DEP_CPP_BMP_C) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\bmp.sbr" : $(SOURCE) $(DEP_CPP_BMP_C) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ScreenObjectRectPropertiesDlg.cpp
DEP_CPP_SCREENOB=\
	".\FEMan.h"\
	".\ScreenObjectRectPropertiesDlg.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\ScreenObjectRectPropertiesDlg.obj" : $(SOURCE) $(DEP_CPP_SCREENOB)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\ScreenObjectRectPropertiesDlg.obj" : $(SOURCE) $(DEP_CPP_SCREENOB)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"

"$(INTDIR)\ScreenObjectRectPropertiesDlg.sbr" : $(SOURCE) $(DEP_CPP_SCREENOB)\
 "$(INTDIR)" "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hwexport.cpp
DEP_CPP_HWEXP=\
	".\bmp.h"\
	".\ccList.h"\
	".\FEManDoc.h"\
	".\hwexport.h"\
	".\screen.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "FEMan - Win32 Release"


"$(INTDIR)\hwexport.obj" : $(SOURCE) $(DEP_CPP_HWEXP) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ELSEIF  "$(CFG)" == "FEMan - Win32 Debug"


"$(INTDIR)\hwexport.obj" : $(SOURCE) $(DEP_CPP_HWEXP) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"

"$(INTDIR)\hwexport.sbr" : $(SOURCE) $(DEP_CPP_HWEXP) "$(INTDIR)"\
 "$(INTDIR)\FEMan.pch"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
