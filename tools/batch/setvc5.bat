rem root paths

set HW_VC5=d:\vc5
set HW_Intel=d:\IntelCompiler
set HW_Root=d:\homeworld

rem set Homeworld variables
set HW_Data=%HW_Root%\data
set HW_Level=HW_Debug

rem set compilers into paths

set path=%HW_Intel%\bin;%HW_VC5%\bin;%HW_VC5%\sharedide\bin;%HW_Root%\tools\batch;%HW_Root%\tools\bin;%path%
set lib=%HW_Intel%\lib;%HW_VC5%\lib;%lib%
set include=%HW_Intel%\include;%HW_VC5%\include;%include%
