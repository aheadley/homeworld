/*
** HWEXPORT.CPP : Code to export FEMan Data to .FIB files.  And that's no lie.
*/

#include "stdafx.h"
#include "assert.h"

#include "hwexport.h"
#include "FEManDoc.h"

#define nHotKeys            (79)

char *gHotKeyList[nHotKeys] =
{
    "LBUTTON",
    "RBUTTON",
    "CANCEL",
    "MBUTTON",
    "BACK",
    "TAB",
    "CLEAR",
    "RETURN",
    "SHIFT",
    "CONTROL",
    "MENU",
    "PAUSE",
    "CAPITAL",
    "ESCAPE",
    "SPACE",
    "PRIOR",
    "NEXT",
    "END",
    "HOME",
    "LEFT",
    "UP",
    "RIGHT",
    "DOWN",
    "SELECT",
    "PRINT",
    "EXECUTE",
    "SNAPSHOT",
    "INSERT",
    "DELETE",
    "HELP",
    "NUMPAD0",
    "NUMPAD1",
    "NUMPAD2",
    "NUMPAD3",
    "NUMPAD4",
    "NUMPAD5",
    "NUMPAD6",
    "NUMPAD7",
    "NUMPAD8",
    "NUMPAD9",
    "MULTIPLY",
    "ADD",
    "SEPARATOR",
    "SUBTRACT",
    "DECIMAL",
    "DIVIDE",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "NUMLOCK",
    "SCROLL",
    "LSHIFT",
    "RSHIFT",
    "LCONTROL",
    "RCONTROL",
    "LMENU",
    "RMENU",
    "TILDE",
};

int gHotKeyTable[nHotKeys] =
{
    VK_LBUTTON,
    VK_RBUTTON,
    VK_CANCEL,
    VK_MBUTTON,
    VK_BACK,
    VK_TAB,
    VK_CLEAR,
    VK_RETURN,
    VK_SHIFT,
    VK_CONTROL,
    VK_MENU,
    VK_PAUSE,
    VK_CAPITAL,
    VK_ESCAPE,
    VK_SPACE,
    VK_PRIOR,
    VK_NEXT,
    VK_END,
    VK_HOME,
    VK_LEFT,
    VK_UP,
    VK_RIGHT,
    VK_DOWN,
    VK_SELECT,
    VK_PRINT,
    VK_EXECUTE,
    VK_SNAPSHOT,
    VK_INSERT,
    VK_DELETE,
    VK_HELP,
    VK_NUMPAD0,
    VK_NUMPAD1,
    VK_NUMPAD2,
    VK_NUMPAD3,
    VK_NUMPAD4,
    VK_NUMPAD5,
    VK_NUMPAD6,
    VK_NUMPAD7,
    VK_NUMPAD8,
    VK_NUMPAD9,
    VK_MULTIPLY,
    VK_ADD,
    VK_SEPARATOR,
    VK_SUBTRACT,
    VK_DECIMAL,
    VK_DIVIDE,
    VK_F1,
    VK_F2,
    VK_F3,
    VK_F4,
    VK_F5,
    VK_F6,
    VK_F7,
    VK_F8,
    VK_F9,
    VK_F10,
    VK_F11,
    VK_F12,
    VK_F13,
    VK_F14,
    VK_F15,
    VK_F16,
    VK_F17,
    VK_F18,
    VK_F19,
    VK_F20,
    VK_F21,
    VK_F22,
    VK_F23,
    VK_F24,
    VK_NUMLOCK,
    VK_SCROLL,
    VK_LSHIFT,
    VK_RSHIFT,
    VK_LCONTROL,
    VK_RCONTROL,
    VK_LMENU,
    VK_RMENU,
    0xc0,
};



SaveFIBFormat::SaveFIBFormat(void)
{
    pFIBFileHeader = 0;
    pFIBScreens = 0;
    pFIBLinks = 0;
    pFIBAtoms = 0;
}

SaveFIBFormat::~SaveFIBFormat(void)
{
    unsigned long i;
    
    for( i=0 ; i<pFIBFileHeader->nScreens ; i++ )
    {
        delete [] pFIBLinks[i];

        delete [] pFIBAtoms[i];
    }
    
    delete [] pFIBLinks;

    delete [] pFIBAtoms;
    
    delete [] pFIBScreens;
    
    delete pFIBFileHeader;
}

