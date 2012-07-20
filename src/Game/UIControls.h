/*=============================================================================
    Name    : UIcontrols.h
    Purpose : Definitions for basic UI control types (buttons etc.)

    Created 7/7/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___UICONTROLS_H
#define ___UICONTROLS_H

#include "Types.h"
#include "Region.h"
#include "FEFlow.h"
#include "font.h"
#include "LinkedList.h"
#include "prim2d.h"

/*=============================================================================
    Switches
=============================================================================*/
#ifndef HW_Release

#define UIC_ERROR_CHECKING      1               //general error checking
#define UIC_VERBOSE_LEVEL       1               //print extra info
#define UIC_TEST                0               //test the module

#else //HW_Debug

#define UIC_ERROR_CHECKING      0               //general error checking
#define UIC_VERBOSE_LEVEL       0               //print extra info
#define UIC_TEST                0               //test the module

#endif //HW_Debug

/*=============================================================================
    Defitions:
=============================================================================*/
//flags for a control
#define CFM_ControlType         0x000000ff
#define CF_DrawGreyed           0x80000000
#define CF_Disabled             0x40000000
#define CF_Greyed               (UIF_DrawGreyed | UIF_Disabled)//button disabled

//messages from a control
#define CM_ButtonClick          0x00000100      //basic button clicked
#define CM_ToggleDown           CM_ButtonClick  //toggle button down
#define CM_ToggleUp             0x00000200      //toggle button up
#define CM_ListExpand           CM_ToggleDown   //list expand +
#define CM_ListContract         CM_ToggleDown   //list expand -
#define CM_ButtonDrag           0x00000400      //dragging mouse cursor.

// the following messages are for listwindows and text entry boxes
#define CM_GainFocus            1               //text entry gained keyboard focus
#define CM_LoseFocus            2               //text entry lost focus
#define CM_AcceptText           3               //text entry accepted (enter)
#define CM_RejectText           4               //text entry rejected (escape)
#define CM_KeyPressed           5               //text entry keypressed
#define CM_NewItemSelected      6               //list window item selected
#define CM_SpecialKey           7               //special key pressed
#define CM_DoubleClick          8               //double click
#define CM_KeyCaptured          9               //key has been captured

#define CM_ThumbMoved           0x00000400      //thumbwheel moved

//additional info passed to control processor functions
#define UIC_RegionMessage       1

//special defines for text entry boxes
#define UTE_DefaultBufferSize   1024

//stock region filters for different types of UI controls
#define UIF_Button              (RPE_EnterHoldLeft | RPE_ExitHoldLeft | RPE_PressLeft | RPE_ReleaseLeft | RPE_Enter | RPE_Exit)
#define UIF_CheckBox            (UIF_Button | RPE_WheelDown | RPE_WheelUp | RPE_HoldLeft | RPE_EnterHoldLeft)
#define UIF_TextEntry           (UIF_Button | RPE_KeyDown)
#define UIF_DragButton          (UIF_Button | RPE_HoldLeft)

// defines for scrollbar region ID's
#define UIC_ScrollbarTab        1
#define UIC_ScrollbarUpButton   2
#define UIC_ScrollbarDownButton 3

// special defines for flags in the text entry boxes.
#define UICTE_FileNameInvalids  "\\/:*?\"<>|"
#define UICTE_NoLossOfFocus     BIT0    // text entry box cannot lose focus on enter pressed
#define UICTE_PasswordEntry     BIT1    // makes any text entered into 'x' so that it is hidden from view
#define UICTE_NumberEntry       BIT2    // only accepts numbers integers only and must be positive
#define UICTE_NoTextures        BIT3    // only draw the text don't draw the textured borders.
#define UICTE_UserNameEntry     BIT4    // only accept Characters and numbers, no spaces
#define UICTE_ChatTextEntry     BIT5    // call the user function each time a key is pressed.
#define UICTE_SpecialChars      BIT6    // call the user function each time special key is pressed, pgup, pgdown
#define UICTE_DropShadow        BIT7    // draw a dropshadow when drawing the text
#define UICTE_FileName          BIT8    // don't accept invalid filename characters

//special defines for the window box
#define UICLW_HasTitleBar       BIT0    // has a title bar
#define UICLW_CanSelect         BIT1    // if you can select items from the list at all
#define UICLW_CanClickOnTitle   BIT2    // can you click on the title or is it static
#define UICLW_JustRedrawTitle   BIT3    // just redraw the title only, can change dynamically
#define UICLW_AutoScroll        BIT4    // the list will autoscroll when items are added.
#define UICLW_CanHaveFocus      BIT5    // can the list take keyboard focus or not
#define UICLW_AutoScrollOff     BIT6    // temporarily disable auto scrolling
#define UICLW_GetKeyPressed     BIT7    // send the next key to the callback region

#define UICLW_AddToHead         1       // add the item to the head of the list
#define UICLW_AddToTail         2       // add the item to the tail of the list

//special defines for the window box list items
#define UICLI_CanSelect         BIT0    // can select this item from the list
#define UICLI_Selected          BIT1    // is this item selected now or not. useful for drawing.

