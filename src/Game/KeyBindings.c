/*=============================================================================
    Name    : KeyBindings.c
    Purpose : This file handles all of the logic for the key bindings

    Created 8/04/1999 by Drew Dunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Types.h"
#include "KeyBindings.h"
#include "UIControls.h"
#include "Key.h"
#include "FontReg.h"
#include "FEColour.h"
#include "Strings.h"

/*=============================================================================
    Defines :
=============================================================================*/


/*=============================================================================
    Data definitions :
=============================================================================*/

udword kbKeySavedKeys[20] =
{
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

kbBoundKeys *kbKeyTable = NULL;

kbBoundKeys kbKeyTableEnglish[kbTOTAL_COMMANDS] =
{  // command                 defaultkey      primary key     key to reset to
    { kbNEXT_FORMATION      , TABKEY        , TABKEY        , TABKEY        },
    { kbBUILD_MANAGER       , BKEY          , BKEY          , BKEY          },
    { kbPREVIOUS_FOCUS      , CKEY          , CKEY          , CKEY          },
    { kbNEXT_FOCUS          , VKEY          , VKEY          , VKEY          },
    { kbDOCK                , DKEY          , DKEY          , DKEY          },
    { kbSELECT_ALL_VISIBLE  , EKEY          , EKEY          , EKEY          },
    { kbFOCUS               , FKEY          , FKEY          , FKEY          },
    { kbRESEARCH_MANAGER    , RKEY          , RKEY          , RKEY          },
    { kbHARVEST             , HKEY          , HKEY          , HKEY          },
    { kbMOVE                , MKEY          , MKEY          , MKEY          },
    { kbNEXT_TACTIC         , RBRACK        , RBRACK        , RBRACK        },
    { kbPREVIOUS_TACTIC     , LBRACK        , LBRACK        , LBRACK        },
    { kbSCUTTLE             , SKEY          , SKEY          , SKEY          },
    { kbSHIP_SPECIAL        , ZKEY          , ZKEY          , ZKEY          },
    { kbTACTICAL_OVERLAY    , CAPSLOCKKEY   , CAPSLOCKKEY   , CAPSLOCKKEY   },
//    { kbSENSORS_MANAGER     , SPACEKEY      , SPACEKEY      , SPACEKEY      },
    { kbMOTHERSHIP          , HOMEKEY       , HOMEKEY       , HOMEKEY       },
    { kbKAMIKAZE            , KKEY          , KKEY          , KKEY          },
    { kbCANCEL_ORDERS       , TILDEKEY      , TILDEKEY      , TILDEKEY      },
    { kbLAUNCH_MANAGER      , LKEY          , LKEY          , LKEY          }
};

kbBoundKeys kbKeyTableFrench[kbTOTAL_COMMANDS] =
{  // command                 defaultkey      primary key     key to reset to
    { kbNEXT_FORMATION      , TABKEY        , TABKEY        , TABKEY        },
    { kbBUILD_MANAGER       , BKEY          , BKEY          , BKEY          },
    { kbPREVIOUS_FOCUS      , CKEY          , XKEY          , XKEY          },
    { kbNEXT_FOCUS          , VKEY          , VKEY          , VKEY          },
    { kbDOCK                , DKEY          , AKEY          , AKEY          },
    { kbSELECT_ALL_VISIBLE  , EKEY          , EKEY          , EKEY          },
    { kbFOCUS               , FKEY          , FKEY          , FKEY          },
    { kbRESEARCH_MANAGER    , RKEY          , RKEY          , RKEY          },
    { kbHARVEST             , HKEY          , CKEY          , CKEY          },
    { kbMOVE                , MKEY          , DKEY          , DKEY          },
    { kbNEXT_TACTIC         , RBRACK        , RBRACK        , RBRACK        },
    { kbPREVIOUS_TACTIC     , LBRACK        , LBRACK        , LBRACK        },
    { kbSCUTTLE             , SKEY          , SKEY          , SKEY          },
    { kbSHIP_SPECIAL        , ZKEY          , ZKEY          , ZKEY          },
    { kbTACTICAL_OVERLAY    , CAPSLOCKKEY   , CAPSLOCKKEY   , CAPSLOCKKEY   },
//    { kbSENSORS_MANAGER     , SPACEKEY      , SPACEKEY      , SPACEKEY      },
    { kbMOTHERSHIP          , HOMEKEY       , HOMEKEY       , HOMEKEY       },
    { kbKAMIKAZE            , KKEY          , KKEY          , KKEY          },
    { kbCANCEL_ORDERS       , TILDEKEY      , TILDEKEY      , TILDEKEY      },
    { kbLAUNCH_MANAGER      , LKEY          , LKEY          , LKEY          }
};

kbBoundKeys kbKeyTableGerman[kbTOTAL_COMMANDS] =
{  // command                 defaultkey      primary key     key to reset to
    { kbNEXT_FORMATION      , TABKEY        , TABKEY        , TABKEY        },
    { kbBUILD_MANAGER       , BKEY          , BKEY          , BKEY          },
    { kbPREVIOUS_FOCUS      , CKEY          , CKEY          , CKEY          },
    { kbNEXT_FOCUS          , VKEY          , VKEY          , VKEY          },
    { kbDOCK                , DKEY          , DKEY          , DKEY          },
    { kbSELECT_ALL_VISIBLE  , EKEY          , EKEY          , EKEY          },
    { kbFOCUS               , FKEY          , FKEY          , FKEY          },
    { kbRESEARCH_MANAGER    , RKEY          , RKEY          , RKEY          },
    { kbHARVEST             , HKEY          , HKEY          , HKEY          },
    { kbMOVE                , MKEY          , WKEY          , WKEY          },
    { kbNEXT_TACTIC         , RBRACK        , RBRACK        , RBRACK        },
    { kbPREVIOUS_TACTIC     , LBRACK        , LBRACK        , LBRACK        },
    { kbSCUTTLE             , SKEY          , SKEY          , SKEY          },
    { kbSHIP_SPECIAL        , ZKEY          , ZKEY          , ZKEY          },
    { kbTACTICAL_OVERLAY    , CAPSLOCKKEY   , CAPSLOCKKEY   , CAPSLOCKKEY   },
//    { kbSENSORS_MANAGER     , SPACEKEY      , SPACEKEY      , SPACEKEY      },
    { kbMOTHERSHIP          , HOMEKEY       , HOMEKEY       , HOMEKEY       },
    { kbKAMIKAZE            , KKEY          , KKEY          , KKEY          },
    { kbCANCEL_ORDERS       , TILDEKEY      , TILDEKEY      , TILDEKEY      },
    { kbLAUNCH_MANAGER      , LKEY          , LKEY          , LKEY          }
};

kbBoundKeys kbKeyTableSpanish[kbTOTAL_COMMANDS] =
{  // command                 defaultkey      primary key     key to reset to
    { kbNEXT_FORMATION      , TABKEY        , TABKEY        , TABKEY        },
    { kbBUILD_MANAGER       , BKEY          , BKEY          , BKEY          },
    { kbPREVIOUS_FOCUS      , CKEY          , CKEY          , CKEY          },
    { kbNEXT_FOCUS          , VKEY          , VKEY          , VKEY          },
    { kbDOCK                , DKEY          , AKEY          , AKEY          },
    { kbSELECT_ALL_VISIBLE  , EKEY          , EKEY          , EKEY          },
    { kbFOCUS               , FKEY          , FKEY          , FKEY          },
    { kbRESEARCH_MANAGER    , RKEY          , RKEY          , RKEY          },
    { kbHARVEST             , HKEY          , HKEY          , HKEY          },
    { kbMOVE                , MKEY          , DKEY          , DKEY          },
    { kbNEXT_TACTIC         , RBRACK        , RBRACK        , RBRACK        },
    { kbPREVIOUS_TACTIC     , LBRACK        , LBRACK        , LBRACK        },
    { kbSCUTTLE             , SKEY          , SKEY          , SKEY          },
    { kbSHIP_SPECIAL        , ZKEY          , ZKEY          , ZKEY          },
    { kbTACTICAL_OVERLAY    , CAPSLOCKKEY   , CAPSLOCKKEY   , CAPSLOCKKEY   },
//    { kbSENSORS_MANAGER     , SPACEKEY      , SPACEKEY      , SPACEKEY      },
    { kbMOTHERSHIP          , HOMEKEY       , HOMEKEY       , HOMEKEY       },
    { kbKAMIKAZE            , KKEY          , KKEY          , KKEY          },
    { kbCANCEL_ORDERS       , TILDEKEY      , TILDEKEY      , TILDEKEY      },
    { kbLAUNCH_MANAGER      , LKEY          , LKEY          , LKEY          }
};

kbBoundKeys kbKeyTableItalian[kbTOTAL_COMMANDS] =
{  // command                 defaultkey      primary key     key to reset to
    { kbNEXT_FORMATION      , TABKEY        , TABKEY        , TABKEY        },
    { kbBUILD_MANAGER       , BKEY          , BKEY          , BKEY          },
    { kbPREVIOUS_FOCUS      , CKEY          , CKEY          , CKEY          },
    { kbNEXT_FOCUS          , VKEY          , VKEY          , VKEY          },
    { kbDOCK                , DKEY          , AKEY          , AKEY          },
    { kbSELECT_ALL_VISIBLE  , EKEY          , EKEY          , EKEY          },
    { kbFOCUS               , FKEY          , FKEY          , FKEY          },
    { kbRESEARCH_MANAGER    , RKEY          , RKEY          , RKEY          },
    { kbHARVEST             , HKEY          , HKEY          , HKEY          },
    { kbMOVE                , MKEY          , MKEY          , MKEY          },
    { kbNEXT_TACTIC         , RBRACK        , RBRACK        , RBRACK        },
    { kbPREVIOUS_TACTIC     , LBRACK        , LBRACK        , LBRACK        },
    { kbSCUTTLE             , SKEY          , SKEY          , SKEY          },
    { kbSHIP_SPECIAL        , ZKEY          , ZKEY          , ZKEY          },
    { kbTACTICAL_OVERLAY    , CAPSLOCKKEY   , CAPSLOCKKEY   , CAPSLOCKKEY   },
//    { kbSENSORS_MANAGER     , SPACEKEY      , SPACEKEY      , SPACEKEY      },
    { kbMOTHERSHIP          , HOMEKEY       , HOMEKEY       , HOMEKEY       },
    { kbKAMIKAZE            , KKEY          , KKEY          , KKEY          },
    { kbCANCEL_ORDERS       , TILDEKEY      , TILDEKEY      , TILDEKEY      },
    { kbLAUNCH_MANAGER      , LKEY          , LKEY          , LKEY          }
};


kbBoundKeys kbKeySaveTable[kbTOTAL_COMMANDS];

// This is a look-up table that has a value of TRUE for
bool8 kbCanMapKey[KEY_TOTAL_KEYS];

// This is a look-up table for the strings
sdword kbKeyToString[KEY_TOTAL_KEYS];

// fonthandle for the keyboard list window font initialized in the options init
fonthandle kbKeyListFont;

// pointer to the listwindow handle
listwindowhandle kbListWindow=NULL;

bool bkDisableKeyRemap=FALSE;

/*=============================================================================
    Private Function Prototypes :
=============================================================================*/

void kbSetNewKey(udword keypressed);
bool kbKeyUsed(udword keytocheck);

/*=============================================================================
    Function Logic :
=============================================================================*/


/*-----------------------------------------------------------------------------
    Name        : kbListTitleDraw
    Description : This function draws the title for the keyboard list window
    Inputs      : Rectangle of draw area
    Outputs     : none
    Parameters  : rectangle *rect
    Return      : void
-----------------------------------------------------------------------------*/
void kbListTitleDraw(rectangle *rect)
{
    fonthandle oldfont;
    rectangle r = *rect;

    oldfont = fontMakeCurrent(kbKeyListFont);

    fontPrintf(rect->x0+6, rect->y0, colWhite, strGetString(strCommandToBind));
    fontPrintf(rect->x0 + (((rect->x1-rect->x0)*3)/5), rect->y0, colWhite, strGetString(strKeyBound));

    r.x0 -= 2;
    r.x1++;
    r.y1--;

    primRectTranslucent2(&r, colRGBA(0,160,100,63));
//    primLine2(r.x0,r.y1,r.x1,r.y1,colRGB(20,200,20));
//    primRectOutline2(&r, 1, colRGB(20,200,20));

    fontMakeCurrent(oldfont);
}


/*-----------------------------------------------------------------------------
    Name        : kbListItemDraw
    Description : This function draws each item in the list window
    Inputs      : rect, and the item pointer
    Outputs     : none
    Parameters  : rectangle *rect, listitemhandle data
    Return      : void
-----------------------------------------------------------------------------*/
void kbListItemDraw(rectangle *rect, listitemhandle data)
{
    fonthandle      oldfont;
    kbBoundKeys    *bkey = (kbBoundKeys *)data->data;
    color           c = FEC_ListItemStandard;

    oldfont = fontMakeCurrent(kbKeyListFont);

    // if selected then this color
    if (bitTest(data->flags, UICLI_Selected))
    {
        c = FEC_ListItemSelected;
    }

    if (bkey->status == KB_NotBound)
    {
        // if not bound then print in red
        c = colRGB(255,20,20);
    }
    else if (bkey->status == KB_GetKeyPress)
    {
        // if not bound then print in yellow color
        c = colRGB(255,255,20);
    }

    fontPrintf(rect->x0 + 6, rect->y0, c, strGetString(bkey->command + strKeyCommandOffset));

    if (bkey->status == KB_GetKeyPress)
    {
        // if getting keypress then show it with ????
        fontPrintf(rect->x0 + (((rect->x1-rect->x0)*3)/5), rect->y0, c, "??????");
    }
    else
    {
        if (bkey->primarykey == 0)
        {
            // no key assigned to this
            fontPrintf(rect->x0 + (((rect->x1-rect->x0)*3)/5), rect->y0, c, "%s", strGetString(strNoKeyBound));
        }
        else
        {
            // key assigned so print the name of that key
            dbgAssert(bkey->primarykey < KEY_TOTAL_KEYS);
            fontPrintf(rect->x0 + (((rect->x1-rect->x0)*3)/5), rect->y0, c, "%s", strGetString(kbKeyToString[bkey->primarykey]));
        }
    }

    fontMakeCurrent(oldfont);
}


/*-----------------------------------------------------------------------------
    Name        : kbListWindowCB
    Description : This function initializes the key bindings list window
    Inputs      : name, and atom
    Outputs     : none
    Parameters  : char *string, featom *atom
    Return      : void
-----------------------------------------------------------------------------*/
void kbListWindowCB(char *string, featom *atom)
{
    fonthandle oldfont;
    sdword     index;
    kbBoundKeys    *bkey;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(kbKeyListFont);
        kbListWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(kbListWindow,
                          kbListTitleDraw,                      //  title draw
                          NULL,                                 //  title click process, no title
                          fontHeight(" ")+(fontHeight(" ")>>1), //  title height, no title
                          kbListItemDraw,                       // item draw funtcion
                          fontHeight(" ")+(fontHeight(" ")>>1),       // item height
                          UICLW_CanSelect|UICLW_CanHaveFocus);

        for (index = 0; index < kbTOTAL_COMMANDS; index++)
        {
            uicListAddItem(kbListWindow, (ubyte *)&kbKeyTable[index], UICLI_CanSelect, UICLW_AddToHead);
        }

        fontMakeCurrent(oldfont);
        return;
    }
    else if (FELASTCALL(atom))
    {
        bitClear(kbListWindow->windowflags, UICLW_GetKeyPressed);

        for (index = 0; index < kbTOTAL_COMMANDS; index++)
        {
            if (kbKeyTable[index].primarykey == 0)
            {
                kbKeyTable[index].status = KB_NotBound;
            }
            else
            {
                kbKeyTable[index].status = KB_NormalBound;
            }
        }

        kbListWindow = NULL;
        return;
    }
    // General callbacks because of user input
    switch (kbListWindow->message)
    {
        case CM_AcceptText:
        case CM_DoubleClick:
            bkey = (kbBoundKeys *)kbListWindow->CurLineSelected->data;
            bitSet(kbListWindow->windowflags, UICLW_GetKeyPressed);
#ifdef DEBUG_STOMP
            regVerify((regionhandle)&kbListWindow->reg);
#endif
            bitSet(kbListWindow->reg.status, RSF_DrawThisFrame);
            bkey->status = KB_GetKeyPress;
            break;
        case CM_KeyCaptured:
            kbSetNewKey(kbListWindow->keypressed);
            break;
        case CM_LoseFocus:
        case CM_NewItemSelected:
            bitClear(kbListWindow->windowflags, UICLW_GetKeyPressed);
            for (index = 0; index < kbTOTAL_COMMANDS; index++)
            {
                if (kbKeyTable[index].primarykey == 0)
                {
                    kbKeyTable[index].status = KB_NotBound;
                }
                else
                {
                    kbKeyTable[index].status = KB_NormalBound;
                }
            }
            break;
    }
}


