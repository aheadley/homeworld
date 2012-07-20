/*=============================================================================
    Name    : dxdraw.cpp
    Purpose : DirectDraw stuff to set display modes w/ OpenGL

    Created 8/15/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>


HWND hwnd;
LPDIRECTDRAW lpDD = NULL;

static int ddWidth, ddHeight, ddDepth;
static unsigned int bActive = 0;

extern "C" unsigned int fullScreen;


static char* ddReportError(HRESULT hr);


/*-----------------------------------------------------------------------------
    Name        : ddDeleteWindow
    Description : restore display mode, release DirectDraw object
    Inputs      :
    Outputs     :
    Return      : >0 success
----------------------------------------------------------------------------*/
extern "C" unsigned int ddDeleteWindow(void)
{
    if (!fullScreen)
    {
        //windows have no DirectDraw object
        return 1;
    }
    if (lpDD != NULL)
    {
        if (bActive)
        {
            //only restore if not minimized
            lpDD->RestoreDisplayMode();
        }
        lpDD->Release();
        lpDD = NULL;
    }

    //we're not active
    bActive = 0;

    return 1;
}

/*-----------------------------------------------------------------------------
    Name        : ddCreateDirectDraw
    Description : create DirectDraw object if not already created
    Inputs      :
    Outputs     :
    Return      : >0 success
----------------------------------------------------------------------------*/
unsigned int ddCreateDirectDraw(void)
{
    if (lpDD == NULL)
    {
        return (DirectDrawCreate(NULL, &lpDD, NULL) == DD_OK) ? 1 : 0;
    }
    else
    {
        return 1;
    }
}

/*-----------------------------------------------------------------------------
    Name        : ddCreateWindow
    Description : set exclusive + fullscreen, switch display modes, set normal
    Inputs      :
    Outputs     :
    Return      : >0 success
----------------------------------------------------------------------------*/
extern "C" unsigned int ddCreateWindow(int ihwnd, int width, int height, int depth)
{
    HRESULT hr;

    hwnd = (HWND)ihwnd;

    if (!fullScreen)
    {
        return 1;
    }

    if (!ddCreateDirectDraw())
    {
        return 0;
    }

    hr = lpDD->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    if (hr != DD_OK)
    {
        (void)ddReportError(hr);

        hr = lpDD->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
        if (hr != DD_OK)
        {
            (void)ddReportError(hr);
            return 0;
        }
    }

    hr = lpDD->SetDisplayMode(width, height, depth);
    if (hr != DD_OK)
    {
        return 0;
    }

    ddWidth  = width;
    ddHeight = height;
    ddDepth  = depth;

    bActive = 1;

    (void)lpDD->SetCooperativeLevel(hwnd, DDSCL_NORMAL);

    return 1;
}

/*-----------------------------------------------------------------------------
    Name        : ddActivate
    Description : restore / set the display mode on minimize / restore
    Inputs      : activate - 0 minimize, 1 restore
    Outputs     :
    Return      : >0 success
----------------------------------------------------------------------------*/
extern "C" unsigned int _ddActivate(int activate)
{
    //DirectDraw / OpenGL takes care of this
    return 1;
}
extern "C" unsigned int ddActivate(int activate)
{
    HRESULT hr;
    int counter;

    if (lpDD == NULL)
    {
        return 1;
    }

    if (activate)
    {
        if (!bActive)
        {
            for (counter = 0;; counter++)
            {
                if (counter > 10)
                {
                    //bail on repeated failures
                    return 1;
                }
                hr = lpDD->SetDisplayMode(ddWidth, ddHeight, ddDepth);
                if (hr == DD_OK)
                {
                    break;
                }
                Sleep(30);
            }

            bActive = 1;
        }

        return 1;
    }
    else
    {
        if (bActive)
        {
            for (counter = 0;; counter++)
            {
                if (counter > 10)
                {
                    //bail on repeated failures
                    return 1;
                }
                hr = lpDD->RestoreDisplayMode();
                if (hr == DD_OK)
                {
                    break;
                }
                Sleep(30);
            }

            bActive = 0;
        }

        return 1;
    }
}