//drag threshold for drag buttons
#define UIC_DragButtonThresholdX   4
#define UIC_DragButtonThresholdY   4
/*=============================================================================
    Type definitions:
=============================================================================*/

typedef struct uicbutton *buttonhandle;
typedef regionhandle controlhandle;
typedef regionfunction uicfunction;

typedef struct uicscrollbar   *scrollbarhandle;
typedef struct uicscrollbarbutton *scrollbarbuttonhandle;
typedef struct uictextentry   *textentryhandle;
typedef struct uiclistwindow  *listwindowhandle;
typedef struct uiclistitem    *listitemhandle;
typedef struct uicslider      *sliderhandle;


typedef void (*listitemdraw)        (rectangle *rect, listitemhandle data);
typedef void (*listtitlebarclick)   (struct tagRegion *reg, sdword xClicked);
typedef void (*listtitlebardraw)    (rectangle *rect);

//structure for a button control.  This includes regular buttons,
//toggle buttons, check boxes and radio buttons
typedef struct uicbutton
{
    region reg;
    uicfunction processFunction;                //function to recieve notification
//    udword regionFilter;                        //region messages to pass to control processor
    color contentColor, borderColor;            //colors of button
    fescreen *screen;                           //optional front-end atom pointer
    sdword clickX, clickY;                      //where the mouse was clicked to start dragging (only for drag buttons)
    bool bDragged;                              //was this button ever dragged?
}
uicbutton;

//structure for a scroll bar up and down buttons
typedef struct uicscrollbarbutton
{
    region reg;
    uicfunction processFunction;
    udword regionFilter;
    color contentColor, borderColor;
    fescreen *screen;
    //in absolute coords
    scrollbarhandle scrollbar;
}
uicscrollbarbutton;

//structure for a scroll bar control
typedef struct uicscrollbar
{
    region reg;
    uicfunction processFunction;
    udword regionFilter;
    color contentColor, borderColor;
    fescreen *screen;
    //in absolute coords
    scrollbarbuttonhandle upbutton;
    scrollbarbuttonhandle downbutton;
    rectangle thumbreg;         //region for thumbwheeling, smaller than reg.rect
    rectangle thumb;            //placement of thumbwheel
    udword  event;
    uword mouseX, mouseY;
    sword mouseMoved;           //relative mouse coord for thumb drag
    real32 divSize;
    bool8 isVertical;
    listwindowhandle listwindow;

    udword clickType;
}
uicscrollbar;

//structure for a slider control.
typedef struct uicslider
{
    region reg;
    uicfunction processFunction;                //function to recieve notification
    udword regionFilter;                        //region messages to pass to control processor
    color contentColor, borderColor;            //colors of button
    fescreen *screen;                           //optional front-end atom pointer
    udword value;                               //current value of slider
    udword maxvalue;                            //max value of slider
    sdword ID;                                  //for a bunch of of sliders
    uword mouseX, mouseY;
    sword mouseMoved;           //relative mouse coord for thumb drag
}
uicslider;



//structure for a text entry control
typedef struct uictextentry
{
    region reg;
    uicfunction processFunction;
    udword regionFilter;
    color contentColor, borderColor;
    fescreen *screen;                           //screen this control belongs to
    char *textBuffer;                           //the text buffer we're editing
    sdword bufferLength;                        //length of the text buffer, as allocated
    sdword iCharacter;                          //current character (position of the cursor)
    sdword iVisible;                            //first visible character
    fonthandle currentFont;                     //font to draw the text in
    real32 flashRate;                           //rate the caret flashes at
    rectangle textRect;                         //the actual rectangle for the text
    udword message;                             //method for passing messages to text entry handlers
    udword textflags;
    udword key;
    color shadowColor;                          //color of the drop shadow, if any
}
uictextentry;

typedef struct uiclistitem
{
    struct Node     link;                       // link to the other items in the list
    udword          flags;                      // flags for this particular item
    sdword          position;
    ubyte          *data;                       // pointer to some user specified data.
}
uiclistitem;

//structure for a list window control
typedef struct uiclistwindow
{
    region              reg;
    uicfunction         processFunction;
    udword              regionFilter;
    color               contentColor, borderColor;
    fescreen           *screen;                 //screen this control belongs to
    udword              windowflags;            // flags for the list window, clickable etc...
    sdword              TitleHeight;            // data for the title bar, callbacks etc.
    listtitlebarclick   titleprocess;           // function called when someone clicks on the title bar, if it is enabled
    listtitlebardraw    titledraw;              // function called when the title bar needs to be drawn
                                                // data for the list items callbacks etc.
    sdword              sorttype;               // info for sorting the list, useful if the list can be sorted in multiple ways.
    bool                sortOrder;
    sdword              itemheight;             // height of each items in pixels, the max number of items on screen at once is calculated dynamically
    LinkedList          listofitems;            // linked list of items in the list
    listitemdraw        itemdraw;               // function called to draw each item, if 10 items onscreen, it is called 10 times.
    listitemhandle      topitem;                // pointer to the topitem on the screen, internal
    scrollbarhandle     scrollbar;              // pointer to the scrollbar that the window uses
    featom              scrollbaratom;
    sdword              ListTotal;              // total number of items in the list
    sdword              UpperIndex;             // index into the list that the topitem resides at, relative to 0
    sdword              MaxIndex;               // maximum number of items that can fit onscreen at once.
    listitemhandle      CurLineSelected;        // current line selcted.  Can read this to see if an item is selected, NULL otherwise.
    udword              message;                // message to pass to the atom callback.
    udword              keypressed;
}
uiclistwindow;

