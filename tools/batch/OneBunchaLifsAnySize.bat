@pushd

@if "%1"==%1 goto about

@cd\homeworld

@if exist datasrc\%1\dependtex.mif del datasrc\%1\dependtex.mif > NUL
@echo # > datasrc\%1\dependtex.mif
@for %filename in (datasrc\%1\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\dependtex.mif
@liflist -I q data\textures.ll datasrc\%1\dependtex.mif
@del datasrc\%1\dependtex.mif > NUL
goto end

:about
    @echo USAGE: OneBunghaLifs <dir_name>

:end

@popd
