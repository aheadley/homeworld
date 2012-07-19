@echo off

attrib %HW_ROOT%\DataSrc\FEMan\*.fem -r /s
echo Attempting to remove temporary files...
del %HW_ROOT%\DataSrc\FEMan\*.tmp

fxlate %HW_ROOT%\DataSrc\FEMan\FEManStrings.txt -h -v %HW_ROOT%\DataSrc\FEMan\FXLateErrors.log %1

echo Attempting to remove temporary files...
del %HW_ROOT%\DataSrc\FEMan\*.tmp