#define uicListWindowRelativePos(listhandle)    (listhandle->CurLineSelected->position-listhandle->topitem->position)

typedef struct approacher
{
    real32 currentvalue;
    real32 target;
    real32 vel;
    real32 acc;
    real32 threshold;
}
approacher;

/*=============================================================================
    Data:
=============================================================================*/
extern sdword uicDragX;
extern sdword uicDragY;

/*=============================================================================
    Macros:
=============================================================================*/
#define uicStructureExtra(type)     (sizeof(type) - sizeof(region))
#define uicContentColor(control, color) ((control)->contentColor = (color))
#define uicBorderColor(control, color) ((control)->borderColor = (color))
#define uicTextEntryMessage(atom)    (((textentryhandle)atom->pData)->message)

/*=============================================================================
    Functions:
=============================================================================*/
//startup/shutdown
void uicStartup(void);
void uicShutdown(void);

//create controls
buttonhandle uicChildButtonAlloc(controlhandle parent, sdword ID, sdword x, sdword y,
                sdword width, sdword height, uicfunction function, udword flags);

scrollbarhandle uicChildScrollBarAlloc(controlhandle parent, sdword ID,
                                       sdword x, sdword y, sdword width, sdword height,
                                       uicfunction function, udword flags);

listwindowhandle uicChildListWindowAlloc(controlhandle parent, sdword ID,
                                         sdword x, sdword y, sdword width, sdword height,
                                         uicfunction function, udword flags);

textentryhandle uicChildTextEntryAlloc(controlhandle parent, featom *atom,
                                       sdword x, sdword y, sdword width, sdword height,
                                       udword filter);
//delete a control
void uicControlDelete(controlhandle handle);

//functions for keyboard front end navigation
udword uicTabProcess(struct tagRegion *reg, sdword num_buttons, udword event, udword data);
udword uicRightArrowProcess(struct tagRegion *reg, sdword num_buttons, udword event, udword data);
udword uicLeftArrowProcess(struct tagRegion *reg, sdword num_buttons, udword event, udword data);
udword uicUpArrowProcess(struct tagRegion *reg, sdword num_buttons, udword event, udword data);
udword uicDownArrowProcess(struct tagRegion *reg, sdword num_buttons, udword event, udword data);
udword uicSpacebarProcess(struct tagRegion *reg, sdword num_buttons, udword event, udword data);
udword uicReturnProcess(struct tagRegion *reg, sdword num_buttons, udword event, udword data);
udword uicEscProcess(struct tagRegion *reg, sdword num_buttons, udword event, udword data);
udword uicHomeProcess(struct tagRegion *reg, sdword num_buttons, udword event, udword data);
udword uicEndProcess(struct tagRegion *reg, sdword num_buttons, udword event, udword data);


//utility functions for front end navigation
regionhandle uicFindFlag(regionhandle find, udword flag, uword type);
regionhandle uicFindTabstop(regionhandle find);
void uicSetCurrent(regionhandle reg, bool bUserInput);
bool uicClearCurrent(regionhandle reg);


//adjust attributes of the basic control
void uicTextEntryInit(textentryhandle entry, udword flags);
void uicTextEntrySet(textentryhandle entry, char *text, sdword cursorPos);
void uicTextEntryGet(textentryhandle entry, char *dest, sdword maxLength);
void uicTextBufferResize(textentryhandle entry, sdword size);
void uicTextEntryCleanUp(textentryhandle entry);
bool uicBackspaceCharacter(textentryhandle entry);

// adjust attributes and manage the list window control
void uicListWindowInit(listwindowhandle     listwindow,
                       listtitlebardraw     titledraw,
                       listtitlebarclick    titleclick,
                       sdword               titleheight,
                       listitemdraw         itemdraw,
                       sdword               itemheight,
                       udword               flags);
void uicListScrollBarAdjust(listwindowhandle listwindow);
listitemhandle uicListAddItem(listwindowhandle listwindow, ubyte *data, udword flags, udword where);
listitemhandle uicListFindItemByData(listwindowhandle listwindow, ubyte *data);
void uicListRemoveItemByData(listwindowhandle listwindow, ubyte *data);
void uicListRemoveItem(listwindowhandle listwindow, listitemhandle item);
void uicListRemoveAllItems(listwindowhandle listwindow);
void uicListCleanUp(listwindowhandle listwindow);
void uicListSort(listwindowhandle listwindow, MergeSortCompareCb compare);
void uicListWindowPageUp(listwindowhandle listhandle);
void uicListWindowPageDown(listwindowhandle listhandle);
void uicListWindowSetCurItem(listwindowhandle listhandle, listitemhandle item);


//...
#endif //___UICONTROLS_H
