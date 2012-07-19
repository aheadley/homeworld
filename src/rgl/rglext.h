#ifndef _RGLEXT_H
#define _RGLEXT_H

typedef GLint RGLenum;

#define RGL_VERTEX_LIST     1
#define RGL_NORMAL_LIST     2
#define RGL_POLY_LIST       3
#define RGL_SIZE            4
#define RGL_X               5
#define RGL_Y               6
#define RGL_Z               7
#define RGL_NORMAL          8
#define RGL_VERTICES        9
#define RGL_MATERIAL        10
#define RGL_TEXCOORDS       11

DLL void rglListSpec(RGLenum pname, RGLenum param, GLint n, GLenum format);
DLL void rglList(RGLenum pname, GLvoid const* list);
DLL void rglNormal(GLint param);
DLL void rglTriangle(GLint iPoly);
DLL void rglTexturedTriangle(GLint iPoly);
DLL void rglSmoothTriangle(GLint iPoly);
DLL void rglSmoothTexturedTriangle(GLint iPoly);
DLL void rglMeshRender(GLint n, void (*callback)(GLint material), GLint* meshPolyMode);

#endif

