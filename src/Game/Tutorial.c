#ifdef _WIN32
#include <windows.h>
#endif

#include "glinc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Types.h"
#include "FEFlow.h"
#include "AIPlayer.h"
#include "utility.h"
#include "LevelLoad.h"
#include "Region.h"
#include "KAS.h"
#include "../Generated/basic.h"
#include "../Generated/advanced.h"
#include "../Generated/new.h"
#include "UIControls.h"
#include "Strings.h"
#include "FontReg.h"
#include "Select.h"
#include "texreg.h"
#include "render.h"
#include "mouse.h"
#include "Tutorial.h"
#include "FastMath.h"
#include "Tutor.h"

extern char *getWord(char *dest, char *source);
extern void utySinglePlayerGameStart(char *name, featom *atom);

#define TUT_MaxFontNameLength   64

#define TUT_TransitionFramesIn 5
#define TUT_TransitionFramesOut 5

#define TUT_TitleAreaHeight 30
#define TUT_TextAreaWidth 120
#define TUT_TipAreaWidth 90

// Computed multipliers for the transition effects speeds (0 = invalid, 1 = transition out, 2 = transition int)
const static float TUT_TitleTransMult[3] = {
    0.0f,
    (float)TUT_TitleAreaHeight / (float)TUT_TransitionFramesOut,
    (float)TUT_TitleAreaHeight / (float)TUT_TransitionFramesIn,
};

const static float TUT_TextTransMult[3] = {
    0.0f,
    (float)TUT_TextAreaWidth / (float)TUT_TransitionFramesOut,
    (float)TUT_TextAreaWidth / (float)TUT_TransitionFramesIn,
};

const static float TUT_TipTransMult[3] = {
    0.0f,
    (float)TUT_TipAreaWidth / (float)TUT_TransitionFramesOut,
    (float)TUT_TipAreaWidth / (float)TUT_TransitionFramesIn,
};

typedef enum
{
    NEXT_ON,
    NEXT_OFF,
    NEXT_MOUSE,
    MOUSE_RIGHT_ARROWS,
    MOUSE_RIGHT_LEFT,
    MOUSE_MIDDLE,
    MOUSE_LEFT,
    MOUSE_LEFT_ARROWS,
    MOUSE_LEFT_X2,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_F,
    KEY_H,
    KEY_M,
    KEY_Z,
    KEY_F1,
    KEY_ESC,
    KEY_TAB,
    KEY_SHIFT,
    KEY_CAPSLOCK,
    KEY_SPACE,
    KEY_CTRL_1,
    KEY_CTRL_ALT,
    RIGHT_CLICK_ICON,
    TASKBAR_ICON,
    NUM_BUTTON_TEXTURES
} BUTTON_TEXTURES;

#ifdef _WIN32
#define FEMAN_TEXDEC_PATH "feman\\texdecorative\\"
#else
#define FEMAN_TEXDEC_PATH "feman/texdecorative/"
#endif
char tutTextureNames[NUM_BUTTON_TEXTURES][64] =
{
{FEMAN_TEXDEC_PATH "Tut_next_on.LiF"},
{FEMAN_TEXDEC_PATH "Tut_next_off.LiF"},
{FEMAN_TEXDEC_PATH "Tut_next_mouse.LiF"},
{FEMAN_TEXDEC_PATH "Tut_mouse_right.LiF"},
{FEMAN_TEXDEC_PATH "Tut_mouse_right_left.LiF"},
{FEMAN_TEXDEC_PATH "Tut_mouse_middle.LiF"},
{FEMAN_TEXDEC_PATH "Tut_mouse_left.LiF"},
{FEMAN_TEXDEC_PATH "Tut_mouse_left_arrows.LiF"},
{FEMAN_TEXDEC_PATH "Tut_mouse_left_x2.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_c.LiF"},            // need correct art for b button
{FEMAN_TEXDEC_PATH "Tut_key_c.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_d.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_f.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_h.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_m.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_z.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_f1.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_esc.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_tab.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_shift.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_capslock.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_space.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_ctrl_1.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_ctrl_alt.LiF"},
{FEMAN_TEXDEC_PATH "Tut_key_c.LiF"},            // need correct art for right-click icon
{FEMAN_TEXDEC_PATH "Tut_key_d.LiF"},            // need correct art for taskbar icon
};

