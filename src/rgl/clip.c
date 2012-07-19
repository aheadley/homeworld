#include <string.h>
#include "kgl.h"
#include "maths.h"
#include "clip.h"


#define LINTERP(T, A, B)  ((A) + (T) * ((B) - (A)))
#define ILINTERP(T, A, B) FAST_TO_INT(((A) + (T) * ((B) - (A))))


static void interpolate_aux(GLcontext* ctx,
			    GLuint dst, GLfloat t, GLuint in, GLuint out)
{
    vertex_buffer* VB = ctx->VB;

    if (ctx->ClipMask & CLIP_FCOLOR_BIT)
    {
        VB->Fcolor[dst][0] = ILINTERP(t, VB->Fcolor[in][0], VB->Fcolor[out][0]);
        VB->Fcolor[dst][1] = ILINTERP(t, VB->Fcolor[in][1], VB->Fcolor[out][1]);
        VB->Fcolor[dst][2] = ILINTERP(t, VB->Fcolor[in][2], VB->Fcolor[out][2]);
        VB->Fcolor[dst][3] = ILINTERP(t, VB->Fcolor[in][3], VB->Fcolor[out][3]);
    }
    if (ctx->ClipMask & CLIP_BCOLOR_BIT)
    {
        VB->Bcolor[dst][0] = ILINTERP(t, VB->Bcolor[in][0], VB->Bcolor[out][0]);
        VB->Bcolor[dst][1] = ILINTERP(t, VB->Bcolor[in][1], VB->Bcolor[out][1]);
        VB->Bcolor[dst][2] = ILINTERP(t, VB->Bcolor[in][2], VB->Bcolor[out][2]);
        VB->Bcolor[dst][3] = ILINTERP(t, VB->Bcolor[in][3], VB->Bcolor[out][3]);
    }
    if (ctx->ClipMask & CLIP_TEXTURE_BIT)
    {
        VB->Eye[dst][2] = LINTERP(t, VB->Eye[in][2], VB->Eye[out][2]);
        VB->TexCoord[dst][0] = LINTERP(t, VB->TexCoord[in][0], VB->TexCoord[out][0]);
        VB->TexCoord[dst][1] = LINTERP(t, VB->TexCoord[in][1], VB->TexCoord[out][1]);
    }
}

GLuint gl_viewclip_point(GLfloat const v[])
{
    if (v[0] > v[3] || v[0] < -v[3] ||
	    v[1] > v[3] || v[1] < -v[3] ||
	    v[2] > v[3] || v[2] < -v[3])
	    return 0;
    else
    	return 1;
}

/*
 * Clip a line segment against the view volume defined by -w<=x,y,z<=w.
 * Input:  i, j - indexes into VB->V* of endpoints of the line
 * Return:  0 = line completely outside of view
 *          1 = line is inside view.
 */