/*-----------------------------------------------------------------------------
    Name        : kbPoolListItemDraw
    Description : This function draws each item in the key pool list window
    Inputs      : rect, and the item pointer
    Outputs     : none
    Parameters  : rectangle *rect, listitemhandle data
    Return      : void
-----------------------------------------------------------------------------*/
void kbPoolListItemDraw(rectangle *rect, listitemhandle data)
{
    fonthandle      oldfont;
    color           c = FEC_ListItemStandard;
    sword           key1, key2;
    udword          udata = (udword)data->data;

    oldfont = fontMakeCurrent(kbKeyListFont);

    // unpack the two key's
    key1 = (sword) (udata&(0x0000FFFF));
    key2 = (sword) ((udata&(0xFFFF0000))>>16);

    // if selected then this color
    if (bitTest(data->flags, UICLI_Selected))
    {
        c = FEC_ListItemSelected;
    }

    if (key1 != -1)
    {
        fontPrintf(rect->x0 + 6, rect->y0, c, strGetString(kbKeyToString[key1]));
    }

    if (key2 != -1)
    {
        fontPrintf(rect->x0 + ( (rect->x1-rect->x0)>>1 ), rect->y0, c, strGetString(kbKeyToString[key2]));
    }

    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : kbPoolListWindowCB
    Description : This function Initializes the list of keys window
    Inputs      : keypressed
    Outputs     : none
    Parameters  : udword keypressed
    Return      : void
-----------------------------------------------------------------------------*/
void kbPoolListWindowCB(char *string, featom *atom)
{
    fonthandle          oldfont;
    static sdword       index;
    listwindowhandle    keypool;
    sdword              done;
    udword              data;
    sword               key1, key2;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(kbKeyListFont);
        keypool = (listwindowhandle)atom->pData;

        uicListWindowInit(keypool,
                          NULL,                      //  title draw
                          NULL,                      //  title click process, no title
                          0,                         //  title height, no title
                          kbPoolListItemDraw,                       // item draw funtcion
                          fontHeight(" ")+(fontHeight(" ")>>1),       // item height
                          UICLW_CanSelect|UICLW_CanHaveFocus);

        done = 0;
        for (index=0;index<KEY_TOTAL_KEYS;index++)
        {
            if (kbCanMapKey[index] == TRUE)
            {
                if (done == 0)
                {
                    key1 = (sword)index;
                    done = 1;
                }
                else if (done == 1)
                {
                    key2 = (sword)index;
                    done = 2;
                }
            }

            if (done == 2)
            {
                // pack the two 16-bit swords into the on udword
                data = (udword) ( (key1)| ((key2<<16)) );
                key1 = -1;
                key2 = -1;
                done = 0;
                uicListAddItem(keypool, (ubyte *)data,  UICLI_CanSelect, UICLW_AddToTail);
            }
        }
        if (done == 1)
        {
            // pack the two 16-bit swords into the on udword
            key2 = -1;
            data = (udword) ( (key1)| ((key2<<16)) );
            key1 = -1;
            done = 0;
            uicListAddItem(keypool, (ubyte *)data,  UICLI_CanSelect, UICLW_AddToTail);
        }

        fontMakeCurrent(oldfont);
        return;
    }
    else if (FELASTCALL(atom))
    {
        return;
    }
}