sword tutBitmapList[TUT_TOTAL_LESSONS][3] =
{
    {-1,                    -1,                 -1                  },  //basic intro
    {MOUSE_RIGHT_ARROWS,    -1,                 -1                  },
    {MOUSE_RIGHT_LEFT,      -1,                 -1                  },
    {KEY_F,                 MOUSE_MIDDLE,       -1                  },
    {KEY_C,                 -1,                 -1                  },
    {MOUSE_LEFT_ARROWS,     -1,                 -1                  },
    {MOUSE_LEFT_ARROWS,     -1,                 -1                  },
    {KEY_ESC,               -1,                 -1                  },
    {KEY_CTRL_1,            -1,                 -1                  },
    {KEY_M,                 RIGHT_CLICK_ICON,   -1                  },
    {KEY_M,                 KEY_SHIFT,          RIGHT_CLICK_ICON    },
    {MOUSE_LEFT,            -1,                 -1                  },
    {KEY_TAB,               RIGHT_CLICK_ICON,   -1                  },
    {MOUSE_LEFT_X2,         KEY_H,              RIGHT_CLICK_ICON    },
    {KEY_SPACE,             TASKBAR_ICON,       -1                  },
    {KEY_M,                 -1,                 -1                  },
    {KEY_B,                 -1,                 -1                  },  // enter build manager
    {-1,                    -1,                 -1                  },  // build a ship
    {-1,                    -1,                 -1                  },  // basic finale
    {-1,                    -1,                 -1                  },  // advanced intro
    {MOUSE_LEFT_X2,         KEY_D,              -1                  },
    {KEY_CTRL_ALT,          -1,                 -1                  },
    {KEY_F1,                -1,                 -1                  },
    {KEY_F1,                -1,                 -1                  },  // need home button art
    {KEY_Z,                 -1,                 -1                  },
    {KEY_CAPSLOCK,          -1,                 -1                  },
    {-1,                    -1,                 -1                  }   // advanced finale
};

#define TUT_BITMAP_X        424     // for the keyboard and mouse bitmaps
#define TUT_BITMAP_Y        360
#define TUT_BITMAP_WIDTH    64
#define TUT_BITMAP_HEIGHT   64

#define TUT_TitleFont           "EyechartDisplayCapsSSK_bi16.hff"
#define TUT_TitleColor          colRGB(255, 255, 105)
#define TUT_Title_X             20
#define TUT_Title_Y             7

#define TUT_MainFont            "SimplixSSK_13.hff"
#define TUT_MainColor           colRGB(255, 255, 105)
#define TUT_Main_X              12
#define TUT_Main_Y              35
#define TUT_MainTextWidth       100
#define TUT_MainTextHeight      200

#define TUT_TipTitleFont        "EyechartDisplayCapsSSK_bi11.hff"
#define TUT_TipTitleColor       colRGB(255, 255, 255)
#define TUT_TipTitle_X          482
#define TUT_TipTitle_Y          35

#define TUT_TipFont             "SimplixSSK_13.hff"
#define TUT_TipColor            colRGB(200, 200, 200)
#define TUT_Tip_X               532
#define TUT_Tip_Y               35
#define TUT_TipTextWidth        100
#define TUT_TipTextHeight       200

#define TUT_BuildColor          colRGB(200,200,200)
#define TUT_Build_X             340
#define TUT_Build_Y             15
#define TUT_BuildTextWidth      280
#define TUT_BuildTextHeight     50

char tutTitleFontName [TUT_MaxFontNameLength] = TUT_TitleFont;
char tutMainFontName [TUT_MaxFontNameLength] = TUT_MainFont;
char tutTipTitleFontName [TUT_MaxFontNameLength] = TUT_TipTitleFont;
char tutTipFontName [TUT_MaxFontNameLength] = TUT_TipFont;

sdword tutLesson;
sdword tutTransition;
sdword tutTransitionCount;
ubyte tutPulseValue;

TutLessonVar tutVar;        // lesson variables

sdword tutSkip = FALSE;

fonthandle tutTitleFont = 0;
fonthandle tutMainFont = 0;
fonthandle tutTipTitleFont = 0;
fonthandle tutTipFont = 0;
regionhandle tutRegion = NULL;

