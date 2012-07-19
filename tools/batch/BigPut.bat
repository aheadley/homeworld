@if %HW_Demo% == DLPublicBeta goto public
@if %HW_Demo% == Downloadable goto public
@if %HW_Demo% == CGW goto CGWDemo
@if %HW_Demo% == OEM goto OEMDemo

@echo *****************************************************
@echo ****** Putting Regular Bigfile  *********************
@echo *****************************************************
@copy %HW_Data%\Homeworld.big z:\homeworld\Homeworld\Data\Homeworld.big
@goto end

:public
@echo *****************************************************
@echo ****** Putting Downloadable Bigfile  ****************
@echo *****************************************************
@copy %HW_Data%\HomeworldDL.big z:\homeworld\Homeworld\Data\HomeworldDL.big
@goto end

:CGWDemo
@echo *****************************************************
@echo ****** Putting CGW Demo Bigfile  ********************
@echo *****************************************************
@copy %HW_Data%\HomeworldCGW.big z:\homeworld\Homeworld\Data\HomeworldCGW.big
@goto end

:OEMDemo
@echo *****************************************************
@echo ****** Putting OEM Demo Bigfile  ********************
@echo *****************************************************
@copy %HW_Data%\HomeworldMOE.big z:\homeworld\Homeworld\Data\HomeworldMOE.big

:end
@echo Done!
