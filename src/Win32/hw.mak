#
# Homeworld Windows interface layer makefile
#

#
# Targets and directories
#
Name        = Homeworld
Target      = glHomeworld
Link_file   = $(Name).lnk
SrcDir      = .
SrcDir2     = $(%HW_Root)\src\game
SrcDir3     = $(%HW_Root)\src\Ships
SrcDir4     = $(%HW_Root)\src\Sigs
SrcDir5     = $(%HW_Root)\src\Titan\inc
SrcDir6     = $(%HW_Root)\src\Generated
SrcDir7     = $(%HW_Root)\src\jpg
LibDir      = $(%HW_Root)\Lib
TitanLibDir = $(%HW_Root)\src\Titan\bin
Data        = $(%HW_Root)\Data
KASDirs     = $(%HW_Root)\src\SinglePlayer;$(%HW_Root)\src\Tutorials
#KASSPRoot   = $(DataSrc)\SinglePlayer
#KASDirs     = $(KASSPRoot)\Mission01;$(KASSPRoot)\Mission02;$(KASSPRoot)\Mission03; &
#                $(KASSPRoot)\Mission04;$(KASSPRoot)\Mission05;$(KASSPRoot)\Mission06; &
#                $(KASSPRoot)\Mission07;$(KASSPRoot)\Mission08;$(KASSPRoot)\Mission09; &
#                $(KASSPRoot)\Mission10;$(KASSPRoot)\Mission11;$(KASSPRoot)\Mission12; &
#                $(KASSPRoot)\Mission13;$(KASSPRoot)\Mission14;$(KASSPRoot)\Mission15; &
#                $(KASSPRoot)\Mission16;$(KASSPRoot)\Mission17;$(KASSPRoot)\Mission18


#SWDefine    = -DSW_Render=1 -D_WINSOCK_VERSION=1
SWDefine    = -D_WINSOCK_VERSION=1

#
# C compiler and Assembler options (Pentium in our case)
#
Processor   = -G5

#
# Default compile and link flags
#
Link        = link
LinkFlags0  = /MAP /SUBSYSTEM:WINDOWS,4.0 /NOLOGO /DEBUG /DEBUGTYPE:CV /PDB:NONE
LinkFlags1  = /MAP /SUBSYSTEM:WINDOWS,4.0 /NOLOGO /DEBUG:NONE /PDB:NONE
LinkFlags2  = /MAP /SUBSYSTEM:WINDOWS,4.0 /NOLOGO /DEBUG /DEBUGTYPE:CV
LinkFlags3  = /MAP /SUBSYSTEM:WINDOWS,4.0 /NOLOGO /DEBUG:NONE
CCMD        = @cl -c -Zp4 -Zi -nologo -W2 -DWIN32 -Zl -I$(%HW_Root)\src\win32 -I$(%HW_Root)\src\game -I$(%HW_Root)\src\Ships -I$(%HW_Root)\src\Sigs -I$(%HW_Root)\src\rgl -I$(%HW_Root)\src\Titan\inc -I$(%HW_Root)\src\JPG $(SWDefine) $(Processor) -D$(%HW_Level) -D$(%USERNAME)
CCMDI       = @icl -c -Zp4 -Zi -nologo -W2 -DWIN32 -Zl -I$(%HW_Root)\src\win32 -I$(%HW_Root)\src\game -I$(%HW_Root)\src\Ships -I$(%HW_Root)\src\Sigs -I$(%HW_Root)\src\rgl -I$(%HW_Root)\src\Titan\inc -I$(%HW_Root)\src\JPG $(SWDefine) $(Processor) -D$(%HW_Level) -D$(%USERNAME) -Qwr137

# preprocessor used for KAS files
CCPRE       = @cl /E -nologo

Libs        = &
                comdlg32.lib user32.lib gdi32.lib advapi32.lib winmm.lib &
                kernel32.lib libcp.lib vfw32.lib dsound.lib libcmt.lib &
                opengl32.lib glu32.lib glaux.lib winmm.lib &
                $(TitanLibDir)\common_w95.lib $(TitanLibDir)\messages_w95.lib &
                $(TitanLibDir)\SocketEngine_ws11_w95.lib &
                wsock32.lib

RC          = rc /i$(%HW_Root)\src\Game -r
RES         = $(Name).res
KAS2C       = KAS2C

#
# Set suffix order
#
.EXTENSIONS:
.EXTENSIONS: . .exe .lib .obj .pch .c .kas .res .rc .sym .map .lnk .cpp

#
# C compiler and Assembler options (Pentium in our case)
#
Processor   = -G5