featom tutAtom=
{
    "TutorialDraw",         //  char  *name;                                //optional name of control
    FAF_Function,           //  udword flags;                               //flags to control behavior
    0,                      //  udword status;                              //status flags for this atom, checked etc.
    FA_Button,              //  ubyte  type;                                //type of control (button, scroll bar, etc.)
    0,                      //  ubyte  borderWidth;                         //width, in pixels, of the border
    0,                      //  uword  tabstop;                             //denotes the tab ordering of UI controls
    150,                    //  color  borderColor;                         //optional color of border
    4,                      //  color  contentColor;                        //optional color of content
    10,                     //  sdword x;                                   //-+
    392,                    //  sdword y;                                   // |>rectangle of region
    64,                     //  sdword width;                               // |
    32,                     //  sdword height;                              //-+
    NULL,                   //  ubyte *pData;                               //pointer to type-specific data
    NULL,                   //  ubyte *attribs;                             //sound(button atom) or font(static text atom) reference
    0,                      //  char   hotKeyModifiers;
    0,                      //  char   hotKey[FE_NumberLanguages];
    0,                      //  udword drawstyle[2];
    0,                      //  void*  region;
    0                       //  udword pad[2];
};

color      tutButtonTexture[NUM_BUTTON_TEXTURES];
lifheader  *tutButtonImage[NUM_BUTTON_TEXTURES];

sdword tutLastMainY = TUT_Main_Y + TUT_MainTextHeight;
sdword tutLastTipY = TUT_Tip_Y + TUT_TipTextHeight;

void tutBasicTutorial(char *name, featom *atom)
{
    dbgAssert(startingGame == FALSE);

    tutorial = 1;
    tutLesson = -1;
    tutTransition = 1;
    tutTransitionCount = TUT_TransitionFramesOut - 1;
    dbgMessagef("\nBasic Tutorial started");

    utySinglePlayerGameStart(name,atom);
}

void tutAdvancedTutorial(char *name, featom *atom)
{
    dbgAssert(startingGame == FALSE);

    tutorial = 2;
    tutLesson = TUT_BASIC_FINALE;
    tutTransition = 1;
    tutTransitionCount = TUT_TransitionFramesOut - 1;
    dbgMessagef("\nAdvanced Tutorial started");

    utySinglePlayerGameStart(name,atom);
}


bool FirstWordNULL(char *s)
{
    char line[100];

    if (s == NULL)
    {
        return TRUE;
    }

    line[0] = 0;
    getWord(line,s);
    if(!strcmp(line,"NULL"))
        return (TRUE);
    else
        return (FALSE);
}

sdword DrawTextBlock(char *s, sdword x, sdword y, sdword width, sdword height, color c)
{
    char *oldpos;
    char oldline[100], line[100];
    bool justified, done;

    if(FirstWordNULL(s))
        return(y);
    done = FALSE;
    while (!done)
    {
        justified = FALSE;
        line[0] = 0;
        while (!justified)
        {
            strcpy(oldline, line);
            oldpos = s;
            s = getWord(line, s);

            if (s[0] == '\n')
            {
                justified = TRUE;
                s++;
                while ( s[0] == ' ' ) s++;
            }
            else
            {
                if (fontWidth(line) > width)
                {
                    strcpy(line, oldline);
                    s = oldpos;
                    while ( s[0] == ' ' ) s++;

                    justified = TRUE;
                }
                if (s[0] == 0)
                {
                    justified = TRUE;
                    done      = TRUE;
                }
            }
        }

        fontPrintf(x, y, c, "%s", line);
        y += fontHeight(" ");
        if (y > (y + height))
            done = TRUE;
    }
    y += fontHeight(" ");
    return(y);
}


/*
    InitTutorial
    This routine is called before beginning a particular tutorial page.  It simply
    performs any specific setup a given tutorial may need.
*/

void InitTutorial(long Num)
{
long    i;

    // Standard init stuff
    tutSkip = FALSE;
    tutVar.value = 0;       // re-init generic variables
    tutVar.count = 0;
    tutVar.lessonID = 0;
    tutVar.shipPtr = NULL;

    // Special case init stuff
    switch(Num)
    {
    case TUT_LESSON_ROTATING:
        tutVar.value = currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.angle;
        tutVar.value2 = 0.0f;
        break;

    case TUT_LESSON_SELECT_ONE:
        tutVar.shipPtr = selSelected.ShipPtr[0];
        break;

    case TUT_LESSON_GROUP:
        for(i=0; i<10; i++)
        {
            selHotKeyGroupRemoveReferencesFromAllGroups();
        }
        break;

    case TUT_LESSON_BUILD_SHIP:
        tutPulseValue = 0;
        break;
    }
}

