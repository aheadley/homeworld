#define WIN32_LEAN_AND_MEAN
#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include "/homeworld/src/rgl/d3d/ddraw.h"
#include "/homeworld/src/rgl/d3d/d3drm.h"
#include "/homeworld/tools/win32/paco/crc.h"

LPSTR gDesc, gName;

typedef struct
{
    DWORD vendor;
    WORD  product;
    DWORD device;
} hwDat;

hwDat dat;

static void hwdOutputDevice(LPSTR desc0, LPSTR name0, LPD3DDEVICEDESC hal)
{
    DWORD crc0, crc1, crc2;
    D3DDEVICEDESC caps;

    caps = *hal;
//    crc0 = (DWORD)crc32Compute((BYTE*)&caps, sizeof(D3DDEVICEDESC));
    crc0 = (DWORD)crc32Compute((BYTE*)&dat, sizeof(hwDat));
    caps.dwSize = 0;
    caps.dwFlags = 0;
    caps.dcmColorModel = (D3DCOLORMODEL)0;
    caps.dwDevCaps = 0;
    caps.bClipping = 0;
    crc1 = (DWORD)crc32Compute((BYTE*)&caps, sizeof(D3DDEVICEDESC));
    
    crc2  = (DWORD)crc32Compute((BYTE*)&caps.dtcTransformCaps, sizeof(D3DTRANSFORMCAPS));
    crc2 += (DWORD)crc32Compute((BYTE*)&caps.dpcLineCaps, sizeof(D3DPRIMCAPS));
    crc2 += (DWORD)crc32Compute((BYTE*)&caps.dpcTriCaps, sizeof(D3DPRIMCAPS));
    
    printf("[%08X][%08X][%08X][%s][%s]\n",
           crc1, crc0, crc2,
           desc0, name0);
}

HRESULT WINAPI hwdDirect3D_cb(GUID* pGUID, LPSTR desc, LPSTR name, LPD3DDEVICEDESC hal, LPD3DDEVICEDESC hel, LPVOID context)
{
    if (pGUID == NULL || hal == NULL || hel == NULL)
    {
        return D3DENUMRET_CANCEL;
    }
    if (hal->dwFlags != 0)
    {
        hwdOutputDevice(gDesc, gName, hal);
    }
    return D3DENUMRET_OK;
}

BOOL WINAPI hwdDirectDraw_cb(GUID FAR* lpGUID, LPSTR desc, LPSTR name, LPVOID context)
{
    LPDIRECTDRAW ddraw1;
    LPDIRECTDRAW4 ddraw4;
    LPDIRECT3D3 d3dObject;
    DDDEVICEIDENTIFIER dddi;

    if (FAILED(DirectDrawCreate(lpGUID, &ddraw1, NULL)))
    {
        return D3DENUMRET_OK;
    }
    if (FAILED(ddraw1->QueryInterface(IID_IDirectDraw4, (void**)&ddraw4)))
    {
        ddraw1->Release();
        return D3DENUMRET_OK;
    }
    if (FAILED(ddraw4->QueryInterface(IID_IDirect3D3, (void**)&d3dObject)))
    {
        ddraw4->Release();
        ddraw1->Release();
        return D3DENUMRET_OK;
    }

    if (FAILED(ddraw4->GetDeviceIdentifier(&dddi, 0)))
    {
        ddraw4->Release();
        ddraw1->Release();
        return D3DENUMRET_OK;
    }
    memset(&dat, 0, sizeof(hwDat));

    dat.vendor  = dddi.dwVendorId;
    dat.product = 4;//HIWORD(dddi.liDriverVersion.HighPart);
    dat.device  = dddi.dwDeviceId;

    gDesc = desc;
    gName = name;
    d3dObject->EnumDevices(hwdDirect3D_cb, NULL);
    d3dObject->Release();
    ddraw4->Release();
    ddraw1->Release();
    
    printf("  szDriver %s\n", dddi.szDriver);
    printf("  szDescription %s\n", dddi.szDescription);
    LARGE_INTEGER* pli = &dddi.liDriverVersion;
    unsigned int* int0 = (unsigned int*)pli;
    unsigned int* int1 = int0;
    int1++;
    printf("  liDriverVersion %u.%u\n", *int0, *int1);
    printf("    wProduct %u, wVersion %u, wSubVersion %u, wBuild %u\n",
           HIWORD(dddi.liDriverVersion.HighPart),
           LOWORD(dddi.liDriverVersion.HighPart),
           HIWORD(dddi.liDriverVersion.LowPart),
           LOWORD(dddi.liDriverVersion.LowPart));
    printf("  dwVendorId %X (%u)\n", dddi.dwVendorId, dddi.dwVendorId);
    printf("  dwDeviceId %X (%u)\n", dddi.dwDeviceId, dddi.dwDeviceId);
    printf("  dwSubSysId %X\n", dddi.dwSubSysId);
    printf("  dwRevision %X\n", dddi.dwSubSysId);

    return D3DENUMRET_OK;
}

static void hwdEnumerate(void)
{
    (void)DirectDrawEnumerate(hwdDirectDraw_cb, NULL);
}

int main(int argc, char* argv[])
{
    if (argc == 3)
    {
        DWORD vendorId, deviceId;
        sscanf(argv[1], "%u", &vendorId);
        sscanf(argv[2], "%u", &deviceId);
        memset(&dat, 0, sizeof(hwDat));
        dat.vendor  = vendorId;
        dat.product = 4;
        dat.device  = deviceId;
        DWORD crc = (DWORD)crc32Compute((BYTE*)&dat, sizeof(hwDat));
        printf("[%08X]\n", crc);
    }
    else
    {
        hwdEnumerate();
    }
    return 0;
}