/*-----------------------------------------------------------------------------
    Name        : kbSetNewKey
    Description : This function sets the new keypressed to the currently selected command.
    Inputs      : keypressed
    Outputs     : none
    Parameters  : udword keypressed
    Return      : void
-----------------------------------------------------------------------------*/
void kbSetNewKey(udword keypressed)
{
    kbBoundKeys *bkey;
    sdword       index;

    keypressed&=0x000000FF;

    dbgAssert(keypressed < KEY_TOTAL_KEYS);

    bkey = (kbBoundKeys *)kbListWindow->CurLineSelected->data;
    if (kbCanMapKey[keypressed])
    {
        for (index=0;index<kbTOTAL_COMMANDS;index++)
        {
            if (kbKeyTable[index].primarykey == keypressed)
            {
                kbKeyTable[index].status = KB_NotBound;
                kbKeyTable[index].primarykey = 0;
            }
        }
        bkey->primarykey = keypressed;
        bkey->status = KB_NormalBound;
#ifdef DEBUG_STOMP
        regVerify((regionhandle)&kbListWindow->reg);
#endif
        bitSet(kbListWindow->reg.status, RSF_DrawThisFrame);
    }
    else
    {
        if (bkey->primarykey != 0)
            bkey->status = KB_NormalBound;
        else
            bkey->status = KB_NotBound;

        if (gameIsRunning)
        {
            feScreenStart(feStack[feStackIndex].baseRegion, "In_Game_Invalid_Key");
        }
        else
        {
            feScreenStart(feStack[feStackIndex].baseRegion, "Invalid_Key");
        }
        // this key is not a key that can be remapped.
    }
}