/*
    PassedTutorial
    This routine evaluates whether the criteria for a particular tutorial have been met.

    Returns 0 = failed, 1 = Advance with transition, -1 = Advance without transition

    If the user hits the "Next" button, default behaviour is to advance with transition.
    Placing an "else return 0" in the switch statement will prevent this on a "per tutorial"
    basis.  See the first case for an example of this.
*/

int PassedTutorial(long Num)
{
long    i;
real32  realNum;

    switch(Num)
    {
    case TUT_BASIC_INTRO:
        if(tutSkip)
            return 1;
        else
            return 0;
        break;

    case TUT_LESSON_ROTATING:
        realNum = (currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.angle - tutVar.value);

        if( realNum > DEG_TO_RAD(180) )
            realNum = realNum - DEG_TO_RAD(360);
        else if(realNum < -(DEG_TO_RAD(180)))
            realNum = realNum + DEG_TO_RAD(360);

        tutVar.value2 += realNum;
        tutVar.value = currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.angle;
        if( (tutVar.value2 > DEG_TO_RAD(360)) || (tutVar.value2 < -(DEG_TO_RAD(360))) )
            return 1;
        break;

    case TUT_LESSON_ZOOM:
        realNum = currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.distance;
        if(realNum > 14000)
            return 1;
        break;

    case TUT_LESSON_FOCUS:
        if((tutVar.lessonID == TUT_LESSON_FOCUS) && (tutVar.count == 2))    // must focus 3 times
            return 1;
        break;

    case TUT_LESSON_CANCEL_FOCUS:
        if((tutVar.lessonID == TUT_LESSON_CANCEL_FOCUS) && (tutVar.count == 1))     // must cancel focus 2 times
            return 1;
        break;

    case TUT_LESSON_SELECT_ONE:
        if((selSelected.numShips == 1) && (tutVar.shipPtr != selSelected.ShipPtr[0]))
            return 1;
        break;

    case TUT_LESSON_SELECT_FIVE:
        if(selSelected.numShips >= 5)
            return 1;
        break;

    case TUT_LESSON_CANCEL_SELECT:
        if(selSelected.numShips == 0)
            return 1;
        break;

    case TUT_LESSON_GROUP:
        for(i=0; i<10; i++)
            if(selHotKeyGroup[i].numShips)
                return 1;
        break;

    case TUT_LESSON_2D_MOVE:
    case TUT_LESSON_3D_MOVE:
    case TUT_LESSON_ATTACK:
    case TUT_LESSON_FORMATION:
    case TUT_LESSON_HARVEST:
        // handled by the (tutVar.lessonID == Num) at the end
        break;

    case TUT_LESSON_SENSORS_MANAGER:
    case TUT_LESSON_SENSORS_MANAGER_MOVE:
        if(tutVar.lessonID == tutLesson && (!smSensorsActive))
            return 1;
        break;

    case TUT_LESSON_ENTER_BUILD_MANAGER:
        if(tutVar.lessonID == TUT_LESSON_ENTER_BUILD_MANAGER || tutSkip)
            return -1;      // No transition
        break;

    case TUT_LESSON_BUILD_SHIP:
        if(tutVar.count == 3)
            return -1;
        else
            return 0;
        break;

    case TUT_BASIC_FINALE:
        break;

    case TUT_ADVANCED_INTRO:
    case TUT_LESSON_DOCK:
    case TUT_LESSON_GUARD:
    case TUT_LESSON_FLEET_VIEW:
    case TUT_LESSON_MOTHERSHIP:
    case TUT_LESSON_SPECIAL_OPS:
    case TUT_LESSON_TACTICAL_OVERLAY:
    case TUT_ADVANCED_FINALE:
    case TUT_TOTAL_LESSONS:
        break;
    }

    if(tutVar.lessonID == Num)
        return 1;

    return tutSkip;
}