GLuint gl_viewclip_line(GLcontext* ctx, GLuint* i, GLuint* j)
{
    vertex_buffer* VB = ctx->VB;
    GLfloat (*coord)[4] = VB->Clip;

    GLfloat t, dx, dy, dz, dw;
    GLuint ii, jj;

    ii = *i;
    jj = *j;

/*
 * We use 6 instances of this code to clip against the 6 planes.
 * For each plane, we define the OUTSIDE and COMPUTE_INTERSECTION
 * macros apprpriately.
 */
#define GENERAL_CLIP \
    if (OUTSIDE(ii)) \
    { \
        if (OUTSIDE(jj)) \
        { \
            /* both verts outside */ \
            return 0; \
        } \
        else \
        { \
            /* ii outside, jj inside */ \
            /* new vert in position VB->Free */ \
            COMPUTE_INTERSECTION(VB->Free, jj, ii) \
            if (ctx->ClipMask) \
                interpolate_aux(ctx, VB->Free, t, jj, ii); \
            ii = VB->Free; \
            VB->Free++; \
            if (VB->Free == VB_SIZE) \
                VB->Free = 1; \
        } \
    } \
    else \
    { \
        if (OUTSIDE(jj)) \
        { \
            /* ii inside, jj outside */ \
            /* new vert in VB->Free */ \
            COMPUTE_INTERSECTION(VB->Free, ii, jj) \
            if (ctx->ClipMask) \
                interpolate_aux(ctx, VB->Free, t, ii, jj); \
            jj = VB->Free; \
            VB->Free++; \
            if (VB->Free == VB_SIZE) \
                VB->Free = 1; \
        } \
    }


#define X(I)	coord[I][0]
#define Y(I)	coord[I][1]
#define Z(I)	coord[I][2]
#define W(I)	coord[I][3]

/*
 * Begin clipping
 */

   /*** Clip against +X side ***/
#define OUTSIDE(K)      (X(K) > W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
	dx = X(out) - X(in); \
	dw = W(out) - W(in); \
	t = (X(in) - W(in)) / (dw - dx); \
	X(new) = X(in) + t * dx; \
	Y(new) = Y(in) + t * (Y(out) - Y(in)); \
	Z(new) = Z(in) + t * (Z(out) - Z(in)); \
	W(new) = W(in) + t * dw;

   GENERAL_CLIP

#undef OUTSIDE
#undef COMPUTE_INTERSECTION


   /*** Clip against -X side ***/
#define OUTSIDE(K)      (X(K) < -W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
	dx = X(out) - X(in); \
	dw = W(out) - W(in); \
    t = -(X(in) + W(in)) / (dw + dx); \
	X(new) = X(in) + t * dx; \
	Y(new) = Y(in) + t * (Y(out) - Y(in)); \
	Z(new) = Z(in) + t * (Z(out) - Z(in)); \
	W(new) = W(in) + t * dw;

    GENERAL_CLIP

#undef OUTSIDE
#undef COMPUTE_INTERSECTION


   /*** Clip against +Y side ***/
#define OUTSIDE(K)      (Y(K) > W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dy = Y(out) - Y(in); \
	dw = W(out) - W(in); \
    t = (Y(in) - W(in)) / (dw - dy); \
	X(new) = X(in) + t * (X(out) - X(in)); \
	Y(new) = Y(in) + t * dy; \
	Z(new) = Z(in) + t * (Z(out) - Z(in)); \
	W(new) = W(in) + t * dw;

    GENERAL_CLIP

#undef OUTSIDE
#undef COMPUTE_INTERSECTION


   /*** Clip against -Y side ***/
#define OUTSIDE(K)      (Y(K) < -W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dy = Y(out) - Y(in); \
    dw = W(out) - W(in); \
    t = -(Y(in) + W(in)) / (dw + dy); \
    X(new) = X(in) + t * (X(out) - X(in)); \
	Y(new) = Y(in) + t * dy; \
	Z(new) = Z(in) + t * (Z(out) - Z(in)); \
	W(new) = W(in) + t * dw;

    GENERAL_CLIP

#undef OUTSIDE
#undef COMPUTE_INTERSECTION


   /*** Clip against +Z side ***/
#define OUTSIDE(K)      (Z(K) > W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dz = Z(out) - Z(in); \
    dw = W(out) - W(in); \
    t = (Z(in) - W(in)) / (dw - dz); \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
	Z(new) = Z(in) + t * dz; \
	W(new) = W(in) + t * dw;

    GENERAL_CLIP

#undef OUTSIDE
#undef COMPUTE_INTERSECTION


   /*** Clip against -Z side ***/
#define OUTSIDE(K)      (Z(K) < -W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dz = Z(out) - Z(in); \
    dw = W(out) - W(in); \
    t = -(Z(in) + W(in)) / (dw + dz); \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
	Z(new) = Z(in) + t * dz; \
	W(new) = W(in) + t * dw;

    GENERAL_CLIP

#undef OUTSIDE
#undef COMPUTE_INTERSECTION

#undef GENERAL_CLIP

   *i = ii;
   *j = jj;
   return 1;
}


/*
 * Clip a polygon against the view volume defined by -w<=x,y,z<=w.
 * Input:  n - number of vertices in input polygon.
 *         vlist - list of indexes into VB->V* of polygon to clip.
 * Output:  vlist - modified list of vertex indexes
 * Return:  number of vertices in resulting polygon
 */
GLuint gl_viewclip_polygon(GLcontext* ctx, GLuint n, GLuint vlist[])

