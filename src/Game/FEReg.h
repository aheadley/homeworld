/*=============================================================================
    Name    : FeReg.h
    Purpose : Header for Front End Texture registry logic and data

    Created 3/9/1998 by fpoiker
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#ifndef ___FEREG_H
#define ___FEREG_H

#include "FontReg.h"
#include "texreg.h"
#include "LinkedList.h"
#include "Memory.h"
#include "Region.h"
#include "UIControls.h"

/*=============================================================================
    Definitions:
=============================================================================*/
//texture list
#ifdef _WIN32
#define FER_PATH                             "FeMan\\Textures\\"
#else
#define FER_PATH                             "FeMan/Textures/"
#endif

//box textures
#define FER_BOX_OUTER_CORNER            FER_PATH"medium_convex.lif"
#define FER_BOX_INNER_CORNER            FER_PATH"medium_concave.lif"
#define FER_BOX_OUTGLOW_STRAIGHT        FER_PATH"medium_convex_straight.lif"
#define FER_BOX_INGLOW_STRAIGHT         FER_PATH"medium_concave_straight.lif"
#define FER_BOX_SMALL_OUTER_CORNER      FER_PATH"small_convex.lif"
#define FER_BOX_SMALL_INNER_CORNER      FER_PATH"small_concave.lif"
#define FER_BOX_SMALL_OUTGLOW_STRAIGHT  FER_PATH"small_convex_straight.lif"
#define FER_BOX_SMALL_INGLOW_STRAIGHT   FER_PATH"small_concave_straight.lif"
#define FER_BOX_LARGE_OUTER_CORNER      FER_PATH"large_convex.lif"
#define FER_BOX_LARGE_INNER_CORNER      FER_PATH"large_concave.lif"
#define FER_BOX_LARGE_OUTGLOW_STRAIGHT  FER_PATH"large_convex_straight.lif"
#define FER_BOX_LARGE_INGLOW_STRAIGHT   FER_PATH"large_concave_straight.lif"
#define FER_BOX_NOGLOW_CORNER           FER_PATH"medium_corner.lif"
#define FER_BOX_NOGLOW_SMALL_CORNER     FER_PATH"small_corner.lif"
#define FER_BOX_NOGLOW_LARGE_CORNER     FER_PATH"large_corner.lif"
#define FER_BOX_NOGLOW_STRAIGHT         FER_PATH"thin_straight.lif"

//button textures
#define FER_BUTTON_OFF_LEFT                 FER_PATH"button_off_left_endcap.lif"
#define FER_BUTTON_OFF_MID                  FER_PATH"button_off_mid.lif"
#define FER_BUTTON_OFF_RIGHT                FER_PATH"button_off_right_endcap.lif"
#define FER_BUTTON_OFF_FOCUS_LEFT           FER_PATH"button_off_focus_left_endcap.lif"
#define FER_BUTTON_OFF_FOCUS_MID            FER_PATH"button_off_focus_mid.lif"
#define FER_BUTTON_OFF_FOCUS_RIGHT          FER_PATH"button_off_focus_right_endcap.lif"
#define FER_BUTTON_OFF_FOCUS_MOUSE_LEFT     FER_PATH"button_off_focus_mouse_left_endcap.lif"
#define FER_BUTTON_OFF_FOCUS_MOUSE_MID      FER_PATH"button_off_focus_mouse_mid.lif"
#define FER_BUTTON_OFF_FOCUS_MOUSE_RIGHT    FER_PATH"button_off_focus_mouse_right_endcap.lif"
#define FER_BUTTON_OFF_MOUSE_LEFT           FER_PATH"button_off_mouse_left_endcap.lif"
#define FER_BUTTON_OFF_MOUSE_MID            FER_PATH"button_off_mouse_mid.lif"
#define FER_BUTTON_OFF_MOUSE_RIGHT          FER_PATH"button_off_mouse_right_endcap.lif"
#define FER_BUTTON_OFF_DISABLED_LEFT        FER_PATH"button_off_disabled_left_endcap.lif"
#define FER_BUTTON_OFF_DISABLED_MID         FER_PATH"button_off_disabled_mid.lif"
#define FER_BUTTON_OFF_DISABLED_RIGHT       FER_PATH"button_off_disabled_right_endcap.lif"
#define FER_BUTTON_ON_FOCUS_MOUSE_LEFT      FER_PATH"button_on_focus_mouse_left_endcap.lif"
#define FER_BUTTON_ON_FOCUS_MOUSE_MID       FER_PATH"button_on_focus_mouse_mid.lif"
#define FER_BUTTON_ON_FOCUS_MOUSE_RIGHT     FER_PATH"button_on_focus_mouse_right_endcap.lif"

