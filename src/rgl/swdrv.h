#ifndef _SWDRV_H
#define _SWDRV_H

typedef struct
{
    GLubyte* BlendedData;
    GLushort Palette[256];
    GLushort Palette444[256];
} sw_texobj;

typedef struct sw_context_s
{
    GLint MonocolorR;
    GLint MonocolorG;
    GLint MonocolorB;
    GLint MonocolorA;

    void (*triangle_func)();
    void (*line_func)();

    GLubyte* alphaTable_oneminussrcalpha;
    GLubyte* alphaTable_one;

    //4*screenWidth in size
    GLubyte* byteTable;

    GLboolean generalTriangle;
    GLboolean generalPerspectiveTriangle;
} sw_context;

#endif
