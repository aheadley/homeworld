@echo off

rem --------------------------------------
rem              HW Get
rem ======================================
rem run file to get usage.
rem --------------------------------------

IF "%1" == ""		goto :help
IF "%1" == "help"	goto :help

rem ---- validate parameters ----
IF "%1" == "data"	goto :check2
IF "%1" == "sound"	goto :check2
IF "%1" == "src"	goto :check2
IF "%1" == "game"	goto :check2
IF "%1" == "all"	goto :check2

echo .
echo *** ERROR: Invalid parameter "%1"
echo .
echo Type "%0 help" for syntax.
echo .
goto :done

:check2
IF "%2" == ""		goto :gotparm
IF "%2" == "make"	goto :gotparm
IF "%2" == "depend"	goto :gotparm
IF "%2" == "clean"	goto :gotparm
IF "%2" == "rgl"	goto :gotparm

echo .
echo *** ERROR: Invalid parameter "%2"
echo .
echo Type "%0 help" for syntax.
echo .
goto :done

:gotparm
rem ---- Check for HW_Root ----
IF DEFINED HW_Root goto :start
echo .
echo *** ERROR: HW_Root is not defined.
echo .
echo Please set HW_Root to your Homeworld root directory (e.g.: C:\Homeworld)
echo .
goto :done

:help
echo ---------------------------------------------------------------
echo                         %0
echo ---------------------------------------------------------------
echo Syntax:
echo   %0 CHOICE (make/depend/clean/rgl)
echo .
echo Where CHOICE is one of:
echo   data     gets data, performs a bigget
echo   sound    gets data/sound/src/tools, performs a getsound
echo   src      gets tools/src
echo   game     gets data/src/tools
echo   all      gets data/dll/exe/lib/sound/src/tools, performs a
echo                 getsound, getmovies and a bigget
echo .
echo Optionally, choose one of the following:
echo   make     build after gets
echo   depend   make depend/make after gets
echo   clean    do a clean build after gets
echo   rgl      get the latest RGL executable off of the network
echo .
echo ------------*** All options are CASE SENSITIVE ***-------------
pause
echo .
echo Output from SourceSafe retrieval is spooled into the file
echo "%HW_Root%\ssout.txt"
echo .
echo Dependancies:
echo   This batch file wil work under the NT and 4NT command prompt,
echo   but it requires SourceSafe to be in your path and HW_Root set
echo   to your Homeworld root directory.
echo .
echo   Also, "%0" must be run from the same partition that your
echo   Homeworld directory is on and it assumes your SourceSafe
echo   working folders are set correctly. (If you used the Visual
echo   SourceSafe tool before, it is already set)
echo ---------------------------------------------------------------
echo *** NOTE: the "clean" option will only work under 4NT since 
echo ***       the "wmake clean" option calls "del /y" which is
echo ***       only used under 4NT
echo ---------------------------------------------------------------
goto :done

:start
rem ------------ The gets ----------------
set SSOPT=-R -I-
cd %HW_Root%
IF EXIST "%HW_Root%\ssout.txt" del %HW_Root%\ssout.txt
echo . >%HW_Root%\ssout.txt


rem ---- data -----
IF "%1" == "sound"	goto :getdata
IF "%1" == "all"	goto :getdata
IF "%1" == "data"	goto :getdata
IF "%1" == "game"	goto :getdata
goto :startdll

:getdata
cd %HW_Root%\data
echo *** Getting data
echo *** Getting data >>%HW_Root%\ssout.txt
ss Get $/data %SSOPT% >>%HW_Root%\ssout.txt

:startdll
rem ---- dll ----
IF "%1" == "all" 	goto :getdll
goto :startexe

:getdll
cd %HW_Root%\dll
echo *** Getting dll
echo *** Getting dll >>%HW_Root%\ssout.txt
ss Get $/dll %SSOPT% >>%HW_Root%\ssout.txt

:startexe
rem ---- exe ----
IF "%1" == "all" 	goto :getexe
goto :startlib

:getexe
cd %HW_Root%\exe
echo *** Getting EXE
echo *** Getting EXE >>%HW_Root%\ssout.txt
ss Get $/exe %SSOPT% >>%HW_Root%\ssout.txt

:startlib
rem ---- lib ----
IF "%1" == "all" 	goto :getlib
goto :startsnd

:getlib
cd %HW_Root%\lib
echo *** Getting LIB
echo *** Getting LIB >>%HW_Root%\ssout.txt
ss Get $/Lib %SSOPT% >>%HW_Root%\ssout.txt

:startsnd
rem ---- sound ----
IF "%1" == "all" 	goto :getsnd
goto :startsrc

:getsnd
cd %HW_Root%\sound
echo *** Getting sound
echo *** Getting sound >>%HW_Root%\ssout.txt
ss Get $/sound %SSOPT% >>%HW_Root%\ssout.txt

:startsrc
rem ---- src ----
IF "%1" == "all" 	goto :getsrc
IF "%1" == "src" 	goto :getsrc
IF "%1" == "game" 	goto :getsrc
IF "%1" == "sound" 	goto :getsrc
goto :starttl

:getsrc
cd %HW_Root%\src
echo *** Getting src
echo *** Getting src >>%HW_Root%\ssout.txt
ss Get $/src %SSOPT% >>%HW_Root%\ssout.txt

:starttl
rem ---- tools ----
IF "%1" == "all" 	goto :gettl
IF "%1" == "src" 	goto :gettl
IF "%1" == "game" 	goto :gettl
IF "%1" == "sound" 	goto :gettl
goto :starts2

:gettl
cd %HW_Root%\tools
echo *** Getting TOOLS
echo *** Getting TOOLS >>%HW_Root%\ssout.txt
ss Get $/tools %SSOPT% >>%HW_Root%\ssout.txt

:starts2
cd %HW_Root%\src\win32

rem *** Get sound from network ***
IF "%1" == "all" 	call getsound
IF "%1" == "sound" 	call getsound

rem *** Get Bigfile ***

IF "%1" == "all" 	call bigget
IF "%1" == "data" 	call bigget

rem *** Get Movies ***
IF "%1" == "all" 	call getmovies

:startmk
rem ******************
rem *   Do the make  *
rem ******************
IF "%2" == "clean"	goto :makecln
IF "%2" == "depend"	goto :makedep
IF "%2" == "make"	goto :makereg
IF "%2" == "rgl"	call getrgl
goto :done

:makecln
wmake clean
:makedep
wmake depend
:makereg
wmake

:done