// text box textures
#define FER_TEXT_OFF_LEFT                   FER_PATH"textbox_off_left_endcap.lif"
#define FER_TEXT_OFF_MID                    FER_PATH"textbox_off_mid.lif"
#define FER_TEXT_OFF_RIGHT                  FER_PATH"textbox_off_right_endcap.lif"
#define FER_TEXT_OFF_FOCUS_LEFT             FER_PATH"textbox_off_focus_left_endcap.lif"
#define FER_TEXT_OFF_FOCUS_MID              FER_PATH"textbox_off_focus_mid.lif"
#define FER_TEXT_OFF_FOCUS_RIGHT            FER_PATH"textbox_off_focus_right_endcap.lif"
#define FER_TEXT_OFF_FOCUS_MOUSE_LEFT       FER_PATH"textbox_off_focus_mouse_left_endcap.lif"
#define FER_TEXT_OFF_FOCUS_MOUSE_MID        FER_PATH"textbox_off_focus_mouse_mid.lif"
#define FER_TEXT_OFF_FOCUS_MOUSE_RIGHT      FER_PATH"textbox_off_focus_mouse_right_endcap.lif"
#define FER_TEXT_OFF_MOUSE_LEFT             FER_PATH"textbox_off_mouse_left_endcap.lif"
#define FER_TEXT_OFF_MOUSE_MID              FER_PATH"textbox_off_mouse_mid.lif"
#define FER_TEXT_OFF_MOUSE_RIGHT            FER_PATH"textbox_off_mouse_right_endcap.lif"
#define FER_TEXT_OFF_DISABLED_LEFT          FER_PATH"textbox_disabled_left_endcap.lif"
#define FER_TEXT_OFF_DISABLED_MID           FER_PATH"textbox_disabled_mid.lif"
#define FER_TEXT_OFF_DISABLED_RIGHT         FER_PATH"textbox_disabled_right_endcap.lif"
#define FER_TEXT_ON_FOCUS_MOUSE_LEFT        FER_PATH"textbox_on_focus_mouse_left_endcap.lif"
#define FER_TEXT_ON_FOCUS_MOUSE_MID         FER_PATH"textbox_on_focus_mouse_mid.lif"
#define FER_TEXT_ON_FOCUS_MOUSE_RIGHT       FER_PATH"textbox_on_focus_mouse_right_endcap.lif"

