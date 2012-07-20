/* There is no corresponding win32glue.h file since these functions
   are called directly from converted W32 object files. */

/* Entirely ripped from the Wine project (thanks!) */

#define DO_FPU(x,y) __asm__ __volatile__( x " %0;fwait" : "=m" (y) : )
#define POP_FPU(x) DO_FPU("fstpl",x)

#define WINAPI      __stdcall
typedef unsigned long DWORD;
typedef unsigned char BYTE;
#define __int64 long long
typedef __int64 LONGLONG;

#ifndef __cdecl
# if defined(__i386__) && defined(__GNUC__)
#  define __cdecl __attribute__((__cdecl__))
# elif !defined(_MSC_VER)
#  define __cdecl
# endif
#endif /* __cdecl */

#ifndef __stdcall
# ifdef __i386__
#  ifdef __GNUC__
#   define __stdcall __attribute__((__stdcall__))
#  elif defined(_MSC_VER)
    /* Nothing needs to be done. __stdcall already exists */
#  else
#   error You need to define __stdcall for your compiler
#  endif
# else  /* __i386__ */
#  define __stdcall
# endif  /* __i386__ */
#endif /* __stdcall */

/*********************************************************************
 *                  _ftol   (NTDLL.@)
 *
 * VERSION
 *      [GNUC && i386]
 */
#if defined(__GNUC__) && defined(__i386__)
LONGLONG __cdecl _ftol(void)
{
        /* don't just do DO_FPU("fistp",retval), because the rounding
         * mode must also be set to "round towards zero"... */
        double fl;
        POP_FPU(fl);
        return (LONGLONG)fl;
}
#endif /* defined(__GNUC__) && defined(__i386__) */

asm (".globl _chkstk\n"
     ".type   _chkstk, @function\n"
     "_chkstk:\n"
     "popl %ecx\n"           /* Copy function's return address */
     "subl %eax, %esp\n"
     "pushl %ecx\n"          /* Restore original return address */
     "ret\n"
     ".size   _chkstk, .-_chkstk\n");

#if 0
#define SIZE_OF_80387_REGISTERS      80
typedef struct _FLOATING_SAVE_AREA
{
    DWORD   ControlWord;
    DWORD   StatusWord;
    DWORD   TagWord;
    DWORD   ErrorOffset;
    DWORD   ErrorSelector;
    DWORD   DataOffset;
    DWORD   DataSelector;
    BYTE    RegisterArea[SIZE_OF_80387_REGISTERS];
    DWORD   Cr0NpxState;
} FLOATING_SAVE_AREA, *PFLOATING_SAVE_AREA;

#define MAXIMUM_SUPPORTED_EXTENSION     512
typedef struct _CONTEXT86
{
    DWORD   ContextFlags;

    /* These are selected by CONTEXT_DEBUG_REGISTERS */
    DWORD   Dr0;
    DWORD   Dr1;
    DWORD   Dr2;
    DWORD   Dr3;
    DWORD   Dr6;
    DWORD   Dr7;

    /* These are selected by CONTEXT_FLOATING_POINT */
    FLOATING_SAVE_AREA FloatSave;

    /* These are selected by CONTEXT_SEGMENTS */
    DWORD   SegGs;
    DWORD   SegFs;
    DWORD   SegEs;
    DWORD   SegDs;

    /* These are selected by CONTEXT_INTEGER */
    DWORD   Edi;
    DWORD   Esi;
    DWORD   Ebx;
    DWORD   Edx;
    DWORD   Ecx;
    DWORD   Eax;

    /* These are selected by CONTEXT_CONTROL */
    DWORD   Ebp;
    DWORD   Eip;
    DWORD   SegCs;
    DWORD   EFlags;
    DWORD   Esp;
    DWORD   SegSs;

    BYTE    ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
} CONTEXT86;

/**************************************************************************
 *                 _chkstk                              [NTDLL.@]
 *
 * Glorified "enter xxxx".
 */
void WINAPI _chkstk( CONTEXT86 *context )
{
    context->Esp -= context->Eax;
}
#endif
