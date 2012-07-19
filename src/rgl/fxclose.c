#include <glide.h>

int main(void)
{
    GrHwConfiguration hwconfig;

    grGlideInit();

    if (grSstQueryHardware(&hwconfig))
    {
        grSstSelect(0);
        grSstWinOpen(0, GR_RESOLUTION_640x480, GR_REFRESH_60Hz,
                     GR_COLORFORMAT_ABGR, GR_ORIGIN_LOWER_LEFT,
                     2, 1);
        grSstControl(GR_CONTROL_DEACTIVATE);
        grGlideShutdown();
    }
    return 0;
}
