@echo off

if exist "h:\data\SoundFX\lock.txt" goto locked

@echo Copying Homeworld Sound Bank files...
attrib -r %HW_Data%\SoundFX\derelict*.lut
del %HW_Data%\SoundFX\derelict*.lut
copy "h:\data\SoundFX\derelict*.lut"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\derelict*.lut

attrib -r %HW_Data%\SoundFX\gun*.lut
del %HW_Data%\SoundFX\gun*.lut
copy "h:\data\SoundFX\gun*.lut"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\gun*.lut

attrib -r %HW_Data%\SoundFX\ship*.lut
del %HW_Data%\SoundFX\ship*.lut
copy "h:\data\SoundFX\ship*.lut"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\ship*.lut

attrib -r %HW_Data%\SoundFX\spec*.lut
del %HW_Data%\SoundFX\spec*.lut
copy "h:\data\SoundFX\spec*.lut"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\spec*.lut

attrib -r %HW_Data%\SoundFX\ui*.lut
del %HW_Data%\SoundFX\ui*.lut
copy "h:\data\SoundFX\ui*.lut"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\ui*.lut

attrib -r %HW_Data%\SoundFX\*.bnk
del %HW_Data%\SoundFX\*.bnk
copy "h:\data\SoundFX\*.bnk"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\*.bnk


attrib -r %HW_Data%\SoundFX\speech*.lut
del %HW_Data%\SoundFX\speech*.lut
attrib -r %HW_Data%\*.vce
del %HW_Data%\*.vce
attrib -r %HW_Data%\SoundFX\*.wxh
del %HW_Data%\SoundFX\*.wxh
attrib -r %HW_Data%\*.wxd
del %HW_Data%\*.wxd


if %HW_Demo%_ == _ goto nodemo
goto next

:nodemo
@echo Copying Homeworld Speech files...
copy "h:\data\SoundFX\speech*.lut"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\speech*.lut
copy "h:\data\SoundFX\hw_comp.vce"   %HW_Data%
attrib +r %HW_Data%\*.vce

@echo Copying Homeworld Music files...
copy "h:\data\SoundFX\HW_Music.wxh"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\*.wxh
copy "h:\data\SoundFX\HW_Music.wxd"   %HW_Data%
attrib +r %HW_Data%\*.wxd

goto end

:next
if %HW_Demo% == DLPublicBeta goto public
goto next1

:public
@echo Copying Homeworld Music files...
copy "h:\data\SoundFX\PB_Music.wxh"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\*.wxh
copy "h:\data\SoundFX\PB_Music.wxd"   %HW_Data%
attrib +r %HW_Data%\*.wxd
goto downloadspeech

:next1
if %HW_Demo% == Downloadable goto downloadable
goto next2

:downloadable
@echo Copying Homeworld Music files...
copy "h:\data\SoundFX\DL_Music.wxh"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\*.wxh
copy "h:\data\SoundFX\DL_Music.wxd"   %HW_Data%
attrib +r %HW_Data%\*.wxd

:downloadspeech
@echo Copying Homeworld Speech files...
copy "h:\data\SoundFX\DL*.lut"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\speech*.lut
copy "h:\data\SoundFX\DL_demo.vce"   %HW_Data%
attrib +r %HW_Data%\*.vce

goto end

:next2
if %HW_Demo% == CGW goto cgw
goto next3

:cgw
@echo Copying Homeworld Speech files...
copy "h:\data\SoundFX\CGW*.lut"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\speech*.lut
copy "h:\data\SoundFX\cgw_demo.vce"   %HW_Data%
attrib +r %HW_Data%\*.vce

@echo Copying Homeworld Music files...
copy "h:\data\SoundFX\CGW_Music.wxh"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\*.wxh
copy "h:\data\SoundFX\CGW_Music.wxd"   %HW_Data%
attrib +r %HW_Data%\*.wxd

goto end

:next3
if %HW_Demo% == OEM goto oem

@echo *******************************************************************
@echo *******************************************************************
@echo *******************************************************************
@echo *************   HW_Demo is set to an invalid value   **************
@echo *******************************************************************
@echo *******************************************************************
@echo *******************************************************************
goto end

:oem
@echo Copying Homeworld Speech files...
copy "h:\data\SoundFX\speech*.lut"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\speech*.lut
copy "h:\data\SoundFX\hw_comp.vce"   %HW_Data%
attrib +r %HW_Data%\*.vce

@echo Copying Homeworld Music files...
copy "h:\data\SoundFX\OEM_Music.wxh"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\*.wxh
copy "h:\data\SoundFX\OEM_Music.wxd"   %HW_Data%
attrib +r %HW_Data%\*.wxd

goto end

:locked
@echo *******************************************************************
@echo *******************************************************************
@echo *******************************************************************
@echo ******* ALL SOUND FILES ARE LOCKED.  PLEASE TRY AGAIN LATER *******
@echo *******************************************************************
@echo *******************************************************************
@echo *******************************************************************

:end

@echo done!
