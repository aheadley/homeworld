/*=============================================================================
    DEBUGWND.C: Code to draw a debug window
=============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Types.h"
#include "main.h"
#include "Task.h"
#include "File.h"
#include "debugwnd.h"

/*=============================================================================
    Data:
=============================================================================*/
//global flag set if debug window enabled
sdword dbwEnabled = FALSE;

//size and location of window
sdword dbwWindowX, dbwWindowY;                  //top-left corner of window
sdword dbwWindowWidth, dbwWindowHeight;         //width of the window (in characters)
sdword dbwFontWidth, dbwFontHeight;             //width and height of font characters (assumes fixed-pitch)
/*HWND   hDebugWindow;                            //HWND for the debug window*/
/*HDC    hDebugDC;                                //device context for debug window*/
/*LOGFONT dbwLogicalFont;                                //current debug window font*/
udword hDebugFont;                              //font handle for selection into DC

//data for the individual panes
pane dbwPane[DBW_NumberPanes];

#if DBW_TO_FILE
extern bool debugToFile;
#endif

/*=============================================================================
    Functions:
=============================================================================*/
/*
//test code
void testItOut(void)
{
    sdword index;
    char string[100];

//!!! just some quick test code to see if this thing works
    for (index = 0; index < 32; index++)                    //test scroll off bottom of window
    {
        sprintf(string, "Hello Nurse #%d\n", index);
        dbwPrint(0, string);
    }

    for (index = 0; index < 70; index++)                    //test scroll off bottom of buffer
    {
        sprintf(string, "Hello Nurse #%d\n", index);
        dbwPrint(0, string);
    }

    dbwPaneClear(0);

    dbwPrint(0, "Hello\nNurse!");                           //test newline stuff
    dbwPrint(0, " - Hello Nurse!");

    dbwPrint(0, "Hello\rMa'am\nHello -- Nurse");            //test carriage return

    dbwPrint(0, "    for (index = 0; index < 32; index++)                    //test scroll off bottom of window");

    dbwPrint(0, "Hello Nurse\n");                           //test clear pane
}
*/

