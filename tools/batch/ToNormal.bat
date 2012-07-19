@if %HW_Root%_ == _ goto notdefined

@ren %HW_Root%\src %HW_Root%\\publicbeta_src
@ren %HW_Root%\Lib %HW_Root%\\publicbeta_lib
@ren %HW_Root%\src_Current %HW_Root%\\src
@ren %HW_Root%\Lib_Current %HW_Root%\\Lib

@echo Directories renamed !!

@goto endfile

:notdefined

@echo You HW_Root environment variable is not define!

:endfile