{
    vertex_buffer* VB = ctx->VB;
    GLfloat (*coord)[4] = VB->Clip;

    GLuint previ, prevj;
    GLuint curri, currj;
    GLuint vlist2[VB_SIZE];
    GLuint n2;
//    GLdouble dx, dy, dz, dw, t, neww;
    GLfloat dx, dy, dz, dw, t, neww;

/*
 * We use 6 instances of this code to implement clipping against the
 * 6 sides of the view volume.  Prior to each we define the macros:
 *    INLIST = array which lists input vertices
 *    OUTLIST = array which lists output vertices
 *    INCOUNT = variable which is the number of vertices in INLIST[]
 *    OUTCOUNT = variable which is the number of vertices in OUTLIST[]
 *    INSIDE(i) = test if vertex v[i] is inside the view volume
 *    COMPUTE_INTERSECTION(in,out,new) = compute intersection of line
 *              from v[in] to v[out] with the clipping plane and store
 *              the result in v[new]
 */

#define GENERAL_CLIP \
    if (INCOUNT < 3) \
        return 0; \
    previ = INCOUNT - 1;    /* previous = last vertex */ \
    prevj = INLIST[previ]; \
    OUTCOUNT = 0; \
    for (curri = 0; curri < INCOUNT; curri++) \
    { \
        currj = INLIST[curri]; \
        if (INSIDE(currj)) \
        { \
            if (INSIDE(prevj)) \
            { \
                /* both verts inside, copy current to outlist */ \
                OUTLIST[OUTCOUNT] = currj; \
                OUTCOUNT++; \
            } \
            else \
            { \
                /* current is inside and previous is outside, clip */ \
                COMPUTE_INTERSECTION(currj, prevj, VB->Free) \
                /* if new point not coincident with previous point ... */ \
                if (t > 0.0) \
                { \
                    /* interpolate aux info using t */ \
                    if (ctx->ClipMask) \
                        interpolate_aux(ctx, VB->Free, t, currj, prevj); \
                    /* output new point */ \
                    OUTLIST[OUTCOUNT] = VB->Free; \
                    VB->Free++; \
                    if (VB->Free == VB_SIZE) \
                        VB->Free = 1; \
                    OUTCOUNT++; \
                } \
                /* output current */ \
                OUTLIST[OUTCOUNT] = currj; \
                OUTCOUNT++; \
            } \
        } \
        else \
        { \
            if (INSIDE(prevj)) \
            { \
                /* current outside, previous inside */ \
                COMPUTE_INTERSECTION(prevj, currj, VB->Free) \
                /* if new point not coincident with prev point ... */ \
                if (t > 0.0) \
                { \
                    /* interpolate aux info using t */ \
                    if (ctx->ClipMask) \
                        interpolate_aux(ctx, VB->Free, t, prevj, currj); \
                    /* output new point */ \
                    OUTLIST[OUTCOUNT] = VB->Free; \
                    VB->Free++; \
                    if (VB->Free == VB_SIZE) \
                        VB->Free = 1; \
                    OUTCOUNT++; \
                } \
            } \
            /* else both verts outside */ \
        } \
        /* let previous = current */ \
        previ = curri; \
        prevj = currj; \
        /* check for overflowing vertex buffer */ \
        if (OUTCOUNT >= VB_SIZE - 1) \
        { \
            /* too many vertices */ \
            if (OUTLIST == vlist2) \
            { \
                /* copy OUTLIST[] to vlist[] */ \
                int i; \
                for (i = 0; i < VB_SIZE; i++) \
                    vlist[i] = OUTLIST[i]; \
            } \
            return VB_SIZE - 1; \
        } \
    }


#define X(I)	coord[I][0]
#define Y(I)	coord[I][1]
#define Z(I)	coord[I][2]
#define W(I)	coord[I][3]

/*
 * Clip against +X
 */
#define INCOUNT n
#define OUTCOUNT n2
#define INLIST vlist
#define OUTLIST vlist2
#define INSIDE(K)      (X(K) <= W(K))

#define COMPUTE_INTERSECTION(in, out, new) \
    dx = X(out) - X(in); \
    dw = W(out) - W(in); \
    t = (X(in) - W(in)) / (dw - dx); \
    neww = W(in) + t * dw; \
    X(new) = neww; \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = neww;

    GENERAL_CLIP

#undef INCOUNT
#undef OUTCOUNT
#undef INLIST
#undef OUTLIST
#undef INSIDE
#undef COMPUTE_INTERSECTION


/*
 * Clip against -X
 */
#define INCOUNT n2
#define OUTCOUNT n
#define INLIST vlist2
#define OUTLIST vlist
#define INSIDE(K)       (X(K) >= -W(K))
#define COMPUTE_INTERSECTION(in, out, new) \
    dx = X(out) - X(in); \
    dw = W(out) - W(in); \
    t = -(X(in) + W(in)) / (dw + dx); \
	neww = W(in) + t * dw; \
    X(new) = -neww; \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = neww;

    GENERAL_CLIP

#undef INCOUNT
#undef OUTCOUNT
#undef INLIST
#undef OUTLIST
#undef INSIDE
#undef COMPUTE_INTERSECTION


/*
 * Clip against +Y
 */
#define INCOUNT n
#define OUTCOUNT n2
#define INLIST vlist
#define OUTLIST vlist2
#define INSIDE(K)       (Y(K) <= W(K))
#define COMPUTE_INTERSECTION(in, out, new) \
    dy = Y(out) - Y(in); \
    dw = W(out) - W(in); \
    t = (Y(in) - W(in)) / (dw - dy); \
	neww = W(in) + t * dw; \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = neww; \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = neww;

    GENERAL_CLIP

#undef INCOUNT
#undef OUTCOUNT
#undef INLIST
#undef OUTLIST
#undef INSIDE
#undef COMPUTE_INTERSECTION


/*
 * Clip against -Y
 */
#define INCOUNT n2
#define OUTCOUNT n
#define INLIST vlist2
#define OUTLIST vlist
#define INSIDE(K)       (Y(K) >= -W(K))
#define COMPUTE_INTERSECTION(in, out, new) \
    dy = Y(out) - Y(in); \
    dw = W(out) - W(in); \
    t = -(Y(in) + W(in)) / (dw + dy); \
	neww = W(in) + t * dw; \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = -neww; \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = neww;

    GENERAL_CLIP

#undef INCOUNT
#undef OUTCOUNT
#undef INLIST
#undef OUTLIST
#undef INSIDE
#undef COMPUTE_INTERSECTION



/*
 * Clip against +Z
 */
#define INCOUNT n
#define OUTCOUNT n2
#define INLIST vlist
#define OUTLIST vlist2
#define INSIDE(K)       (Z(K) <= W(K))
#define COMPUTE_INTERSECTION(in, out, new) \
    dz = Z(out) - Z(in); \
    dw = W(out) - W(in); \
    t = (Z(in) - W(in)) / (dw - dz); \
	neww = W(in) + t * dw; \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = neww; \
    W(new) = neww;

    GENERAL_CLIP

#undef INCOUNT
#undef OUTCOUNT
#undef INLIST
#undef OUTLIST
#undef INSIDE
#undef COMPUTE_INTERSECTION


/*
 * Clip against -Z
 */
#define INCOUNT n2
#define OUTCOUNT n
#define INLIST vlist2
#define OUTLIST vlist
#define INSIDE(K)       (Z(K) >= -W(K))
#define COMPUTE_INTERSECTION(in, out, new) \
    dz = Z(out) - Z(in); \
    dw = W(out) - W(in); \
    t = -(Z(in) + W(in)) / (dw + dz); \
	neww = W(in) + t * dw; \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = -neww; \
    W(new) = neww;

    GENERAL_CLIP

#undef INCOUNT
#undef INLIST
#undef OUTLIST
#undef INSIDE
#undef COMPUTE_INTERSECTION

   /* 'OUTCOUNT' clipped vertices are now back in v[] */
   return OUTCOUNT;

#undef GENERAL_CLIP
#undef OUTCOUNT
}


