/*=============================================================================
    KEY.H: Definitions for keyboard interface.

    Created June 1997 by Gary Shaw
=============================================================================*/

#ifndef ___KEY_H
#define ___KEY_H

#include "Types.h"
#include "SDL_keysym.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifndef HW_Release
#define KEY_ERROR_CHECKING      1
#define KEY_VERBOSE_LEVEL       1
#else //HW_Debug
#define KEY_ERROR_CHECKING      0
#define KEY_VERBOSE_LEVEL       0
#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define KEY_TOTAL_KEYS          (SDLK_LAST+8)  // Use SDL keysyms, plus some for mouse
#define KEY_BufferLength        16

#define KEY_NUMPRESSED_BITS     5
#define KEY_NUMPRESSED_MAX ((1<<6)-1)
#define BACKSPACEKEY            SDLK_BACKSPACE
#define TABKEY                  SDLK_TAB
#define ENTERKEY                SDLK_KP_ENTER
#define SHIFTKEY                SDLK_LSHIFT
/*#define LSHIFTKEY               SDLK_LSHIFT*/
/*#define RSHIFTKEY               SDLK_RSHIFT*/
#define CONTROLKEY              SDLK_LCTRL
/*#define LCONTROLKEY             SDLK_LCTRL*/
/*#define RCONTROLKEY             SDLK_RCTRL*/
#define ALTKEY                  SDLK_LALT
/*#define LALTKEY                 SDLK_LALT*/
/*#define RALTKEY                 SDLK_RALT*/
#define METAKEY                 SDLK_LMETA      // Apple/Command/"Windows" key
/*#define LMETAKEY                SDLK_LMETA*/
/*#define RMETAKEY                SDLK_RMETA*/
#define PAUSEKEY                SDLK_PAUSE
#define SCROLLKEY               SDLK_SCROLLOCK
#define PRINTKEY                SDLK_PRINT
#define CAPSLOCKKEY             SDLK_CAPSLOCK
#define ESCKEY                  SDLK_ESCAPE
#define ARRLEFT                 SDLK_LEFT
#define ARRRIGHT                SDLK_RIGHT
#define ARRUP                   SDLK_UP
#define ARRDOWN                 SDLK_DOWN
#define ENDKEY                  SDLK_END
#define HOMEKEY                 SDLK_HOME
#define PAGEUPKEY               SDLK_PAGEUP
#define PAGEDOWNKEY             SDLK_PAGEDOWN
#define INSERTKEY               SDLK_INSERT
#define DELETEKEY               SDLK_DELETE
#define LESSTHAN                SDLK_COMMA
#define GREATERTHAN             SDLK_PERIOD
#define ZEROKEY                 '0'
#define ONEKEY                  '1'
#define TWOKEY                  '2'
#define THREEKEY                '3'
#define FOURKEY                 '4'
#define FIVEKEY                 '5'
#define SIXKEY                  '6'
#define SEVENKEY                '7'
#define EIGHTKEY                '8'
#define NINEKEY                 '9'
#define AKEY                    'a'
#define BKEY                    'b'
#define CKEY                    'c'
#define DKEY                    'd'
#define EKEY                    'e'
#define FKEY                    'f'
#define GKEY                    'g'
#define HKEY                    'h'
#define IKEY                    'i'
#define JKEY                    'j'
#define KKEY                    'k'
#define LKEY                    'l'
#define MKEY                    'm'
#define NKEY                    'n'
#define OKEY                    'o'
#define PKEY                    'p'
#define QKEY                    'q'
#define RKEY                    'r'
#define SKEY                    's'
#define TKEY                    't'
#define UKEY                    'u'
#define VKEY                    'v'
#define WKEY                    'w'
#define XKEY                    'x'
#define YKEY                    'y'
#define ZKEY                    'z'
#define SPACEKEY                ' '
#define RETURNKEY               SDLK_RETURN
#define NUMPAD0                 SDLK_KP0
#define NUMPAD1                 SDLK_KP1
#define NUMPAD2                 SDLK_KP2
#define NUMPAD3                 SDLK_KP3
#define NUMPAD4                 SDLK_KP4
#define NUMPAD5                 SDLK_KP5
#define NUMPAD6                 SDLK_KP6
#define NUMPAD7                 SDLK_KP7
#define NUMPAD8                 SDLK_KP8
#define NUMPAD9                 SDLK_KP9
#define LBRACK                  SDLK_LEFTBRACKET
#define RBRACK                  SDLK_RIGHTBRACKET
#define F1KEY                   SDLK_F1
#define F2KEY                   SDLK_F2
#define F3KEY                   SDLK_F3
#define F4KEY                   SDLK_F4
#define F5KEY                   SDLK_F5
#define F6KEY                   SDLK_F6
#define F7KEY                   SDLK_F7
#define F8KEY                   SDLK_F8
#define F9KEY                   SDLK_F9
#define F10KEY                  SDLK_F10
#define F11KEY                  SDLK_F11
#define F12KEY                  SDLK_F12
#define TILDEKEY                SDLK_BACKQUOTE
#define NUMMINUSKEY             SDLK_KP_MINUS
#define NUMPLUSKEY              SDLK_KP_PLUS
#define NUMSTARKEY              SDLK_KP_MULTIPLY
#define NUMSLASHKEY             SDLK_KP_DIVIDE
#define NUMPADSLASH             SDLK_KP_DIVIDE
#define NUMDOTKEY               SDLK_KP_PERIOD
#define MINUSKEY                SDLK_MINUS
#define PLUSKEY                 SDLK_EQUALS
#define BACKSLASHKEY            SDLK_BACKSLASH

