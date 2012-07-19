/*=============================================================================
    KEY.H: Definitions for keyboard interface.

    Created June 1997 by Gary Shaw
=============================================================================*/

#ifndef ___KEY_H
#define ___KEY_H

#include "types.h"

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
#define KEY_TOTAL_KEYS          256
#define KEY_BufferLength        16

#define KEY_NUMPRESSED_BITS     5
#define KEY_NUMPRESSED_MAX ((1<<6)-1)
#define BACKSPACEKEY            0x08
#define TABKEY                  0x09
#define ENTERKEY                0x0d
#define SHIFTKEY                0x10
#define CONTROLKEY              0x11
#define ALTKEY                  0x12
#define PAUSEKEY                0x13
#define SCROLLKEY               0x91
#define PRINTKEY                0x2A
#define CAPSLOCKKEY             0x14
#define ESCKEY                  0x1b
#define ARRLEFT                 0x25
#define ARRRIGHT                0x27
#define ARRUP                   0x26
#define ARRDOWN                 0x28
#define ENDKEY                  0x23
#define HOMEKEY                 0x24
#define PAGEUPKEY               0x21
#define PAGEDOWNKEY             0x22
#define INSERTKEY               0x2D
#define DELETEKEY               0x2E
#define LESSTHAN                0xbc // 188
#define GREATERTHAN             0xbe // 190
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
#define AKEY                    'A'
#define BKEY                    'B'
#define CKEY                    'C'
#define DKEY                    'D'
#define EKEY                    'E'
#define FKEY                    'F'
#define GKEY                    'G'
#define HKEY                    'H'
#define IKEY                    'I'
#define JKEY                    'J'
#define KKEY                    'K'
#define LKEY                    'L'
#define MKEY                    'M'
#define NKEY                    'N'
#define OKEY                    'O'
#define PKEY                    'P'
#define QKEY                    'Q'
#define RKEY                    'R'
#define SKEY                    'S'
#define TKEY                    'T'
#define UKEY                    'U'
#define VKEY                    'V'
#define WKEY                    'W'
#define XKEY                    'X'
#define YKEY                    'Y'
#define ZKEY                    'Z'
#define SPACEKEY                ' '
#define RETURNKEY               0x0d
#define NUMPAD0                 0x60
#define NUMPAD1                 0x61
#define NUMPAD2                 0x62
#define NUMPAD3                 0x63
#define NUMPAD4                 0x64
#define NUMPAD5                 0x65
#define NUMPAD6                 0x66
#define NUMPAD7                 0x67
#define NUMPAD8                 0x68
#define NUMPAD9                 0x69
#define LBRACK                  0xdb    // 219
#define RBRACK                  0xdd    // 221
#define F1KEY                   0x70
#define F2KEY                   0x71
#define F3KEY                   0x72
#define F4KEY                   0x73
#define F5KEY                   0x74
#define F6KEY                   0x75
#define F7KEY                   0x76
#define F8KEY                   0x77
#define F9KEY                   0x78
#define F10KEY                  0x79
#define F11KEY                  0x7a
#define F12KEY                  0x7b
#define TILDEKEY                0xc0    // 192
#define NUMMINUSKEY             0x6d
#define NUMPLUSKEY              0x6b
#define NUMSTARKEY              0x6a
#define NUMSLASHKEY             0x6f
#define NUMPADSLASH             0x6f
#define NUMDOTKEY               0x6e
#define MINUSKEY                0xbd    // 189
#define PLUSKEY                 0xbb    // 187
#define BACKSLASHKEY            0xdc    // 220

#define LMOUSE_BUTTON           1
#define RMOUSE_BUTTON           2
#define MMOUSE_BUTTON           4

#define FLYWHEEL_UP             3
#define FLYWHEEL_DOWN           5

#define LMOUSE_DOUBLE           6
#define RMOUSE_DOUBLE           7
#define MMOUSE_DOUBLE           0xf8

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

typedef ubyte keyindex;

typedef struct
{
    ubyte key;
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