// toggle button textures 10 states
#define FER_TOGGLE_ON_LEFT                  FER_PATH"toggle_on_left_endcap.lif"
#define FER_TOGGLE_ON_MID                   FER_PATH"toggle_on_mid.lif"
#define FER_TOGGLE_ON_RIGHT                 FER_PATH"toggle_on_right_endcap.lif"
#define FER_TOGGLE_ON_FOCUS_LEFT            FER_PATH"toggle_on_focus_left_endcap.lif"
#define FER_TOGGLE_ON_FOCUS_MID             FER_PATH"toggle_on_focus_mid.lif"
#define FER_TOGGLE_ON_FOCUS_RIGHT           FER_PATH"toggle_on_focus_right_endcap.lif"
#define FER_TOGGLE_ON_MOUSE_LEFT            FER_PATH"toggle_on_mouse_left_endcap.lif"
#define FER_TOGGLE_ON_MOUSE_MID             FER_PATH"toggle_on_mouse_mid.lif"
#define FER_TOGGLE_ON_MOUSE_RIGHT           FER_PATH"toggle_on_mouse_right_endcap.lif"
#define FER_TOGGLE_ON_FOCUS_MOUSE_LEFT      FER_PATH"toggle_on_focus_mouse_left_endcap.lif"
#define FER_TOGGLE_ON_FOCUS_MOUSE_MID       FER_PATH"toggle_on_focus_mouse_mid.lif"
#define FER_TOGGLE_ON_FOCUS_MOUSE_RIGHT     FER_PATH"toggle_on_focus_mouse_right_endcap.lif"
#define FER_TOGGLE_ON_DISABLED_LEFT         FER_PATH"toggle_disabled_on_left_endcap.lif"
#define FER_TOGGLE_ON_DISABLED_MID          FER_PATH"toggle_disabled_on_mid.lif"
#define FER_TOGGLE_ON_DISABLED_RIGHT        FER_PATH"toggle_disabled_on_right_endcap.lif"
#define FER_TOGGLE_OFF_LEFT                 FER_PATH"toggle_off_left_endcap.lif"
#define FER_TOGGLE_OFF_MID                  FER_PATH"toggle_off_mid.lif"
#define FER_TOGGLE_OFF_RIGHT                FER_PATH"toggle_off_right_endcap.lif"
#define FER_TOGGLE_OFF_FOCUS_LEFT           FER_PATH"toggle_off_focus_left_endcap.lif"
#define FER_TOGGLE_OFF_FOCUS_MID            FER_PATH"toggle_off_focus_mid.lif"
#define FER_TOGGLE_OFF_FOCUS_RIGHT          FER_PATH"toggle_off_focus_right_endcap.lif"
#define FER_TOGGLE_OFF_MOUSE_LEFT           FER_PATH"toggle_off_mouse_left_endcap.lif"
#define FER_TOGGLE_OFF_MOUSE_MID            FER_PATH"toggle_off_mouse_mid.lif"
#define FER_TOGGLE_OFF_MOUSE_RIGHT          FER_PATH"toggle_off_mouse_right_endcap.lif"
#define FER_TOGGLE_OFF_FOCUS_MOUSE_LEFT     FER_PATH"toggle_off_focus_mouse_left_endcap.lif"
#define FER_TOGGLE_OFF_FOCUS_MOUSE_MID      FER_PATH"toggle_off_focus_mouse_mid.lif"
#define FER_TOGGLE_OFF_FOCUS_MOUSE_RIGHT    FER_PATH"toggle_off_focus_mouse_right_endcap.lif"
#define FER_TOGGLE_OFF_DISABLED_LEFT        FER_PATH"toggle_disabled_off_left_endcap.lif"
#define FER_TOGGLE_OFF_DISABLED_MID         FER_PATH"toggle_disabled_off_mid.lif"
#define FER_TOGGLE_OFF_DISABLED_RIGHT       FER_PATH"toggle_disabled_off_right_endcap.lif"

// radio button textures 10 states
#define FER_RADIO_ON                        FER_PATH"radio_on.lif"
#define FER_RADIO_ON_FOCUS                  FER_PATH"radio_on_focus.lif"
#define FER_RADIO_ON_MOUSE                  FER_PATH"radio_on_mouse.lif"
#define FER_RADIO_ON_FOCUS_MOUSE            FER_PATH"radio_on_focus_mouse.lif"
#define FER_RADIO_ON_DISABLED               FER_PATH"radio_disabled_on.lif"
#define FER_RADIO_OFF                       FER_PATH"radio_off.lif"
#define FER_RADIO_OFF_FOCUS                 FER_PATH"radio_off_focus.lif"
#define FER_RADIO_OFF_MOUSE                 FER_PATH"radio_off_mouse.lif"
#define FER_RADIO_OFF_FOCUS_MOUSE           FER_PATH"radio_off_focus_mouse.lif"
#define FER_RADIO_OFF_DISABLED              FER_PATH"radio_disabled_off.lif"

//checkbox textures 10 states
#define FER_CHECK_ON                        FER_PATH"checkbox_on.lif"
#define FER_CHECK_ON_FOCUS                  FER_PATH"checkbox_on_focus.lif"
#define FER_CHECK_ON_MOUSE                  FER_PATH"checkbox_on_mouse.lif"
#define FER_CHECK_ON_FOCUS_MOUSE            FER_PATH"checkbox_on_focus_mouse.lif"
#define FER_CHECK_ON_DISABLED               FER_PATH"checkbox_disabled_on.lif"
#define FER_CHECK_OFF                       FER_PATH"checkbox_off.lif"
#define FER_CHECK_OFF_FOCUS                 FER_PATH"checkbox_off_focus.lif"
#define FER_CHECK_OFF_MOUSE                 FER_PATH"checkbox_off_mouse.lif"
#define FER_CHECK_OFF_FOCUS_MOUSE           FER_PATH"checkbox_off_focus_mouse.lif"
#define FER_CHECK_OFF_DISABLED              FER_PATH"checkbox_disabled_off.lif"


//scrollbar textures

