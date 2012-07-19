/*=============================================================================
    Name    : d3dtex.h
    Purpose : Direct3D texture-related functions

    Created 10/2/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _D3DTEX_H
#define _D3DTEX_H

void enum_texture_formats(d3d_context* d3d);
GLboolean d3d_match_texture_formats(d3d_context* d3d);

GLboolean d3d_blt_texture(gl_texture_object* tex, DDPIXELFORMAT* ddpf);
GLboolean d3d_create_texture(gl_texture_object* tex);
void d3d_free_texture(d3d_texobj* t3d);
d3d_texobj* d3d_alloc_texobj(void);
void d3d_fill_texobj(gl_texture_object* tex);
GLboolean d3d_bind_texture(d3d_texobj* t3d);
void d3d_load_all_textures(GLcontext* ctx);
void d3d_free_all_textures(GLcontext* ctx);
GLboolean d3d_create_shared_palette(void);
GLboolean d3d_modify_shared_palette(void);
GLboolean d3d_attach_shared_palette(gl_texture_object* tex);
GLboolean d3d_attach_palette(gl_texture_object* tex);


#endif
