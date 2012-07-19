guilibs = libc.lib oldnames.lib kernel32.lib ws2_32.lib mswsock.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib ddraw.lib
!ifeq %HW_Level HW_Release
debugopts =
!else
debugopts = /Z7
!endif
mycflag = /c /nologo /G6 /Ot /Oi $(debugopts) /W2 /D__MSC__=yes /Foobj\
copts = /Ox
ccc = @cl /D__MSC__=yes /I.\3dfx /I.\glide3\include /I.\D3D /I..\titan\inc
icc = @icl /I.\3dfx /I.\glide3\include /I.\D3D /I..\titan\inc
iopts = /O2 /Qmem $(debugopts) /c /nologo /D__MSC__=yes
#/Qrcd /Qpc32 /Ob2
DX6LIBS = ..\..\lib\d3d\
cc = $(icc)
opts = $(iopts)
ccopts = $(copts)

all: rgl.dll rglsw.dll rgld3d.dll

obj\fxclose.obj: fxclose.c
    $(cc) $(cdebug) $(cflags) $(mycflag) $(cvars) fxclose.c

obj\draw.obj: draw.cpp kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(cvars) draw.cpp

obj\maths.obj: maths.c kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) maths.c

obj\kgl.obj: kgl.c kgl.h
    $(ccc) $(cdebug) $(cflags) $(mycflag) $(cvars) kgl.c

obj\wgl.obj: wgl.c kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(cvars) wgl.c

obj\kvb.obj: kvb.c kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) kvb.c

obj\clip.obj: clip.c kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) clip.c

obj\invert.obj: invert.c kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) invert.c

obj\asm.obj: asm.c asm.h kgl.h
    $(icc) $(cdebug) $(cflags) $(mycflag) $(iopts) $(cvars) asm.c

obj\span.obj: span.c span.h stipple.c stipple.h kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) span.c

obj\scan.obj: scan.c
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) scan.c

obj\kglu.obj: kglu.c kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) kglu.c

obj\swdriver.obj: swdriver.c kgl.h swdrv.h newtriangle.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) swdriver.c

obj\fxdriver.obj: fxdriver.c kgl.h fxdrv.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) fxdriver.c

obj\fxtmg.obj: fxtmg.c kgl.h fxdrv.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(cvars) fxtmg.c

obj\d3driver.obj: d3driver.cpp d3drv.h d3dlist.h d3dtex.h d3denum.h d3dblt.h d3dinit.h kgl.h
    $(ccc) -GX $(cdebug) $(cflags) $(mycflag) $(cvars) d3driver.cpp

obj\d3dtex.obj: d3dtex.cpp d3drv.h d3dtex.h d3dblt.h kgl.h
    $(ccc) -GX $(cdebug) $(cflags) $(mycflag) $(ccopts) $(cvars) d3dtex.cpp

obj\d3denum.obj: d3denum.cpp d3drv.h d3dlist.h d3dinit.h kgl.h
    $(ccc) -GX $(cdebug) $(cflags) $(mycflag) $(ccopts) $(cvars) d3denum.cpp

obj\d3dblt.obj: d3dblt.cpp d3drv.h d3dblt.h kgl.h
    $(ccc) -GX $(cdebug) $(cflags) $(mycflag) $(ccopts) $(cvars) d3dblt.cpp

obj\d3dinit.obj: d3dinit.cpp d3drv.h d3denum.h d3dinit.h kgl.h
    $(ccc) -GX $(cdebug) $(cflags) $(mycflag) $(ccopts) $(cvars) d3dinit.cpp

obj\fx3driver.obj: fx3driver.c kgl.h fx3drv.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(cvars) fx3driver.c

obj\fx3tmg.obj: fx3tmg.c kgl.h fx3drv.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(cvars) fx3tmg.c

obj\hash.obj: hash.c hash.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) hash.c

obj\rglext.obj: rglext.c rglext.h kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) rglext.c

obj\newtriangle.obj: newtriangle.c newtriangle.h newtritemp.h kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) newtriangle.c

obj\newtextri.obj: newtextri.c newtriangle.h newtritemp.h kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) newtextri.c

obj\newrgbatextri.obj: newrgbatextri.c newtriangle.h newtritemp.h kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) newrgbatextri.c

obj\newblendtextri.obj: newblendtextri.c newtriangle.h newtritemp.h kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) newblendtextri.c