#define FER_VERTSB_TOP                      FER_PATH"vscrollbar_topcap.lif"
#define FER_VERTSB_MID                      FER_PATH"vscrollbar_mid.lif"
#define FER_VERTSB_BOTTOM                   FER_PATH"vscrollbar_botcap.lif"
#define FER_VERTST_OFF_TOP                  FER_PATH"vscrolltab_off_topcap.lif"
#define FER_VERTST_OFF_MID                  FER_PATH"vscrolltab_off_mid.lif"
#define FER_VERTST_OFF_BOTTOM               FER_PATH"vscrolltab_off_botcap.lif"
#define FER_VERTST_OFF_MOUSE_TOP            FER_PATH"vscrolltab_mouse_topcap.lif"
#define FER_VERTST_OFF_MOUSE_MID            FER_PATH"vscrolltab_mouse_mid.lif"
#define FER_VERTST_OFF_MOUSE_BOTTOM         FER_PATH"vscrolltab_mouse_botcap.lif"
#define FER_VERTST_ON_TOP                   FER_PATH"vscrolltab_on_topcap.lif"
#define FER_VERTST_ON_MID                   FER_PATH"vscrolltab_on_mid.lif"
#define FER_VERTST_ON_BOTTOM                FER_PATH"vscrolltab_on_botcap.lif"
#define FER_VERTSA_OFF_UP                   FER_PATH"vscrollbar_off_up.lif"
#define FER_VERTSA_OFF_DOWN                 FER_PATH"vscrollbar_off_down.lif"
#define FER_VERTSA_OFF_MOUSE_UP             FER_PATH"vscrollbar_mouse_up.lif"
#define FER_VERTSA_OFF_MOUSE_DOWN           FER_PATH"vscrollbar_mouse_down.lif"
#define FER_VERTSA_ON_UP                    FER_PATH"vscrollbar_on_up.lif"
#define FER_VERTSA_ON_DOWN                  FER_PATH"vscrollbar_on_down.lif"

#define FER_LIST_NW_CORNER                  FER_PATH"info_nw_corner.lif"
#define FER_LIST_NE_CORNER                  FER_PATH"info_ne_corner.lif"
#define FER_LIST_SE_CORNER                  FER_PATH"info_se_corner.lif"
#define FER_LIST_SW_CORNER                  FER_PATH"info_sw_corner.lif"
#define FER_LIST_N_STRAIGHT                 FER_PATH"info_n_straight.lif"
#define FER_LIST_E_STRAIGHT                 FER_PATH"info_e_straight.lif"
#define FER_LIST_S_STRAIGHT                 FER_PATH"info_s_straight.lif"
#define FER_LIST_W_STRAIGHT                 FER_PATH"info_w_straight.lif"
#define FER_LIST_MID                        FER_PATH"info_mid.lif"
#define FER_LIST_FOCUS_NW_CORNER            FER_PATH"info_focus_nw_corner.lif"
#define FER_LIST_FOCUS_NE_CORNER            FER_PATH"info_focus_ne_corner.lif"
#define FER_LIST_FOCUS_SE_CORNER            FER_PATH"info_focus_se_corner.lif"
#define FER_LIST_FOCUS_SW_CORNER            FER_PATH"info_focus_sw_corner.lif"
#define FER_LIST_FOCUS_N_STRAIGHT           FER_PATH"info_focus_n_straight.lif"
#define FER_LIST_FOCUS_E_STRAIGHT           FER_PATH"info_focus_e_straight.lif"
#define FER_LIST_FOCUS_S_STRAIGHT           FER_PATH"info_focus_s_straight.lif"
#define FER_LIST_FOCUS_W_STRAIGHT           FER_PATH"info_focus_w_straight.lif"
#define FER_LIST_FOCUS_MID                  FER_PATH"info_focus_mid.lif"


//menu item textures
#define FER_MENU_MOUSEOVER_CENTRE       FER_PATH"menu_mouseover_mid.lif"
#define FER_MENU_MOUSEOVER_LEFT_CAP     FER_PATH"menu_mouseover_leftcap.lif"
#define FER_MENU_MOUSEOVER_RIGHT_CAP    FER_PATH"menu_mouseover_rightcap.lif"
#define FER_MENU_SELECTED_DOT           FER_PATH"menu_selected.lif"
#define FER_MENU_POPOUT_ARROW           FER_PATH"menu_arrow.lif"


//new left/right arrow buttons
#define FER_ARROW_LEFT_OFF              FER_PATH"arrow_left_off.lif"
#define FER_ARROW_LEFT_MOUSE            FER_PATH"arrow_left_mouse.lif"
#define FER_ARROW_LEFT_ON               FER_PATH"arrow_left_on.lif"
#define FER_ARROW_RIGHT_OFF             FER_PATH"arrow_right_off.lif"
#define FER_ARROW_RIGHT_MOUSE           FER_PATH"arrow_right_mouse.lif"
#define FER_ARROW_RIGHT_ON              FER_PATH"arrow_right_on.lif"