/*-----------------------------------------------------------------------------
    Name        : ddReportError
    Description : convert DirectDraw error codes to a string
    Inputs      : hr - error code
    Outputs     :
    Return      : error string, or "???" if not in table
----------------------------------------------------------------------------*/
static char* ddReportError(HRESULT hr)
{
    char* str;

    switch (hr)
    {
    case DDERR_ALREADYINITIALIZED:
        str = "DDERR_ALREADYINITIALIZED";
        break;
    case DDERR_CANNOTATTACHSURFACE:
        str = "DDERR_CANNOTATTACHSURFACE";
        break;
    case DDERR_CANNOTDETACHSURFACE:
        str = "DDERR_CANNOTDETACHSURFACE";
        break;
    case DDERR_CURRENTLYNOTAVAIL:
        str = "DDERR_CURRENTLYNOTAVAIL";
        break;
    case DDERR_EXCEPTION:
        str = "DDERR_EXCEPTION";
        break;
    case DDERR_GENERIC:
        str = "DDERR_GENERIC";
        break;
    case DDERR_HEIGHTALIGN:
        str = "DDERR_HEIGHTALIGN";
        break;
    case DDERR_INCOMPATIBLEPRIMARY:
        str = "DDERR_INCOMPATIBLEPRIMARY";
        break;
    case DDERR_INVALIDCAPS:
        str = "DDERR_INVALIDCAPS";
        break;
    case DDERR_INVALIDCLIPLIST:
        str = "DDERR_INVALIDCLIPLIST";
        break;
    case DDERR_INVALIDMODE:
        str = "DDERR_INVALIDMODE";
        break;
    case DDERR_INVALIDOBJECT:
        str = "DDERR_INVALIDOBJECT";
        break;
    case DDERR_INVALIDPARAMS:
        str = "DDERR_INVALIDPARAMS";
        break;
    case DDERR_INVALIDPIXELFORMAT:
        str = "DDERR_INVALIDPIXELFORMAT";
        break;
    case DDERR_INVALIDRECT:
        str = "DDERR_INVALIDRECT";
        break;
    case DDERR_LOCKEDSURFACES:
        str = "DDERR_LOCKEDSURFACES";
        break;
    case DDERR_NO3D:
        str = "DDERR_NO3D";
        break;
    case DDERR_NOALPHAHW:
        str = "DDERR_NOALPHAHW";
        break;
    case DDERR_NOCLIPLIST:
        str = "DDERR_NOCLIPLIST";
        break;
    case DDERR_NOCOLORCONVHW:
        str = "DDERR_NOCOLORCONVHW";
        break;
    case DDERR_NOCOOPERATIVELEVELSET:
        str = "DDERR_NOCOOPERATIVELEVELSET";
        break;
    case DDERR_NOCOLORKEY:
        str = "DDERR_NOCOLORKEY";
        break;
    case DDERR_NOCOLORKEYHW:
        str = "DDERR_NOCOLORKEYHW";
        break;
    case DDERR_NODIRECTDRAWSUPPORT:
        str = "DDERR_NODIRECTDRAWSUPPORT";
        break;
    case DDERR_NOEXCLUSIVEMODE:
        str = "DDERR_NOEXCLUSIVEMODE";
        break;
    case DDERR_NOFLIPHW:
        str = "DDERR_NOFLIPHW";
        break;
    case DDERR_NOGDI:
        str = "DDERR_NOGDI";
        break;
    case DDERR_NOMIRRORHW:
        str = "DDERR_NOMIRRORHW";
        break;
    case DDERR_NOTFOUND:
        str = "DDERR_NOTFOUND";
        break;
    case DDERR_NOOVERLAYHW:
        str = "DDERR_NOOVERLAYHW";
        break;
    case DDERR_OVERLAPPINGRECTS:
        str = "DDERR_OVERLAPPINGRECTS";
        break;
    case DDERR_NORASTEROPHW:
        str = "DDERR_NORASTEROPHW";
        break;
    case DDERR_NOROTATIONHW:
        str = "DDERR_NOROTATIONHW";
        break;
    case DDERR_NOSTRETCHHW:
        str = "DDERR_NOSTRETCHHW";
        break;
    case DDERR_NOT4BITCOLOR:
        str = "DDERR_NOT4BITCOLOR";
        break;
    case DDERR_NOT4BITCOLORINDEX:
        str = "DDERR_NOT4BITCOLORINDEX";
        break;
    case DDERR_NOT8BITCOLOR:
        str = "DDERR_NOT8BITCOLOR";
        break;
    case DDERR_NOTEXTUREHW:
        str = "DDERR_NOTEXTUREHW";
        break;
    case DDERR_NOVSYNCHW:
        str = "DDERR_NOVSYNCHW";
        break;
    case DDERR_NOZBUFFERHW:
        str = "DDERR_NOZBUFFERHW";
        break;
    case DDERR_NOZOVERLAYHW:
        str = "DDERR_NOZOVERLAYHW";
        break;
    case DDERR_OUTOFCAPS:
        str = "DDERR_OUTOFCAPS";
        break;
    case DDERR_OUTOFMEMORY:
        str = "DDERR_OUTOFMEMORY";
        break;
    case DDERR_OUTOFVIDEOMEMORY:
        str = "DDERR_OUTOFVIDEOMEMORY";
        break;
    case DDERR_OVERLAYCANTCLIP:
        str = "DDERR_OVERLAYCANTCLIP";
        break;
    case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
        str = "DDERR_OVERLAYCOLORKEYONLYONEACTIVE";
        break;
    case DDERR_PALETTEBUSY:
        str = "DDERR_PALETTEBUSY";
        break;
    case DDERR_COLORKEYNOTSET:
        str = "DDERR_COLORKEYNOTSET";
        break;
    case DDERR_SURFACEALREADYATTACHED:
        str = "DDERR_SURFACEALREADYATTACHED";
        break;
    case DDERR_SURFACEALREADYDEPENDENT:
        str = "DDERR_SURFACEALREADYDEPENDENT";
        break;
    case DDERR_SURFACEBUSY:
        str = "DDERR_SURFACEBUSY";
        break;
    case DDERR_CANTLOCKSURFACE:
        str = "DDERR_CANTLOCKSURFACE";
        break;
    case DDERR_SURFACEISOBSCURED:
        str = "DDERR_SURFACEISOBSCURED";
        break;
    case DDERR_SURFACELOST:
        str = "DDERR_SURFACELOST";
        break;
    case DDERR_SURFACENOTATTACHED:
        str = "DDERR_SURFACENOTATTACHED";
        break;
    case DDERR_TOOBIGHEIGHT:
        str = "DDERR_TOOBIGHEIGHT";
        break;
    case DDERR_TOOBIGSIZE:
        str = "DDERR_TOOBIGSIZE";
        break;
    case DDERR_TOOBIGWIDTH:
        str = "DDERR_TOOBIGWIDTH";
        break;
    case DDERR_UNSUPPORTED:
        str = "DDERR_UNSUPPORTED";
        break;
    case DDERR_UNSUPPORTEDFORMAT:
        str = "DDERR_UNSUPPORTEDFORMAT";
        break;
    case DDERR_UNSUPPORTEDMASK:
        str = "DDERR_UNSUPPORTEDMASK";
        break;
    case DDERR_INVALIDSTREAM:
        str = "DDERR_INVALIDSTREAM";
        break;
    case DDERR_VERTICALBLANKINPROGRESS:
        str = "DDERR_VERTICALBLANKINPROGRESS";
        break;
    case DDERR_WASSTILLDRAWING:
        str = "DDERR_WASSTILLDRAWING";
        break;
    case DDERR_XALIGN:
        str = "DDERR_XALIGN";
        break;
    case DDERR_INVALIDDIRECTDRAWGUID:
        str = "DDERR_INVALIDDIRECTDRAWGUID";
        break;
    case DDERR_DIRECTDRAWALREADYCREATED:
        str = "DDERR_DIRECTDRAWALREADYCREATED";
        break;
    case DDERR_NODIRECTDRAWHW:
        str = "DDERR_NODIRECTDRAWHW";
        break;
    case DDERR_PRIMARYSURFACEALREADYEXISTS:
        str = "DDERR_PRIMARYSURFACEALREADYEXISTS";
        break;
    case DDERR_NOEMULATION:
        str = "DDERR_NOEMULATION";
        break;
    case DDERR_REGIONTOOSMALL:
        str = "DDERR_REGIONTOOSMALL";
        break;
    case DDERR_CLIPPERISUSINGHWND:
        str = "DDERR_CLIPPERISUSINGHWND";
        break;
    case DDERR_NOCLIPPERATTACHED:
        str = "DDERR_NOCLIPPERATTACHED";
        break;
    case DDERR_NOHWND:
        str = "DDERR_NOHWND";
        break;
    case DDERR_HWNDSUBCLASSED:
        str = "DDERR_HWNDSUBCLASSED";
        break;
    case DDERR_HWNDALREADYSET:
        str = "DDERR_HWNDALREADYSET";
        break;
    case DDERR_NOPALETTEATTACHED:
        str = "DDERR_NOPALETTEATTACHED";
        break;
    case DDERR_NOPALETTEHW:
        str = "DDERR_NOPALETTEHW";
        break;
    case DDERR_BLTFASTCANTCLIP:
        str = "DDERR_BLTFASTCANTCLIP";
        break;
    case DDERR_NOBLTHW:
        str = "DDERR_NOBLTHW";
        break;
    case DDERR_NODDROPSHW:
        str = "DDERR_NODDROPSHW";
        break;
    case DDERR_OVERLAYNOTVISIBLE:
        str = "DDERR_OVERLAYNOTVISIBLE";
        break;
    case DDERR_NOOVERLAYDEST:
        str = "DDERR_NOOVERLAYDEST";
        break;
    case DDERR_INVALIDPOSITION:
        str = "DDERR_INVALIDPOSITION";
        break;
    case DDERR_NOTAOVERLAYSURFACE:
        str = "DDERR_NOTAOVERLAYSURFACE";
        break;
    case DDERR_EXCLUSIVEMODEALREADYSET:
        str = "DDERR_EXCLUSIVEMODEALREADYSET";
        break;
    case DDERR_NOTFLIPPABLE:
        str = "DDERR_NOTFLIPPABLE";
        break;
    case DDERR_CANTDUPLICATE:
        str = "DDERR_CANTDUPLICATE";
        break;
    case DDERR_NOTLOCKED:
        str = "DDERR_NOTLOCKED";
        break;
    case DDERR_CANTCREATEDC:
        str = "DDERR_CANTCREATEDC";
        break;
    case DDERR_NODC:
        str = "DDERR_NODC";
        break;
    case DDERR_WRONGMODE:
        str = "DDERR_WRONGMODE";
        break;
    default:
        str = "???";
    }

    return str;
}