void tutDrawLinePulse(sdword x0, sdword y0, sdword x1, sdword y1, ubyte pulsevalue, sdword segsize)
{
    real32 x0f = (real32)x0;
    real32 y0f = (real32)y0;
    real32 x1f = (real32)x1;
    real32 y1f = (real32)y1;

    real32 diffx = (x1f - x0f);
    real32 diffy = (y1f - y0f);
    sdword segment;
    real32 segments;
    bool blendon;

    segments = fsqrt(diffx*diffx + diffy*diffy) / (real32)segsize;

    diffx /= segments;
    diffy /= segments;

    if (glCapFeatureExists(GL_LINE_SMOOTH))
    {
        blendon = (bool)glIsEnabled(GL_BLEND);
        if (!blendon) glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
    }
    glBegin(GL_LINES);
    for(segment = 0; segment <= segments; segment++)
    {
        glColor3ub(pulsevalue, pulsevalue, pulsevalue);
        glVertex2f(primScreenToGLX((sdword)x0f), primScreenToGLY((sdword)y0f));
        x0f += diffx;
        y0f += diffy;
        glVertex2f(primScreenToGLX((sdword)x0f), primScreenToGLY((sdword)y0f));
        pulsevalue += 25;
    }
    glEnd();
    if (glCapFeatureExists(GL_LINE_SMOOTH))
    {
        glDisable(GL_LINE_SMOOTH);
        if (!blendon) glDisable(GL_BLEND);
    }
}