unsigned char SaveFIBFormat::GetAtomType(ScreenObject *pObject)
{
    unsigned char type = 0;
    char *pData, *pTemp;
    char pDelimiter[] = {' ', 0x0D, 0x0A, 0x00};
    
    if(!pObject->GetMyUserInfo())
        return(0);

    pData = new char [strlen(pObject->GetMyUserInfo()) + 1];

    strcpy(pData, pObject->GetMyUserInfo());
    
    pTemp = strtok(pData, pDelimiter);

    while(pTemp)
    {
        strupr(pTemp);
        
        if(!strcmp("USERREGION", pTemp))
        {
            type = FA_UserRegion;

            break;
        }

        if(!strcmp("STATICTEXT", pTemp))
        {
            type = FA_StaticText;

            break;
        }

        if(!strcmp("BUTTON", pTemp))
        {
            type = FA_Button;

            break;
        }

        if(!strcmp("CHECKBOX", pTemp))
        {
            type = FA_CheckBox;

            break;
        }

        if(!strcmp("TOGGLEBUTTON", pTemp))
        {
            type = FA_ToggleButton;

            break;
        }

        if(!strcmp("DRAGBUTTON", pTemp))
        {
            type = FA_DragButton;

            break;
        }

        if(!strcmp("SCROLLBAR", pTemp))
        {
            type = FA_ScrollBar;

            break;
        }

        if(!strcmp("STATUSBAR", pTemp))
        {
            type = FA_StatusBar;

            break;
        }

        if(!strcmp("TEXTENTRY", pTemp))
        {
            type = FA_TextEntry;

            break;
        }

        if(!strcmp("LISTVIEWEXPANDBUTTON", pTemp))
        {
            type = FA_ListViewExpandButton;

            break;
        }

        if(!strcmp("TITLEBAR", pTemp))
        {
            type = FA_TitleBar;

            break;
        }

        if(!strcmp("MENUITEM", pTemp))
        {
            type = FA_MenuItem;

            break;
        }

        if(!strcmp("RADIOBUTTON", pTemp))
        {
            type = FA_RadioButton;

            break;
        }

        if(!strcmp("CUTOUTREGION", pTemp))
        {
            type = FA_CutoutRegion;

            break;
        }

        if(!strcmp("DECORATIVEREGION", pTemp))
        {
            type = FA_DecorativeRegion;

            break;
        }

        if(!strcmp("OPAQUEDECORATIVEREGION", pTemp))
        {
            type = FA_OpaqueDecorativeRegion;

            break;
        }

        if(!strcmp("DIVIDER", pTemp))
        {
            type = FA_Divider;

            break;
        }

        if(!strcmp("LISTWINDOW", pTemp))
        {
            type = FA_ListWindow;

            break;
        }

        if (!strcmp("BITMAPBUTTON",pTemp))
        {
            type = FA_BitmapButton;

            break;
        }

        if (!strcmp("HSLIDER",pTemp))
        {
            type = FA_HorizSlider;

            break;
        }

        if (!strcmp("VSLIDER",pTemp))
        {
            type = FA_VertSlider;

            break;
        }

        pTemp = strtok(NULL, pDelimiter);
    }

    delete [] pData;
    
    return(type);
}

char *SaveFIBFormat::GetAtomData(ScreenObject *pObject)
{
    char *pData, *pTemp, *pRet = 0;
    char pDelimiter[] = {' ', 0x0D, 0x0A, '(', ')', 0x00};
    
    if(!pObject->GetMyUserInfo())
        return((char *)0);

    pData = new char [strlen(pObject->GetMyUserInfo()) + 1];

    strcpy(pData, pObject->GetMyUserInfo());
    
    pTemp = strtok(pData, pDelimiter);

    while(pTemp)
    {
        strupr(pTemp);

        if(!strcmp("LINK", pTemp))
        {
            pTemp = strtok(NULL, pDelimiter);
            
            assert(pTemp);

            pRet = new char [strlen(pTemp) + 1];

            strcpy(pRet, pTemp);

            break;
        }

        if(!strcmp("FUNCTION", pTemp))
        {
            pTemp = strtok(NULL, pDelimiter);
            
            assert(pTemp);

            pRet = new char [strlen(pTemp) + 1];

            strcpy(pRet, pTemp);

            break;
        }

        if(!strcmp("TEXTUREMAP", pTemp))
        {
            pTemp = strtok(NULL, pDelimiter);

            assert(pTemp);

            pRet = new char [strlen(pTemp) + 1];

            strcpy(pRet, pTemp);

            break;
        }

        pTemp = strtok(NULL, pDelimiter);
    }

    delete [] pData;
    
    return(pRet);
}

char *SaveFIBFormat::GetBitmapName(ScreenObject *pObject)
{
    char *pData, *pTemp, *pRet = 0;
    char pDelimiter[] = {' ', 0x0D, 0x0A, '(', ')', 0x00};
    
    if(!pObject->GetMyUserInfo())
        return((char *)0);

    pData = new char [strlen(pObject->GetMyUserInfo()) + 1];

    strcpy(pData, pObject->GetMyUserInfo());
    
    pTemp = strtok(pData, pDelimiter);

    while(pTemp)
    {
        strupr(pTemp);

        if(!strcmp("BITMAPNAME", pTemp))
        {
            pTemp = strtok(NULL, pDelimiter);

            assert(pTemp);

            pRet = new char [strlen(pTemp) + 1];

            strcpy(pRet, pTemp);

            break;
        }

        pTemp = strtok(NULL, pDelimiter);
    }

    delete [] pData;
    
    return(pRet);
}


