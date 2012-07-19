#include <windows.h>
#include <winreg.h>
#include <commdlg.h>
#include "PatcherLocalize.h"
#include <stdio.h>

// LOCALIZATION

#define ENGLISH 0
#define FRENCH 1
#define GERMAN 2
#define SPANISH 3
#define ITALIAN 4
#define NUM_LANGUAGES   5

char *patcherNameStr[NUM_LANGUAGES] = { E_PATCHER_NAME, F_PATCHER_NAME, G_PATCHER_NAME, S_PATCHER_NAME, I_PATCHER_NAME };
char *patcherFinishedRunHWStr[NUM_LANGUAGES] = { E_PATCH_FINISHED_RUN_HW, F_PATCH_FINISHED_RUN_HW, G_PATCH_FINISHED_RUN_HW, S_PATCH_FINISHED_RUN_HW, I_PATCH_FINISHED_RUN_HW };
char *patchSuccessStr[NUM_LANGUAGES] = { E_PATCHSUCCESS, F_PATCHSUCCESS, G_PATCHSUCCESS, S_PATCHSUCCESS, I_PATCHSUCCESS };
char *copyFilesFailedStr[NUM_LANGUAGES] = { E_COPYFILESFAILED, F_COPYFILESFAILED, G_COPYFILESFAILED, S_COPYFILESFAILED, I_COPYFILESFAILED };
char *couldNotLocateHWStr[NUM_LANGUAGES] = { E_COULDNOTLOCATEHW, F_COULDNOTLOCATEHW, G_COULDNOTLOCATEHW, S_COULDNOTLOCATEHW, I_COULDNOTLOCATEHW };
char *unknownErrorStr[NUM_LANGUAGES] = { E_UNKNOWN_ERROR, F_UNKNOWN_ERROR, G_UNKNOWN_ERROR, S_UNKNOWN_ERROR, I_UNKNOWN_ERROR };
char *locateHomeworld[NUM_LANGUAGES] = { E_LOCATE_HOMEWORLD, F_LOCATE_HOMEWORLD, G_LOCATE_HOMEWORLD, S_LOCATE_HOMEWORLD, I_LOCATE_HOMEWORLD };

int language = ENGLISH;

// ERRORS

#define PATCH_SUCCESS           0
#define PATCH_COPYFILESFAILED   -1
#define PATCH_COULDNOTLOCATEHOMEWORLD   -2

char *GetPatchErrorString(int patchErrorStatus)
{
    switch (patchErrorStatus)
    {
        case PATCH_SUCCESS: return patchSuccessStr[language];
        case PATCH_COPYFILESFAILED: return copyFilesFailedStr[language];
        case PATCH_COULDNOTLOCATEHOMEWORLD: return couldNotLocateHWStr[language];
        
        default:
            return unknownErrorStr[language];
    }
}

bool GetHomeworldDataDirFromRegistry(char *hwdir)
{
    HKEY key;
    DWORD size = 300;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Sierra On-Line\\Homeworld",
                        0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
    {
        return false;
    }

    if (RegQueryValueEx(key,"HW_Data",NULL,NULL,(unsigned char *)hwdir,&size) != ERROR_SUCCESS)
    {
        RegCloseKey(key);
        return false;
    }

    RegCloseKey(key);

    return true;    
}

bool GetCommandLineFromRegistry(char *hwdir)
{
    HKEY key;
    DWORD size = 300;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Sierra On-Line\\Homeworld",
                        0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
    {
        return false;
    }

    if (RegQueryValueEx(key,"CmdLine",NULL,NULL,(unsigned char *)hwdir,&size) != ERROR_SUCCESS)
    {
        RegCloseKey(key);
        return false;
    }

    RegCloseKey(key);

    return true;    
}

void GetInstallLanguage(void)
{
    HKEY key;
    DWORD size = 300;
    char langstr[300];

    langstr[0] = 0;
    
    language = ENGLISH;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Sierra On-Line\\Homeworld",
                        0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
    {
        return;
    }

    if (RegQueryValueEx(key,"HW_Language",NULL,NULL,(unsigned char *)langstr,&size) != ERROR_SUCCESS)
    {
        RegCloseKey(key);
        return;
    }

    RegCloseKey(key);

    if (stricmp(langstr,"French") == 0)
        language = FRENCH;
    else if (stricmp(langstr,"German") == 0)
        language = GERMAN;
    else if (stricmp(langstr,"Spanish") == 0)
        language = SPANISH;
    else if (stricmp(langstr,"Italian") == 0)
        language = ITALIAN;
    else
        language = ENGLISH;
}