void tutExecute(regionhandle reg)
{
    fonthandle currentfont;
    sdword texty, bitmap;

    rectangle rect;                         // used for translucent boxes behind text
    rectangle *buttonrect = &reg->rect;     // used for "next" button
    bool modeset = FALSE;
    sdword bitmap_x, bitmap_vel;

	if(tutorial == 3)
		return;

    if(tutRegion->previous)
        regSiblingMoveToFront(tutRegion);

    if(tutTransition == 0 && tutLesson != TUT_LESSON_ENTER_BUILD_MANAGER && tutLesson != TUT_LESSON_BUILD_SHIP) // don't draw button on build manager lessons
    {
        if(mouseInRect(buttonrect))
        {
            if(mouseLeftButton())
            {
                tutSkip = TRUE;
                trRGBTextureMakeCurrent(tutButtonTexture[NEXT_ON]);
            }
            else
                trRGBTextureMakeCurrent(tutButtonTexture[NEXT_MOUSE]);
        }
        else
            trRGBTextureMakeCurrent(tutButtonTexture[NEXT_OFF]);
        rndPerspectiveCorrection(FALSE);
//      glEnable(GL_BLEND);         // needs to be enabled once Keith fixes alpha blending intel compiler optimization bug
        primRectSolidTextured2(buttonrect);
//      glDisable(GL_BLEND);        // needs to be enabled once Keith fixes alpha blending intel compiler optimization bug
    }

    if(tutTransition == 0)
    {
        switch(PassedTutorial(tutLesson))
        {
        case 0:     // Didn't pass
            break;

        case 1:     // Perform transition
            tutTransition = 1;
            break;

        case -1:    // No Transition
            tutLesson++;
            InitTutorial(tutLesson);
            break;
        }
    }
    else if(tutTransition == 1)
    {
        tutTransitionCount++;
        if(tutTransitionCount == TUT_TransitionFramesOut)
        {
            tutTransition = 2;
            tutTransitionCount = TUT_TransitionFramesIn;
            tutLesson++;
            InitTutorial(tutLesson);
            if((tutLesson >= TUT_ADVANCED_INTRO && tutorial == 1) || (tutLesson >= TUT_TOTAL_LESSONS && tutorial == 2))
            {
                tutorialdone = TRUE;
                return;
            }
        }
    }
    else if(tutTransition == 2)
    {
        tutTransitionCount--;
        if(tutTransitionCount == 0)
            tutTransition = 0;
    }

    if(tutBitmapList[tutLesson][2] != -1)
    {
        bitmap_x = TUT_BITMAP_X;
        bitmap_vel = 210;
    }
    else if(tutBitmapList[tutLesson][1] != -1)
    {
        bitmap_x = TUT_BITMAP_X + (TUT_BITMAP_WIDTH + 8);
        bitmap_vel = 150;
    }
    else
    {
        bitmap_x = TUT_BITMAP_X + ((2 * TUT_BITMAP_WIDTH) + 8);
        bitmap_vel = 90;
    }

    if(tutTransition == 0)
        bitmap_vel = 0;
    else if(tutTransition == 1)
        bitmap_vel = (long) (((float)bitmap_vel/(float)TUT_TransitionFramesOut) * (float)tutTransitionCount);
    else if(tutTransition == 2)
        bitmap_vel = (long) (((float)bitmap_vel/(float)TUT_TransitionFramesIn) * (float)tutTransitionCount);

//  glEnable(GL_BLEND);
    for(bitmap = 0; bitmap < 3; bitmap++)
    {
        if(tutBitmapList[tutLesson][bitmap] != -1)
        {
            trRGBTextureMakeCurrent(tutButtonTexture[tutBitmapList[tutLesson][bitmap]]);
            rndPerspectiveCorrection(FALSE);
            rect.x0 = bitmap_x + ((TUT_BITMAP_WIDTH + 8) * bitmap) + bitmap_vel;
            rect.y0 = TUT_BITMAP_Y;
            rect.x1 = bitmap_x + ((TUT_BITMAP_WIDTH + 8) * bitmap) + TUT_BITMAP_WIDTH + bitmap_vel;
            rect.y1 = TUT_BITMAP_Y + TUT_BITMAP_HEIGHT;
            primRectSolidTextured2(&rect);
        }
    }
//  glDisable(GL_BLEND);

    if (!primModeEnabled)   // draw translucent polys before text
    {
        primModeSet2();
        modeset = TRUE;
    }

    if(tutLesson != TUT_LESSON_BUILD_SHIP)      // default behaviour for lessons
    {
    long    TitlePos = (long)(TUT_TitleTransMult[tutTransition] * (float)tutTransitionCount);
    long    TextPos = (long)(TUT_TextTransMult[tutTransition] * (float)tutTransitionCount);

        rect.x0 = 0;
        rect.y0 = TUT_Title_Y - TitlePos - 7;
        rect.x1 = 640;
        rect.y1 = TUT_Title_Y - TitlePos + 22;
        primRectTranslucent2(&rect, colRGBA(0, 0, 0, 128));

        rect.x0 = TUT_Main_X - TextPos - 12;
        rect.y0 = TUT_Main_Y - 6;
        rect.x1 = TUT_Main_X - TextPos + TUT_MainTextWidth + 4;
        rect.y1 = tutLastMainY;
        primRectTranslucent2(&rect, colRGBA(0, 0, 0, 128));

        currentfont = fontMakeCurrent(tutTitleFont);    // draw text
        fontPrintf(TUT_Title_X, TUT_Title_Y - TitlePos, TUT_TitleColor, "%s", strGetString(strTutorial00Title + (tutLesson * 7)));
        currentfont = fontMakeCurrent(tutMainFont);
        texty = DrawTextBlock(strGetString(strTutorial00Line01 + (tutLesson * 7)), TUT_Main_X - TextPos, TUT_Main_Y, TUT_MainTextWidth, TUT_MainTextHeight, TUT_MainColor);
        texty = DrawTextBlock(strGetString(strTutorial00Line02 + (tutLesson * 7)), TUT_Main_X - TextPos, texty, TUT_MainTextWidth, TUT_MainTextHeight, TUT_MainColor);
        texty = DrawTextBlock(strGetString(strTutorial00Line03 + (tutLesson * 7)), TUT_Main_X - TextPos, texty, TUT_MainTextWidth, TUT_MainTextHeight, TUT_MainColor);
        texty = DrawTextBlock(strGetString(strTutorial00Line04 + (tutLesson * 7)), TUT_Main_X - TextPos, texty, TUT_MainTextWidth, TUT_MainTextHeight, TUT_MainColor);
        tutLastMainY = DrawTextBlock(strGetString(strTutorial00Line05 + (tutLesson * 7)), TUT_Main_X - TextPos, texty, TUT_MainTextWidth, TUT_MainTextHeight, TUT_MainColor);
        fontMakeCurrent(currentfont);
    }
    else                                    // special case for build ship lesson
    {
        currentfont = fontMakeCurrent(tutMainFont);
        texty = DrawTextBlock(strGetString(strTutorial17Line01 + tutVar.count), TUT_Build_X, TUT_Build_Y, TUT_BuildTextWidth, TUT_BuildTextHeight, TUT_BuildColor);
        fontMakeCurrent(currentfont);

        rect.x0 = TUT_Build_X - 8;          // draw outline box around text
        rect.y0 = TUT_Build_Y - 6;
        rect.x1 = TUT_Build_X + TUT_BuildTextWidth + 8;
        rect.y1 = texty;
        primRectOutline2(&rect, 2, colRGB(tutPulseValue, tutPulseValue, tutPulseValue));

        if(tutVar.count == 0)               // point to appropriate place on screen
            tutDrawLinePulse(TUT_Build_X - 8, texty, 160, 100, tutPulseValue, 8);
        else if(tutVar.count == 1)
            tutDrawLinePulse(TUT_Build_X - 8, texty, 180, 410, tutPulseValue, 8);
        else if(tutVar.count == 2)
            tutDrawLinePulse(TUT_Build_X - 8, texty, 550, 410, tutPulseValue, 8);

        tutPulseValue -= 8;
    }

    if(!FirstWordNULL(strGetString(strTutorial00Tip + (tutLesson * 7))))
    {
    long    TipPos = (long)(TUT_TipTransMult[tutTransition] * (float)tutTransitionCount);
    long    TextPos = (long)(TUT_TextTransMult[tutTransition] * (float)tutTransitionCount);

        if (!primModeEnabled)   // draw translucent polys before tip text
        {
            primModeSet2();
            modeset = TRUE;
        }

        rect.x0 = TUT_TipTitle_X - 8;
        rect.y0 = TUT_TipTitle_Y - TipPos - 6;
        rect.x1 = TUT_Tip_X - 8;
        rect.y1 = TUT_TipTitle_Y - TipPos + 18;
        primRectTranslucent2(&rect, colRGBA(0, 0, 0, 128));

        rect.x0 = TUT_Tip_X + TextPos - 8;
        rect.y0 = TUT_Tip_Y - 6;
        rect.x1 = TUT_Tip_X + TextPos + TUT_TipTextWidth + 8;
        rect.y1 = tutLastTipY;
        primRectTranslucent2(&rect, colRGBA(0, 0, 0, 128));

        if(modeset)
            primModeClear2();

        currentfont = fontMakeCurrent(tutTipTitleFont);
        fontPrintf(TUT_TipTitle_X, TUT_TipTitle_Y - TipPos, TUT_TipTitleColor, "%s", strGetString(strTutorialTip));
        currentfont = fontMakeCurrent(tutTipFont);
        tutLastTipY = DrawTextBlock(strGetString(strTutorial00Tip + (tutLesson * 7)), TUT_Tip_X + TextPos, TUT_Tip_Y, TUT_TipTextWidth, TUT_TipTextHeight, TUT_TipColor);
    }

    fontMakeCurrent(currentfont);
}

