# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=LifList - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to LifList - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "LifList - Win32 Release" && "$(CFG)" !=\
 "LifList - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "liflist.mak" CFG="LifList - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LifList - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "LifList - Win32 Debug" (based on "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "LifList - Win32 Debug"
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "LifList - Win32 Release"

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

ALL : "$(OUTDIR)\liflist.exe"

CLEAN : 
	-@erase "$(INTDIR)\color.obj"
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\Debug.obj"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\layerimg.obj"
	-@erase "$(INTDIR)\liflist.obj"
	-@erase "$(INTDIR)\memory.obj"
	-@erase "$(INTDIR)\psd.obj"
	-@erase "$(INTDIR)\Quantize.obj"
	-@erase "$(INTDIR)\tga.obj"
	-@erase "$(INTDIR)\twiddle.obj"
	-@erase "$(OUTDIR)\liflist.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /Zp2 /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "HW_Debug" /YX /c
CPP_PROJ=/nologo /Zp2 /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "HW_Debug" /Fp"$(INTDIR)/liflist.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/liflist.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(OUTDIR)/liflist.pdb" /machine:I386\
 /out:"$(OUTDIR)/liflist.exe" 
LINK32_OBJS= \
	"$(INTDIR)\color.obj" \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\Debug.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\layerimg.obj" \
	"$(INTDIR)\liflist.obj" \
	"$(INTDIR)\memory.obj" \
	"$(INTDIR)\psd.obj" \
	"$(INTDIR)\Quantize.obj" \
	"$(INTDIR)\tga.obj" \
	"$(INTDIR)\twiddle.obj"

"$(OUTDIR)\liflist.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "LifList - Win32 Debug"

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

ALL : "$(OUTDIR)\liflist.exe"

CLEAN : 
	-@erase "$(INTDIR)\color.obj"
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\Debug.obj"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\layerimg.obj"
	-@erase "$(INTDIR)\liflist.obj"
	-@erase "$(INTDIR)\memory.obj"
	-@erase "$(INTDIR)\psd.obj"
	-@erase "$(INTDIR)\Quantize.obj"
	-@erase "$(INTDIR)\tga.obj"
	-@erase "$(INTDIR)\twiddle.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\liflist.exe"
	-@erase "$(OUTDIR)\liflist.ilk"
	-@erase "$(OUTDIR)\liflist.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /Zp2 /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "HW_Debug" /YX /c
CPP_PROJ=/nologo /Zp2 /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D\
 "_CONSOLE" /D "HW_Debug" /Fp"$(INTDIR)/liflist.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/liflist.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/liflist.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/liflist.exe" 
LINK32_OBJS= \
	"$(INTDIR)\color.obj" \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\Debug.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\layerimg.obj" \
	"$(INTDIR)\liflist.obj" \
	"$(INTDIR)\memory.obj" \
	"$(INTDIR)\psd.obj" \
	"$(INTDIR)\Quantize.obj" \
	"$(INTDIR)\tga.obj" \
	"$(INTDIR)\twiddle.obj"

"$(OUTDIR)\liflist.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "LifList - Win32 Release"
# Name "LifList - Win32 Debug"

!IF  "$(CFG)" == "LifList - Win32 Release"

!ELSEIF  "$(CFG)" == "LifList - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\twiddle.c
DEP_CPP_TWIDD=\
	".\twiddle.h"\
	".\types.h"\
	

"$(INTDIR)\twiddle.obj" : $(SOURCE) $(DEP_CPP_TWIDD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Debug.c
DEP_CPP_DEBUG=\
	".\debug.h"\
	".\resource.h"\
	".\types.h"\
	

"$(INTDIR)\Debug.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\file.c
DEP_CPP_FILE_=\
	".\debug.h"\
	".\file.h"\
	".\memory.h"\
	".\types.h"\
	

"$(INTDIR)\file.obj" : $(SOURCE) $(DEP_CPP_FILE_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\layerimg.c
DEP_CPP_LAYER=\
	".\color.h"\
	".\debug.h"\
	".\layerimg.h"\
	".\memory.h"\
	".\prim2d.h"\
	".\types.h"\
	

"$(INTDIR)\layerimg.obj" : $(SOURCE) $(DEP_CPP_LAYER) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\liflist.c
DEP_CPP_LIFLI=\
	".\color.h"\
	".\crc32.h"\
	".\debug.h"\
	".\layerimg.h"\
	".\memory.h"\
	".\prim2d.h"\
	".\psd.h"\
	".\quantize.h"\
	".\tga.h"\
	".\twiddle.h"\
	".\types.h"\
	

"$(INTDIR)\liflist.obj" : $(SOURCE) $(DEP_CPP_LIFLI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\memory.c
DEP_CPP_MEMOR=\
	".\debug.h"\
	".\memory.h"\
	".\types.h"\
	

"$(INTDIR)\memory.obj" : $(SOURCE) $(DEP_CPP_MEMOR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\psd.c
DEP_CPP_PSD_C=\
	".\color.h"\
	".\debug.h"\
	".\file.h"\
	".\layerimg.h"\
	".\memory.h"\
	".\prim2d.h"\
	".\psd.h"\
	".\types.h"\
	

"$(INTDIR)\psd.obj" : $(SOURCE) $(DEP_CPP_PSD_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Quantize.c
DEP_CPP_QUANT=\
	".\color.h"\
	".\debug.h"\
	".\memory.h"\
	".\quantize.h"\
	".\types.h"\
	

"$(INTDIR)\Quantize.obj" : $(SOURCE) $(DEP_CPP_QUANT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\color.c
DEP_CPP_COLOR=\
	".\color.h"\
	".\types.h"\
	

"$(INTDIR)\color.obj" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\crc32.c
DEP_CPP_CRC32=\
	".\crc32.h"\
	".\types.h"\
	

"$(INTDIR)\crc32.obj" : $(SOURCE) $(DEP_CPP_CRC32) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tga.c
DEP_CPP_TGA_C=\
	".\color.h"\
	".\debug.h"\
	".\file.h"\
	".\tga.h"\
	".\types.h"\
	

"$(INTDIR)\tga.obj" : $(SOURCE) $(DEP_CPP_TGA_C) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
