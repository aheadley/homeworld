#ifndef _iKVB_H
#define _iKVB_H

#define MAX_CLIP_PLANES	6

#define VB_MAX	8192
#define VB_SIZE	(VB_MAX + 2 * (6 + MAX_CLIP_PLANES))

typedef struct vertex_buffer_s
{
    GLfloat Obj[VB_SIZE][4];
    GLfloat Eye[VB_SIZE][4];
    GLfloat Clip[VB_SIZE][4];
    GLfloat Win[VB_SIZE][3];

    GLfloat Normal[VB_SIZE][3];

    GLubyte (*Color)[4];
    GLubyte Fcolor[VB_SIZE][4];
    GLubyte Bcolor[VB_SIZE][4];

    GLfloat TexCoord[VB_SIZE][2];

    GLubyte ClipMask[VB_SIZE];
    GLubyte ClipOrMask;
    GLubyte ClipAndMask;

    GLuint Start;
    GLuint Count;
    GLuint Free;		/* next empty position (for clipping) */

    /* FIXME: materials */
} vertex_buffer;

#define CLIP_RIGHT_BIT	0x01
#define CLIP_LEFT_BIT	0x02
#define CLIP_TOP_BIT	0x04
#define CLIP_BOTTOM_BIT	0x08
#define CLIP_NEAR_BIT	0x10
#define CLIP_FAR_BIT	0x20
#define CLIP_USER_BIT	0x40
#define CLIP_ALL_BITS	0x3f

#define CLIP_ALL	1
#define CLIP_NONE	2
#define CLIP_SOME	3

vertex_buffer* gl_alloc_vb(void);
//void gl_render_vb(GLcontext*, GLboolean);
//void gl_reset_vb(GLcontext*, GLboolean);

struct gl_context_s;
void gl_transform_vb_part1(struct gl_context_s*, GLboolean);  /* kgl.c needs this, too. */
//void gl_transform_vb_part2();

#endif