#define EPSILON -0.8e-03f

#define INSIDE(J, A, B, C, D) \
   ((VB->Eye[J][0] * A + VB->Eye[J][1] * B \
    + VB->Eye[J][2] * C + VB->Eye[J][3] * D) >= EPSILON)


#define OUTSIDE(J, A, B, C, D) \
   ((VB->Eye[J][0] * A + VB->Eye[J][1] * B \
    + VB->Eye[J][2] * C + VB->Eye[J][3] * D) < EPSILON)

GLuint gl_userclip_polygon(GLcontext* ctx, GLuint n, GLuint vlist[])
{
    vertex_buffer* VB = ctx->VB;

    GLuint vlist2[VB_SIZE];
    GLuint *inlist, *outlist;
    GLuint incount, outcount;
    GLuint curri, currj;
    GLuint previ, prevj;
    GLuint p;

    //initialize input vertex list
    incount = n;
    inlist = vlist;
    outlist = vlist2;

    for (p = 0; p < MAX_CLIP_PLANES; p++)
    {
        if (ctx->ClipEnabled[p])
        {
            float a = ctx->ClipEquation[p][0];
            float b = ctx->ClipEquation[p][1];
            float c = ctx->ClipEquation[p][2];
            float d = ctx->ClipEquation[p][3];

            if (incount < 3)
            {
                return 0;
            }

            //initialize prev to be last in the input list
            previ = incount - 1;
            prevj = inlist[previ];

            outcount = 0;

            for (curri = 0; curri < incount; curri++)
            {
                currj = inlist[curri];

                if (INSIDE(currj, a, b, c, d))
                {
                    if (INSIDE(prevj, a, b, c, d))
                    {
                        //both verts inside, copy current to outlist
                        outlist[outcount++] = currj;
                    }
                    else
                    {
                        //current inside and prev outside, clip
                        GLfloat dx, dy, dz, dw, t, denom;

                        //compute t;
                        dx = VB->Eye[prevj][0] - VB->Eye[currj][0];
                        dy = VB->Eye[prevj][1] - VB->Eye[currj][1];
                        dz = VB->Eye[prevj][2] - VB->Eye[currj][2];
                        dw = VB->Eye[prevj][3] - VB->Eye[currj][3];
                        denom = dx*a + dy*b + dz*c + dw*d;
                        if (denom == 0.0f)
                        {
                            t = 0.0f;
                        }
                        else
                        {
                            t = -(VB->Eye[currj][0]*a + VB->Eye[currj][1]*b
                                + VB->Eye[currj][2]*c + VB->Eye[currj][3]*d)
                                / denom;
                            if (t > 1.0f)
                            {
                                t = 1.0f;
                            }
                        }

                        //interpolate new vertex position
                        VB->Eye[VB->Free][0] = VB->Eye[currj][0] + t*dx;
                        VB->Eye[VB->Free][1] = VB->Eye[currj][1] + t*dy;
                        VB->Eye[VB->Free][2] = VB->Eye[currj][2] + t*dz;
                        VB->Eye[VB->Free][3] = VB->Eye[currj][3] + t*dw;

                        //interpolate color, texture
                        interpolate_aux(ctx, VB->Free, t, currj, prevj);

                        //output new vertex
                        outlist[outcount++] = VB->Free;
                        VB->Free++;
                        if (VB->Free == VB_SIZE)
                        {
                            VB->Free = 1;
                        }
                        //output current vertex
                        outlist[outcount++] = currj;
                    }
                }
                else
                {
                    if (INSIDE(prevj, a, b, c, d))
                    {
                        //current outside and previous inside, clip
                        GLfloat dx, dy, dz, dw, t, denom;

                        //compute t;
                        dx = VB->Eye[currj][0] - VB->Eye[prevj][0];
                        dy = VB->Eye[currj][1] - VB->Eye[prevj][1];
                        dz = VB->Eye[currj][2] - VB->Eye[prevj][2];
                        dw = VB->Eye[currj][3] - VB->Eye[prevj][3];
                        denom = dx*a + dy*b + dz*c + dw*d;
                        if (denom == 0.0f)
                        {
                            t = 0.0f;
                        }
                        else
                        {
                            t = -(VB->Eye[prevj][0]*a + VB->Eye[prevj][1]*b
                                + VB->Eye[prevj][2]*c + VB->Eye[prevj][3]*d)
                                / denom;
                            if (t > 1.0f)
                            {
                                t = 1.0f;
                            }
                        }

                        //interpolate new vertex position
                        VB->Eye[VB->Free][0] = VB->Eye[prevj][0] + t*dx;
                        VB->Eye[VB->Free][1] = VB->Eye[prevj][1] + t*dy;
                        VB->Eye[VB->Free][2] = VB->Eye[prevj][2] + t*dz;
                        VB->Eye[VB->Free][3] = VB->Eye[prevj][3] + t*dw;

                        //interpolate color, texture
                        interpolate_aux(ctx, VB->Free, t, prevj, currj);

                        //output new vertex
                        outlist[outcount++] = VB->Free;
                        VB->Free++;
                        if (VB->Free == VB_SIZE)
                        {
                            VB->Free = 1;
                        }
                    }
                }

                previ = curri;
                prevj = currj;

                if (outcount >= VB_SIZE-1)
                {
                    //too many vertices
                    if (outlist != vlist2)
                    {
                        MEMCPY(vlist, vlist2, outcount*sizeof(GLuint));
                    }
                    return VB_SIZE-1;
                }
            }

            //swap inlist and outlist pointers
            {
                GLuint* tmp;
                tmp = inlist;
                inlist = outlist;
                outlist = tmp;
                incount = outcount;
            }
        }
    }

    if (outlist != vlist2)
    {
        MEMCPY(vlist, vlist2, outcount * sizeof(GLuint));
    }

    return outcount;
}