unsigned long SaveFIBFormat::GetAtomFlags(ScreenObject *pObject)
{
    unsigned long flags = 0;
    char *pData, *pTemp;
    char pDelimiter[] = {' ', 0x0D, 0x0A, '(', ')', 0x00};
    
    if(!pObject->GetMyUserInfo())
        return(0);

    pData = new char [strlen(pObject->GetMyUserInfo()) + 1];

    strcpy(pData, pObject->GetMyUserInfo());
    
    pTemp = strtok(pData, pDelimiter);

    while(pTemp)
    {
        strupr(pTemp);

        if(!strcmp("LINK", pTemp))
        {
            bitSet(flags, FAF_Link);
        }

        if(!strcmp("FUNCTION", pTemp))
        {
            bitSet(flags, FAF_Function);
        }

        if(!strcmp("MODAL", pTemp))
        {
            bitSet(flags, FAF_Modal);
        }

        if(!strcmp("POPUP", pTemp))
        {
            bitSet(flags, FAF_Popup);
        }

        if(!strcmp("ALWAYSONTOP", pTemp))
        {
            bitSet(flags, FAF_AlwaysOnTop);
        }

        if(!strcmp("DRAGGABLE", pTemp))
        {
            bitSet(flags, FAF_Draggable);
        }

        if(!strcmp("CALLONCREATE", pTemp))
        {
            bitSet(flags, FAF_CallOnCreate);
        }

        if(!strcmp("DEFAULTOK", pTemp))
        {
            bitSet(flags, FAF_DefaultOK);
        }
        
        if(!strcmp("DEFAULTBACK", pTemp))
        {
            bitSet(flags, FAF_DefaultBack);
        }

        if (!strcmp("DISABLED", pTemp))
        {
            bitSet(flags, FAF_Disabled);
        }

        if (!strcmp("DONTCUTOUTBASE", pTemp))
        {
            bitSet(flags, FAF_DontCutoutBase);
        }

        if (!strcmp("CALLONDELETE", pTemp))
        {
            bitSet(flags, FAF_CallOnDelete);
        }

        if (!strcmp("BACKGROUND", pTemp))
        {
            bitSet(flags, FAF_Background);
        }

        if (!strcmp("DROPSHADOW", pTemp))
        {
            int index;
            static char *cardinals[] =
            {
                "N",
                "S",
                "E",
                "W",
                "NE",
                "NW",
                "SE",
                "SW"
            };
            pTemp = strtok(NULL, pDelimiter);
            for (index = 0; index < 8; index++)
            {
                if (!strcmp(cardinals[index], pTemp))
                {
                    flags = (flags & (~FAM_DropShadow)) + ((1 << index) << FSB_DropShadow);
                    break;
                }
            }
        }

        pTemp = strtok(NULL, pDelimiter);
    }

    delete [] pData;
    
    return(flags);
}

unsigned long SaveFIBFormat::ParseClickSounds(ScreenObject *pObject)
{
    char *pData, *pTemp;
    char pDelimiter[] = {' ', 0x0D, 0x0A, '(', ')', 0x00};
    unsigned long iRet=0;
    
    if(!pObject->GetMyUserInfo())
        return(iRet);

    pData = new char [strlen(pObject->GetMyUserInfo()) + 1];

    strcpy(pData, pObject->GetMyUserInfo());
    
    pTemp = strtok(pData, pDelimiter);

    while(pTemp)
    {
        strupr(pTemp);

        if(!strcmp("CLICK", pTemp))
        {
            pTemp = strtok(NULL, pDelimiter);
            
            assert(pTemp);

            iRet = atoi(pTemp);

            break;
        }

        pTemp = strtok(NULL, pDelimiter);
    }

    delete [] pData;
    
    return(iRet);
}

//get a radio button's index
unsigned long SaveFIBFormat::GetRadioIndex(ScreenObject *pObject)
{
    char *pData, *pTemp;
    char pDelimiter[] = {' ', 0x0D, 0x0A, '(', ')', 0x00};
    unsigned long iRet=0;
    
    if(!pObject->GetMyUserInfo())
        return(iRet);

    pData = new char [strlen(pObject->GetMyUserInfo()) + 1];

    strcpy(pData, pObject->GetMyUserInfo());
    
    pTemp = strtok(pData, pDelimiter);

    while(pTemp)
    {
        strupr(pTemp);

        if(!strcmp("BUTTONNUMBER", pTemp))
        {
            pTemp = strtok(NULL, pDelimiter);
            
            assert(pTemp);

            iRet = atoi(pTemp);

            break;
        }

        pTemp = strtok(NULL, pDelimiter);
    }

    delete [] pData;
    
    return(iRet);
}

static struct
{
    unsigned long language;
    char *name;
}
languageNameTable[] =
{
    {0, "English"},
    {0, "Eng"},
    {1, "French"},
    {1, "Francais"},
    {1, "Fr"},
    {2, "German"},
    {2, "Deutch"},
    {2, "De"},
    {2, "Gr"},
    {3, "Spanish"},
    {3, "Es"},
    {4, "Italian"},
    {4, "It"},
/*    {5, "Sanskrit"},
    {5, "Sk"},*/

    {0, NULL}
};

