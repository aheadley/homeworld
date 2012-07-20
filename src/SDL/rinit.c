/*=============================================================================
    Name    : rinit.c
    Purpose : rGL / OpenGL enumeration initialization routines

    Created 1/5/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/


/*#define WIN32_LEAN_AND_MEAN*/
#define STRICT
#define D3D_OVERLOADS

#if 0	/* Only needed for Direct3D...hey, we might reenable it one of these days... */
#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include <dinput.h>
#include <d3drm.h>
#endif

#include "SDL.h"
#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rinit.h"
#include "devstats.h"
#include "Types.h"


extern unsigned int glNT;
extern unsigned int gl95;
extern unsigned int mainSoftwareDirectDraw;
extern unsigned int mainOutputCRC;
extern unsigned long strCurLanguage;

#define RIN_MODESINDEVSTATLIST 10
unsigned int rinDevstatModeTable[RIN_MODESINDEVSTATLIST][3] =
{
    { 16,  640, DEVSTAT_NO_64048016 },
    { 16,  800, DEVSTAT_NO_80060016 },
    { 16, 1024, DEVSTAT_NO_102476816 },
    { 16, 1280, DEVSTAT_NO_1280102416 },
    { 16, 1600, DEVSTAT_NO_1600120016 },
    { 32,  640, DEVSTAT_NO_64048032 },
    { 32,  800, DEVSTAT_NO_80060032 },
    { 32, 1024, DEVSTAT_NO_102476832 },
    { 32, 1280, DEVSTAT_NO_1280102432 },
    { 32, 1600, DEVSTAT_NO_1600120032 },
};

extern unsigned int* devTable;
extern int devTableLength;

//3dfx fullscreen-only present boolean
static bool bFullscreenDevice;

static rdevice* rDeviceList;
static int nDevices;

unsigned int gDevcaps  = 0xFFFFFFFF;
unsigned int gDevcaps2 = 0x00000000;

#if 0	/* Direct3D stuff...not used. */
char* gDescription = NULL;
char* gDriverName = NULL;

typedef HRESULT(WINAPI * DIRECTDRAWCREATE)(GUID*, LPDIRECTDRAW*, IUnknown*);
typedef HRESULT(WINAPI * DIRECTINPUTCREATE)(HINSTANCE, DWORD, LPDIRECTINPUT*, IUnknown*);
#endif

#define SST_VOODOO  0
#define SST_VOODOO2 1
#define SST_OTHER   2

extern unsigned int sstHardwareExists(int*);
extern unsigned int glCapNT(void);

static void* rinMemAlloc(int size)
{
    unsigned char* block;

    block = (unsigned char*)malloc(size);
    memset(block, 0, size);

    return (void*)block;
}

static void rinMemFree(void* memblock)
{
    free(memblock);
}

extern unsigned int crc32Compute(void*, unsigned int);

/*-----------------------------------------------------------------------------
    Name        : rinDeviceCRC
    Description : generate a CRC of the detected devices for detection of
                  changing hardware (light assurance that previous video
                  mode will still be working correctly)
    Inputs      :
    Outputs     :
    Return      : crc32
----------------------------------------------------------------------------*/
unsigned int rinDeviceCRC(void)
{
    rdevice* cdev;
    rdevice* devlist;
    rdevice* pdevlist;
    unsigned int crc;

    if (nDevices == 0 || rDeviceList == NULL)
    {
        return 0;
    }

    devlist = (rdevice*)rinMemAlloc(nDevices * sizeof(rdevice));
    pdevlist = devlist;

    cdev = rDeviceList;
    do
    {
        memcpy(pdevlist, cdev, sizeof(rdevice));
        pdevlist->modes = NULL;
        pdevlist->next = NULL;
        pdevlist++;
        cdev = cdev->next;
    } while (cdev != NULL);

    crc = crc32Compute((void*)devlist, nDevices * sizeof(rdevice));

    rinMemFree(devlist);

    return crc;
}

