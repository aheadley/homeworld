/*=============================================================================
    Name    : d3dinit.h
    Purpose : Direct3D initialization

    Created 10/3/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _D3DINIT_H
#define _D3DINIT_H

GLboolean initialize_directx(GLcontext* ctx);
void d3d_shutdown(GLcontext* ctx);

extern LPDIRECTDRAW lpDirectDraw;

#endif
