@echo off

if exist "z:\homeworld\homeworld\data\SoundFX\SpeechDe\lock.txt" goto locked

attrib -r %HW_Data%\SoundFX\speech*.lut
del %HW_Data%\SoundFX\speech*.lut
attrib -r %HW_Data%\hw_comp.vce
del %HW_Data%\hw_comp.vce

echo Copying Homeworld German Speech files...
copy "z:\homeworld\homeworld\data\SoundFX\SpeechDe\speech*.lut"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\speech*.lut
copy "z:\homeworld\homeworld\data\SoundFX\SpeechDe\hw_comp.vce"   %HW_Data%
attrib +r %HW_Data%\*.vce

goto end

:locked
echo ***************************************************************
echo * ALL GERMAN SPEECH FILES ARE LOCKED.  PLEASE TRY AGAIN LATER *
echo ***************************************************************
pause

:end

echo Done!