static struct
{
    char modifier;
    char *name;
}
hotkeyModifierString[] =
{
    {HKM_Control, "Control"},
    {HKM_Control, "Ctrl"},
    {HKM_Alt, "Alt"},
    {HKM_Shift, "Shift"},
    {HKM_Shift, "Shft"},
    {HKM_Control | HKM_Alt, "ControlAlt"},
    {HKM_Control | HKM_Alt, "CtrlAlt"},
    {HKM_Control | HKM_Shift, "ControlShift"},
    {HKM_Control | HKM_Shift, "CtrlShft"},
    {HKM_Control | HKM_Shift, "ControlShift"},
    {HKM_Alt | HKM_Shift, "ShiftAlt"},
    {HKM_Alt | HKM_Shift, "AltShift"},
    {HKM_Alt | HKM_Shift, "ShftAlt"},
    {HKM_Alt | HKM_Shift, "AltShft"},
    {HKM_Control | HKM_Alt | HKM_Shift, "ControlShiftAlt"},
    {HKM_Control | HKM_Alt | HKM_Shift, "CtrlShftAlt"},
    {0, NULL}
};
// parses the accelerator keys for the front end buttons
void SaveFIBFormat::ParseAccelKeys(ScreenObject *pObject, char *buf, char *modifier)
{
    char *pData, *pTemp, pHotKeys[256];
    char pDelimiter[] = ", \t+";
    unsigned long i, language;
    
    *modifier = 0;
    memset(buf, 0x00, NUMBER_LANGUAGES);

    if(!pObject->GetMyUserInfo())
        return;

    pData = new char [strlen(pObject->GetMyUserInfo()) + 1];

    strcpy(pData, pObject->GetMyUserInfo());
    strupr(pData);

    pTemp = strstr(pData, "HOTKEY");
    while (pTemp != NULL)
    {
        // isolate the string within the brackets after the "HOTKEY" string
        if (pTemp[strlen("HOTKEY")] == '(' && strchr(pTemp, ')'))
        {
            int length = strchr(pTemp, ')') - pTemp - strlen("HOTKEY(");
            char *pThisKey;
            strncpy(pHotKeys, pTemp + strlen("HOTKEY("), length);
            pHotKeys[length] = 0;
            pThisKey = strtok(pHotKeys, pDelimiter);
            language = 0;
            //... now parse the string
            while (pThisKey)
            {
                if(strlen(pThisKey) > 1)
                {
                    // Language-specific modifier
                    for (i = 0; languageNameTable[i].name != NULL; i++)
                    {
                        if (_stricmp(pThisKey, languageNameTable[i].name) == 0)
                        {
                            language = languageNameTable[i].language;
                            goto foundLanguage;
                        }
                    }
                    for (i = 0; hotkeyModifierString[i].name != NULL; i++)
                    {
                        if (_stricmp(pThisKey, hotkeyModifierString[i].name) == 0)
                        {
                            *modifier |= hotkeyModifierString[i].modifier;
                            goto foundModifer;
                        }
                    }
                    // Extended character.
                    for( i=0 ; i<nHotKeys ; i++)
                    {
                        if(!_stricmp(pThisKey, gHotKeyList[i]))
                        {
                            buf[language] = gHotKeyTable[i];
                            goto foundKey;
                        }
                    }
                }
                else
                {
                    // Single character.
                    buf[language] = pThisKey[0];
                    goto foundKey;
                }
foundLanguage:
foundModifer:
                pThisKey = strtok(NULL, pDelimiter);
            }
foundKey:;
//          delete [] pHotKeys;
        }
        pTemp = strstr(pTemp + 1, "HOTKEY");
    }
    delete [] pData;
    return;
}


// Parse Miscellaneous attributes
// Parses the tabstop attribute
void SaveFIBFormat::ParseMisc(ScreenObject *pObject, FIBAtom *buf)
{
    char *pData, *pTemp;
    char pDelimiter[] = {' ', 0x0D, 0x0A, '(', ')', ',', 0x00};
    unsigned long   temp;
    
    buf->drawstyle[0] = (unsigned long)MediumTextures;
    buf->drawstyle[1] = (unsigned long)ExternalGlow;

    if(!pObject->GetMyUserInfo())
    {
        return;
    }

    pData = new char [strlen(pObject->GetMyUserInfo()) + 1];

    strcpy(pData, pObject->GetMyUserInfo());
    
    pTemp = strtok(pData, pDelimiter);

    while(pTemp)
    {
        strupr(pTemp);

        if(!strcmp("TABSTOP", pTemp))
        {
            pTemp = strtok(NULL, pDelimiter);
            
            assert(pTemp);

            sscanf(pTemp, "%u", &temp);

            buf->tabstop = (unsigned short)temp;
        }

        if(!strcmp("SMALLTEXTURES", pTemp))
        {
            buf->drawstyle[0] = (unsigned long)SmallTextures;
        }

        if(!strcmp("LARGETEXTURES", pTemp))
        {
            buf->drawstyle[0] = (unsigned long)LargeTextures;
        }

        if(!strcmp("INTERNALGLOW", pTemp))
        {
            buf->drawstyle[1] = (unsigned long)InternalGlow;
        }

        if(!strcmp("NOGLOW", pTemp))
        {
            buf->drawstyle[1] = (unsigned long)NoGlow;
        }



        pTemp = strtok(NULL, pDelimiter);
    }

    delete [] pData;
    
    return;
}


