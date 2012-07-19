@if %HW_Root%_ == _ goto notdefined

@ren %HW_Root%\\src %HW_Root%\\src_Current
@ren %HW_Root%\\Lib %HW_Root%\\Lib_Current
@ren %HW_Root%\\publicbeta_src %HW_Root%\\src
@ren %HW_Root%\\publicbeta_lib %HW_Root%\\Lib

@echo Directories renamed !!

@goto endfile

:notdefined

@echo You HW_Root environment variable is not define!

:endfile
