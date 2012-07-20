
#define MANVERSION              2
#define MANREVISION             53

#ifndef GLIDE3
#define VERSIONSTR "2.53\0"
#else
#define VERSIONSTR "3.0\0"
#endif

#if defined(CVG) || defined(VOODOO2)
#   define HWSTR   " Voodoo(tm)\0"
#   ifdef NT_BUILD
#     define PRODNAME "Glide(tm) for Voodoo\251 and Windows\256 NT\0"
#   else
#     define PRODNAME "Glide(tm) for Voodoo\251 and Windows\256 95/98\0"
#   endif /* NT_BUILD */
#elif defined(H3)
#   define HWSTR   " Banshee(tm)\0"
#   ifdef NT_BUILD
#     define PRODNAME "Glide(tm) for Banshee\251 and Windows\256 NT\0"
#   else
#     define PRODNAME "Glide(tm) for Banshee\251 and Windows\256 95/98\0"
#   endif /* NT_BUILD */
#else
#   define PRODNAME "Something really, really important\0"
#   define HWSTR   "Unknown Chip\0"
#endif
