@if %HW_Demo%_ == _ goto default
@if %HW_Demo% == DLPublicBeta goto public
@if %HW_Demo% == Downloadable goto public
@if %HW_Demo% == CGW goto CGWDemo
@if %HW_Demo% == OEM goto OEMDemo
@echo Environment variable HW_Demo is set to an invalid string! Please check and try again...

:default
@echo *****************************************************
@echo ****** Getting Regular Bigfile  *********************
@echo *****************************************************
@del %HW_Data%\HomeworldDL.big > NUL
@del %HW_Data%\HomeworldCGW.big > NUL
@del %HW_Data%\HomeworldMOE.big > NUL
@copy h:\Data\Homeworld.big %HW_Data%
@goto end

:public
@echo *****************************************************
@echo ****** Getting Downloadable Bigfile  ****************
@echo *****************************************************
@del %HW_Data%\Homeworld.big > NUL
@del %HW_Data%\HomeworldCGW.big > NUL
@del %HW_Data%\HomeworldMOE.big > NUL
@copy h:\Data\HomeworldDL.big %HW_Data%
@goto end

:CGWDemo
@echo *****************************************************
@echo ****** Getting CGW Demo Bigfile  ********************
@echo *****************************************************
@del %HW_Data%\Homeworld.big > NUL
@del %HW_Data%\HomeworldDL.big > NUL
@del %HW_Data%\HomeworldMOE.big > NUL
@copy h:\Data\HomeworldCGW.big %HW_Data%
@goto end

:OEMDemo
@echo *****************************************************
@echo ****** Getting OEM Demo Bigfile  ********************
@echo *****************************************************
@del %HW_Data%\HomeworldMOE.big > NUL
@del %HW_Data%\HomeworldDL.big > NUL
@del %HW_Data%\HomeworldCGW.big > NUL
@copy h:\Data\Homeworld.big %HW_Data%

:end
@echo Done!

