/*=============================================================================
    REGKEY.H: Registry key definitions for Homeworld.

    Created Sept 1999 by Janik Joire
=============================================================================*/

// Do not change these registry key definitions! The installer depends on them...
#if defined(Downloadable)
#define BASEKEYNAME "SOFTWARE\\Sierra On-Line\\Homeworld Downloadable"
#define CONFIGDIR ".homeworldDownloadable"
#elif defined(OEM)
#define BASEKEYNAME "SOFTWARE\\Sierra On-Line\\Homeworld OEM"
#define CONFIGDIR ".homeworldOEM"
#else
#define BASEKEYNAME "SOFTWARE\\Sierra On-Line\\Homeworld"
#define CONFIGDIR ".homeworld"
#endif
