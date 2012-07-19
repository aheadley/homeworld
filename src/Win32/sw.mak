#
# Homeworld Windows interface layer makefile
#

#
# Targets and directories
#
Name        = Homeworld
Target      = SWHomeworld
Link_file   = $(Name).lnk
SrcDir      = .
SrcDir2     = $(%HW_Root)\src\game
SrcDir3     = $(%HW_Root)\src\Ships
SrcDir4     = $(%HW_Root)\src\Sigs
LibDir      = $(%HW_Root)\Lib
Data        = $(%HW_Root)\Data

SWDefine    = -DSW_Render=1

#
# Default compile and link flags
#
Link        = link
LinkFlags0  = /MAP /SUBSYSTEM:WINDOWS,4.0 /NOLOGO /DEBUG /DEBUGTYPE:BOTH
LinkFlags1  = /MAP /SUBSYSTEM:WINDOWS,4.0 /NOLOGO /DEBUG:NONE
CCMD0       = @cl -c -Zp4 -Zi -nologo -W3 -DWIN32 -Zl -I$(%HW_Root)\src\win32 -I$(%HW_Root)\src\game -I$(%HW_Root)\src\Ships -I$(%HW_Root)\src\Sigs -I$(%HW_Root)\src\rgl $(SWDefine)
CCMD1       = @cl -c -Zp4 -Zi -nologo -W3 -DWIN32 -Zl -I$(%HW_Root)\src\win32 -I$(%HW_Root)\src\game -I$(%HW_Root)\src\Ships -I$(%HW_Root)\src\Sigs -I$(%HW_Root)\src\rgl $(SWDefine)
Libs        = comdlg32.lib user32.lib gdi32.lib advapi32.lib winmm.lib &
                kernel32.lib libc.lib vfw32.lib dsound.lib &
                $(%HW_Root)\src\rgl\rgl.lib

RC          = rc /i$(%HW_Root)\src\Game -r
RES         = $(Name).res

#
# Set suffix order
#
.EXTENSIONS:
.EXTENSIONS: . .exe .lib .obj .pch .c .res .rc .sym .map .lnk

#
# C compiler and Assembler options (Pentium in our case)
#
Processor   = -G5

#
# Set debugging options
#
!ifeq  %HW_Level HW_Debug
CCMD        = $(CCMD0) $(Processor) -Od -D$(%HW_Level) -D$(%USERNAME)
LinkFlags   = $(LinkFlags0)
DestDir     = $(%HW_Root)\obj\Debug
!else # %HW_Level != HW_Debug
CCMD        = $(CCMD1) $(Processor) -Ot -D$(%HW_Level) -D$(%USERNAME)
LinkFlags   = $(LinkFlags0)
DestDir     = $(%HW_Root)\obj\Retail
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
.obj:   $(DestDir)
.c:     $(SrcDir);$(SrcDir2);$(SrcDir3);$(SrcDir4)
.c.obj:
        @$(CCMD) -Fo$(DestDir)\$^&.obj $?

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
#	mapsym32 -o $*.sym $?

#
# Explicit make rules
#

$(DestDir)\$(Target).exe: $(OBJS) $(DestDir)\$(RES) $(StatLibs) $(DestDir)\$(Link_file) rgl.dll
#    @type $(DestDir)\$(Link_file)
    @echo Linking...
    @$(Link) $(LinkFlags) @$(DestDir)\$(Link_file)
    @copy $(DestDir)\$(Target).exe $(%HW_Root)\exe\$(Target).exe > NUL:
    @echo $(DestDir)\$(Target).exe built successfully.

$(DestDir)\$(RES):         $(Name).rc resource.h

#
# Explicit make rule for link file
#   Note: we don't include the .DEF or .RES because there are none yet
#
$(DestDir)\$(Link_file): sw.mak makefile $(Name).mif
        @echo Reconstructing link file
        @del $(DestDir)\$(Link_file) > NUL:
        @echo $(LinkFlags) > $(DestDir)\$(Link_file)
        @for %i in ($(Objs)) do @echo $(DestDir)\%i >> $(DestDir)\$(Link_file)
        @for %i in ($(StatLibs)) do @echo Library %i >> $(DestDir)\$(Link_file)
        @for %i in ($(Libs)) do @echo %i >> $(DestDir)\$(Link_file)
        @echo $(DestDir)\$(RES) >> $(DestDir)\$(Link_file)
        @echo -Out:$(DestDir)\$(Target).exe >> $(DestDir)\$(Link_file)

rgl.dll:
	@echo Making the software DLL
	@cd ..\rgl
	@nmake /f dll.mak
	@copy rgl.dll $(%HW_Root)\exe
	@copy rgl.dll $(DestDir)

clean:  .SYMBOLIC
        @del $(DestDir)\$(Target).exe > NUL:
        @del $(DestDir)\*.obj > NUL:
        @del $(DestDir)\*.res > NUL:
        @del $(DestDir)\*.map > NUL:
        @del $(DestDir)\*.lnk > NUL:
	@cd ..\rgl
	@nmake clean
        @echo $(Name) Files cleaned successfully.  Have to rebuild everything now.

copy:   .SYMBOLIC
        @copy $(DestDir)\$(Target).exe $(%HW_Root)\exe\$(Target).exe > NUL:
