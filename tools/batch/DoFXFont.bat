@echo off


attrib %HW_ROOT%\DataSrc\FEMan\*.fem -r /s
echo Attempting to remove temporary files...
del %HW_ROOT%\DataSrc\FEMan\*.tmp

fxfont %HW_ROOT%\DataSrc\FEMan\FXList.txt %1

echo Attempting to remove temporary files...
del %HW_ROOT%\DataSrc\FEMan\*.tmp