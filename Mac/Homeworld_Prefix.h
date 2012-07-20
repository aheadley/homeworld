//
// Prefix header for all source files of the 'Homeworld' target in the 'Homeworld' project
//

#ifdef __OBJC__
    #import <Cocoa/Cocoa.h>
#endif

// _MACOSX        - use for platform-specific fixes (#define'd on gcc command line)
// _MACOSX_FIX_ME - use to temporarily turn off code in order to get around the compiler
// ENDIAN_BIG     - use where data needs byte-swapping (effectively synonymous with _MACOSX
//                  at the moment but more useful should another big endian platform join us)

#ifdef _MACOSX
    #ifndef _MACOSX_FIX_ME
        #define _MACOSX_FIX_ME 1
    #endif
#endif