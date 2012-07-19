cd\homeworld

echo # >! datasrc\SpeechIcons\R1\dependtex.mif
@for %filename in (datasrc\SpeechIcons\R1\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\SpeechIcons\R1\dependtex.mif
@liflist q data\textures.ll datasrc\SpeechIcons\R1\dependtex.mif

echo # >! datasrc\SpeechIcons\R2\dependtex.mif
@for %filename in (datasrc\SpeechIcons\R2\*.psd) echo TEXTURES += %@name[%filename].lif >> datasrc\SpeechIcons\R2\dependtex.mif
@liflist q data\textures.ll datasrc\SpeechIcons\R2\dependtex.mif

@del datasrc\SpeechIcons\R1\dependtex.mif
@del datasrc\SpeechIcons\R2\dependtex.mif