/*-----------------------------------------------------------------------------
    Name        : rinAddMode
    Description : add a display mode to a video driver
    Inputs      : dev - the device to add the mode to
                  width, height - dimensions
                  depth - bitdepth
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rinAddMode(rdevice* dev, int width, int height, int depth)
{
    rmode* mode;
    rmode* cmode;

    mode = (rmode*)rinMemAlloc(sizeof(rmode));
    mode->width  = width;
    mode->height = height;
    mode->depth  = depth;

    if (dev->modes == NULL)
    {
        //no modes yet
        dev->modes = mode;
        mode->next = NULL;
    }
    else
    {
        //add to tail
        cmode = dev->modes;
        while (cmode->next != NULL)
        {
            cmode = cmode->next;
        }
        cmode->next = mode;
        mode->next = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rinModeAccepted
    Description : decides whether a given mode is supported
    Inputs      : dev - the device in question
                  width, height, depth - display mode characteristics
    Outputs     :
    Return      : true or false
----------------------------------------------------------------------------*/
static bool rinModeAccepted(rdevice* dev, int width, int height, int depth)
{
    int index;

    //accept 16 & 32 bit mode only
    if (depth != 16 && depth != 32)
    {
        return FALSE;
    }

    //possibly eliminate 32bit modes, if devstats tells us to
    if (depth == 32)
    {
        if (dev->type == RIN_TYPE_DIRECT3D)
        {
            if (dev->devcaps & DEVSTAT_NO_32BIT)
            {
                return FALSE;
            }
        }
        else if (dev->type == RIN_TYPE_OPENGL)
        {
            if ((dev->devcaps & DEVSTAT_NO_32BIT_GL) ||
                (dev->devcaps & DEVSTAT_NO_32BIT))
            {
                return FALSE;
            }
        }
    }

    //only sane, supported, modes
    switch (width)
    {
    case 640:
        if (height !=  480) return FALSE;
        break;
    case 800:
        if (height !=  600) return FALSE;
        break;
    case 1024:
        if (height !=  768) return FALSE;
        break;
    case 1280:
        if (height != 1024) return FALSE;
        break;
    case 1600:
        if (height != 1200) return FALSE;
        break;
    default:
        return FALSE;
    }

    //check for specifically disabled display modes
    for (index = 0; index < RIN_MODESINDEVSTATLIST; index++)
    {
        if (rinDevstatModeTable[index][0] == depth &&
            rinDevstatModeTable[index][1] == width)
        {
            if (dev->devcaps & rinDevstatModeTable[index][2])
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : rinSortModes
    Description : sorts a device's display modes the way I like them
                  (one bitdepth at a time)
    Inputs      : dev - the device whose modes are to be sorted
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rinSortModes(rdevice* dev)
{
    rmode* cmode;
    rmode* freeMode;
    rdevice dummy;
    int depths[] = {16,32,0};
    int depth;

    if (dev->modes == NULL)
    {
        return;
    }

    memset(&dummy, 0, sizeof(rdevice));

    for (depth = 0; depths[depth] != 0; depth++)
    {
        cmode = dev->modes;
        do
        {
            if (cmode->depth == depths[depth])
            {
                if (rinModeAccepted(dev, cmode->width, cmode->height, cmode->depth))
                {
                    rinAddMode(&dummy, cmode->width, cmode->height, cmode->depth);
                }
            }
            cmode = cmode->next;
        } while (cmode != NULL);
    }

    //free modes on dev
    cmode = dev->modes;
    while (cmode != NULL)
    {
        freeMode = cmode;    //save to free
        cmode = cmode->next; //next
        rinMemFree(freeMode);   //free
    }

    //attach new modes
    dev->modes = dummy.modes;
}

/*-----------------------------------------------------------------------------
    Name        : rinCopyModes
    Description : copy one driver's display modes to another
    Inputs      : devOut - output device
                  devIn - input device
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rinCopyModes(rdevice* devOut, rdevice* devIn)
{
    rmode* cmode;

    cmode = devIn->modes;
    if (cmode == NULL)
    {
        devOut->modes = NULL;
    }
    else
    {
        do
        {
            rinAddMode(devOut, cmode->width, cmode->height, cmode->depth);
            cmode = cmode->next;
        } while (cmode != NULL);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rinCopyModesSelectively
    Description : copy certain display modes from one device to another
    Inputs      : devOut - output device
                  devIn - input device
                  depth - bitdepth of acceptable modes
                  maxWidth - maximum acceptable width
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rinCopyModesSelectively(rdevice* devOut, rdevice* devIn, int depth, int maxWidth)
{
    rmode* cmode;

    cmode = devIn->modes;
    if (cmode == NULL)
    {
        devOut->modes = NULL;
    }
    else
    {
        do
        {
            if ((cmode->depth == depth) &&
                (cmode->width <= maxWidth))
            {
                rinAddMode(devOut, cmode->width, cmode->height, cmode->depth);
            }
            cmode = cmode->next;
        } while (cmode != NULL);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rinAddDevice
    Description : add a device to the head of the device list
    Inputs      : dev - the device to add
    Outputs     : rDeviceList is modified
    Return      :
----------------------------------------------------------------------------*/
void rinAddDevice(rdevice* dev)
{
    rdevice* cdev;

    if (rDeviceList == NULL)
    {
        rDeviceList = dev;
        dev->next = NULL;
    }
    else
    {
        //add to head
        cdev = rDeviceList;
        dev->next = cdev;
        rDeviceList = dev;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rinGetDeviceList
    Description : return pointer to device list
    Inputs      :
    Outputs     :
    Return      : rDeviceList
----------------------------------------------------------------------------*/
rdevice* rinGetDeviceList(void)
{
    return rDeviceList;
}

/*-----------------------------------------------------------------------------
    Name        : rinFreeDevices
    Description : clear the device list (free memory and such)
    Inputs      :
    Outputs     : rDeviceList and constituents are freed
    Return      :
----------------------------------------------------------------------------*/
int rinFreeDevices(void)
{
    rdevice* dev;
    rdevice* freeDev;
    rmode* mode;
    rmode* freeMode;

    //device loop
    dev = rDeviceList;
    while (dev != NULL)
    {
        //mode loop
        mode = dev->modes;
        while (mode != NULL)
        {
            freeMode = mode;    //save to free
            mode = mode->next;  //next
            rinMemFree(freeMode);  //free
        }

        freeDev = dev;          //save to free
        dev = dev->next;        //next
        rinMemFree(freeDev);       //free
    }

    rDeviceList = NULL;

    //success
    return 1;
}

#if 0	/* Direct3D-specific enumeration. */
/*-----------------------------------------------------------------------------
    Name        : rinEnumDisplayModes_cb
    Description : callback during display mode enumeration
    Inputs      : ...
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static HRESULT WINAPI rinEnumDisplayModes_cb(LPDDSURFACEDESC2 ddsd, LPVOID lpContext)
{
    rdevice* dev;

    dev = (rdevice*)lpContext;

    if ((ddsd->dwFlags & DDSD_WIDTH) &&
        (ddsd->dwFlags & DDSD_HEIGHT) &&
        (ddsd->dwFlags & DDSD_PIXELFORMAT))
    {
        if (ddsd->ddpfPixelFormat.dwRGBBitCount != 16 &&
            ddsd->ddpfPixelFormat.dwRGBBitCount != 32)
        {
            goto SKIP_MODE;
        }
        if (ddsd->dwWidth < 640 || ddsd->dwHeight < 480)
        {
            goto SKIP_MODE;
        }
        if (ddsd->dwWidth > 1600)
        {
            goto SKIP_MODE;
        }

        rinAddMode(dev,
                   ddsd->dwWidth, ddsd->dwHeight,
                   ddsd->ddpfPixelFormat.dwRGBBitCount);
    }

  SKIP_MODE:
    return D3DENUMRET_OK;
}

static FILE* crcLog;

static void rinResetCRCLog(void)
{
    crcLog = fopen("d3dDeviceCRC.txt", "wt");
}

static void rinOutputCRC(LPSTR desc, LPSTR name, unsigned int crc)
{
    if (crcLog != NULL)
    {
        fprintf(crcLog, "%08X [%s][%s]\n", crc, desc, name);
    }
}

static void rinCloseCRCLog(void)
{
    if (crcLog != NULL)
    {
        fclose(crcLog);
        crcLog = NULL;
    }
}

D3DDEVICEDESC _caps;

/*-----------------------------------------------------------------------------
    Name        : rinEnoughCaps
    Description : check for at least alphablending and depthbuffering before
                  considering this a valid device
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool rinEnoughCaps(LPSTR strDesc, LPSTR strName, LPD3DDEVICEDESC caps)
{
    D3DPRIMCAPS* tri = &caps->dpcTriCaps;

    _caps = *caps;
    //reset some possibly variable fields
    _caps.dwSize = 0;
    _caps.dwFlags = 0;
    _caps.dcmColorModel = 0;
    _caps.dwDevCaps = 0;
    _caps.bClipping = 0;

    //check for alphablending, source caps first
    if (!(tri->dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) &&
        !(tri->dwSrcBlendCaps & D3DPBLENDCAPS_ONE))
    {
        return false;
    }

    //check dest blend caps
    if (!(tri->dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) &&
        !(tri->dwDestBlendCaps & D3DPBLENDCAPS_ONE) &&
        !(tri->dwDestBlendCaps & D3DPBLENDCAPS_ZERO))
    {
        return false;
    }

    //it can blend, but can it depthbuffer ?
    if (!(tri->dwZCmpCaps & D3DPCMPCAPS_LESS) &&
        !(tri->dwZCmpCaps & D3DPCMPCAPS_LESSEQUAL))
    {
        return false;
    }

    //check for some kind of texture filtering
    if (!(tri->dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR) &&
        !(tri->dwTextureFilterCaps & D3DPTFILTERCAPS_NEAREST))
    {
        return false;
    }

    //check texture address caps
    if (!(tri->dwTextureAddressCaps & D3DPTADDRESSCAPS_CLAMP) &&
        !(tri->dwTextureAddressCaps & D3DPTADDRESSCAPS_WRAP))
    {
        return false;
    }

    return true;
}

//internal count of num valid devs on D3D object
static int D3Ddevs;
static int numD3Ddevs;

static HRESULT WINAPI rinEnumDirect3DDevices_cb(
    GUID* pGUID, LPSTR strDesc, LPSTR strName, LPD3DDEVICEDESC pHALDesc,
    LPD3DDEVICEDESC pHELDesc, LPVOID pvContext)
{
    int* devs = (int*)pvContext;

    if (pGUID == NULL || pHALDesc == NULL || pHELDesc == NULL)
    {
        return D3DENUMRET_CANCEL;
    }

    if (pHALDesc->dwFlags != 0)
    {
        //has a HAL, might be something we want.  check caps
        if (rinEnoughCaps(strDesc, strName, pHALDesc))
        {
            D3Ddevs++;
        }
    }

    return D3DENUMRET_OK;
}

void rinRemoveDriverFromName(char* name)
{
    int   pos;
    char  buf[64];
    char* pdest;

    if (strlen(name) < 7)
    {
        return;
    }

    strncpy(buf, name, 63);
    buf[63] = '\0';
    _strlwr(buf);
    pdest = strstr(buf, "driver");
    if (pdest != NULL && pdest != name)
    {
        if (name[pdest - buf - 1] == ' ')
        {
            name[pdest - buf - 1] = '\0';
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rinFindDevCaps
    Description : find a given device's eliminated caps in the devTable
    Inputs      : crc0 - crc of driver's caps
                  crc1 - crc of device type
                  cap[01] - where the cap flags go
    Outputs     : cap[01] holds cap flags, or 0
    Return      :
----------------------------------------------------------------------------*/
static void rinFindDevCaps(unsigned int crc0, unsigned int crc1, unsigned int* cap0, unsigned int* cap1)
{
    int index;

    if (cap0 == NULL || cap1 == NULL)
    {
        return;
    }

    if (crc1 != 0x00000000)
    {
        //try device-specific crclist first
        for (index = 0; index < (4*devTableLength); index += 4)
        {
            if (devTable[index+2] == crc1)
            {
                *cap0 = devTable[index+1];
                *cap1 = devTable[index+3];
                return;
            }
        }
    }

    //try driver-specific crclist next
    for (index = 0; index < (4*devTableLength); index += 4)
    {
        if (devTable[index] == crc0)
        {
            *cap0 = devTable[index+1];
            *cap1 = devTable[index+3];
            return;
        }
    }

    *cap0 = 0;
    *cap1 = 0;
}

typedef struct
{
    DWORD vendor;
    WORD  product;
    DWORD device;
} hwDat;

/*-----------------------------------------------------------------------------
    Name        : rinEnumDirectDraw_cb
    Description : callback during DirectDraw device enumeration
    Inputs      : ...
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static BOOL WINAPI rinEnumDirectDraw_cb(
    GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext)
{
    LPDIRECTDRAW  ddraw1;
    LPDIRECTDRAW4 ddraw4;
    LPDIRECT3D3       d3dObject;
    DDCAPS   hal, hel;
    HRESULT  hr;
    rdevice* dev;
    unsigned int cap0, cap1;
    DDDEVICEIDENTIFIER dddi;
    hwDat dat;

    gDescription = lpDriverDescription;
    gDriverName  = lpDriverName;

    if (FAILED(DirectDrawCreate(lpGUID, &ddraw1, NULL)))
    {
        return D3DENUMRET_OK;
    }

    hr = ddraw1->QueryInterface(IID_IDirectDraw4, (void**)&ddraw4);
    if (FAILED(hr))
    {
        ddraw1->Release();
        return D3DENUMRET_OK;
    }

    hr = ddraw4->QueryInterface(IID_IDirect3D3, (void**)&d3dObject);
    if (FAILED(hr))
    {
        ddraw4->Release();
        ddraw1->Release();
        return D3DENUMRET_OK;
    }

    //query caps for fullscreen-only-ness
    ZeroMemory(&hal, sizeof(DDCAPS));
    ZeroMemory(&hel, sizeof(DDCAPS));
    hal.dwSize = sizeof(DDCAPS);
    hel.dwSize = sizeof(DDCAPS);
    hr = ddraw4->GetCaps(&hal, &hel);
    if (FAILED(hr))
    {
        d3dObject->Release();
        ddraw4->Release();
        ddraw1->Release();
        return D3DENUMRET_OK;
    }

    D3Ddevs = 0;
    d3dObject->EnumDevices(rinEnumDirect3DDevices_cb, NULL);
    if (D3Ddevs == 0)
    {
        d3dObject->Release();
        ddraw4->Release();
        ddraw1->Release();
        return D3DENUMRET_OK;
    }

    d3dObject->Release();

    if (!(hal.dwCaps2 & DDCAPS2_CANRENDERWINDOWED))
    {
        //no 3dfx fullscreen devices
//        ddraw4->Release();
//        ddraw1->Release();
        bFullscreenDevice = true;
//        return D3DENUMRET_OK;
    }

    memset(&dat, 0, sizeof(hwDat));
    if (FAILED(ddraw4->GetDeviceIdentifier(&dddi, 0)))
    {
        ddraw4->Release();
        ddraw1->Release();
        return D3DENUMRET_OK;
    }
    dat.vendor  = dddi.dwVendorId;
    dat.product = 4;//HIWORD(dddi.liDriverVersion.HighPart);
    dat.device  = dddi.dwDeviceId;

    dev = (rdevice*)memAlloc(sizeof(rdevice));
    dev->type = RIN_TYPE_DIRECT3D;
    rinFindDevCaps(crc32Compute((void*)&_caps, sizeof(D3DDEVICEDESC)),
                   crc32Compute((void*)&dat, sizeof(hwDat)),
                   &cap0, &cap1);
    if (mainOutputCRC)
    {
        rinOutputCRC(dddi.szDescription,
                     dddi.szDriver,
                     crc32Compute((void*)&dat, sizeof(hwDat)));
    }
    dev->devcaps  = cap0;
    dev->devcaps2 = cap1;
    if (gDevcaps == 0xFFFFFFFF)
    {
        //only update global devcaps for primary
        gDevcaps  = dev->devcaps;
        gDevcaps2 = dev->devcaps2;
    }
    strncpy(dev->data, lpDriverName, 63);
    strncpy(dev->name, lpDriverDescription, 63);
    dev->data[63] = '\0';
    dev->name[63] = '\0';
    rinRemoveDriverFromName(dev->name);
    if (lpGUID == NULL)
    {
        dev->data[0] = '\0';
    }
    dev->modes = NULL;

    ddraw4->EnumDisplayModes(0, NULL, (LPVOID)dev, rinEnumDisplayModes_cb);

    ddraw4->Release();
    ddraw1->Release();

    if (!(dev->devcaps & DEVSTAT_NOD3D_9X))
    {
        if (gl95)
        {
            if (dev->devcaps2 & DEVSTAT2_NOD3D_95)
            {
                return D3DENUMRET_OK;
            }
        }
        //only add device if Direct3D allowed for it
        rinAddDevice(dev);
        rinSortModes(dev);
        numD3Ddevs++;
        D3Ddevs++;
    }

    return D3DENUMRET_OK;
}

/*-----------------------------------------------------------------------------
    Name        : rinEnumerateDirect3D
    Description : enumerate available Direct3D devices and populate the device list
    Inputs      :
    Outputs     : rDeviceList is added to, if any compatible devices exist
    Return      : number of D3D devices found
----------------------------------------------------------------------------*/
int rinEnumerateDirect3D(void)
{
    numD3Ddevs = 0;
    HRESULT hr = DirectDrawEnumerate(rinEnumDirectDraw_cb, NULL);
    if (FAILED(hr))
    {
        return 0;
    }
    return numD3Ddevs;
}
#endif	/* Direct3D-specific enumeration. */

/*-----------------------------------------------------------------------------
    Name        : rinMaxWidth
    Description : return max width from all modes of all devices
    Inputs      :
    Outputs     :
    Return      : max available width, or 9999 if no devices available
----------------------------------------------------------------------------*/
int rinMaxWidth(void)
{
    rdevice* cdev;
    rmode* cmode;
    int maxWidth;

    if (nDevices == 0)
    {
        return 9999;
    }

    cdev = rDeviceList;
    maxWidth = 0;
    while (cdev != NULL)
    {
        cmode = cdev->modes;
        while (cmode != NULL)
        {
            if (cmode->width > maxWidth)
            {
                maxWidth = cmode->width;
            }
            cmode = cmode->next;
        }
        cdev = cdev->next;
    }

    return (maxWidth == 0) ? 9999 : maxWidth;
}

/*-----------------------------------------------------------------------------
    Name        : rinEnumeratePrimary
    Description : enumerate available display modes on the primary display
    Inputs      : dev - the device whose modes are to be filled
    Outputs     :
    Return      : true or false (could or couldn't enumerate)
----------------------------------------------------------------------------*/
bool rinEnumeratePrimary(rdevice* dev)
{
	SDL_PixelFormat fmt;
	SDL_Rect** modes;
    Uint32 flags;
	int max_width;
	int i;

	if (!dev)
		return FALSE;

	max_width = rinMaxWidth();

	/* Make sure SDL video is initialized. */
	flags = SDL_WasInit(SDL_INIT_EVERYTHING);
	if (!flags)
	{
		if (SDL_Init(SDL_INIT_VIDEO) == -1)
			return FALSE;
	}
	else if (!(flags & SDL_INIT_VIDEO))
	{
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
			return FALSE;
	}

	/* Enumerate 16-bit modes. */
	fmt.BitsPerPixel = 16;
	modes = SDL_ListModes(&fmt, SDL_FULLSCREEN | SDL_HWSURFACE);
	if (modes)
	{
		if (modes == (SDL_Rect**)-1)
		{
			/* Add basic modes (shouldn't really happen). */
			rinAddMode(dev,  640, 480, 16);
			rinAddMode(dev,  800, 600, 16);
			rinAddMode(dev, 1024, 768, 16);
			if (max_width >= 1280)
				rinAddMode(dev, 1280, 1024, 16);
			if (max_width >= 1600)
				rinAddMode(dev, 1600, 1200, 16);
		}
		else
		{
			for (i = 0; modes[i]; i++)
			{
				if (modes[i]->w < 640 || modes[i]->h < 480 ||
					modes[i]->w > 1600)
					continue;
				rinAddMode(dev, modes[i]->w, modes[i]->h, 16);
			}
		}
	}

	/* Enumerate 32-bit modes. */
	fmt.BitsPerPixel = 32;
	modes = SDL_ListModes(&fmt, SDL_FULLSCREEN | SDL_HWSURFACE);
	if (modes)
	{
		if (modes == (SDL_Rect**)-1)
		{
			/* Add basic modes (shouldn't really happen). */
			rinAddMode(dev,  640, 480, 32);
			rinAddMode(dev,  800, 600, 32);
			rinAddMode(dev, 1024, 768, 32);
			if (max_width >= 1280)
				rinAddMode(dev, 1280, 1024, 32);
			if (max_width >= 1600)
				rinAddMode(dev, 1600, 1200, 32);
		}
		else
		{
			for (i = 0; modes[i]; i++)
			{
				if (modes[i]->w < 640 || modes[i]->h < 480 ||
					modes[i]->w > 1600)
					continue;
				rinAddMode(dev, modes[i]->w, modes[i]->h, 32);
			}
		}
	}

	return (dev->modes != 0);
}

/*-----------------------------------------------------------------------------
    Name        : rinEnumerate3Dfx
    Description : enumerate available display modes on possible 3Dfx display
    Inputs      : dev - the device whose modes are to be filled
    Outputs     :
    Return      : true or false (could or couldn't enumerate)
----------------------------------------------------------------------------*/
bool rinEnumerate3Dfx(rdevice* dev)
{
	bool res;

	if (!dev)
		return FALSE;

	/* Use primary display enumeration. */
	res = rinEnumeratePrimary(dev);

	/* Set 3dfx OpenGL device info. */
	dev->type = RIN_TYPE_OPENGL;

	return res;
}

/*-----------------------------------------------------------------------------
    Name        : rinEnumerateDevices
    Description : populate the device list by enumerating available renderers
    Inputs      :
    Outputs     : rDeviceList is filled
    Return      :
----------------------------------------------------------------------------*/
int rinEnumerateDevices(void)
{
    rdevice* dev;
    rdevice* gldev;
    rdevice  primaryDev;
    bool primaryVal;
    
#ifndef _MACOSX_FIX_ME
    int voodoo, maxWidth;
#endif

#if 0	/* CRC log only used by Direct3D. */
    if (mainOutputCRC)
    {
        rinResetCRCLog();
    }
#endif

    rDeviceList = NULL;
    nDevices = 0;
    gDevcaps  = 0xFFFFFFFF;
    gDevcaps2 = 0x00000000;

    bFullscreenDevice = FALSE;

    //add Direct3D devices
    /*nDevices += rinEnumerateDirect3D();*/

    //add OpenGL device
    dev = (rdevice*)rinMemAlloc(sizeof(rdevice));
    if (nDevices == 1)
    {
        dev->devcaps  = rDeviceList->devcaps;
        dev->devcaps2 = rDeviceList->devcaps2;
    }
    else if (nDevices == 2)
    {
        dev->devcaps  = rDeviceList->next->devcaps;
        dev->devcaps2 = rDeviceList->next->devcaps2;
    }
    else if (gDevcaps != 0xFFFFFFFF)
    {
        dev->devcaps  = gDevcaps;
        dev->devcaps2 = gDevcaps2;
    }
    else
    {
        dev->devcaps = gDevcaps = 0;
        dev->devcaps2 = gDevcaps2 = 0;
    }
    if (dev->devcaps & DEVSTAT_NO_DDRAWSW)
    {
        mainSoftwareDirectDraw = 0;
    }
    dev->type = RIN_TYPE_OPENGL;
    dev->data[0] = '\0';
    if (strCurLanguage == 1)
    {
        strncpy(dev->name, "OpenGL Standard", 63);
    }
    else if (strCurLanguage == 2)
    {
        strncpy(dev->name, "Standard-OpenGL", 63);
    }
    if (strCurLanguage == 3)
    {
        strncpy(dev->name, "OpenGL predeterminado", 63);
    }
    else if (strCurLanguage == 4)
    {
        strncpy(dev->name, "OpenGL di Default", 63);
    }
    else
    {
        strncpy(dev->name, "Default OpenGL", 63);
    }
    dev->modes = NULL;
    //try good old DirectDraw <= 3 enumeration
    memset(&primaryDev, 0, sizeof(rdevice));
    primaryVal = rinEnumeratePrimary(&primaryDev);
    if (primaryVal)
    {
        rinCopyModes(dev, &primaryDev);
    }
    else
    {
        //assume these are supported
        rinAddMode(dev,  640, 480, 16);
        rinAddMode(dev,  800, 600, 16);
        rinAddMode(dev, 1024, 768, 16);
    }
    rinSortModes(dev);
    gldev = dev;
    nDevices++;

#ifndef _MACOSX_FIX_ME
    //add possible 3dfx OpenGL device
    voodoo = SST_VOODOO2; //sane default if sstHardwareExists doesn't get called
    if (!(gDevcaps & DEVSTAT_NO_3DFXGL) &&
        (bFullscreenDevice || sstHardwareExists(&voodoo)))
    {
        rdevice* odev = dev;
        dev = (rdevice*)rinMemAlloc(sizeof(rdevice));
        dev->devcaps = 0;
        dev->type = RIN_TYPE_OPENGL;
        dev->data[0] = '\0';
        dev->modes = NULL;

        (void)rinEnumerate3Dfx(dev);
        strncpy(dev->name, "3dfx OpenGL", 63);
        if (dev->modes == NULL)
        {
            switch (voodoo)
            {
            case SST_VOODOO:
                maxWidth = 800;
                break;
            case SST_VOODOO2:
                maxWidth = 1024;
                break;
            default:
                maxWidth = 0;
            }
            rinCopyModesSelectively(dev, odev, 16, maxWidth);
        }
        rinAddDevice(dev);
        rinSortModes(dev);
        nDevices++;
    }
#endif // _MACOSX_FIX_ME

    if (!(gDevcaps & DEVSTAT_NOGL_9X))
    {
        if (gl95)
        {
            if (!(gDevcaps2 & DEVSTAT2_NOGL_95))
            {
                rinAddDevice(gldev);
            }
        }
        else
        {
            rinAddDevice(gldev);
        }
    }

#ifndef _MACOSX_FIX_ME
    //add software renderer, an explicitly known device
    dev = (rdevice*)rinMemAlloc(sizeof(rdevice));
    dev->devcaps = 0;
    dev->type = RIN_TYPE_SOFTWARE;
    dev->data[0] = '\0';
    strncpy(dev->name, "Software", 63);
    dev->modes = NULL;
    if (primaryVal)
    {
        if (gDevcaps & DEVSTAT_DISABLE_SW)
        {
            primaryDev.devcaps = gDevcaps;
            rinSortModes(&primaryDev);
        }
        rinCopyModesSelectively(dev, &primaryDev, 16, 1600);
        if (!dev->modes)
            rinCopyModesSelectively(dev, &primaryDev, 24, 1600);
        if (!dev->modes)
            rinCopyModesSelectively(dev, &primaryDev, 32, 1600);
    }
    else
    {
        maxWidth = rinMaxWidth();
        rinAddMode(dev, 640, 480, 16);
        rinAddMode(dev, 800, 600, 16);
        rinAddMode(dev, 1024, 768, 16);
        if (maxWidth >= 1280)
        {
            rinAddMode(dev, 1280, 1024, 16);
        }
        if (maxWidth >= 1600)
        {
            rinAddMode(dev, 1600, 1200, 16);
        }
    }
    rinSortModes(dev);
    rinAddDevice(dev);
    nDevices++;
#endif // _MACOSX

#if 0	/* CRC log only used by Direct3D. */
    if (mainOutputCRC)
    {
        rinCloseCRCLog();
    }
#endif

    //success
    return 1;
}