/*-----------------------------------------------------------------------------
    Name        : kbKeyResetToDefault
    Description : This function will reset all of the keyboard bindings to
                  default.
    Inputs      : name, and atom
    Outputs     : none
    Parameters  : char *string, featom *atom
    Return      : void
-----------------------------------------------------------------------------*/
void kbKeyResetToDefault(char *string, featom *atom)
{
    sdword index;

    for (index=kbTOTAL_COMMANDS-1; index >= 0; index--)
    {
        kbKeyTable[index].primarykey = kbKeyTable[index].resettokey;
        kbKeyTable[index].status = KB_NormalBound;
    }

#ifdef DEBUG_STOMP
    regVerify(&kbListWindow->reg);
#endif
    bitSet(kbListWindow->reg.status, RSF_DrawThisFrame);
}


/*-----------------------------------------------------------------------------
    Name        : kbCheckBindings
    Description : This function will checks to see if this key is a remappable
                  key and then do the conversion.
    Inputs      : keypressed
    Outputs     : none
    Parameters  : sdword keypressed
    Return      : void
-----------------------------------------------------------------------------*/
sdword kbCheckBindings(sdword keypressed)
{
    sdword index;

    if (kbCanMapKey[keypressed])
    {
        for (index=0;index<kbTOTAL_COMMANDS;index++)
        {
            if (kbKeyTable[index].primarykey == keypressed)
            {
                return (kbKeyTable[index].defaultkey);
            }
        }

        // key not bound to anything !!!!!
        return(0);
    }
    else
    {
        return (keypressed);
    }
}


