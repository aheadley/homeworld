cd\homeworld

rem check optional third parameter
if "%3" EQ "0" (goto just0)
if "%3" EQ "1" (goto just1)
if "%3" EQ "2" (goto just2)
if "%3" EQ "3" (goto just3)
if "%3" EQ "4" (goto just4)

rem no third parameter, build all LODs
@echo off

@if NOT EXIST datasrc\%1\%2\rl0\Lod0\%2_0.mif goto regularLod0
@if NOT EXIST datasrc\%1\%2\rl0\Lod0\%2_1.mif goto regularLod0
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod0\%2_0.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod0\%2_1.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod0\%2_2.mif
@goto skipLod0

:regularLod0
@if exist datasrc\%1\%2\rl0\Lod0\dependtex.mif del datasrc\%1\%2\rl0\Lod0\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\Lod0\dependtex.mif
@for %filename in (datasrc\%1\%2\rl0\Lod0\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\Lod0\dependtex.mif

@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod0\dependtex.mif

:skipLod0
@lwexport -+datasrc\%1\%2\rl0\Lod0\%2.lws -=data\%1\%2\rl0\Lod0\%2.geo
if "%1" EQ "Derelicts" goto skipmad0
@lwexport -+datasrc\%1\%2\rl0\Lod0\%2.lws -=data\%1\%2.mad
@if %@filesize[data\%1\%2.mad] EQ 0 del data\%1\%2.mad > NUL
:skipmad0

@del data\%1\%2\rl0\lod0\page*.lif > NUL
@paco -s %1 %2 0
@del /z data\%1\%2\rl0\lod0\%2.geo > NUL
@if exist datasrc\%1\%2\rl0\lod0\dependtex.mif del datasrc\%1\%2\rl0\lod0\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\lod0\dependtex.mif
@for %filename in (data\%1\%2\rl0\lod0\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod0\dependtex.mif
@liflist a data\textures.ll datasrc\%1\%2\rl0\lod0\dependtex.mif
@liflist s data\textures.ll datasrc\%1\%2\rl0\lod0\sharedtex.mif
@for %f in (@datasrc\%1\%2\rl0\lod0\sharedtex.mif) del /z data\%1\%2\rl0\lod0\%f > NUL
@del datasrc\%1\%2\rl0\lod0\sharedtex.mif > NUL
@del datasrc\%1\%2\rl0\lod0\dependtex.mif > NUL

@echo .
@echo ^^^^^^^^^ THIS IS Lod0
@echo.


@if NOT EXIST datasrc\%1\%2\rl0\Lod1\%2_0.mif goto regularLod1
@if NOT EXIST datasrc\%1\%2\rl0\Lod1\%2_1.mif goto regularLod1
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod1\%2_0.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod1\%2_1.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod1\%2_2.mif
@goto skipLod1

:regularLod1
@if exist datasrc\%1\%2\rl0\Lod1\dependtex.mif del datasrc\%1\%2\rl0\Lod1\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\Lod1\dependtex.mif
@for %filename in (datasrc\%1\%2\rl0\Lod1\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\Lod1\dependtex.mif

@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod1\dependtex.mif

:skipLod1
@lwexport -+datasrc\%1\%2\rl0\Lod1\%2.lws -=data\%1\%2\rl0\Lod1\%2.geo
if "%1" EQ "Derelicts" goto skipmad1
@lwexport -+datasrc\%1\%2\rl0\Lod1\%2.lws -=data\%1\%2.mad
@if %@filesize[data\%1\%2.mad] EQ 0 del data\%1\%2.mad > NUL
:skipmad1

@del data\%1\%2\rl0\lod1\page*.lif > NUL
@paco -s %1 %2 1
@del /z data\%1\%2\rl0\lod1\%2.geo > NUL
@if exist datasrc\%1\%2\rl0\lod1\dependtex.mif del datasrc\%1\%2\rl0\lod1\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\lod1\dependtex.mif
@for %filename in (data\%1\%2\rl0\lod1\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod1\dependtex.mif
@liflist a data\textures.ll datasrc\%1\%2\rl0\lod1\dependtex.mif
@liflist s data\textures.ll datasrc\%1\%2\rl0\lod1\sharedtex.mif
@for %f in (@datasrc\%1\%2\rl0\lod1\sharedtex.mif) del /z data\%1\%2\rl0\lod1\%f > NUL
@del datasrc\%1\%2\rl0\lod1\sharedtex.mif > NUL
@del datasrc\%1\%2\rl0\lod1\dependtex.mif > NUL

@echo .
@echo ^^^^^^^^^ THIS IS Lod1
@echo.


@if NOT EXIST datasrc\%1\%2\rl0\Lod2\%2_0.mif goto regularLod2
@if NOT EXIST datasrc\%1\%2\rl0\Lod2\%2_1.mif goto regularLod2
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod2\%2_0.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod2\%2_1.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod2\%2_2.mif
@goto skipLod2

:regularLod2
@if exist datasrc\%1\%2\rl0\Lod2\dependtex.mif del datasrc\%1\%2\rl0\Lod2\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\Lod2\dependtex.mif
@for %filename in (datasrc\%1\%2\rl0\Lod2\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\Lod2\dependtex.mif

@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod2\dependtex.mif

:skipLod2
@lwexport -+datasrc\%1\%2\rl0\Lod2\%2.lws -=data\%1\%2\rl0\Lod2\%2.geo
if "%1" EQ "Derelicts" goto skipmad2
@lwexport -+datasrc\%1\%2\rl0\Lod2\%2.lws -=data\%1\%2.mad
@if %@filesize[data\%1\%2.mad] EQ 0 del data\%1\%2.mad > NUL
:skipmad2

@del data\%1\%2\rl0\lod2\page*.lif > NUL
@paco -s %1 %2 2
@del /z data\%1\%2\rl0\lod2\%2.geo > NUL
@if exist datasrc\%1\%2\rl0\lod2\dependtex.mif del datasrc\%1\%2\rl0\lod2\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\lod2\dependtex.mif
@for %filename in (data\%1\%2\rl0\lod2\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod2\dependtex.mif
@liflist a data\textures.ll datasrc\%1\%2\rl0\lod2\dependtex.mif
@liflist s data\textures.ll datasrc\%1\%2\rl0\lod2\sharedtex.mif
@for %f in (@datasrc\%1\%2\rl0\lod2\sharedtex.mif) del /z data\%1\%2\rl0\lod2\%f > NUL
@del datasrc\%1\%2\rl0\lod2\sharedtex.mif > NUL
@del datasrc\%1\%2\rl0\lod2\dependtex.mif > NUL

@echo .
@echo ^^^^^^^^^ THIS IS Lod2
@echo.


@if NOT EXIST datasrc\%1\%2\rl0\Lod3\%2_0.mif goto regularLod3
@if NOT EXIST datasrc\%1\%2\rl0\Lod3\%2_1.mif goto regularLod3
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod3\%2_0.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod3\%2_1.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod3\%2_2.mif
@goto skipLod3

:regularLod3
@if exist datasrc\%1\%2\rl0\Lod3\dependtex.mif del datasrc\%1\%2\rl0\Lod3\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\Lod3\dependtex.mif
@for %filename in (datasrc\%1\%2\rl0\Lod3\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\Lod3\dependtex.mif

@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod3\dependtex.mif

:skipLod3
@lwexport -+datasrc\%1\%2\rl0\Lod3\%2.lws -=data\%1\%2\rl0\Lod3\%2.geo
if "%1" EQ "Derelicts" goto skipmad3
@lwexport -+datasrc\%1\%2\rl0\Lod3\%2.lws -=data\%1\%2.mad
@if %@filesize[data\%1\%2.mad] EQ 0 del data\%1\%2.mad > NUL
:skipmad3

@del data\%1\%2\rl0\lod3\page*.lif > NUL
@paco -s %1 %2 3
@del /z data\%1\%2\rl0\lod3\%2.geo > NUL
@if exist datasrc\%1\%2\rl0\lod3\dependtex.mif del datasrc\%1\%2\rl0\lod3\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\lod3\dependtex.mif
@for %filename in (data\%1\%2\rl0\lod3\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod3\dependtex.mif
@liflist a data\textures.ll datasrc\%1\%2\rl0\lod3\dependtex.mif
@liflist s data\textures.ll datasrc\%1\%2\rl0\lod3\sharedtex.mif
@for %f in (@datasrc\%1\%2\rl0\lod3\sharedtex.mif) del /z data\%1\%2\rl0\lod3\%f > NUL
@del datasrc\%1\%2\rl0\lod3\sharedtex.mif > NUL
@del datasrc\%1\%2\rl0\lod3\dependtex.mif > NUL

@echo .
@echo ^^^^^^^^^ THIS IS Lod3
@echo.


@if NOT EXIST datasrc\%1\%2\rl0\Lod4\%2_0.mif goto regularLod4
@if NOT EXIST datasrc\%1\%2\rl0\Lod4\%2_1.mif goto regularLod4
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod4\%2_0.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod4\%2_1.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod4\%2_2.mif
@goto skipLod4

:regularLod4
@if exist datasrc\%1\%2\rl0\Lod4\dependtex.mif del datasrc\%1\%2\rl0\Lod4\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\Lod4\dependtex.mif
@for %filename in (datasrc\%1\%2\rl0\Lod4\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\Lod4\dependtex.mif

@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod4\dependtex.mif

:skipLod4
@lwexport -+datasrc\%1\%2\rl0\Lod4\%2.lws -=data\%1\%2\rl0\Lod4\%2.geo
if "%1" EQ "Derelicts" goto skipmad4
@lwexport -+datasrc\%1\%2\rl0\Lod4\%2.lws -=data\%1\%2.mad
@if %@filesize[data\%1\%2.mad] EQ 0 del data\%1\%2.mad > NUL
:skipmad4

@del data\%1\%2\rl0\lod4\page*.lif > NUL
@paco -s %1 %2 4
@del /z data\%1\%2\rl0\lod4\%2.geo > NUL
@if exist datasrc\%1\%2\rl0\lod4\dependtex.mif del datasrc\%1\%2\rl0\lod4\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\lod4\dependtex.mif
@for %filename in (data\%1\%2\rl0\lod4\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod4\dependtex.mif
@liflist a data\textures.ll datasrc\%1\%2\rl0\lod4\dependtex.mif
@liflist s data\textures.ll datasrc\%1\%2\rl0\lod4\sharedtex.mif
@for %f in (@datasrc\%1\%2\rl0\lod4\sharedtex.mif) del /z data\%1\%2\rl0\lod4\%f > NUL
@del datasrc\%1\%2\rl0\lod4\sharedtex.mif > NUL
@del datasrc\%1\%2\rl0\lod4\dependtex.mif > NUL

@echo .
@echo ^^^^^^^^^ THIS IS Lod4
@echo.


@goto end



@rem **************************************
@rem conditionaly build just 1 of the LODs
@rem **************************************
:just0

@if NOT EXIST datasrc\%1\%2\rl0\Lod0\%2_0.mif goto regularLod0
@if NOT EXIST datasrc\%1\%2\rl0\Lod0\%2_1.mif goto regularLod0
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod0\%2_0.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod0\%2_1.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod0\%2_2.mif
@goto skipLod0

:regularLod0
@if exist datasrc\%1\%2\rl0\Lod0\dependtex.mif del datasrc\%1\%2\rl0\Lod0\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\Lod0\dependtex.mif
@for %filename in (datasrc\%1\%2\rl0\Lod0\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\Lod0\dependtex.mif

@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod0\dependtex.mif
@del datasrc\%1\%2\rl0\Lod0\dependtex.mif > NUL

:skipLod0
@lwexport -+datasrc\%1\%2\rl0\Lod0\%2.lws -=data\%1\%2\rl0\Lod0\%2.geo
if "%1" EQ "Derelicts" goto skipmad0b
@lwexport -+datasrc\%1\%2\rl0\Lod0\%2.lws -=data\%1\%2.mad
@if %@filesize[data\%1\%2.mad] EQ 0 del data\%1\%2.mad > NUL
:skipmad0b

@del data\%1\%2\rl0\lod0\page*.lif > NUL
@paco -s %1 %2 0
@del /z data\%1\%2\rl0\lod0\%2.geo > NUL
@if exist datasrc\%1\%2\rl0\lod0\dependtex.mif del datasrc\%1\%2\rl0\lod0\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\lod0\dependtex.mif
@for %filename in (data\%1\%2\rl0\lod0\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod0\dependtex.mif
@liflist a data\textures.ll datasrc\%1\%2\rl0\lod0\dependtex.mif
@liflist s data\textures.ll datasrc\%1\%2\rl0\lod0\sharedtex.mif
@for %f in (@datasrc\%1\%2\rl0\lod0\sharedtex.mif) del /z data\%1\%2\rl0\lod0\%f > NUL
del datasrc\%1\%2\rl0\lod0\sharedtex.mif > NUL
del datasrc\%1\%2\rl0\lod0\dependtex.mif > NUL

@echo .
@echo ^^^^^^^^^ THIS IS Lod0
@echo.

goto end

:just1

@if NOT EXIST datasrc\%1\%2\rl0\Lod1\%2_0.mif goto regularLod1
@if NOT EXIST datasrc\%1\%2\rl0\Lod1\%2_1.mif goto regularLod1
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod1\%2_0.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod1\%2_1.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod1\%2_2.mif
@goto skipLod1

:regularLod1
@if exist datasrc\%1\%2\rl0\Lod1\dependtex.mif del datasrc\%1\%2\rl0\Lod1\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\Lod1\dependtex.mif
@for %filename in (datasrc\%1\%2\rl0\Lod1\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\Lod1\dependtex.mif

@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod1\dependtex.mif
@del datasrc\%1\%2\rl0\Lod1\dependtex.mif > NUL

:skipLod1
@lwexport -+datasrc\%1\%2\rl0\Lod1\%2.lws -=data\%1\%2\rl0\Lod1\%2.geo
if "%1" EQ "Derelicts" goto skipmad1b
@lwexport -+datasrc\%1\%2\rl0\Lod1\%2.lws -=data\%1\%2.mad
@if %@filesize[data\%1\%2.mad] EQ 0 del data\%1\%2.mad > NUL
:skipmad1b

@del data\%1\%2\rl0\lod1\page*.lif > NUL
@paco -s %1 %2 1
@del /z data\%1\%2\rl0\lod1\%2.geo > NUL
@if exist datasrc\%1\%2\rl0\lod1\dependtex.mif del datasrc\%1\%2\rl0\lod1\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\lod1\dependtex.mif
@for %filename in (data\%1\%2\rl0\lod1\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod1\dependtex.mif
@liflist a data\textures.ll datasrc\%1\%2\rl0\lod1\dependtex.mif
@liflist s data\textures.ll datasrc\%1\%2\rl0\lod1\sharedtex.mif
@for %f in (@datasrc\%1\%2\rl0\lod1\sharedtex.mif) del /z data\%1\%2\rl0\lod1\%f > NUL
del datasrc\%1\%2\rl0\lod1\sharedtex.mif > NUL
del datasrc\%1\%2\rl0\lod1\dependtex.mif > NUL

@echo .
@echo ^^^^^^^^^ THIS IS Lod1
@echo.

goto end

:just2

@if NOT EXIST datasrc\%1\%2\rl0\Lod2\%2_0.mif goto regularLod2
@if NOT EXIST datasrc\%1\%2\rl0\Lod2\%2_1.mif goto regularLod2
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod2\%2_0.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod2\%2_1.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod2\%2_2.mif
@goto skipLod2

:regularLod2
@if exist datasrc\%1\%2\rl0\Lod2\dependtex.mif del datasrc\%1\%2\rl0\Lod2\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\Lod2\dependtex.mif
@for %filename in (datasrc\%1\%2\rl0\Lod2\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\Lod2\dependtex.mif

@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod2\dependtex.mif
@del datasrc\%1\%2\rl0\Lod2\dependtex.mif > NUL

:skipLod2
@lwexport -+datasrc\%1\%2\rl0\Lod2\%2.lws -=data\%1\%2\rl0\Lod2\%2.geo
if "%1" EQ "Derelicts" goto skipmad2b
@lwexport -+datasrc\%1\%2\rl0\Lod2\%2.lws -=data\%1\%2.mad
@if %@filesize[data\%1\%2.mad] EQ 0 del data\%1\%2.mad > NUL
:skipmad2b

@del data\%1\%2\rl0\lod2\page*.lif > NUL
@paco -s %1 %2 2
@del /z data\%1\%2\rl0\lod2\%2.geo > NUL
@if exist datasrc\%1\%2\rl0\lod2\dependtex.mif del datasrc\%1\%2\rl0\lod2\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\lod2\dependtex.mif
@for %filename in (data\%1\%2\rl0\lod2\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod2\dependtex.mif
@liflist a data\textures.ll datasrc\%1\%2\rl0\lod2\dependtex.mif
@liflist s data\textures.ll datasrc\%1\%2\rl0\lod2\sharedtex.mif
@for %f in (@datasrc\%1\%2\rl0\lod2\sharedtex.mif) del /z data\%1\%2\rl0\lod2\%f > NUL
del datasrc\%1\%2\rl0\lod2\sharedtex.mif > NUL
del datasrc\%1\%2\rl0\lod2\dependtex.mif > NUL

@echo .
@echo ^^^^^^^^^ THIS IS Lod2
@echo.

goto end

:just3

@if NOT EXIST datasrc\%1\%2\rl0\Lod3\%2_0.mif goto regularLod3
@if NOT EXIST datasrc\%1\%2\rl0\Lod3\%2_1.mif goto regularLod3
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod3\%2_0.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod3\%2_1.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod3\%2_2.mif
@goto skipLod3

:regularLod3
@if exist datasrc\%1\%2\rl0\Lod3\dependtex.mif del datasrc\%1\%2\rl0\Lod3\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\Lod3\dependtex.mif
@for %filename in (datasrc\%1\%2\rl0\Lod3\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\Lod3\dependtex.mif

@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod3\dependtex.mif
@del datasrc\%1\%2\rl0\Lod3\dependtex.mif > NUL

:skipLod3
@lwexport -+datasrc\%1\%2\rl0\Lod3\%2.lws -=data\%1\%2\rl0\Lod3\%2.geo
if "%1" EQ "Derelicts" goto skipmad3b
@lwexport -+datasrc\%1\%2\rl0\Lod3\%2.lws -=data\%1\%2.mad
@if %@filesize[data\%1\%2.mad] EQ 0 del data\%1\%2.mad > NUL
:skipmad3b

@del data\%1\%2\rl0\lod3\page*.lif > NUL
@paco -s %1 %2 3
@del /z data\%1\%2\rl0\lod3\%2.geo > NUL
@if exist datasrc\%1\%2\rl0\lod3\dependtex.mif del datasrc\%1\%2\rl0\lod3\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\lod3\dependtex.mif
@for %filename in (data\%1\%2\rl0\lod3\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod3\dependtex.mif
@liflist a data\textures.ll datasrc\%1\%2\rl0\lod3\dependtex.mif
@liflist s data\textures.ll datasrc\%1\%2\rl0\lod3\sharedtex.mif
@for %f in (@datasrc\%1\%2\rl0\lod3\sharedtex.mif) del /z data\%1\%2\rl0\lod3\%f > NUL
del datasrc\%1\%2\rl0\lod3\sharedtex.mif > NUL
del datasrc\%1\%2\rl0\lod3\dependtex.mif > NUL

@echo .
@echo ^^^^^^^^^ THIS IS Lod3
@echo.

goto end

:just4

@if NOT EXIST datasrc\%1\%2\rl0\Lod4\%2_0.mif goto regularLod4
@if NOT EXIST datasrc\%1\%2\rl0\Lod4\%2_1.mif goto regularLod4
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod4\%2_0.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod4\%2_1.mif
@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod4\%2_2.mif
@goto skipLod4

:regularLod4
@if exist datasrc\%1\%2\rl0\Lod4\dependtex.mif del datasrc\%1\%2\rl0\Lod4\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\Lod4\dependtex.mif
@for %filename in (datasrc\%1\%2\rl0\Lod4\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\Lod4\dependtex.mif

@liflist q data\textures.ll datasrc\%1\%2\rl0\Lod4\dependtex.mif
@del datasrc\%1\%2\rl0\Lod4\dependtex.mif > NUL

:skipLod4
@lwexport -+datasrc\%1\%2\rl0\Lod4\%2.lws -=data\%1\%2\rl0\Lod4\%2.geo
if "%1" EQ "Derelicts" goto skipmad4b
@lwexport -+datasrc\%1\%2\rl0\Lod4\%2.lws -=data\%1\%2.mad
@if %@filesize[data\%1\%2.mad] EQ 0 del data\%1\%2.mad > NUL
:skipmad4b

@del data\%1\%2\rl0\lod4\page*.lif > NUL
@paco -s %1 %2 4
@del /z data\%1\%2\rl0\lod4\%2.geo > NUL
@if exist datasrc\%1\%2\rl0\lod4\dependtex.mif del datasrc\%1\%2\rl0\lod4\dependtex.mif > NUL
@echo # > datasrc\%1\%2\rl0\lod4\dependtex.mif
@for %filename in (data\%1\%2\rl0\lod4\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod4\dependtex.mif
@liflist a data\textures.ll datasrc\%1\%2\rl0\lod4\dependtex.mif
@liflist s data\textures.ll datasrc\%1\%2\rl0\lod4\sharedtex.mif
@for %f in (@datasrc\%1\%2\rl0\lod4\sharedtex.mif) del /z data\%1\%2\rl0\lod4\%f > NUL
del datasrc\%1\%2\rl0\lod4\sharedtex.mif > NUL
del datasrc\%1\%2\rl0\lod4\dependtex.mif > NUL

@echo .
@echo ^^^^^^^^^ THIS IS Lod4
@echo.

goto end

:end
