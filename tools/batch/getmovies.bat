@echo off

attrib -r %HW_Data%\Movies\*smk
attrib -r %HW_Data%\Movies\*bik
del %HW_Data%\Movies\*smk
del %HW_Data%\Movies\*bik

echo Copying movies (a slow process) ...

copy "h:\data\movies\*lst" %HW_Data%\Movies
copy "h:\data\movies\*smk" %HW_Data%\Movies
copy "h:\data\movies\*bik" %HW_Data%\Movies

echo done!

