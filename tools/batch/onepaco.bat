cd \homeworld

if exist datasrc\%1\%2\rl0\lod0\dependtex.mif del datasrc\%1\%2\rl0\lod0\dependtex.mif > NUL
if exist datasrc\%1\%2\rl0\lod1\dependtex.mif del datasrc\%1\%2\rl0\lod1\dependtex.mif > NUL
if exist datasrc\%1\%2\rl0\lod2\dependtex.mif del datasrc\%1\%2\rl0\lod2\dependtex.mif > NUL
if exist datasrc\%1\%2\rl0\lod3\dependtex.mif del datasrc\%1\%2\rl0\lod3\dependtex.mif > NUL
if exist datasrc\%1\%2\rl0\lod4\dependtex.mif del datasrc\%1\%2\rl0\lod4\dependtex.mif > NUL

echo # > datasrc\%1\%2\rl0\lod0\dependtex.mif
echo # > datasrc\%1\%2\rl0\lod1\dependtex.mif
echo # > datasrc\%1\%2\rl0\lod2\dependtex.mif
echo # > datasrc\%1\%2\rl0\lod3\dependtex.mif
echo # > datasrc\%1\%2\rl0\lod4\dependtex.mif

@paco %1 %2 0
for %filename in (data\%1\%2\rl0\lod0\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod0\dependtex.mif
liflist a data\textures.ll datasrc\%1\%2\rl0\lod0\dependtex.mif

@paco %1 %2 1
for %filename in (data\%1\%2\rl0\lod1\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod1\dependtex.mif
liflist a data\textures.ll datasrc\%1\%2\rl0\lod1\dependtex.mif

@paco %1 %2 2
for %filename in (data\%1\%2\rl0\lod2\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod2\dependtex.mif
liflist a data\textures.ll datasrc\%1\%2\rl0\lod2\dependtex.mif

@paco %1 %2 3
for %filename in (data\%1\%2\rl0\lod3\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod3\dependtex.mif
liflist a data\textures.ll datasrc\%1\%2\rl0\lod3\dependtex.mif

@paco %1 %2 4
for %filename in (data\%1\%2\rl0\lod4\page*.lif) echo TEXTURES += %@name[%filename].lif >> datasrc\%1\%2\rl0\lod4\dependtex.mif
liflist a data\textures.ll datasrc\%1\%2\rl0\lod4\dependtex.mif

:end