char *SaveFIBFormat::FixFontName(ScreenObjectString *sos)
{
    char *pRet, pPtSize[10];
    unsigned long lenPRet = 0;
    char bold = 0, underline = 0, strikeout = 0, italics = 0;
    LOGFONT *tFont;
    unsigned long i;

    tFont = sos->GetMyFont();

    pRet = new char [strlen("default.hff") + 1];

    strcpy(pRet, "default.hff");

    if(!sos->GetFontName())
        return(pRet);

    if(abs(tFont->lfHeight) < 1)
        return(pRet);

    delete [] pRet;

    lenPRet += strlen(sos->GetFontName()) + 4;

    // Add underscore.
    lenPRet ++;

    // Add bold?
    if(tFont->lfWeight == 700)
    {
        lenPRet ++;
        bold = 1;
    }

    if(tFont->lfUnderline)
    {
        lenPRet ++;
        underline = 1;
    }

    if(tFont->lfStrikeOut)
    {
        lenPRet ++;
        strikeout = 1;
    }

    if(tFont->lfItalic)
    {
        lenPRet ++;
        italics = 1;
    }

    // Add point size.
    lenPRet += strlen(itoa(abs(tFont->lfHeight), pPtSize, 10));

    pRet = new char [lenPRet + 1];

    strcpy(pRet, sos->GetFontName());

    strcat(pRet, "_");

    if(bold)
    {
        strcat(pRet, "b");
    }

    if(underline)
    {
        strcat(pRet, "u");
    }

    if(strikeout)
    {
        strcat(pRet, "s");
    }

    if(italics)
    {
        strcat(pRet, "i");
    }

    strcat(pRet, pPtSize);

    strcat(pRet, ".hff");

    for(i=0;i<strlen(pRet);i++)
    {
        if(pRet[i] == ' ')
            pRet[i] = '_';
    }

    return(pRet);
}

