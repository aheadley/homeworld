pushd

cd\homeworld

for %filename in (datasrc\ETG\Meshes\%1\*.lws) @lwexport -+%filename -=data\ETG\Meshes\%1\%@name[%filename].geo

popd
