# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Fontastic - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Fontastic - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Fontastic - Win32 Release" && "$(CFG)" !=\
 "Fontastic - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Fontastic.mak" CFG="Fontastic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Fontastic - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Fontastic - Win32 Debug" (based on "Win32 (x86) Application")
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
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "Fontastic - Win32 Release"

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

ALL : "$(OUTDIR)\Fontastic.exe"

CLEAN : 
	-@erase "$(INTDIR)\ChooseTextDlg.obj"
	-@erase "$(INTDIR)\Fontastic.obj"
	-@erase "$(INTDIR)\Fontastic.pch"
	-@erase "$(INTDIR)\Fontastic.res"
	-@erase "$(INTDIR)\FontasticDlg.obj"
	-@erase "$(INTDIR)\hff.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(OUTDIR)\Fontastic.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)/Fontastic.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Fontastic.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Fontastic.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/Fontastic.pdb" /machine:I386 /out:"$(OUTDIR)/Fontastic.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChooseTextDlg.obj" \
	"$(INTDIR)\Fontastic.obj" \
	"$(INTDIR)\Fontastic.res" \
	"$(INTDIR)\FontasticDlg.obj" \
	"$(INTDIR)\hff.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\Fontastic.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Fontastic - Win32 Debug"

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

ALL : "$(OUTDIR)\Fontastic.exe"

CLEAN : 
	-@erase "$(INTDIR)\ChooseTextDlg.obj"
	-@erase "$(INTDIR)\Fontastic.obj"
	-@erase "$(INTDIR)\Fontastic.pch"
	-@erase "$(INTDIR)\Fontastic.res"
	-@erase "$(INTDIR)\FontasticDlg.obj"
	-@erase "$(INTDIR)\hff.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Fontastic.exe"
	-@erase "$(OUTDIR)\Fontastic.ilk"
	-@erase "$(OUTDIR)\Fontastic.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_MBCS" /Fp"$(INTDIR)/Fontastic.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Fontastic.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Fontastic.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/Fontastic.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/Fontastic.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChooseTextDlg.obj" \
	"$(INTDIR)\Fontastic.obj" \
	"$(INTDIR)\Fontastic.res" \
	"$(INTDIR)\FontasticDlg.obj" \
	"$(INTDIR)\hff.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\Fontastic.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "Fontastic - Win32 Release"
# Name "Fontastic - Win32 Debug"

!IF  "$(CFG)" == "Fontastic - Win32 Release"

!ELSEIF  "$(CFG)" == "Fontastic - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "Fontastic - Win32 Release"

!ELSEIF  "$(CFG)" == "Fontastic - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fontastic.cpp
DEP_CPP_FONTA=\
	".\Fontastic.h"\
	".\FontasticDlg.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\Fontastic.obj" : $(SOURCE) $(DEP_CPP_FONTA) "$(INTDIR)"\
 "$(INTDIR)\Fontastic.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\FontasticDlg.cpp
DEP_CPP_FONTAS=\
	".\ChooseTextDlg.h"\
	".\Fontastic.h"\
	".\FontasticDlg.h"\
	".\hff.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\FontasticDlg.obj" : $(SOURCE) $(DEP_CPP_FONTAS) "$(INTDIR)"\
 "$(INTDIR)\Fontastic.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "Fontastic - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS"\
 /Fp"$(INTDIR)/Fontastic.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Fontastic.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Fontastic - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_MBCS" /Fp"$(INTDIR)/Fontastic.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Fontastic.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fontastic.rc
DEP_RSC_FONTAST=\
	".\res\Fontastic.ico"\
	".\res\Fontastic.rc2"\
	

"$(INTDIR)\Fontastic.res" : $(SOURCE) $(DEP_RSC_FONTAST) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ChooseTextDlg.cpp
DEP_CPP_CHOOS=\
	".\ChooseTextDlg.h"\
	".\Fontastic.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\ChooseTextDlg.obj" : $(SOURCE) $(DEP_CPP_CHOOS) "$(INTDIR)"\
 "$(INTDIR)\Fontastic.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\hff.cpp
DEP_CPP_HFF_C=\
	".\Fontastic.h"\
	".\hff.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\hff.obj" : $(SOURCE) $(DEP_CPP_HFF_C) "$(INTDIR)"\
 "$(INTDIR)\Fontastic.pch"


# End Source File
# End Target
# End Project
################################################################################
