#include <windows.h>
#include <winreg.h>
#include "ClientCDKey.h"

bool WriteCDKeyToRegistryUnencrypted(char *string)
{
	HKEY key;
	bool retvalue;

    if (strlen(string) <= 0)
		return false;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Sierra On-Line\\Homeworld",
                     0, KEY_SET_VALUE, &key) != ERROR_SUCCESS)
    {
		return false;
    }

	retvalue = true;
    if (RegSetValueEx(key, "CDKey", 0, REG_SZ, (CONST BYTE *)string, strlen(string)+1) != ERROR_SUCCESS)
	{
		retvalue = false;
	}
	
	RegCloseKey(key);

	return retvalue;
}

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                        LPSTR commandLine, int nCmdShow)
{
	if (strlen(commandLine) <= 0)
	{
		return -1;
	}

    WONCDKey::ClientCDKey aCDKey("Homeworld");
    
	if (aCDKey.Init(_strupr(commandLine)))
	{
		if (aCDKey.IsValid())
		{
			if (aCDKey.Save())
			{
				__int64 rawdata = aCDKey.AsRaw();	// just for debugging purposes
				WriteCDKeyToRegistryUnencrypted(commandLine);
				return 0;
			}
		}
	}

	return -1;
}