#define FER_BUILD_ARROW_LEFT_OFF        FER_PATH"build_arrow_left_off.lif"
#define FER_BUILD_ARROW_LEFT_MOUSE      FER_PATH"build_arrow_left_mouse.lif"
#define FER_BUILD_ARROW_LEFT_ON         FER_PATH"build_arrow_left_on.lif"
#define FER_BUILD_ARROW_RIGHT_OFF       FER_PATH"build_arrow_right_off.lif"
#define FER_BUILD_ARROW_RIGHT_MOUSE     FER_PATH"build_arrow_right_mouse.lif"
#define FER_BUILD_ARROW_RIGHT_ON        FER_PATH"build_arrow_right_on.lif"

#define FER_SHIPVIEW_LIGHTS_OFF         FER_PATH"shipview_lights_off.lif"
#define FER_SHIPVIEW_LIGHTS_TOP         FER_PATH"shipview_lights_top.lif"
#define FER_SHIPVIEW_LIGHTS_BOTTOM      FER_PATH"shipview_lights_bottom.lif"
#define FER_SHIPVIEW_LIGHTS_BOTH        FER_PATH"shipview_lights_both.lif"


#define FER_VOLUME_BUTTON               FER_PATH"volume_button.lif"
#define FER_VOLUME_DISABLED_LEFTCAP     FER_PATH"volume_disabled_leftcap.lif"
#define FER_VOLUME_DISABLED_MID         FER_PATH"volume_disabled_mid.lif"
#define FER_VOLUME_DISABLED_RIGHTCAP    FER_PATH"volume_disabled_rightcap.lif"
#define FER_VOLUME_OFF_LEFTCAP          FER_PATH"volume_off_leftcap.lif"
#define FER_VOLUME_OFF_MID              FER_PATH"volume_off_mid.lif"
#define FER_VOLUME_OFF_RIGHTCAP         FER_PATH"volume_off_rightcap.lif"
#define FER_VOLUME_MOUSE_LEFTCAP        FER_PATH"volume_mouse_leftcap.lif"
#define FER_VOLUME_MOUSE_MID            FER_PATH"volume_mouse_mid.lif"
#define FER_VOLUME_MOUSE_RIGHTCAP       FER_PATH"volume_mouse_rightcap.lif"
#define FER_VOLUME_ON_LEFTCAP           FER_PATH"volume_on_leftcap.lif"
#define FER_VOLUME_ON_MID               FER_PATH"volume_on_mid.lif"
#define FER_VOLUME_ON_RIGHTCAP          FER_PATH"volume_on_rightcap.lif"
#define FER_VOLUME_BUTTON2              FER_PATH"volume_button.lif"



#define FER_EQ_BAR_BUTTON               FER_PATH"eq_bar_button.lif"
#define FER_EQ_BAR_MOUSE_STRAIGHT       FER_PATH"eq_bar_mouse_straight.lif"
#define FER_EQ_BAR_ON_STRAIGHT          FER_PATH"eq_bar_on_straight.lif"
#define FER_EQ_BAR_OFF_STRAIGHT         FER_PATH"eq_bar_off_straight.lif"
#define FER_EQ_BAR_MOUSE_MIDDLE         FER_PATH"eq_bar_mouse_middle.lif"
#define FER_EQ_BAR_ON_MIDDLE            FER_PATH"eq_bar_on_middle.lif"
#define FER_EQ_BAR_OFF_MIDDLE           FER_PATH"eq_bar_off_middle.lif"
#define FER_EQ_BAR_MOUSE_BOTCAP         FER_PATH"eq_bar_mouse_botcap.lif"
#define FER_EQ_BAR_ON_BOTCAP            FER_PATH"eq_bar_on_botcap.lif"
#define FER_EQ_BAR_OFF_BOTCAP           FER_PATH"eq_bar_off_botcap.lif"
#define FER_EQ_BAR_MOUSE_TOPCAP         FER_PATH"eq_bar_mouse_topcap.lif"
#define FER_EQ_BAR_ON_TOPCAP            FER_PATH"eq_bar_on_topcap.lif"
#define FER_EQ_BAR_OFF_TOPCAP           FER_PATH"eq_bar_off_topcap.lif"

//

//default offset for concave curves (convex curves have no offset)
#define FER_CCBM    8

//color change for tabbed buttons
#define FER_COLOR_INCREMENT       20
#define FER_COLOR_DECREMENT       -20