/*-----------------------------------------------------------------------------
    Name        : dbwAllPanesFree
    Description : Frees memory of all panes and disables them.
    Inputs      : void
    Outputs     : void
    Return      : void
----------------------------------------------------------------------------*/
void dbwAllPanesFree(void)
{
    sdword index;

    for (index = 0; index < DBW_NumberPanes; index++)
    {
        if (bitTest(dbwPane[index].flags, DPF_Enabled))
        {
            free(dbwPane[index].buffer);   //!!! use proper free call
            bitClear(dbwPane[index].flags, DPF_Enabled);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : dbwCharsPrint
    Description : Prints a number of characters to current cursor location.
    Inputs      : pane - index of pane to print to.  Because this is an internal
                    function, no error checking is performed on this index.
                  string - start of characters to print
                  nChars - number of characters to print.  No error checking
                    here either.
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void dbwCharsPrint(sdword pane, char *string, sdword nChars, sdword bufferFlag)
{
    sdword x, y;
//    RECT rect;

    if (nChars <= 0)
    {
        return;
    }

    taskStackSaveIf(4);                                     //save task stack context
    if (GetDC(hDebugWindow) != hDebugDC)
    {
        taskStackRestoreIf();
        return;
    }
    if (bufferFlag)
    {
        memcpy(dbwPane[pane].buffer + dbwPane[pane].width * //copy string to buffer
               dbwPane[pane].cursorY + dbwPane[pane].cursorX,
               string, nChars);
    }
    x = (dbwPane[pane].x + dbwPane[pane].cursorX) * dbwFontWidth;
    y = (dbwPane[pane].y + dbwPane[pane].cursorY - dbwPane[pane].viewTop) * dbwFontHeight;

    //SetBkMode(hDebugDC, TRANSPARENT);
    SetBkColor(hDebugDC, GetSysColor(COLOR_WINDOW));
    TextOut(hDebugDC, x, y, string, nChars);

//  rect.left = 0;
//  rect.right = dbwWindowWidth * dbwFontWidth;
//  rect.top = y;
//  rect.bottom = y + dbwFontHeight + 2;
//  ValidateRect(hDebugWindow, &rect);

    ReleaseDC(hDebugWindow, hDebugDC);
    taskStackRestoreIf();                                   //restore task stack context
}

/*-----------------------------------------------------------------------------
    Name        : dbwAllPanesRepaint
    Description : Repaints all panes.
    Inputs      : hDC - device context (should be same as hDebugDC)
    Outputs     : ..
    Return      : ..
----------------------------------------------------------------------------*/
void dbwAllPanesRepaint(HDC hDC)
{
    sdword pane;
    sdword line;
    char *source;
    sdword oldCursorX, oldCursorY;

    for (pane =0; pane < DBW_NumberPanes; pane++)           //for all panes
    {
        if (!bitTest(dbwPane[pane].flags, DPF_Enabled))     //don't process disabled ones
            continue;

        oldCursorX = dbwPane[pane].cursorX;                 //save current cursor settings
        oldCursorY = dbwPane[pane].cursorY;

        dbwPane[pane].cursorX = 0;
        dbwPane[pane].cursorY = dbwPane[pane].viewTop;

        for (line = dbwPane[pane].viewTop; line < dbwPane[pane].viewTop +
             dbwPane[pane].height; line++)                  //for each line of this pane
        {
            source = dbwPane[pane].buffer + dbwPane[pane].width * line;
            dbwCharsPrint(pane, source, dbwPane[pane].width, FALSE);
            dbwPane[pane].cursorY++;
        }
         dbwPane[pane].cursorX = oldCursorX;                //restore old cursor settings
         dbwPane[pane].cursorY = oldCursorY;
    }
}

/*-----------------------------------------------------------------------------
    Name        : dbwWindowProc
    Description : Window Procedure for debug window
    Inputs      : see Windows docs
    Outputs     : "   "       "
    Return      : "   "       "
----------------------------------------------------------------------------*/
long FAR PASCAL dbwWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    RECT rect;
    switch (message)
    {
        case WM_SETFOCUS:
            SetFocus(ghMainWindow);
            break;
        case WM_MOVE:
            GetWindowRect(hWnd, &rect);
            dbwWindowX = rect.left;
            dbwWindowY = rect.top;
            return 0;
        case WM_PAINT:
            dbwAllPanesRepaint((HDC)wParam);
            ValidateRect(hDebugWindow, NULL);
            return 0;
        case WM_DESTROY:
            return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
/*-----------------------------------------------------------------------------
    Name        : dbwPaneAlloc
    Description : Allocated memory for the pane.
    Inputs      : pane - index of pane to allocate
    Outputs     : dbwPane[index].buffer -> newly allocated memory
    Return      : OKAY if memory allocated fine
    Note        :
        The width, height and buffer height of pane must be set at this point
----------------------------------------------------------------------------*/
sdword dbwPaneAlloc(sdword pane)
{
    if (dbwPaneOutRange(pane))
    {
        return(ERROR);
    }
    //!!!use proper alloc call
    dbwPane[pane].buffer = malloc(dbwPane[pane].width * dbwPane[pane].bufferHeight);

    if (dbwPane[pane].buffer == NULL)
    {
        return(ERROR);
    }
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : dbwFontChoose
    Description : Chooses the current font using the ChooseFont dialog
    Inputs      : ..
    Outputs     : Set the dbwFontWidth and dbwFontHeight variables
    Return      : ERROR if user cancelled or cannot select a font
----------------------------------------------------------------------------*/
sdword dbwFontChoose(void)
{
    CHOOSEFONT chooseFont;

    chooseFont.lStructSize = sizeof(CHOOSEFONT);
    chooseFont.hwndOwner = hDebugWindow;
    chooseFont.hDC = NULL;                                  //only used for printer fonts
    chooseFont.lpLogFont = &dbwLogicalFont;
    chooseFont.Flags = CF_FIXEDPITCHONLY | CF_LIMITSIZE | CF_SCREENFONTS;
    chooseFont.nSizeMin = DBW_FontPointsMin;
    chooseFont.nSizeMax = DBW_FontPointsMax;

    if (ChooseFont(&chooseFont) != TRUE)                    //select the font
        return(ERROR);
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : dbwFindString
    Description : Search for and read in a string from a text file.
    Inputs      : fileName - name of file to search through
                  keyString - name of key string
                  defaultString - default string if not found
                  length - length of returnString
    Outputs     : returnString - string to fill in
    Return      : TRUE if the string was found and scanned in properly.
----------------------------------------------------------------------------*/
bool dbwFindString(char *fileName, char *keyString, char *defaultString, char *returnString, sdword length)
{
    char stringBuffer[1024], *dataStart;
    FILE *f;
    sdword keyLength = strlen(keyString);
    static char blanks[] = " \t=";

    f = fopen(fileName, "rt");
    if (f == NULL)
    {
        goto returnError;
    }
    while (fgets(stringBuffer, 1024, f) != NULL)
    {
        if (!strncmp(stringBuffer, keyString, keyLength) && strchr(blanks, stringBuffer[keyLength]))
        {
            for (dataStart = stringBuffer + keyLength; ; dataStart++)
            {
                if (*dataStart == 0)
                {                                           //no data in this key
                    break;
                }
                if (!strchr(blanks, *dataStart))
                {                                           //if non-blank character
                    fclose(f);
                    strncpy(returnString, dataStart, length);
                    returnString[length - 1] = 0;           //make sure it's NULL-terminated
                    if (strchr(returnString, '\n'))
                    {
                        *strchr(returnString, '\n') = 0;
                    }
                    return(TRUE);
                }
            }
        }
    }
    fclose(f);                                              //nothing found
returnError:
    strcpy(returnString, defaultString);
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : dbgDataNybble
    Description : Convert a hex character into a nybble.
    Inputs      :
    Outputs     :
    Return      : value of hex nybble or -1 on error
----------------------------------------------------------------------------*/
sdword dbgDataNybble(char c)
{
    if (c >= '0' && c <= '9')
    {
        return((udword)c - '0');
    }
    c = toupper(c);
    if (c >= 'A' && c <= 'F')
    {
        return(0xa + c - 'A');
    }
    return(-1);
}

/*-----------------------------------------------------------------------------
    Name        : dbwFindBinary
    Description : Search for and read in a string from a text file.
    Inputs      : fileName - file to search through
                  keyName - name of key
                  length - length of buffer
    Outputs     : buffer - buffer to fill in.
    Return      :
----------------------------------------------------------------------------*/
bool dbwFindBinary(char *fileName, char *keyString, ubyte *buffer, udword length)
{
    char stringBuffer[1024], *dataStart;
    FILE *f;
    sdword keyLength = strlen(keyString), index;
    static char blanks[] = " \t=";
    sdword lowNybble, highNybble;

    f = fopen(fileName, "rt");
    if (f == NULL)
    {
        return(FALSE);
    }
    while (fgets(stringBuffer, 1024, f) != NULL)
    {
        if (!strncmp(stringBuffer, keyString, keyLength) && strchr(blanks, stringBuffer[keyLength]))
        {
            for (dataStart = stringBuffer + keyLength; ; dataStart++)
            {
                if (*dataStart == 0)
                {                                           //no data in this key
                    break;
                }
                if (!strchr(blanks, *dataStart))
                {                                           //if non-blank character
                    fclose(f);
                    if (strlen(dataStart) <= length * 2)
                    {                                       //not enough data to fill the string
                        return(FALSE);
                    }
                    for (index = 0; index < length; index++)
                    {                                       //read in each byte
                        lowNybble = dbgDataNybble(dataStart[index * 2 + 1]);
                        highNybble = dbgDataNybble(dataStart[index * 2]);
                        if (lowNybble < 0 || highNybble < 0)
                        {                                   //if bad nybbles
                            return(FALSE);
                        }
                        buffer[index] = (highNybble << 4) + lowNybble;
                    }
                    return(TRUE);
                }
            }
        }
    }
    fclose(f);                                              //nothing found
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : dbwStart
    Description : Start up the debug window and initialize all panes.
    Inputs      : void
    Outputs     : Clears all pane buffers to spaces
    Return      : Success flag (OKAY if window created OK)
----------------------------------------------------------------------------*/
sdword dbwStart(udword hInstance, udword hWndParent)
{
    WNDCLASS windowClass;
    SIZE   fontSize;
    sdword index, width, height;
    char keyString[DIS_StringLength];
    char returnString[DIS_StringLength];
    char defaultString[DIS_StringLength];
    BOOL fontLoaded = FALSE;
    LOGBRUSH brush =
    {
        BS_SOLID,
        GetSysColor(COLOR_WINDOW),
        0
    };

#if DBW_TO_FILE
    if (debugToFile)
    {
        logfileClear(DBW_FILE_NAME);
    }
#endif
    if (dbwEnabled)                                         //don't start if already started
    {
        return ERROR;
    }

    //get location from .INI file
    sprintf(defaultString, "%d, %d", DBW_WindowX, DBW_WindowY);
//    GetPrivateProfileString(DIS_SectionName, DIS_Location,
//                            defaultString, returnString, DIS_StringLength, DIS_FileName);
    dbwFindString(DIS_FileName, DIS_Location, defaultString, returnString, DIS_StringLength);
    sscanf(returnString, "%d, %d", &dbwWindowX, &dbwWindowY);

    //get size from .INI file
    sprintf(defaultString, "%d, %d", DBW_WindowWidth, DBW_WindowHeight);
//    GetPrivateProfileString(DIS_SectionName, DIS_Size,
//                            defaultString, returnString, DIS_StringLength, DIS_FileName);
    dbwFindString(DIS_FileName, DIS_Size, defaultString, returnString, DIS_StringLength);
    sscanf(returnString, "%d, %d", &dbwWindowWidth, &dbwWindowHeight);

    dbwFontWidth = dbwFontHeight = 8;                       //assume a default size for now

    // set up and register window class
    windowClass.style         = CS_NOCLOSE | CS_OWNDC;
    windowClass.lpfnWndProc   = dbwWindowProc;
    windowClass.cbClsExtra    = 0;
    windowClass.cbWndExtra    = 0;
    windowClass.hInstance     = (HINSTANCE)hInstance;
    windowClass.hIcon         = NULL;
    windowClass.hCursor       = LoadCursor( NULL, IDC_ARROW );
    windowClass.hbrBackground = CreateBrushIndirect(&brush);
    windowClass.lpszMenuName  = NULL;
    windowClass.lpszClassName = DBW_ClassName;

    RegisterClass(&windowClass);

    width = dbwWindowWidth * dbwFontWidth +  + GetSystemMetrics(SM_CXSIZEFRAME) * 2;
    height = dbwWindowHeight * dbwFontHeight + GetSystemMetrics(SM_CYCAPTION) +
            GetSystemMetrics(SM_CYSIZEFRAME);
    //now create the window
    hDebugWindow = CreateWindowEx(
        0,//WS_EX_STATICEDGE,                                   //cannot be sized
        DBW_ClassName,                                      //class name string
        DBW_WindowTitle,                                    //title string
        WS_POPUP | WS_CAPTION,
        dbwWindowX, dbwWindowY,                             //location
        width,                                              //size based upon font
        height,
        (HWND)hWndParent,                                   //parent window
        NULL,                                               //no menu
        (HINSTANCE)hInstance,                               //app instance
        NULL );                                             //no lParam

    if (hDebugWindow == NULL)                               //if function failed
    {
        return(ERROR);
    }

    //now we must compute the proper size of the font, and thus the window
//    if (GetPrivateProfileStruct(DIS_SectionName, DIS_Font,
//                            &dbwLogicalFont, sizeof(LOGFONT), DIS_FileName) == FALSE)
    if (!dbwFindBinary(DIS_FileName, DIS_Font, (ubyte *)&dbwLogicalFont, sizeof(LOGFONT)))
    {
        if (dbwFontChoose() != OKAY)
        {
            return(ERROR);
        }
    }
    hDebugFont = CreateFontIndirect(&dbwLogicalFont);       //create a font object
    if (hDebugFont == NULL)
    {
        return(ERROR);
    }

    hDebugDC = GetDC(hDebugWindow);                         //get device context
    if (hDebugDC == NULL)
    {
        return(ERROR);
    }

    SelectObject(hDebugDC, hDebugFont);                     //select our font in
    GetTextExtentPoint32(hDebugDC, " ", 1, &fontSize);      //get size of a single character
    dbwFontWidth = fontSize.cx;
    dbwFontHeight = fontSize.cy;

    width = dbwWindowWidth * dbwFontWidth +  + GetSystemMetrics(SM_CXSIZEFRAME) * 2;
    height = dbwWindowHeight * dbwFontHeight + GetSystemMetrics(SM_CYCAPTION) +
            GetSystemMetrics(SM_CYSIZEFRAME) * 2;
    MoveWindow(hDebugWindow, dbwWindowX, dbwWindowY,        //resize the window
               width, height, FALSE);

    ShowWindow(hDebugWindow, SW_SHOW);                      //show the window
    UpdateWindow(hDebugWindow);                             //update the window

    dbwEnabled = TRUE;

    //now that we have a window, let's init our panes
    for (index = 0; index < DBW_NumberPanes; index++)
    {
        //attempt to load pane structure from disk
        sprintf(keyString, "%s%d", DIS_PaneBase, index);
        strcpy(defaultString, "");
//        sprintf(defaultString, "%d, %d, %d, %d, %d", 0, 0, dbwWindowWidth,
//                dbwWindowHeight, DBW_BufferHeight);
//        if (GetPrivateProfileString(DIS_SectionName,
//               keyString, defaultString, returnString,
//               DIS_StringLength, DIS_FileName) <= 0)
        if (!dbwFindString(DIS_FileName, keyString, defaultString, returnString, DIS_StringLength))
        {
            //by default there is only 1 pane
            if (index != 0)                                //only init the first one
                continue;
            dbwPane[index].flags = DPF_Enabled;
            dbwPane[index].x = 0;                          //set size of pane to defaults
            dbwPane[index].y = 0;
            dbwPane[index].width = dbwWindowWidth;
            dbwPane[index].height = dbwWindowHeight;
            dbwPane[index].bufferHeight = DBW_BufferHeight;
        }
        else
        {
            dbwPane[index].flags = DPF_Enabled;
            sscanf(returnString, "%d, %d, %d, %d, %d", &dbwPane[index].x,
                    &dbwPane[index].y, &dbwPane[index].width,
                    &dbwPane[index].height, &dbwPane[index].bufferHeight);
        }

        if (dbwPaneAlloc(index) != OKAY)                    //allocate mem for pane
        {
            bitClear(dbwPane[index].flags, DPF_Enabled);
            continue;
        }
        dbwPaneClear(index);                                //clear the pane out
        dbwPane[index].logFile = NULL;                      //by default, logging is off
    }

//    testItOut(); //!!!
    return OKAY;
}

/*-----------------------------------------------------------------------------
    Name        : dbwWriteOptions
    Description : write the debug window's options to the configuration file
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void dbwWriteOptions(void)
{
    sdword index;
    ubyte *data;
    FILE *f;

    if (!dbwEnabled)
    {
        return;
    }

    f = fopen(DIS_FileName, "at");
    if (f != NULL)
    {
        //save location to .INI file
        fprintf(f, "%s    %d, %d\n", DIS_Location, dbwWindowX, dbwWindowY);
//        WritePrivateProfileString(DIS_SectionName, DIS_Location,
//                                  string, DIS_FileName);

        //save size to .INI file
        fprintf(f, "%s    %d, %d\n", DIS_Size, dbwWindowWidth, dbwWindowHeight);
//        WritePrivateProfileString(DIS_SectionName, DIS_Size,
//                                  string, DIS_FileName);

        //save the font to .INI file
        fprintf(f, "%s    ", DIS_Font);
//        WritePrivateProfileStruct(DIS_SectionName, DIS_Font,
//                                  &dbwLogicalFont, sizeof(LOGFONT), DIS_FileName);
        data = (ubyte *)&dbwLogicalFont;
        for (index = 0; index < sizeof(LOGFONT); index++, data++)
        {
            fprintf(f, "%c%c", "0123456789ABCDEF"[((*data) & 0xf0) >> 4], "0123456789ABCDEF"[(*data) & 0x0f]);
        }
        fprintf(f, "\n");

        //save the pane information to the .INI file
        for (index = 0; index < DBW_NumberPanes; index++)
        {
            if (bitTest(dbwPane[index].flags, DPF_Enabled))
            {
//                sprintf(key, "%s%d", DIS_PaneBase, index);
                fprintf(f, "%s%d    %d, %d, %d, %d, %d\n\n", DIS_PaneBase, index,
                        dbwPane[index].x, dbwPane[index].y, dbwPane[index].width,
                        dbwPane[index].height, dbwPane[index].bufferHeight);
//                WritePrivateProfileString(DIS_SectionName, key, string, DIS_FileName);
            }
        }
        fclose(f);
    }
}

/*-----------------------------------------------------------------------------
    Name        : dbwClose
    Description : Close the debugging window by creating WM_DESTROY message.
    Inputs      : void
    Outputs     : void
    Return      : void
----------------------------------------------------------------------------*/
void dbwClose(void)
{
//    char string[DIS_StringLength];
//    char key[DIS_StringLength];

    if (!dbwEnabled)                                        //don't start if already started
    {
        return;
    }

    dbwAllPanesFree();                                      //free memory associated with these panes

    DeleteObject(hDebugFont);                               //delete the font
    ReleaseDC(hDebugWindow, hDebugDC);                      //done with the DC, release it
    DestroyWindow(hDebugWindow);                            //kill the window

    dbwEnabled = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : dbwPaneClear
    Description : Clears the current pane and moves cursor to top left.
    Inputs      : pane - index of pane to clear
    Outputs     : ..
    Return      : ..
----------------------------------------------------------------------------*/
void dbwPaneClear(sdword pane)
{
    sdword index;

    if (!dbwEnabled)                                        //don't start if already started
    {
        return;
    }

    if (dbwPaneOutRange(pane) || !dbwPaneEnabled(pane))
    {
        return;
    }
    dbwPane[pane].cursorX = 0;      //cursor to top left
    dbwPane[pane].viewTop = 0;                              //reset scroll-back buffer
    memset(dbwPane[pane].buffer, ' ', dbwPane[pane].bufferHeight * dbwPane[pane].width);
    for (index = 0; index < dbwPane[pane].height; index++)
    {
        dbwPane[pane].cursorY = index;
        dbwCharsPrint(pane, dbwPane[pane].buffer, dbwPane[pane].width, FALSE);
    }
    dbwPane[pane].cursorY = 0;
}

/*-----------------------------------------------------------------------------
    Name        : dbwLineFeed
    Description : Line feed the pane, possibly causeing scrolling.
    Inputs      : pane - index of pane to be line fed.
    Outputs     : ..
    Return      : TRUE if pane has scrolled
----------------------------------------------------------------------------*/
sdword dbwLineFeed(sdword pane)
{
    RECT clipRect, scrollRect, updateRect;
    sdword index;

    if (dbwPaneOutRange(pane) || !dbwPaneEnabled(pane))
    {
        return FALSE;
    }

    dbwPane[pane].cursorX = 0;                              //cursor to beggining of line

    dbwPane[pane].cursorY++;                            //increment cursor vertically

    if (dbwPane[pane].cursorY >= dbwPane[pane].viewTop + dbwPane[pane].height)
    {                                                       //if off bottom of view
        dbwPane[pane].viewTop++;                            //increment the view top index
        if (dbwPane[pane].viewTop + dbwPane[pane].height >= //if buffer needs scrolling
            dbwPane[pane].bufferHeight)
        {
            for (index = 0; index < dbwPane[pane].bufferHeight - 1; index++)
            {                                               //scroll the buffer
                memcpy(dbwPane[pane].buffer + index * dbwPane[pane].width,
                       dbwPane[pane].buffer + (index + 1) * dbwPane[pane].width,
                       dbwPane[pane].width);
            }
            memset(dbwPane[pane].buffer + dbwPane[pane].width * //clear the bottom line
                   (dbwPane[pane].bufferHeight - 1), ' ', dbwPane[pane].width);
            dbwPane[pane].viewTop = dbwPane[pane].bufferHeight - dbwPane[pane].height;
        }

        clipRect.left = clipRect.top = 0;                   //cliprect:full window
        clipRect.right = dbwWindowWidth * dbwFontWidth;
        clipRect.bottom = dbwWindowHeight * dbwFontHeight;

        scrollRect.left = dbwPane[pane].x * dbwFontWidth;   //rectangle to be scrolled (not including top line of pane)
        scrollRect.top = (dbwPane[pane].y + 1) * dbwFontHeight;
        scrollRect.right = (dbwPane[pane].x + dbwPane[pane].width) * dbwFontWidth;
        scrollRect.bottom = (dbwPane[pane].y + dbwPane[pane].height) * dbwFontHeight;

        if (dbwPane[pane].cursorY >= dbwPane[pane].bufferHeight)
        {                                                   //but not off bottom of buffer
            dbwPane[pane].cursorY = dbwPane[pane].bufferHeight - 1;
        }
                                                            //scroll the window
        taskStackSaveIf(1);                                 //save task stack context
        ScrollWindowEx(hDebugWindow, 0, -dbwFontHeight, &scrollRect, &clipRect, NULL, &updateRect, 0);
        taskStackRestoreIf();                               //restore task stack context
        dbwCharsPrint(pane, dbwPane[pane].buffer + dbwPane[pane].width * (dbwPane[pane].bufferHeight - 1), dbwPane[pane].width, FALSE);
//        ValidateRect(hDebugWindow, NULL);

    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : dbwPrint
    Description : Prints a string to the selected pane, processing newlines and
        carriage returns.
    Inputs      : pane - index of pane to print to
                  string - NULL terminated string to print
    Outputs     :
    Return      : ..
----------------------------------------------------------------------------*/
sdword dbwPrint(sdword pane, char *string)
{
    char *start, *current;

#if DBW_TO_FILE
    if (debugToFile)
    {
        logfileLog(DBW_FILE_NAME, string);
    }
#endif

    if (!dbwEnabled)                                        //don't start if already started
    {
        return(FALSE);
    }

    if (dbwPaneOutRange(pane) || !dbwPaneEnabled(pane))
    {
        return FALSE;
    }

    for (start = current = string; ; current++)
    {
        if (current - start + dbwPane[pane].cursorX >= dbwPane[pane].width - 1)//if this one will go off the edge of pane
        {
            dbwCharsPrint(pane, start, current - start, TRUE);
            start = current;
            dbwLineFeed(pane);
        }
        if (*current == '\r')                                //carriage-return
        {
            dbwCharsPrint(pane, start, current - start, TRUE);
            start = ++current;
            dbwPane[pane].cursorX = 0;                       //cursor back to start of line
        }
        if (*current == '\n')                                //line-feed
        {
            dbwCharsPrint(pane, start, current - start, TRUE);
            start = ++current;
            dbwLineFeed(pane);
        }
        if (*current == 0)                                  //NULL terminator
        {
            dbwCharsPrint(pane, start, current - start, TRUE);
            dbwPane[pane].cursorX += current - start;
            break;                                          //finished printing
        }
    }
    return(OKAY);
}