void SaveFIBFormat::ParseFEManData(CFEManDoc *pDocument)
{
    pFIBFileHeader = new FIBFileHeader;
    unsigned long pad=0;
    Screen *pWorkScreen;
    ScreenLink *pWorkLink;
    ScreenObject *pWorkObject;
    ScreenObjectRect *pWorkObjectRect;
    ScreenObjectString *pWorkObjectString;
    CRect *tpBounds;
    unsigned long i, j, k, dataOffset, nActiveLinks, totalStringLength;

    char *tempPtr;

    strcpy(pFIBFileHeader->identify, FIBIDENTIFY);

    pFIBFileHeader->version = FIBVERSION;

    pFIBFileHeader->nScreens = (unsigned short)pDocument->GetNumScreens();

    dataOffset = sizeof(FIBFileHeader) + pFIBFileHeader->nScreens * sizeof(FIBScreen);
    assert((dataOffset&0x03)==0);
    
    //////////////////
    //////////////////

    pFIBScreens = new FIBScreen [pFIBFileHeader->nScreens];
    pFIBLinks = new FIBLink * [pFIBFileHeader->nScreens];
    pFIBAtoms = new FIBAtom * [pFIBFileHeader->nScreens];

    pWorkScreen = pDocument->GetFirstScreen();
    i = 0;

    while(pWorkScreen)
    {
        pFIBLinks[i] = new FIBLink [pWorkScreen->GetNumScreenLinks()];

        pWorkLink = pWorkScreen->GetFirstScreenLink();
        j = 0;
        nActiveLinks = 0;

        while(pWorkLink)
        {
            if(!pWorkLink->LinkedTo())
            {
                pWorkLink = (ScreenLink *)pWorkLink->GetNext();

                continue;
            }

            pFIBLinks[i][j].oName = 0; // Set this up below.

            pFIBLinks[i][j].flags = 0;
            
            if(pWorkLink->GetType() == ScreenLink::SLT_DEFAULT_PREV)
                bitSet(pFIBLinks[i][j].flags, FL_DefaultPrev);

            if(pWorkLink->GetType() == ScreenLink::SLT_DEFAULT_NEXT)
                bitSet(pFIBLinks[i][j].flags, FL_DefaultNext);

            pFIBLinks[i][j].oLinkedToName = 0; // Set this up below.
            bitSet(pFIBLinks[i][j].flags, FL_Enabled);
            
            pWorkLink = (ScreenLink *)pWorkLink->GetNext();
            j++;
            nActiveLinks ++;
        }

        //////////////////
        //////////////////

        pFIBAtoms[i] = new FIBAtom [pWorkScreen->GetNumScreenObjects()];

        pWorkObject = pWorkScreen->GetFirstScreenObject();
        j = 0;

        while(pWorkObject)
        {
            pFIBAtoms[i][j].tabstop = 0;  //default value
            pFIBAtoms[i][j].oName = 0; // Set this up below.
            pFIBAtoms[i][j].oData = 0; // Set this up later.
            pFIBAtoms[i][j].type = GetAtomType(pWorkObject);

            // None defined yet.
            pFIBAtoms[i][j].flags = GetAtomFlags(pWorkObject);

            tpBounds = pWorkObject->GetBounds();
            pFIBAtoms[i][j].left = tpBounds->left;
            pFIBAtoms[i][j].top = tpBounds->top;
            pFIBAtoms[i][j].right = tpBounds->right;
            pFIBAtoms[i][j].bottom = tpBounds->bottom;

            switch(pWorkObject->GetType())
            {
            case ScreenObject::SOT_SCREENOBJECTRECT:
                pWorkObjectRect = (ScreenObjectRect *)pWorkObject;
                if (pWorkObjectRect->GetContentType() != ScreenObjectRect::CT_NONE)
                {
                    bitSet(pFIBAtoms[i][j].flags, FAF_ContentsVisible);
                }
                if (pWorkObjectRect->GetBorderType() != ScreenObjectRect::BT_NONE)
                {
                    bitSet(pFIBAtoms[i][j].flags, FAF_BorderVisible);
                }

                pFIBAtoms[i][j].contentColor = (unsigned long)pWorkObjectRect->GetContentColor()|(unsigned long)0xff000000;
                pFIBAtoms[i][j].borderColor = (unsigned long)pWorkObjectRect->GetBorderColor()|(unsigned long)0xff000000;
                pFIBAtoms[i][j].borderWidth = (unsigned char)pWorkObjectRect->GetBorderWidth();

                // Set up the attribs member with the click index.
                pFIBAtoms[i][j].attribs = ParseClickSounds(pWorkObject);

                // Set up the accelerator member with the keys.
                ParseAccelKeys(pWorkObject, pFIBAtoms[i][j].hotKey, &pFIBAtoms[i][j].hotKeyModifiers);

                // Parse other items (tabstops)
                ParseMisc(pWorkObject, &pFIBAtoms[i][j]);
                break;

            case ScreenObject::SOT_SCREENOBJECTBITMAP:
                pWorkObjectRect = (ScreenObjectRect *)pWorkObject;

                if (pWorkObjectRect->GetBorderType() != ScreenObjectRect::BT_NONE)
                {
                    bitSet(pFIBAtoms[i][j].flags, FAF_BorderVisible);
                }
                pFIBAtoms[i][j].contentColor = 0;
                pFIBAtoms[i][j].borderColor = 0;

                bitSet(pFIBAtoms[i][j].flags, FAF_Bitmap);

                // Set up the attribs member with the click index.
                pFIBAtoms[i][j].attribs = ParseClickSounds(pWorkObject);

                // Set up the accelerator member with the keys.
                ParseAccelKeys(pWorkObject, pFIBAtoms[i][j].hotKey, &pFIBAtoms[i][j].hotKeyModifiers);

                // Parse other items (tabstops)
                ParseMisc(pWorkObject, &pFIBAtoms[i][j]);
                break;

            case ScreenObject::SOT_SCREENOBJECTSTRING:
                pWorkObjectString = (ScreenObjectString *)pWorkObject;

                if (pWorkObjectRect->GetContentType() != ScreenObjectRect::CT_NONE)
                {
                    bitSet(pFIBAtoms[i][j].flags, FAF_ContentsVisible);
                }
                if (pWorkObjectRect->GetBorderType() != ScreenObjectRect::BT_NONE)
                {
                    bitSet(pFIBAtoms[i][j].flags, FAF_BorderVisible);
                }

                pFIBAtoms[i][j].contentColor = pWorkObjectString->GetTextColor()|0xff000000;
                pFIBAtoms[i][j].borderColor = pWorkObjectString->GetTextBackground()|0xff000000;
                break;
            }

            pWorkObject = (ScreenObject *)pWorkObject->GetNext();
            j ++;
        }

        pFIBScreens[i].oName = dataOffset;
        dataOffset += strlen(pWorkScreen->GetName()) + 1;
        pad = (strlen(pWorkScreen->GetName()) + 1)&0x03;
        if (pad != 0) dataOffset += (4-pad);
        assert((dataOffset&0x03)==0);

        pFIBScreens[i].flags = 0; // None yet.
        pFIBScreens[i].nLinks = (unsigned short)nActiveLinks;
        pFIBScreens[i].nAtoms = (unsigned short)pWorkScreen->GetNumScreenObjects();

        pFIBScreens[i].oLinks = dataOffset;
        dataOffset += sizeof(FIBLink) * pFIBScreens[i].nLinks;
        assert((dataOffset&0x03)==0);
        
        pWorkLink = pWorkScreen->GetFirstScreenLink();

        for( j=0 ; j<pFIBScreens[i].nLinks ; j++)
        {
            assert(pWorkLink);

            while(!pWorkLink->LinkedTo())
            {
                pWorkLink = (ScreenLink *)pWorkLink->GetNext();
            }

            pFIBLinks[i][j].oName = dataOffset;

            dataOffset += strlen(pWorkLink->GetName()) + 1;
            pad = (strlen(pWorkLink->GetName()) + 1)&0x03;
            if (pad != 0) dataOffset += (4-pad);
            assert((dataOffset&0x03)==0);

            Screen *pTempScreen = pDocument->FindScreenWithID(pWorkLink->GetLinkedToID());

            assert(pTempScreen);

            pFIBLinks[i][j].oLinkedToName = dataOffset;

            dataOffset += strlen(pTempScreen->GetName()) + 1;
            pad = (strlen(pTempScreen->GetName()) + 1)&0x03;
            if (pad != 0) dataOffset += (4-pad);
            assert((dataOffset&0x03)==0);

            pWorkLink = (ScreenLink *)pWorkLink->GetNext();
        }
        
        pFIBScreens[i].oAtoms = dataOffset;
        dataOffset += sizeof(FIBAtom) * pFIBScreens[i].nAtoms;
        assert((dataOffset&0x03)==0);

        pWorkObject = pWorkScreen->GetFirstScreenObject();

        for( j=0 ; j<pFIBScreens[i].nAtoms ; j++)
        {
            assert(pWorkObject);

            tempPtr = GetAtomData(pWorkObject);

            if(tempPtr)
            {
                pFIBAtoms[i][j].oName = dataOffset;

                dataOffset += strlen(tempPtr) + 1;
                pad = (strlen(tempPtr) + 1)&0x03;
                if (pad != 0) dataOffset += (4-pad);
                assert((dataOffset&0x03)==0);

                delete [] tempPtr;
            }

            switch(pWorkObject->GetType())
            {
            case ScreenObject::SOT_SCREENOBJECTRECT:
                // No Data here.
                pFIBAtoms[i][j].oData = 0;
                if (pFIBAtoms[i][j].type == FA_RadioButton)
                {
                    pFIBAtoms[i][j].oData = GetRadioIndex(pWorkObject);
                }
                // Check to see if it is a bitmapbutton
                if (pFIBAtoms[i][j].type == FA_BitmapButton)
                {
                    tempPtr = GetBitmapName(pWorkObject);

                    pFIBAtoms[i][j].attribs = dataOffset;

                    dataOffset += strlen(tempPtr) + 1;
                    pad = (strlen(tempPtr) + 1)&0x03;
                    if (pad != 0) dataOffset += (4-pad);
                    assert((dataOffset&0x03)==0);
                }

                break;

            case ScreenObject::SOT_SCREENOBJECTBITMAP:
                {
                    // Insert bitmap name.
                    ScreenObjectBitmap *sob = (ScreenObjectBitmap *)pWorkObject;
                    pFIBAtoms[i][j].oData = dataOffset;
                    dataOffset += strlen(sob->GetFileName()) + 1;
                    pad = (strlen(sob->GetFileName()) + 1)&0x03;
                    if (pad != 0) dataOffset += (4-pad);
                    assert((dataOffset&0x03)==0);
                }
                break;

            case ScreenObject::SOT_SCREENOBJECTSTRING:
                {
                // Insert text string.
                ScreenObjectString *sos = (ScreenObjectString *)pWorkObject;
                pFIBAtoms[i][j].oData = dataOffset;
                for (k = totalStringLength = 0; k < NUMBER_LANGUAGES_TO_SAVE; k++)
                {
                    dataOffset += strlen(sos->GetMyString(k)) + 1;
                    totalStringLength += strlen(sos->GetMyString(k)) + 1;
//                    pad = (strlen(sos->GetMyString(k)) + 1)&0x03;
//                    if (pad != 0) dataOffset += (4-pad);
//                    assert((dataOffset&0x03)==0);
                }
                pad = (totalStringLength)&0x03;
                if (pad != 0) dataOffset += (4-pad);
                assert((dataOffset&0x03)==0);

                // Set up the attribs member with an offset to the font name.
                char *tempFontName = FixFontName(sos);
                
                pFIBAtoms[i][j].attribs = dataOffset;

                dataOffset += strlen(tempFontName) + 1;
                pad = (strlen(tempFontName) + 1)&0x03;
                if (pad != 0) dataOffset += (4-pad);
                assert((dataOffset&0x03)==0);

                switch (sos->GetJustification())
                {
                    case JUST_LEFT:
                        bitSet(pFIBAtoms[i][j].flags, FAM_JustLeft);
                        break;
                    case JUST_RIGHT:
                        bitSet(pFIBAtoms[i][j].flags, FAM_JustRight);
                        break;
                    case JUST_CENTRE:
                        bitSet(pFIBAtoms[i][j].flags, FAM_JustCentre);
                        break;
                }

                delete [] tempFontName;

                // Clear the accelerator member.
                memset(pFIBAtoms[i][j].hotKey, 0x00, 4);
                pFIBAtoms[i][j].hotKeyModifiers = 0;
                }
                break;
            }
            
            pWorkObject = (ScreenObject *)pWorkObject->GetNext();
        }

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
        i ++;
    }
}

