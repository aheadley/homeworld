pushd

if "%1"==%1 goto about

cd\homeworld

if exist datasrc\ETG\Textures\%1\dependtex.mif del datasrc\ETG\Textures\%1\dependtex.mif > NUL
echo # > datasrc\ETG\Textures\%1\dependtex.mif
for %filename in (datasrc\ETG\Textures\%1\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\ETG\Textures\%1\dependtex.mif
@liflist q data\textures.ll datasrc\ETG\Textures\%1\dependtex.mif
goto end

:about
    @echo USAGE: ONETEXANIM <dir_name>

:end

popd