#
# Set debugging/optimization options
# There are now 3 different build:
# HW_Debug - uses VC compiler, full debug, no optimizations
# HW_Interim - uses Intel compiler, full debug, no optimizations
# HW_anythingelse - uses Intel Compiler, no debug info, optimizations
# which CCMD<n> to use is determined in the dependency creation script
#
CCMD0       = $(CCMD) -Od
CCMD1       = $(CCMDI) -Od
CCMD2       = $(CCMDI) -Ot
CCMD0CPP    = $(CCMD) -Od /GX /GR
CCMD1CPP    = $(CCMDI) -Od
CCMD2CPP    = $(CCMDI) -Ot
!ifeq  %HW_Level HW_Debug
#debug build - VC compiler, debug info

!ifeq %HW_PDB HW_YESPDB
LinkFlags   = $(LinkFlags2)
!else
LinkFlags   = $(LinkFlags0)
!endif
DestDir     = $(%HW_Root)\obj\glDebug

!else # %HW_Level != HW_Debug
!ifeq  %HW_Level HW_Interim
#interim build - Intel compiler, debug info

!ifeq %HW_PDB HW_YESPDB
LinkFlags   = $(LinkFlags2)
!else
LinkFlags   = $(LinkFlags0)
!endif
DestDir     = $(%HW_Root)\obj\glInterim

!else # %HW_Level != HW_Interim
#Release build - Intel compiler, no debug info, optimizations for most files.

!ifeq %HW_PDB HW_YESPDB
LinkFlags   = $(LinkFlags3)
!else
LinkFlags   = $(LinkFlags1)
!endif
DestDir     = $(%HW_Root)\obj\glRelease

!endif
!endif

#
# List of object files
#
Objs =
!include $(Name).mif

StatLibs =


#
# Implicit make rules
#
#.kas:   $(KASDirs)
#.kas:   $(Data)\Missions\SinglePlayer\Mission01
#.kas.c:
#        @$(KAS2C) $? $(SrcDir6)\$^&.c $(SrcDir6)\$^&.h
#.kas.obj:
#        @$(KAS2C) $? $(SrcDir6)\$^&.c $(SrcDir6)\$^&.h
#        @$(CCMD) -Fo$(DestDir)\$^&.obj $?
.obj:   $(DestDir)
.c:     $(SrcDir);$(SrcDir2);$(SrcDir3);$(SrcDir4);$(SrcDir5);$(SrcDir6);$(SrcDir7)
.cpp:   $(SrcDir);$(SrcDir2);$(SrcDir3);$(SrcDir4);$(SrcDir5);$(SrcDir6);$(SrcDir7)
.c.obj:
        @$(CCMD) -Fo$(DestDir)\$^&.obj $?
.cpp.obj:
        @$(CCMD) /GX /GR -Fo$(DestDir)\$^&.obj $?

.res:   $(DestDir)
.rc:    $(SrcDir)
.rc.res:
        @echo $?
        @$(RC) -fo$(DestDir)\$^&.res $?

#
# Convert .map files to .sym files...only useful if using kernel debugger
#
#.map:   $(DestDir)
#.sym:   $(DestDir)
#.map.sym:
#       mapsym32 -o $*.sym $?

#
# Explicit make rules
#

$(DestDir)\$(Target).exe: $(OBJS) $(DestDir)\$(RES) $(StatLibs) $(DestDir)\$(Link_file)
#    @type $(DestDir)\$(Link_file)
    @echo Linking...
    @$(Link) $(LinkFlags) @$(DestDir)\$(Link_file)
    @copy $(DestDir)\$(Target).exe $(%HW_Root)\exe\$(Target).exe > NUL:
    @echo $(DestDir)\$(Target).exe built successfully.

$(DestDir)\$(RES):         $(Name).rc resource.h

hw: .SYMBOLIC
    @echo Making OpenGL Homeworld ...
    @wmake /h /z /f hw.mak

#
# Explicit make rule for link file
#   Note: we don't include the .DEF or .RES because there are none yet
#
$(DestDir)\$(Link_file): hw.mak $(Name).mif
        @echo Reconstructing link file
        @del $(DestDir)\$(Link_file) > NUL:
        @echo $(LinkFlags) > $(DestDir)\$(Link_file)
        @for %i in ($(Objs)) do @echo $(DestDir)\%i >> $(DestDir)\$(Link_file)
        @for %i in ($(StatLibs)) do @echo Library %i >> $(DestDir)\$(Link_file)
        @for %i in ($(Libs)) do @echo %i >> $(DestDir)\$(Link_file)
        @echo $(DestDir)\$(RES) >> $(DestDir)\$(Link_file)
        @echo -Out:$(DestDir)\$(Target).exe >> $(DestDir)\$(Link_file)

copy:   .SYMBOLIC
        @copy $(DestDir)\$(Target).exe $(%HW_Root)\exe\$(Target).exe > NUL:

!include $(Name).depend

