cd \homeworld
call datasrc\makedepend.bat DataSrc\FEMan\Textures
call datasrc\makedepend.bat DataSrc\FEMan\TexDecorative
call datasrc\makedepend.bat DataSrc\FEMan\Construction_Manager
liflist -a -i q feman.ll DataSrc\FEMan\Textures\dependtex.mif
liflist -a -i q feman.ll DataSrc\FEMan\TexDecorative\dependtex.mif
liflist -i q conMgr.ll DataSrc\FEMan\Construction_Manager\dependtex.mif