obj\newpersptri.obj: newpersptri.c newtriangle.h newtritemp.h kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) newpersptri.c

obj\newrgbapersptri.obj: newrgbapersptri.c newtriangle.h newtritemp.h kgl.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) newrgbapersptri.c

obj\lines.obj: lines.c linetemp.h kgl.h lines.h
    $(cc) $(cdebug) $(cflags) $(mycflag) $(opts) $(cvars) lines.c

fxclose.exe: obj\fxclose.obj
    $(cc) $(cdebug) $(cflags) obj\fxclose.obj 3dfx\glide2x.lib

rgl.dll: obj\maths.obj obj\kgl.obj obj\kvb.obj obj\clip.obj obj\asm.obj &
         obj\invert.obj draw.def kgl.h obj\hash.obj obj\rglext.obj obj\wgl.obj
    @link -debug:full -debugtype:cv /NODEFAULTLIB /INCREMENTAL:NO /PDB:NONE &
        /RELEASE /NOLOGO -subsystem:windows,4.0 /dll -out:rgl.dll /DEF:draw.def &
            obj\maths.obj obj\kgl.obj obj\kvb.obj obj\clip.obj obj\invert.obj $(guilibs) ddraw.lib obj\hash.obj obj\rglext.obj obj\wgl.obj obj\asm.obj

glu32.dll: obj\kglu.obj obj\maths.obj obj\invert.obj
    @link -debug:full -debugtype:cv /NODEFAULTLIB /INCREMENTAL:NO /PDB:NONE /RELEASE /NOLOGO &
    -subsystem:windows,4.0 /dll -out:glu32.dll obj\kglu.obj obj\maths.obj obj\invert.obj $(guilibs) &
    ddraw.lib rgl.lib

rglsw.dll: obj\draw.obj obj\asm.obj obj\newtriangle.obj obj\newtextri.obj obj\newrgbatextri.obj obj\newblendtextri.obj obj\newpersptri.obj obj\newrgbapersptri.obj obj\lines.obj newtritemp.h obj\scan.obj obj\span.obj obj\swdriver.obj stipple.c
    @link -debug:full -debugtype:cv /NODEFAULTLIB /INCREMENTAL:NO /PDB:NONE /RELEASE /NOLOGO &
        -subsystem:windows,4.0 /dll -out:rglsw.dll obj\draw.obj obj\swdriver.obj obj\asm.obj obj\scan.obj obj\span.obj obj\newtriangle.obj &
        obj\newtextri.obj obj\newrgbatextri.obj obj\newblendtextri.obj obj\newpersptri.obj obj\newrgbapersptri.obj obj\lines.obj $(guilibs) rgl.lib

rglfx.dll: obj\fxdriver.obj obj\fxtmg.obj
    @link -debug:full -debugtype:cv /NODEFAULTLIB /INCREMENTAL:NO /PDB:NONE /RELEASE /NOLOGO &
    -subsystem:windows,4.0 /dll -out:rglfx.dll obj\fxdriver.obj obj\fxtmg.obj $(guilibs) rgl.lib 3dfx\glide2x.lib

rgld3d.dll: obj\d3driver.obj obj\d3dblt.obj obj\d3dtex.obj obj\d3denum.obj obj\d3dinit.obj
    @link -debug:full -debugtype:cv /NODEFAULTLIB /INCREMENTAL:NO /PDB:NONE /RELEASE /NOLOGO &
        -subsystem:windows,4.0 /dll -out:rgld3d.dll obj\d3driver.obj obj\d3dblt.obj obj\d3dtex.obj obj\d3denum.obj obj\d3dinit.obj $(guilibs) $(DX6LIBS)ddraw.lib $(DX6LIBS)dxguid.lib $(DX6LIBS)d3dim.lib rgl.lib

rglfx3.dll: obj\fx3driver.obj obj\fx3tmg.obj
    @link -debug:full -debugtype:cv /NODEFAULTLIB /INCREMENTAL:NO /PDB:NONE /RELEASE /NOLOGO &
        -subsystem:windows,4.0 /dll -out:rglfx3.dll obj\fx3driver.obj obj\fx3tmg.obj $(guilibs) rgl.lib glide3\lib\glide3x.lib

# Rules for cleaning out those old files
clean: .SYMBOLIC
    @del *.bak *.pdb *.obj *.res *.exp *.map *.sbr *.bsc *.dll *.exe *.lib > NUL:
    @del obj\*.obj > NUL:

