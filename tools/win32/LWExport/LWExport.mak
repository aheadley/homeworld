# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=LWExport - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to LWExport - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "LWExport - Win32 Release" && "$(CFG)" !=\
 "LWExport - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "LWExport.mak" CFG="LWExport - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LWExport - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "LWExport - Win32 Debug" (based on "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "LWExport - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LWExport - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\LWExport.exe"

CLEAN : 
	-@erase "$(INTDIR)\cclist.obj"
	-@erase "$(INTDIR)\export.obj"
	-@erase "$(INTDIR)\fastmath.obj"
	-@erase "$(INTDIR)\geoparse.obj"
	-@erase "$(INTDIR)\hsfparse.obj"
	-@erase "$(INTDIR)\lwmath.obj"
	-@erase "$(INTDIR)\lwoparse.obj"
	-@erase "$(INTDIR)\lwsparse.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\mexparse.obj"
	-@erase "$(INTDIR)\nisparse.obj"
	-@erase "$(INTDIR)\parsefunc.obj"
	-@erase "$(INTDIR)\readwrite.obj"
	-@erase "$(INTDIR)\reorient.obj"
	-@erase "$(INTDIR)\vector.obj"
	-@erase "$(OUTDIR)\LWExport.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /Fp"$(INTDIR)/LWExport.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/LWExport.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/LWExport.pdb" /machine:I386 /out:"$(OUTDIR)/LWExport.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cclist.obj" \
	"$(INTDIR)\export.obj" \
	"$(INTDIR)\fastmath.obj" \
	"$(INTDIR)\geoparse.obj" \
	"$(INTDIR)\hsfparse.obj" \
	"$(INTDIR)\lwmath.obj" \
	"$(INTDIR)\lwoparse.obj" \
	"$(INTDIR)\lwsparse.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\mexparse.obj" \
	"$(INTDIR)\nisparse.obj" \
	"$(INTDIR)\parsefunc.obj" \
	"$(INTDIR)\readwrite.obj" \
	"$(INTDIR)\reorient.obj" \
	"$(INTDIR)\vector.obj"

"$(OUTDIR)\LWExport.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\LWExport.exe"

CLEAN : 
	-@erase "$(INTDIR)\cclist.obj"
	-@erase "$(INTDIR)\export.obj"
	-@erase "$(INTDIR)\fastmath.obj"
	-@erase "$(INTDIR)\geoparse.obj"
	-@erase "$(INTDIR)\hsfparse.obj"
	-@erase "$(INTDIR)\lwmath.obj"
	-@erase "$(INTDIR)\lwoparse.obj"
	-@erase "$(INTDIR)\lwsparse.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\mexparse.obj"
	-@erase "$(INTDIR)\nisparse.obj"
	-@erase "$(INTDIR)\parsefunc.obj"
	-@erase "$(INTDIR)\readwrite.obj"
	-@erase "$(INTDIR)\reorient.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\vector.obj"
	-@erase "$(OUTDIR)\LWExport.exe"
	-@erase "$(OUTDIR)\LWExport.ilk"
	-@erase "$(OUTDIR)\LWExport.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /D "_MBCS" /Fp"$(INTDIR)/LWExport.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/LWExport.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/LWExport.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/LWExport.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cclist.obj" \
	"$(INTDIR)\export.obj" \
	"$(INTDIR)\fastmath.obj" \
	"$(INTDIR)\geoparse.obj" \
	"$(INTDIR)\hsfparse.obj" \
	"$(INTDIR)\lwmath.obj" \
	"$(INTDIR)\lwoparse.obj" \
	"$(INTDIR)\lwsparse.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\mexparse.obj" \
	"$(INTDIR)\nisparse.obj" \
	"$(INTDIR)\parsefunc.obj" \
	"$(INTDIR)\readwrite.obj" \
	"$(INTDIR)\reorient.obj" \
	"$(INTDIR)\vector.obj"

"$(OUTDIR)\LWExport.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "LWExport - Win32 Release"
# Name "LWExport - Win32 Debug"

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\export.cpp
DEP_CPP_EXPOR=\
	".\cclist.h"\
	".\export.h"\
	".\geoparse.h"\
	".\hsfparse.h"\
	".\lwoparse.h"\
	".\lwsparse.h"\
	".\matrix.h"\
	".\mexparse.h"\
	".\nisparse.h"\
	".\parsefunc.h"\
	".\types.h"\
	".\vector.h"\
	

"$(INTDIR)\export.obj" : $(SOURCE) $(DEP_CPP_EXPOR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\geoparse.cpp
DEP_CPP_GEOPA=\
	".\cclist.h"\
	".\export.h"\
	".\geoparse.h"\
	".\lwoparse.h"\
	".\matrix.h"\
	".\parsefunc.h"\
	".\types.h"\
	".\vector.h"\
	

"$(INTDIR)\geoparse.obj" : $(SOURCE) $(DEP_CPP_GEOPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\lwoparse.cpp
DEP_CPP_LWOPA=\
	".\cclist.h"\
	".\export.h"\
	".\lwmath.h"\
	".\lwoparse.h"\
	".\matrix.h"\
	".\parsefunc.h"\
	".\readwrite.h"\
	".\reorient.h"\
	".\types.h"\
	".\vector.h"\
	

"$(INTDIR)\lwoparse.obj" : $(SOURCE) $(DEP_CPP_LWOPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\lwsparse.cpp
DEP_CPP_LWSPA=\
	".\cclist.h"\
	".\export.h"\
	".\lwoparse.h"\
	".\lwsparse.h"\
	".\matrix.h"\
	".\parsefunc.h"\
	".\readwrite.h"\
	".\reorient.h"\
	".\types.h"\
	".\vector.h"\
	

"$(INTDIR)\lwsparse.obj" : $(SOURCE) $(DEP_CPP_LWSPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\parsefunc.cpp
DEP_CPP_PARSE=\
	".\parsefunc.h"\
	

"$(INTDIR)\parsefunc.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\readwrite.cpp
DEP_CPP_READW=\
	".\readwrite.h"\
	

"$(INTDIR)\readwrite.obj" : $(SOURCE) $(DEP_CPP_READW) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\cclist.cpp
DEP_CPP_CCLIS=\
	".\cclist.h"\
	

"$(INTDIR)\cclist.obj" : $(SOURCE) $(DEP_CPP_CCLIS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\lwmath.cpp
DEP_CPP_LWMAT=\
	".\cclist.h"\
	".\lwmath.h"\
	".\lwoparse.h"\
	".\matrix.h"\
	".\parsefunc.h"\
	".\types.h"\
	".\vector.h"\
	

"$(INTDIR)\lwmath.obj" : $(SOURCE) $(DEP_CPP_LWMAT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mexparse.cpp
DEP_CPP_MEXPA=\
	".\cclist.h"\
	".\export.h"\
	".\geoparse.h"\
	".\lwoparse.h"\
	".\lwsparse.h"\
	".\matrix.h"\
	".\mexparse.h"\
	".\parsefunc.h"\
	".\types.h"\
	".\vector.h"\
	

"$(INTDIR)\mexparse.obj" : $(SOURCE) $(DEP_CPP_MEXPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\matrix.cpp

!IF  "$(CFG)" == "LWExport - Win32 Release"

DEP_CPP_MATRI=\
	".\matrix.h"\
	".\vector.h"\
	

"$(INTDIR)\matrix.obj" : $(SOURCE) $(DEP_CPP_MATRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

DEP_CPP_MATRI=\
	".\matrix.h"\
	".\types.h"\
	".\vector.h"\
	

"$(INTDIR)\matrix.obj" : $(SOURCE) $(DEP_CPP_MATRI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\vector.cpp
DEP_CPP_VECTO=\
	".\fastmath.h"\
	".\types.h"\
	".\vector.h"\
	

"$(INTDIR)\vector.obj" : $(SOURCE) $(DEP_CPP_VECTO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\fastmath.cpp
DEP_CPP_FASTM=\
	".\fastmath.h"\
	".\types.h"\
	

"$(INTDIR)\fastmath.obj" : $(SOURCE) $(DEP_CPP_FASTM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\nisparse.cpp
DEP_CPP_NISPA=\
	".\cclist.h"\
	".\export.h"\
	".\lwoparse.h"\
	".\lwsparse.h"\
	".\matrix.h"\
	".\nisparse.h"\
	".\parsefunc.h"\
	".\raceDefs.h"\
	".\types.h"\
	".\vector.h"\
	

"$(INTDIR)\nisparse.obj" : $(SOURCE) $(DEP_CPP_NISPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\hsfparse.cpp
DEP_CPP_HSFPA=\
	".\cclist.h"\
	".\export.h"\
	".\hsfparse.h"\
	".\lwsparse.h"\
	".\matrix.h"\
	".\parsefunc.h"\
	".\types.h"\
	".\vector.h"\
	

"$(INTDIR)\hsfparse.obj" : $(SOURCE) $(DEP_CPP_HSFPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\reorient.cpp
DEP_CPP_REORI=\
	".\matrix.h"\
	".\reorient.h"\
	".\types.h"\
	".\vector.h"\
	

"$(INTDIR)\reorient.obj" : $(SOURCE) $(DEP_CPP_REORI) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
