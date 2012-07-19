/*=============================================================================
    Name    : d3denum.h
    Purpose : Direct3D enumeration functions

    Created 10/2/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _D3DENUM_H
#define _D3DENUM_H

void enum_d3d_devices(d3d_context* d3d, char* name);
void enum_d3d_output(d3d_context* d3d);
void enum_dd_devices(d3d_context* d3d);
void enum_dd_cleanup(d3d_context* d3d);
HRESULT WINAPI enum_depthbuffer_cb(DDPIXELFORMAT* ddpf, VOID* userData);

#endif