bool GetHomeworldDataDirFromUser(char *hwdir)
{
    OPENFILENAME OpenFileName;
    TCHAR         szFile[MAX_PATH]      = "\0";

    char Filter[] = "Homeworld.exe\0Homeworld.exe\0";

    strcpy( szFile, "");

    // Fill in the OPENFILENAME structure to support a template and hook.
    OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = NULL;
    OpenFileName.hInstance         = 0;
    OpenFileName.lpstrFilter       = Filter;
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = 0;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = sizeof(szFile);
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    OpenFileName.lpstrInitialDir   = "C:\\";
    OpenFileName.lpstrTitle        = locateHomeworld[language];
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = NULL;
    OpenFileName.lCustData         = 0;
    OpenFileName.lpfnHook          = 0;
    OpenFileName.lpTemplateName    = 0; //MAKEINTRESOURCE(IDD_COMDLG32);
    OpenFileName.Flags             = OFN_SHOWHELP | OFN_EXPLORER | OFN_FILEMUSTEXIST;

    // Call the common dialog function.
    if (GetOpenFileName(&OpenFileName))
    {
        strcpy(hwdir,OpenFileName.lpstrFile);

        int scan = strlen(hwdir)-1;
        char *sc;

        while (scan >= 0)
        {
            sc = &hwdir[scan];

            if (*sc != '\\')
                *sc = 0;
            else
            {
                *sc = 0;
                return true;
            }

            scan--;
        }

        return false;
    }

    return false;
}

bool GetHomeworldDataDir(char *hwdir)
{
    // try to get data directory from registry first
    if (GetHomeworldDataDirFromRegistry(hwdir))
    {
        return true;
    }

    if (GetHomeworldDataDirFromUser(hwdir))
    {
        return true;
    }
#if 0           // only use registry
    char *ptr = getenv("HW_Data");

    if (ptr)
    {
        strcpy(hwdir,ptr);
        return true;
    }
#endif
    return false;
}

void WaitForHomeworldToShutdown()
{
    static  char szSettingsApp[] = "Homeworld";
    static  char szSettingsClass[] = "Homeworld";
    UINT    uMsg = RegisterWindowMessage("CloseHomeworld");

    if (FindWindow(szSettingsApp, szSettingsClass))
    {
        PostMessage(HWND_BROADCAST, uMsg, 0, 0);
        do
        {
            Sleep(100);
        } while (FindWindow(szSettingsApp, szSettingsClass));

        Sleep(500);     // sleep a bit more just for fun
    }
}

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                        LPSTR commandLine, int nCmdShow)
{
    char curdir[300];
    char destfile[300];
    //char exewithpath[300];
    char hwdatadir[300];
    HANDLE ffHandle;
    WIN32_FIND_DATA ffData;
    int patchErrorStatus = PATCH_SUCCESS;

    GetCurrentDirectory(300,curdir);

    WaitForHomeworldToShutdown();

    GetInstallLanguage();

    if (!GetHomeworldDataDir(hwdatadir))
    {
        patchErrorStatus = PATCH_COULDNOTLOCATEHOMEWORLD;
        goto goterror;
    }   

    SetCurrentDirectory(curdir);

    strcat(curdir,"\\*.*");
    ffHandle = FindFirstFile(curdir,&ffData);

    if (ffHandle == INVALID_HANDLE_VALUE)
    {
        patchErrorStatus = PATCH_COPYFILESFAILED;
        goto goterror;
    }

    do
    {
        // ffData has filename we're interested in:

        if (ffData.cFileName[0] == '.')
            continue;

        if (stricmp(ffData.cFileName,"Patcher.exe") == 0)
            continue;       // don't copy myself

        strcpy(destfile,hwdatadir);
        strcat(destfile,"\\");
        strcat(destfile,ffData.cFileName);

        if (!CopyFile(ffData.cFileName,destfile,FALSE))
        {
            patchErrorStatus = PATCH_COPYFILESFAILED;
            DWORD error = GetLastError();
            FindClose(ffHandle);

            char errstr[300];
            sprintf(errstr,GetPatchErrorString(patchErrorStatus),ffData.cFileName,destfile);
            MessageBox(NULL,errstr,patcherNameStr[language],MB_OK|MB_SYSTEMMODAL);
            return patchErrorStatus;
        }

    }
    while (FindNextFile(ffHandle,&ffData));

    FindClose(ffHandle);

goterror:
    if (patchErrorStatus)
    {
       MessageBox(NULL,GetPatchErrorString(patchErrorStatus),patcherNameStr[language],MB_OK|MB_SYSTEMMODAL);
       return patchErrorStatus;
    }

    if (MessageBox(NULL,patcherFinishedRunHWStr[language],patcherNameStr[language],MB_YESNO|MB_SYSTEMMODAL) == IDYES)
    {
        char commandLine[300];
        char winexecstr[300];

        SetCurrentDirectory(hwdatadir);
        strcpy(winexecstr,"Homeworld.exe ");
        if (GetCommandLineFromRegistry(commandLine))
        {           
            strcat(winexecstr,commandLine);
        }

        WinExec(winexecstr,SW_NORMAL);
    }

    return 0;
}

