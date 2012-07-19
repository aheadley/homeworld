/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Header: /devel/sst1/glide/src/glideutl.h 6     8/14/97 5:32p Pgj $
** $Log: /devel/sst1/glide/src/glideutl.h $
 * 
 * 6     8/14/97 5:32p Pgj
 * remove dead code per GMT
 * 
 * 5     6/12/97 5:19p Pgj
 * Fix bug 578
 * 
 * 4     3/05/97 9:36p Jdt
 * Removed guFbWriteRegion added guEncodeRLE16
 * 
 * 3     1/16/97 3:45p Dow
 * Embedded fn protos in ifndef FX_GLIDE_NO_FUNC_PROTO 
*/

/* Glide Utility routines */

#ifndef __GLIDEUTL_H__
#define __GLIDEUTL_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FX_GLIDE_NO_FUNC_PROTO
/*
** rendering functions
*/
FX_ENTRY void FX_CALL
guAADrawTriangleWithClip( const GrVertex *a, const GrVertex
                         *b, const GrVertex *c);

FX_ENTRY void FX_CALL
guDrawTriangleWithClip(
                       const GrVertex *a,
                       const GrVertex *b,
                       const GrVertex *c
                       );

FX_ENTRY void FX_CALL
guDrawPolygonVertexListWithClip( int nverts, const GrVertex vlist[] );

/*
** hi-level rendering utility functions
*/
FX_ENTRY void FX_CALL
guAlphaSource( GrAlphaSource_t mode );

FX_ENTRY void FX_CALL
guColorCombineFunction( GrColorCombineFnc_t fnc );

FX_ENTRY int FX_CALL
guEncodeRLE16( void *dst, 
               void *src, 
               FxU32 width, 
               FxU32 height );

FX_ENTRY FxU16 * FX_CALL
guTexCreateColorMipMap( void );

/*
** fog stuff
*/
FX_ENTRY float FX_CALL
guFogTableIndexToW( int i );

FX_ENTRY void FX_CALL
guFogGenerateExp( GrFog_t fogtable[GR_FOG_TABLE_SIZE], float density );

FX_ENTRY void FX_CALL
guFogGenerateExp2( GrFog_t fogtable[GR_FOG_TABLE_SIZE], float density );

FX_ENTRY void FX_CALL
guFogGenerateLinear(
                    GrFog_t fogtable[GR_FOG_TABLE_SIZE],
                    float nearZ, float farZ );

/*
** endian stuff
*/
FX_ENTRY FxU32 FX_CALL
guEndianSwapWords( FxU32 value );

FX_ENTRY FxU16 FX_CALL
guEndianSwapBytes( FxU16 value );

/*
** hi-level texture manipulation tools.
*/
FX_ENTRY FxBool FX_CALL
gu3dfGetInfo( const char *filename, Gu3dfInfo *info );

FX_ENTRY FxBool FX_CALL
gu3dfLoad( const char *filename, Gu3dfInfo *data );

#endif /* FX_GLIDE_NO_FUNC_PROTO */

#ifdef __cplusplus
}
#endif

#endif /* __GLIDEUTL_H__ */
