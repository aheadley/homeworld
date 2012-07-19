@echo off

if exist "z:\homeworld\homeworld\data\SoundFX\SpeechFr\lock.txt" goto locked

attrib -r %HW_Data%\SoundFX\speech*.lut
del %HW_Data%\SoundFX\speech*.lut
attrib -r %HW_Data%\hw_comp.vce
del %HW_Data%\hw_comp.vce

echo Copying Homeworld French Speech files...
copy "z:\homeworld\homeworld\data\SoundFX\SpeechFr\speech*.lut"   %HW_Data%\SoundFX
attrib +r %HW_Data%\SoundFX\speech*.lut
copy "z:\homeworld\homeworld\data\SoundFX\SpeechFr\hw_comp.vce"   %HW_Data%
attrib +r %HW_Data%\*.vce

goto end

:locked
echo ***************************************************************
echo * ALL FRENCH SPEECH FILES ARE LOCKED.  PLEASE TRY AGAIN LATER *
echo ***************************************************************
pause

:end

echo Done!