#define LMOUSE_BUTTON           (SDLK_LAST+0)
#define RMOUSE_BUTTON           (SDLK_LAST+1)
#define MMOUSE_BUTTON           (SDLK_LAST+3)

#define FLYWHEEL_UP             (SDLK_LAST+2)
#define FLYWHEEL_DOWN           (SDLK_LAST+4)

#define LMOUSE_DOUBLE           (SDLK_LAST+5)
#define RMOUSE_DOUBLE           (SDLK_LAST+6)
#define MMOUSE_DOUBLE           (SDLK_LAST+7)

/*=============================================================================
    Type definitions:
=============================================================================*/
typedef struct
{
   ubyte keypressed : 1;
   ubyte keystick : 2;
   sbyte keynumpressed : KEY_NUMPRESSED_BITS;
}
keyScanType;

/* Been having some problems using sizes < 4 in variable argument lists.
   (segfault core dump AUGH MY NARDS!@$#1) */
/*typedef uword keyindex;*/
typedef udword keyindex;

typedef struct
{
    keyindex key;
    bool8 bShift;
}
keybufferchar;

/*=============================================================================
    Data:
=============================================================================*/
extern volatile keyScanType keyScanCode[KEY_TOTAL_KEYS];
extern volatile keyScanType keySaveScan[KEY_TOTAL_KEYS];
extern real32 keyLastTimeHit;

/*=============================================================================
    Macros:
=============================================================================*/

#define keyIsHit(key)           (keyScanCode[(key)].keypressed)
#define keyIsStuck(key)         (keyScanCode[(key)].keystick)
#define keyIsRepeat(key)        (keyScanCode[(key)].keynumpressed > 0)
#define keyDecRepeat(key)       keyScanCode[(key)].keynumpressed--,keySaveScan[(key)].keynumpressed--
#define keyClearRepeat(key)     keyScanCode[(key)].keynumpressed = 0,keySaveScan[(key)].keynumpressed = 0
#define keyClearSticky(key)     if (keyScanCode[(key)].keystick) {keyScanCode[(key)].keystick--,keySaveScan[(key)].keystick--;}
#define keySetSticky(key)       if (keyScanCode[(key)].keystick < 3) {keyScanCode[(key)].keystick++,keySaveScan[(key)].keystick++;}

/*=============================================================================
    Functions:
=============================================================================*/
void keyInit(void);
void keyClose(void);
bool keyAnyKeyHit(void);
bool keyAnyKeyStuck(void);
void keyClearAll(void);
void keyClearAllStuckKeys(void);

//called from the Windows interface layer, set/clear the sticky and on bits
void keyPressDown(udword key);
void keyPressUp(udword key);
void keyRepeat(udword key);

//functions for buffered keystrokes
udword keyBufferedKeyGet(bool *bShift);
void keyBufferAdd(udword key, bool bShift);
void keyBufferClear(void);

udword keyGermanToEnglish(udword virtkey);
udword keyFrenchToEnglish(udword virtkey);
udword keySpanishToEnglish(udword virtkey);
udword keyItalianToEnglish(udword virtkey);

#endif //___KEY_H

