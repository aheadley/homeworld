/*
*	Sierra Internet Gaming Systems
*	(c) Sierra On-Line.  1996

*	File name: SNWValidateSDK.h

  ********* PRELIMINARY **************
*
*	Prototypes and defines for SIGS SNWValid.dll
*	Work on this module is in beta. SIGS reserves the right to make continuing
*	changes to the interface to improve on it's usability an features.
*	
*/

#ifndef _INC_SNWValidateSDK
#define _INC_SNWValidateSDK

typedef enum tagDLL_Exit_Status
{
	DLL_EXIT_FOR_SELF_UPDATE = -2,
	DLL_EXIT_FAILED_TO_UPDATE,
	DLL_EXIT_CONTINUE_TO_CONNECT
} VALIDATE_DLL_EXIT;

// function
typedef VALIDATE_DLL_EXIT (SIGSCALL *lpfnSNWValidateFunc)(	char *GPName,
															char *GPIPAddr,
															long *GPInPort,
															long *GPOutPort,
															char *pcProdDIR,
															char *pcAppLaunch,
															HWND hMainWnd);


/**********	INPUT PARAMETERS
*	GPName -	The game name. This value is predefined in the SIGS database
*				It is used to determine the next three parameters to pass
*				back to the calling application
*
*	GPIPAddr -	Returned value of the Gathering Place IP address
*				Defined in the SIGS database. Use a char that can hold
*				IP addresses of the format XXX.XXX.XXX.XXX
*	
*	GPInPort -	Returned value of the Gathering Place communication In Port
*				Defined in the SIGS database
*
*	GPOutPort -	Returned value of the Gathering Place communication Out Port
*				Defined in the SIGS database
*
*	pcProdDIR - The Installed path for the product, (must end in a '\')
*				Example:	c:\sierra\blackjak\
*
*	pcAppExe -	This is the full command to restart your EXE. You can include commandline
*				parameters to restart your exe. Avoid the '/' character.
*				Example:	c:\sierra\blackjak\blackjak.exe
*
*	hMainWnd -	The window handle to you main window. It will be used to make sure
*				we can shut down you APP when you get patched. By default it will be NULL
*				And we will try to calculate it ourself.
*
*
********	RETURN VALUES
*	DLL_EXIT_FOR_SELF_UPDATE -	This indicates there is a patch on the server
*								for the aplication. You should terminate as quickly
*								as possible. SIGSPat.exe will restart your app
*								when patching is complete.
*
*	DLL_EXIT_FAILED_TO_UPDATE - Validation failed for any number of reason. For
*								all accounts, Internet play is not available.
*
*	DLL_EXIT_CONTINUE_TO_CONNECT -	Everything has completed normally. You should
*								free SNWValid at this time.
*/

/*!!!!!!!!!!!!! Code sample !!!!!!!!!!!!!!!!!
*
*	The following is a code fragement of how you might chose to
*	to implement SNWValid.DLL. Make sure you have a valid window for
*	your app before calling SNWValid.
*	This code was designed and tested under Microsoft VC4.1, BC5.0a, and Watcom10.6
*
*
	lpfnSNWValidateFunc lpfnSNWValidate;
	HINSTANCE DllHandle = NULL;
	char cpIPAdress[]="XXX.XXX.XXX.XXX";
	long lInPort = 0;
	long lOutPort = 0;
	VALIDATE_DLL_EXIT ret;

	DllHandle = LoadLibrary("SNWValid.dll");
	if (DllHandle != NULL)
	{
		lpfnSNWValidate = (lpfnSNWValidateFunc)GetProcAddress((HINSTANCE)DllHandle, "SNWValidate");
		if (lpfnSNWValidate != NULL)
		{
			// make the call to SNWValidate with your constructed parameters
			ret = lpfnSNWValidate("Blackjack",
									cpIPAdress,
									&lInPort,
									&lOutPort,
									"c:\\sierra\\blackjak\\",	// hard coded for example only
									"c:\\sierra\\blackjak\\blackjak.exe", // hard coded for example only
									hMainWnd); // you main window handle
		}
		else
		{
			// report an error
			// it's up to you.
		}
		FreeModule(DllHandle);
		switch (ret)
		{
			case DLL_EXIT_FOR_SELF_UPDATE:
				// exit your code now!
				// you are about to be updated
				// make sure you have no dialogs to answer when exiting!!!
				break;
			case DLL_EXIT_FAILED_TO_UPDATE:
				// let the user know that they could not connect
				break;
			case DLL_EXIT_CONTINUE_TO_CONNECT:
				// everything was cool
				// let them play
				break;
			default:
				break;
				// we'll let you figure out what to do here.
		}
	}
	else // no DLL to load
	{
		// your error handling here
	}

*
*/


/*	!!!!!!!!!!!!the following is for information only.!!!!!!!!!!!!!!
*
*	this is being supplied to indicate to the developers
*	how SNWValid.dll was built to help facilitate the developement
*	of their code. 	
*
*/
#ifdef MAKE_DLL

// this is now defined in SNTypes.h
//#define DllExport	__declspec( dllexport )
		
DllExport VALIDATE_DLL_EXIT SIGSCALL SNWValidate (	char *GPName,
													char *GPIPAddr,
													long *GPInPort,
													long *GPOutPort,
													char *pcProdDIR,
													char *pcAppLaunch,
													HWND hMainWnd);

#endif	//MAKE_DLL

#endif  //_INC_SNWValidateSDK

