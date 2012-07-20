/*=============================================================================
    Name    : devstats.h
    Purpose : bitflags for disabling certain features on hardware w/ known problems

    Created 9/9/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _DEVSTATS_H
#define _DEVSTATS_H

//bit 00 (000001) disable fast front end d3d, gl
//bit 01 (000002) disable fast front end gl
//bit 02 (000004) disable gl under 9x
//bit 03 (000008) disable d3d ffe
//bit 04 (000010) increase texture minimums (8)
//bit 05 (000020) gl double buffer is triple buffer
//bit 06 (000040) disable 32bit display modes
//bit 07 (000080) disable 32bit gl display modes
//bit 08 (000100) disable d3d under 9x
//bit 09 (000200) disable 3dfx gl special-case option
//bit 10 (000400) don't use ddraw for gl setup
//bit 11 (000800) fractional y font adjustment
//bit 12 (001000) fractional x font adjustment
//bit 13 (002000) no paletted textures
//bit 14 (004000) no directdraw for software
//bit 15 (008000) no bg colour
//bit 16 (010000) no d3d bg colour
//bit 17 (020000) no antialiasing (it screws up the TO)
//bit 18 (040000) fractional -x font adjustment
//bit 19 (080000) don't GetTexImage
//bit 20 (100000) no pointsmoothing
//bit 21 (200000) mode disabling also applies to sw

//bit 31 (80000000) disable 640x480 16
//bit 30 (40000000) disable 800x600 16
//bit 29 (20000000) disable 1024x768 16
//bit 28 (10000000) disable 1280x1024 16
//bit 27 (08000000) disable 1600x1200 16
//bit 26 (04000000) disable 640x480 32
//bit 25 (02000000) disable 800x600 32
//bit 24 (01000000) disable 1024x768 32
//bit 23 (00800000) disable 1280x1024 32
//bit 22 (00400000) disable 1600x1200 32

#define DEVSTAT2_BROKEN_DEPTH   0x00000001
#define DEVSTAT2_NO_ADDITIVE    0x00000002
#define DEVSTAT2_NO_IALPHA      0x00000004
#define DEVSTAT2_NO_DRAWPIXELS  0x00000008
#define DEVSTAT2_NO_USERCLIP    0x00000010
#define DEVSTAT2_NO_DALPHA      0x00000020
#define DEVSTAT2_NOGL_95        0x00000040
#define DEVSTAT2_NOD3D_95       0x00000080

#define DEVSTAT_NOFFE           0x00000001
#define DEVSTAT_NOFFE_GL        0x00000002
#define DEVSTAT_NOGL_9X         0x00000004
#define DEVSTAT_NOFFE_D3D       0x00000008
#define DEVSTAT_LARGE_TEXTURES  0x00000010
#define DEVSTAT_GL_TRIPLE       0x00000020
#define DEVSTAT_NO_32BIT        0x00000040
#define DEVSTAT_NO_32BIT_GL     0x00000080
#define DEVSTAT_NOD3D_9X        0x00000100
#define DEVSTAT_NO_3DFXGL       0x00000200
#define DEVSTAT_NO_DDRAW        0x00000400
#define DEVSTAT_YADJUST         0x00000800
#define DEVSTAT_XADJUST         0x00001000
#define DEVSTAT_NOPAL           0x00002000
#define DEVSTAT_NO_DDRAWSW      0x00004000
#define DEVSTAT_NO_BGCOLOUR     0x00008000
#define DEVSTAT_NOD3D_BGCOLOUR  0x00010000
#define DEVSTAT_NO_AA           0x00020000
#define DEVSTAT_NEGXADJUST      0x00040000
#define DEVSTAT_NO_GETTEXIMAGE  0x00080000
#define DEVSTAT_NO_POINTSMOOTH  0x00100000
#define DEVSTAT_DISABLE_SW      0x00200000

#define DEVSTAT_NO_64048016     0x80000000
#define DEVSTAT_NO_80060016     0x40000000
#define DEVSTAT_NO_102476816    0x20000000
#define DEVSTAT_NO_1280102416   0x10000000
#define DEVSTAT_NO_1600120016   0x08000000
#define DEVSTAT_NO_64048032     0x04000000
#define DEVSTAT_NO_80060032     0x02000000
#define DEVSTAT_NO_102476832    0x01000000
#define DEVSTAT_NO_1280102432   0x00800000
#define DEVSTAT_NO_1600120032   0x00400000

#endif