void SaveFIBFormat::WriteFEManData(CString &fileName, CFEManDoc *pDocument)
{
    FILE *fileHandle;
    unsigned long i, j, k, pad, totalStringLength;
    unsigned long zeros=0;
    Screen *pWorkScreen;
    ScreenLink *pWorkLink;
    ScreenObject *pWorkObject;
    char *tempPtr;

    fileHandle = fopen(fileName, "wb");

    assert(fileHandle);

    // Write the file Header.
    fwrite(pFIBFileHeader, sizeof(FIBFileHeader), 1, fileHandle);

    // Write the screen headers.
    fwrite(pFIBScreens, sizeof(FIBScreen), pFIBFileHeader->nScreens, fileHandle);

    pWorkScreen = pDocument->GetFirstScreen();

    for( i=0 ; i<pFIBFileHeader->nScreens ; i++ )
    {
        assert(pWorkScreen);

        // Write the screenName.
        fwrite(pWorkScreen->GetName(), 1, strlen(pWorkScreen->GetName()) + 1, fileHandle);
        pad = (strlen(pWorkScreen->GetName()) + 1)&0x03;
        if (pad != 0)
            fwrite(&zeros, 1, 4-pad, fileHandle);

        // Write the link headers.
        fwrite(pFIBLinks[i], sizeof(FIBLink), pFIBScreens[i].nLinks, fileHandle);

        pWorkLink = pWorkScreen->GetFirstScreenLink();

        for( j=0 ; j<pFIBScreens[i].nLinks ; j++)
        {
            while(!pWorkLink->LinkedTo())
            {
                pWorkLink = (ScreenLink *)pWorkLink->GetNext();
            }
            
            assert(pWorkLink);

//          fwrite(pWorkLink->GetName(), 1, strlen(pWorkLink->GetName()) + 1, fileHandle);

            fwrite(pWorkLink->GetName(), 1, strlen(pWorkLink->GetName()) + 1, fileHandle);
            pad = (strlen(pWorkLink->GetName()) + 1)&0x03;
            if (pad != 0)
                fwrite(&zeros, 1, 4-pad, fileHandle);
            
            Screen *pTempScreen = pDocument->FindScreenWithID(pWorkLink->GetLinkedToID());

            assert(pTempScreen);

            fwrite(pTempScreen->GetName(), 1, strlen(pTempScreen->GetName()) + 1, fileHandle);
            pad = (strlen(pTempScreen->GetName()) + 1)&0x03;
            if (pad != 0)
                fwrite(&zeros, 1, 4-pad, fileHandle);

//          fwrite(pTempScreen->GetName(), 1, strlen(pTempScreen->GetName()) + 1, fileHandle);

            pWorkLink = (ScreenLink *)pWorkLink->GetNext();
        }

        // Write the atom headers.
        fwrite(pFIBAtoms[i], sizeof(FIBAtom), pFIBScreens[i].nAtoms, fileHandle);

        pWorkObject = pWorkScreen->GetFirstScreenObject();
        
        for( j=0 ; j<pFIBScreens[i].nAtoms ; j++)
        {
            assert(pWorkObject);

            tempPtr = GetAtomData(pWorkObject);

            if(tempPtr)
            {
                fwrite(tempPtr, 1, strlen(tempPtr) + 1, fileHandle);
                pad = (strlen(tempPtr) + 1)&0x03;
                if (pad != 0)
                    fwrite(&zeros, 1, 4-pad, fileHandle);

                delete [] tempPtr;
            }

            switch(pWorkObject->GetType())
            {
            case ScreenObject::SOT_SCREENOBJECTRECT:
                // No Data here.
                // Check to see if it is a bitmapbutton
                if (pFIBAtoms[i][j].type == FA_BitmapButton)
                {
                    tempPtr = GetBitmapName(pWorkObject);

                    fwrite(tempPtr, 1, strlen(tempPtr) + 1, fileHandle);

                    pad = (strlen(tempPtr) + 1)&0x03;
                    if (pad != 0)
                        fwrite(&zeros, 1, 4-pad, fileHandle);
                }

                break;

            case ScreenObject::SOT_SCREENOBJECTBITMAP:
                {
                // Insert bitmap name.
                ScreenObjectBitmap *sob = (ScreenObjectBitmap *)pWorkObject;
                fwrite(sob->GetFileName(), 1, strlen(sob->GetFileName()) + 1, fileHandle);
                pad = (strlen(sob->GetFileName()) + 1)&0x03;
                if (pad != 0)
                    fwrite(&zeros, 1, 4-pad, fileHandle);
                }
                break;

            case ScreenObject::SOT_SCREENOBJECTSTRING:
                {
                // Insert text string.
                ScreenObjectString *sos = (ScreenObjectString *)pWorkObject;
                for (k = totalStringLength = 0; k < NUMBER_LANGUAGES_TO_SAVE; k++)
                {
                    fwrite(sos->GetMyString(k), 1, strlen(sos->GetMyString(k)) + 1, fileHandle);
                    totalStringLength += strlen(sos->GetMyString(k)) + 1;
                }
                pad = (totalStringLength)&0x03;
                if (pad != 0)
                    fwrite(&zeros, 1, 4-pad, fileHandle);

                char *tempFontName = FixFontName(sos);

                fwrite(tempFontName, 1, strlen(tempFontName) + 1, fileHandle);
                pad = (strlen(tempFontName) + 1)&0x03;
                if (pad != 0)
                    fwrite(&zeros, 1, 4-pad, fileHandle);

                delete [] tempFontName;
                }
                break;
            }

            pWorkObject = (ScreenObject *)pWorkObject->GetNext();
        }

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }

    fclose(fileHandle);
}