void tutStartup(void)
{
sdword count;

	if(tutorial == 3)
	{
		tutInitialize();
		return;
	}

    tutorialdone = FALSE;
    if(tutRegion != NULL)
    {
        regChildInsert(tutRegion, &regRootRegion);
    }
    else
    {
        tutRegion = regChildAlloc(&regRootRegion, (sdword)&tutAtom, tutAtom.x, tutAtom.y, tutAtom.width, tutAtom.height, 0, 0);
        tutAtom.region = (void*)tutRegion;
        tutRegion->atom = &tutAtom;
        regDrawFunctionSet(tutRegion, tutExecute);
    }

    tutTitleFont  = frFontRegister(tutTitleFontName);
    tutMainFont  = frFontRegister(tutMainFontName);
    tutTipTitleFont  = frFontRegister(tutTipTitleFontName);
    tutTipFont  = frFontRegister(tutTipFontName);

    for(count = 0; count < NUM_BUTTON_TEXTURES; count++)
    {
        tutButtonTexture[count] = TR_InvalidInternalHandle;
        tutButtonImage[count] = NULL;
        tutButtonImage[count] = trLIFFileLoad(tutTextureNames[count], NonVolatile);
        dbgAssert(tutButtonImage[count] != NULL);
        tutButtonTexture[count] = trRGBTextureCreate((color *)tutButtonImage[count]->data, tutButtonImage[count]->width, tutButtonImage[count]->height, TRUE);
    }
}

void tutShutdown(void)
{
    sdword count;

	if(tutorial == 3)
	{
		tutUnInitialize();
		return;
	}

    for(count = 0; count < NUM_BUTTON_TEXTURES; count++)
    {
        if (tutButtonImage[count] != NULL)
        {
            memFree(tutButtonImage[count]);
            tutButtonImage[count] = NULL;
        }
        if (tutButtonTexture[count] != TR_InvalidInternalHandle)
        {
            trRGBTextureDelete(tutButtonTexture[count]);
            tutButtonTexture[count] = TR_InvalidInternalHandle;
        }
    }

    if(tutRegion != NULL)
    {
        regLinkRemove(tutRegion);
    }
}