/*-----------------------------------------------------------------------------
    Name        : kbRestoreSavedSettings
    Description : This function will restore the previosly saved settings.
    Inputs      : none
    Outputs     : none
    Parameters  : void
    Return      : void
-----------------------------------------------------------------------------*/
void kbRestoreSavedSettings(void)
{
    sdword index;

    for (index=0; index < kbTOTAL_COMMANDS; index++)
    {
        kbKeyTable[index] = kbKeySaveTable[index];
    }
}


/*-----------------------------------------------------------------------------
    Name        : kbSaveSettings
    Description : This function will save the current settings to a temporary
                  structure.  This is called when the options are started so
                  That a user can cancel any changes they don't like.
    Inputs      : void
    Outputs     : none
    Parameters  : void
    Return      : void
-----------------------------------------------------------------------------*/
void kbSaveSettings(void)
{
    sdword index;

    for (index=0; index < kbTOTAL_COMMANDS; index++)
    {
        kbKeySaveTable[index] = kbKeyTable[index];
        kbKeySavedKeys[index] = kbKeyTable[index].primarykey;
    }
}

/*-----------------------------------------------------------------------------
    Name        : kbKeyUsed
    Description : This functino returns true if the key is already bound to something, FALSE otherwise
    Inputs      : key to check
    Outputs     : boolean
    Parameters  : sdword keytocheck
    Return      : bool
-----------------------------------------------------------------------------*/
bool kbKeyUsed(udword keytocheck)
{
    sdword index, count=0;

    for (index=0; index < kbTOTAL_COMMANDS; index++)
    {
        if (kbKeyTable[index].primarykey == keytocheck)
        {
            count++;
        }
    }

    if (count > 1) return(TRUE);

    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : kbCommandKeyIsHit
    Description : This function will return true if the key is hit for the command specified
    Inputs      : command
    Outputs     : boolean
    Parameters  : udword command
    Return      : bool
-----------------------------------------------------------------------------*/
bool kbCommandKeyIsHit(udword command)
{
    return(keyIsHit(kbKeyTable[command].primarykey));
}

/*-----------------------------------------------------------------------------
    Name        : kbKeyBoundToCommand
    Description : This function returns the key that is bound to that command
    Inputs      : Command to get the key for
    Outputs     : key bound
    Parameters  : udword command
    Return      : udword
-----------------------------------------------------------------------------*/
udword kbKeyBoundToCommand(udword command)
{
    if (kbKeyTable != NULL) return(kbKeyTable[command].primarykey);

    return (0);
}

/*-----------------------------------------------------------------------------
    Name        : kbInitKeyBindings
    Description : This function Initializes the keybindings.
    Inputs      : void
    Outputs     : none
    Parameters  : void
    Return      : void
-----------------------------------------------------------------------------*/
void kbInitKeyBindings(void)
{
    sdword index;
    bool   bInFile=FALSE;

    kbKeyListFont = frFontRegister("hw_eurosecond_11.hff");

    for (index=0;index<KEY_TOTAL_KEYS;index++)
    {
        kbCanMapKey[index] = FALSE;
    }

    if (!bkDisableKeyRemap)
    {
        kbCanMapKey[TABKEY]         = TRUE;
        kbCanMapKey[BKEY]           = TRUE;
        kbCanMapKey[CKEY]           = TRUE;
        kbCanMapKey[VKEY]           = TRUE;
        kbCanMapKey[DKEY]           = TRUE;
        kbCanMapKey[EKEY]           = TRUE;
        kbCanMapKey[FKEY]           = TRUE;
        kbCanMapKey[RKEY]           = TRUE;
        kbCanMapKey[HKEY]           = TRUE;
        kbCanMapKey[MKEY]           = TRUE;
        kbCanMapKey[RBRACK]         = TRUE;
        kbCanMapKey[LBRACK]         = TRUE;
        kbCanMapKey[SKEY]           = TRUE;
        kbCanMapKey[ZKEY]           = TRUE;
        kbCanMapKey[CAPSLOCKKEY]    = TRUE;
//        kbCanMapKey[SPACEKEY]       = TRUE;
        kbCanMapKey[HOMEKEY]        = TRUE;
        kbCanMapKey[KKEY]           = TRUE;
        kbCanMapKey[TILDEKEY]       = TRUE;
        kbCanMapKey[LKEY]           = TRUE;
        // start of extra keys
//        kbCanMapKey[PRINTKEY]       = TRUE;
        kbCanMapKey[ARRUP]          = TRUE;
        kbCanMapKey[ARRDOWN]        = TRUE;
        kbCanMapKey[ENDKEY]         = TRUE;
        kbCanMapKey[INSERTKEY]      = TRUE;
        kbCanMapKey[DELETEKEY]      = TRUE;
//        kbCanMapKey[LESSTHAN]       = TRUE;
//        kbCanMapKey[GREATERTHAN]    = TRUE;
        kbCanMapKey[AKEY]           = TRUE;
        kbCanMapKey[NKEY]           = TRUE;
        kbCanMapKey[OKEY]           = TRUE;

        if (pilotView)
        {
            kbCanMapKey[QKEY]           = FALSE;
        }
        else
        {
            kbCanMapKey[QKEY]           = TRUE;
        }

        kbCanMapKey[UKEY]           = TRUE;
        kbCanMapKey[WKEY]           = TRUE;
        kbCanMapKey[XKEY]           = TRUE;
        kbCanMapKey[YKEY]           = TRUE;
        kbCanMapKey[NUMPAD0]        = TRUE;
        kbCanMapKey[NUMPAD1]        = TRUE;
        kbCanMapKey[NUMPAD2]        = TRUE;
        kbCanMapKey[NUMPAD3]        = TRUE;
        kbCanMapKey[NUMPAD4]        = TRUE;
        kbCanMapKey[NUMPAD5]        = TRUE;
        kbCanMapKey[NUMPAD6]        = TRUE;
        kbCanMapKey[NUMPAD7]        = TRUE;
        kbCanMapKey[NUMPAD8]        = TRUE;
        kbCanMapKey[NUMPAD9]        = TRUE;
        kbCanMapKey[NUMSTARKEY]     = TRUE;
        kbCanMapKey[NUMSLASHKEY]    = TRUE;
        kbCanMapKey[NUMDOTKEY]      = TRUE;
 //       kbCanMapKey[BACKSLASHKEY]   = TRUE;
    }

    kbKeyToString[AKEY] =         strAKEY;
    kbKeyToString[BKEY] =         strBKEY;
    kbKeyToString[CKEY] =         strCKEY;
    kbKeyToString[DKEY] =         strDKEY;
    kbKeyToString[EKEY] =         strEKEY;
    kbKeyToString[FKEY] =         strFKEY;
    kbKeyToString[GKEY] =         strGKEY;
    kbKeyToString[HKEY] =         strHKEY;
    kbKeyToString[IKEY] =         strIKEY;
    kbKeyToString[JKEY] =         strJKEY;
    kbKeyToString[KKEY] =         strKKEY;
    kbKeyToString[LKEY] =         strLKEY;
    kbKeyToString[MKEY] =         strMKEY;
    kbKeyToString[NKEY] =         strNKEY;
    kbKeyToString[OKEY] =         strOKEY;
    kbKeyToString[PKEY] =         strPKEY;
    kbKeyToString[QKEY] =         strQKEY;
    kbKeyToString[RKEY] =         strRKEY;
    kbKeyToString[SKEY] =         strSKEY;
    kbKeyToString[TKEY] =         strTKEY;
    kbKeyToString[UKEY] =         strUKEY;
    kbKeyToString[VKEY] =         strVKEY;
    kbKeyToString[WKEY] =         strWKEY;
    kbKeyToString[XKEY] =         strXKEY;
    kbKeyToString[YKEY] =         strYKEY;
    kbKeyToString[ZKEY] =         strZKEY;
    kbKeyToString[BACKSPACEKEY] = strBACKSPACEKEY;
    kbKeyToString[TABKEY] =       strTABKEY;
    kbKeyToString[ARRLEFT] =      strARRLEFT;
    kbKeyToString[ARRRIGHT] =     strARRRIGHT;
    kbKeyToString[ARRUP] =        strARRUP;
    kbKeyToString[ARRDOWN] =      strARRDOWN;
    kbKeyToString[ENDKEY] =       strENDKEY;
    kbKeyToString[LBRACK] =       strLBRACK;
    kbKeyToString[RBRACK] =       strRBRACK;
    kbKeyToString[CAPSLOCKKEY] =  strCAPSLOCKKEY;
    kbKeyToString[SPACEKEY] =     strSPACEKEY;
    kbKeyToString[ENTERKEY] =     strENTERKEY;
    kbKeyToString[HOMEKEY] =      strHOMEKEY;
    kbKeyToString[PAGEDOWNKEY] =  strPAGEDOWNKEY;
    kbKeyToString[PAGEUPKEY] =    strPAGEUPKEY;
    kbKeyToString[BACKSLASHKEY] = strBACKSLASHKEY;
    kbKeyToString[PAUSEKEY] =     strPAUSEKEY;
    kbKeyToString[SCROLLKEY] =    strSCROLLKEY;
    kbKeyToString[PRINTKEY] =     strPRINTKEY;
    kbKeyToString[INSERTKEY] =    strINSERTKEY;
    kbKeyToString[DELETEKEY] =    strDELETEKEY;
    kbKeyToString[LESSTHAN] =     strLESSTHAN;
    kbKeyToString[GREATERTHAN] =  strGREATERTHAN;
    kbKeyToString[TILDEKEY] =     strTILDEKEY;
    kbKeyToString[NUMPAD0] =      strNUMPAD0;
    kbKeyToString[NUMPAD1] =      strNUMPAD1;
    kbKeyToString[NUMPAD2] =      strNUMPAD2;
    kbKeyToString[NUMPAD3] =      strNUMPAD3;
    kbKeyToString[NUMPAD4] =      strNUMPAD4;
    kbKeyToString[NUMPAD5] =      strNUMPAD5;
    kbKeyToString[NUMPAD6] =      strNUMPAD6;
    kbKeyToString[NUMPAD7] =      strNUMPAD7;
    kbKeyToString[NUMPAD8] =      strNUMPAD8;
    kbKeyToString[NUMPAD9] =      strNUMPAD9;
    kbKeyToString[NUMMINUSKEY] =  strNUMMINUSKEY;
    kbKeyToString[NUMPLUSKEY] =   strNUMPLUSKEY;
    kbKeyToString[NUMSTARKEY] =   strNUMSTARKEY;
    kbKeyToString[NUMSLASHKEY] =  strNUMSLASHKEY;
    kbKeyToString[NUMDOTKEY] =    strNUMDOTKEY;
    kbKeyToString[MINUSKEY] =     strMINUSKEY;
    kbKeyToString[PLUSKEY] =      strPLUSKEY;
    kbKeyToString[F1KEY] =        strF1KEY;
    kbKeyToString[F2KEY] =        strF2KEY;
    kbKeyToString[F3KEY] =        strF3KEY;
    kbKeyToString[F4KEY] =        strF4KEY;
    kbKeyToString[F5KEY] =        strF5KEY;
    kbKeyToString[F6KEY] =        strF6KEY;
    kbKeyToString[F7KEY] =        strF7KEY;
    kbKeyToString[F8KEY] =        strF8KEY;
    kbKeyToString[F9KEY] =        strF9KEY;
    kbKeyToString[F10KEY] =       strF10KEY;
    kbKeyToString[F11KEY] =       strF11KEY;
    kbKeyToString[F12KEY] =       strF12KEY;

    for (index=0;index<kbTOTAL_COMMANDS;index++)
    {
        if (kbKeySavedKeys[index] != 0) bInFile = TRUE;
    }

    switch (strCurLanguage)
    {
        case languageEnglish:
            kbKeyTable = &kbKeyTableEnglish[0];
        break;
        case languageFrench:
            kbKeyTable = &kbKeyTableFrench[0];
        break;
        case languageGerman:
            kbKeyTable = &kbKeyTableGerman[0];
        break;
        case languageSpanish:
            kbKeyTable = &kbKeyTableSpanish[0];
        break;
        case languageItalian:
            kbKeyTable = &kbKeyTableItalian[0];
        break;
    }

    if (bInFile)
    {
        for (index=0;index<kbTOTAL_COMMANDS;index++)
        {
            kbKeyTable[index].primarykey = kbKeySavedKeys[index];
        }
    }

    // check the key-bindings and reset anything that's messed up
    for (index=0;index<kbTOTAL_COMMANDS;index++)
    {
        if (kbKeyTable[index].primarykey == 0)
        {
            // i guess the user can have no key for a function ?? don't know why but ...
            kbKeyTable[index].status = KB_NotBound;
            continue;
        }
        // is the key even a mappable one ?
        if (!kbCanMapKey[kbKeyTable[index].primarykey])
        {
            // just assign it to default key
            kbKeyTable[index].primarykey = kbKeyTable[index].defaultkey;
            kbKeyTable[index].status = KB_NormalBound;
        }
        // make sure that it's only used once
        if (kbKeyUsed(kbKeyTable[index].primarykey))
        {   // set to not mapped, can't have the same key do multiple things
            kbKeyTable[index].primarykey = 0;
            kbKeyTable[index].status = KB_NotBound;
        }
    }
}
