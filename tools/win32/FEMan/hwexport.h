/*
** HWEXPORT.HPP : Header file for HWEXPORT.CPP.
*/

#ifndef __HWEXPORT_H
#define __HWEXPORT_H

#include "FEManDoc.h"

#define FIBIDENTIFY         "Bananna"
#define FIBVERSION          (0x00000212)
#define NUMBER_LANGUAGES_TO_SAVE    5

//macros to set/test/clear bits
#define bitSet(dest, bitFlags)      ((dest) |= (bitFlags))
#define bitClear(dest, bitFlags)    ((dest) &= ~(bitFlags))
#define bitTest(dest, bitFlags)     ((dest) & (bitFlags))

    typedef struct tagFIBFileHeader
    {
        char identify[8];
        unsigned short version;
        unsigned short nScreens;
    } FIBFileHeader;

    typedef struct tagFIBScreen
    {
        unsigned long oName;
        unsigned long flags;
        unsigned short nLinks;
        unsigned short nAtoms;
        unsigned long oLinks;
        unsigned long oAtoms;
    } FIBScreen;

    typedef struct tagFIBLink
    {
        unsigned long oName;
        unsigned long flags;
        signed long oLinkedToName;
    } FIBLink;

    enum tagFIBLinkFlags
    {
        FL_Enabled = 1,
        FL_DefaultPrev = 2,
        FL_DefaultNext = 4,

        FL_LastFLF
    };

    typedef struct tagFIBAtom
    {
        unsigned long   oName;
        unsigned long   flags;
        unsigned long   status;
        unsigned char   type;
        unsigned char   borderWidth;
        unsigned short  tabstop;
        unsigned long   borderColor;
        unsigned long   contentColor;
        signed long     left;
        signed long     top;
        signed long     right;
        signed long     bottom;
        unsigned long   oData;
        unsigned long   attribs;
        char            hotKeyModifiers;
        char            hotKey[NUMBER_LANGUAGES_TO_SAVE];
        char            pad2[2];
        unsigned long   drawstyle[2];
        unsigned long pad[3];
    } FIBAtom;

    enum tagFIBAtomType
    {
        FA_UserRegion = 1,
        FA_StaticText,
        FA_Button,
        FA_CheckBox,
        FA_ToggleButton,
        FA_ScrollBar,
        FA_StatusBar,
        FA_TextEntry,
        FA_ListViewExpandButton,
        FA_TitleBar,
        FA_MenuItem,
        FA_RadioButton,
        FA_CutoutRegion,
        FA_DecorativeRegion,
        FA_Divider,
        FA_ListWindow,
        FA_BitmapButton,
        FA_HorizSlider,
        FA_VertSlider,
        FA_DragButton,
        FA_OpaqueDecorativeRegion,

        FA_LastFA
    };

    //we have up to 32 flags here
    enum tagFIBAtomFlags
    {
        FAF_Link            = 0x00000001,
        FAF_Function        = 0x00000002,
        FAF_Bitmap          = 0x00000004,
        FAF_Modal           = 0x00000008,
        FAF_Popup           = 0x00000010,
        FAF_CallOnCreate    = 0x00000020,
        FAF_ContentsVisible = 0x00000040,
        FAF_DefaultOK       = 0x00000080,
        FAF_DefaultBack     = 0x00000100,
        FAF_AlwaysOnTop     = 0x00000200,
        FAF_Draggable       = 0x00000400,
        FAF_BorderVisible   = 0x00000800,
        FAF_Disabled        = 0x00001000,
        FAF_DontCutoutBase  = 0X00002000,
        FAF_CallOnDelete    = 0x00004000,
        FAM_Justification   = 0x00030000,
        FAM_JustLeft        = 0x00010000,
        FAM_JustRight       = 0x00020000,
        FAM_JustCentre      = 0x00030000,
        FAF_Background      = 0x00040000,
        FAM_DropShadow      = 0x0ff00000,          //these bits not used by strings

        FAF_LastFAF
    };
#define FSB_DropShadow          20

//hot key modifiers
#define HKM_Control         0x01
#define HKM_Alt             0x02
#define HKM_Shift           0x04

    enum tagFIBAtomDrawType
    {
        //texture flags
        SmallTextures,
        MediumTextures,
        LargeTextures,
        InternalGlow,
        ExternalGlow,
        NoGlow,

        LastDrawType
    };

    class SaveFIBFormat
    {
    private:
    protected:
        FIBFileHeader   *pFIBFileHeader;
        FIBScreen       *pFIBScreens;
        FIBLink         **pFIBLinks;
        FIBAtom         **pFIBAtoms;

    public:
        SaveFIBFormat(void);
        ~SaveFIBFormat(void);

        char *FixFontName(ScreenObjectString *sos);
        
        unsigned char GetAtomType(ScreenObject *pObject);

        char *GetAtomData(ScreenObject *pObject);

        char *GetBitmapName(ScreenObject *pObject);

        unsigned long GetAtomFlags(ScreenObject *pObject);

        unsigned long ParseClickSounds(ScreenObject *pObject);

        void ParseAccelKeys(ScreenObject *pObject, char *buf, char *modifier);

        void ParseMisc(ScreenObject *pObject, FIBAtom *buf);

        unsigned long GetRadioIndex(ScreenObject *pObject);
        
        void ParseFEManData(CFEManDoc *pDocument);

        void WriteFEManData(CString &fileName, CFEManDoc *pDocument);
    };

#endif