//Front end registry parameters
#define FER_NumDecorative       128
#define FER_NumTextures         (FER_NumDecorative+end_texholder)
#define FER_PrependPath         ""

/*=============================================================================
    Type definitions:
=============================================================================*/
//note that due to GL implementation, (x0, y0) is topleft
typedef enum
{
    none,                                       // default texture type
    top, right, left, bottom,                   // texture type for sides
    topleft, topright, bottomleft, bottomright, // texture type for corners
    decorative                                   // texture type for decorative regions
} textype;

typedef enum
{
    convex, concave
} curvetype;

typedef enum
{
    ferNormal=-1, ferSmallTextures, ferMediumTextures, ferLargeTextures,
    ferInternalGlow, ferExternalGlow, ferNoGlow, ferLast
} drawtype;

typedef enum
{
    b_off, b_off_focus, b_off_focus_mouse, b_off_mouse, b_off_disabled, b_on_focus_mouse,
    te_off, te_off_focus, te_off_focus_mouse, te_off_mouse, te_off_disabled, te_on_focus_mouse,
    r_on, r_on_focus, r_on_mouse, r_on_focus_mouse, r_on_disabled, r_off, r_off_focus, r_off_mouse, r_off_focus_mouse, r_off_disabled,
    c_on, c_on_focus, c_on_mouse, c_on_focus_mouse, c_on_disabled, c_off, c_off_focus, c_off_mouse, c_off_focus_mouse, c_off_disabled,
    tb_on, tb_on_focus, tb_on_mouse, tb_on_focus_mouse, tb_on_disabled, tb_off, tb_off_focus, tb_off_mouse, tb_off_focus_mouse, tb_off_disabled
} ferbuttonstate;

typedef enum
{
    s_off, s_off_mouse, s_on_mouse, s_disabled
} ferscrollbarstate;

typedef enum
{
    sub_off, sub_off_mouse, sub_on_mouse,
    sdb_off, sdb_off_mouse  , sdb_on_mouse
} ferscrollbarbuttonstate;

typedef enum
{
    lw_normal, lw_focus
} ferfocuswindowstate;

typedef enum
{
    bb_off, bb_off_mouse, bb_on, bb_on_mouse
} ferbitmapbuttonstate;

typedef struct
{
    Node         node;
    textype      type;   //what part of the region does it cut out
    char         *name;  //name of decorative bitmap (if region is decorative)
    rectangle    rect;   //size of region
    regionhandle region; //pointer to the region associated with the cutout
} cutouttype;


/*=============================================================================
    Macros:
=============================================================================*/
#define IS_BETWEEN(a, a0, a1)            (((a) > (a0)) && ((a) < (a1)))
#define IS_IN_RECT(x, y, x0, y0, x1, y1) (IS_BETWEEN(x, x0, x1) && IS_BETWEEN(y, y0, y1))

/*=============================================================================
    Functions:
=============================================================================*/
//startup/shutdown
void ferStartup(void);
void ferReset(void);
void ferShutdown(void);

//draw a textured box
void ferDrawBoxRegion(rectangle dimensions, drawtype textures,
                       drawtype glow, LinkedList *cutouts, bool bUseAlpha);

//draw UI Elements
void ferDrawButton(rectangle dimensions, ferbuttonstate);
void ferDrawMenuItemSelected(rectangle *rect);
void ferDrawCheckbox(rectangle dimensions, ferbuttonstate state);
void ferDrawRadioButton(rectangle dimensions, ferbuttonstate state);
void ferDrawScrollbar(scrollbarhandle shandle, ferscrollbarstate state);
void ferDrawScrollbarButton(regionhandle region, ferscrollbarbuttonstate state);
void ferDrawFocusWindow(regionhandle region, ferfocuswindowstate state);
void ferDrawDecorative(regionhandle region);
void ferDrawOpaqueDecorative(regionhandle region);
void ferDrawSelectedDot(rectangle *rect);
void ferDrawPopoutArrow(rectangle *rect);
void ferDrawBitmapButton(regionhandle region, ferbitmapbuttonstate state);

void ferDrawBoxIntoBuffer(sdword width, sdword height, ubyte* data);
void ferDrawHorizSlider(sliderhandle shandle, uword state);
void ferDrawVertSlider(sliderhandle shandle, uword state);

void ferDraw(sdword x, sdword y, lifheader *texture);
lifheader *ferTextureRegisterSpecial(char *fileName, textype newtype, textype origtype);

#endif
