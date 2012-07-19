/*=============================================================================
    Name    : kvb.c
    Purpose : rGL's vertex buffer routines, things like shading, hooks to driver
              for rendering different primitives, &c

    Created 12/??/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "kgl.h"
#include "kgl_macros.h"
#include "kvb.h"
#include "maths.h"
#include "clip.h"
#include "asm.h"

#define KATMAI_THRESH_3D  61
#define KATMAI_THRESH_PER 125

double chop_temp;

extern GLint scrMultByte[];
extern GLint scrMult[];

extern GLuint g_NumPolys;
extern GLuint g_CulledPolys;

#define PARANOID_W              0
#define DRIVER_TRIANGLE_FAN     0
#define DRIVER_TRIANGLE_STRIP   0
#define DRIVER_TRIANGLE_ARRAY   0
#define FANCY_RESET 0

#define EPSILON -0.8e-03f
#define C_MATH  0

#define CALL_TRIANGLE(CTX,VL,PV) \
    { \
        g_NumPolys++; \
        if (CTX->DriverFuncs.draw_triangle != NULL) \
        { \
            CTX->DriverFuncs.draw_triangle(VL, PV); \
        } \
    }

#define CALL_QUAD(CTX,VL,PV) \
    { \
        g_NumPolys += 2; \
        if (CTX->DriverFuncs.draw_quad != NULL) \
        { \
            CTX->DriverFuncs.draw_quad(VL, PV); \
        } \
    }

GLfloat gl_pow(GLfloat a, GLfloat b)
{
    return (GLfloat)pow((double)a, (double)b);
}

/*
 * spechack shader.
 * alpha component is scaled by n_dot_VP.
 * veye is -ve z transformed
 */
void gl_spechack_shade_vertices(
        GLcontext* ctx,
		GLuint side,
        GLuint n,
		GLfloat vertex[][4],
		GLfloat normal[][3],
		GLubyte color[][4]
      )
{
    GLint j, l;
    GLfloat ascale, alpha, n_dot_VP;
    GLfloat veye[3] = {0.0f, 0.0f, 1.0f};
    GLfloat adjust = 1.0f - ctx->LightingAdjust;

    ascale = ctx->Buffer.ascale;

    for (j = 0; j < (GLint)n; j++)
    {
        GLfloat nx, ny, nz;
        gl_light* light;
        if (side == 0)
        {
            nx = normal[j][0];
            ny = normal[j][1];
            nz = normal[j][2];
        }
        else
        {
            nx = -normal[j][0];
            ny = -normal[j][1];
            nz = -normal[j][2];
        }

        alpha = 0.0f;

        n_dot_VP = nx * veye[0] + ny * veye[1] + nz * veye[2];
        if (n_dot_VP > 0.0f)
        {
            alpha += gl_pow(CLAMP(n_dot_VP, 0.0f, 1.0f), ctx->SpecularExponent[0]);
        }

        color[j][3] = FAST_TO_INT((GLfloat)color[j][3] * CLAMP(alpha, 0.0f, 1.0f) * adjust);
    }
}

/*
 * spechack2 shader.
 * a confusing mess, this shader handles nebulae
 */
void gl_spechack2_shade_vertices(
        GLcontext* ctx,
		GLuint side,
        GLuint n,
		GLfloat vertex[][4],
		GLfloat normal[][3],
		GLubyte color[][4]
      )
{
    GLint j, l;
    GLfloat ascale, alpha, n_dot_VP;
    GLfloat veye[3] = {0.0f, 0.0f, 1.0f};
    gl_light* light;

    ascale = ctx->Buffer.ascale;

    for (j = 0; j < (GLint)n; j++)
    {
        GLfloat nx, ny, nz;
        gl_light* light;
        if (side == 0)
        {
            nx = normal[j][0];
            ny = normal[j][1];
            nz = normal[j][2];
        }
        else
        {
            nx = -normal[j][0];
            ny = -normal[j][1];
            nz = -normal[j][2];
        }

        alpha = 0.0f;
        n_dot_VP = nx * nx + nz * nz;

        for (light = ctx->ActiveLight; light != NULL; light = light->next)
        {
            if (color[j][3] == 0)
                continue;

            n_dot_VP = nx * light->VP_inf_norm[0]
                     + ny * light->VP_inf_norm[1]
                     + nz * light->VP_inf_norm[2];
            if (n_dot_VP > 0.0f)
            {
                alpha += gl_pow(n_dot_VP, ctx->SpecularExponent[1]);
            }
        }
        {
            GLint c;

            c = FAST_TO_INT((GLfloat)color[j][3] * alpha);
            if (c < 0) c = 0;
            else if (c > 255) c = 255;

            color[j][3] = c;
        }
    }
}

/*
 * spechack3 shader.
 * this is the perspective adjusted specular shader
 */
void gl_spechack3_shade_vertices(
        GLcontext* ctx,
		GLuint side,
        GLuint n,
		GLfloat vertex[][4],
		GLfloat normal[][3],
		GLubyte color[][4]
      )
{
    GLint j, l;
    GLfloat ascale, alpha, n_dot_VP;
    GLfloat veye[3];
    GLfloat adjust = 1.0f - ctx->LightingAdjust;

    ascale = ctx->Buffer.ascale;

    for (j = 0; j < (GLint)n; j++)
    {
        GLfloat nx, ny, nz;
        gl_light* light;
        if (side == 0)
        {
            nx = normal[j][0];
            ny = normal[j][1];
            nz = normal[j][2];
        }
        else
        {
            nx = -normal[j][0];
            ny = -normal[j][1];
            nz = -normal[j][2];
        }

        alpha = 0.0f;

        veye[0] = vertex[j][0];
        veye[1] = vertex[j][1];
        veye[2] = vertex[j][2];
        v3_normalize(veye);

        n_dot_VP = fabs(nx * veye[0] + ny * veye[1] + nz * veye[2]);
        if (n_dot_VP > 0.0f)
        {
            alpha += gl_pow(CLAMP(n_dot_VP, 0.0f, 1.0f), ctx->SpecularExponent[2]);
        }

        color[j][1] = FAST_TO_INT((GLfloat)color[j][1] * CLAMP(alpha, 0.0f, 0.92f));
        color[j][3] = FAST_TO_INT((GLfloat)color[j][3] * CLAMP(alpha, 0.0f, 1.0f) * adjust);
    }
}

void gl_illum_shade_vertices(
        GLcontext* ctx,
		GLuint side,
        GLuint n,
		GLfloat vertex[][4],
		GLfloat normal[][3],
		GLubyte color[][4]
      )
{
    GLint j;
    GLint sumA;
    GLfloat* baseColor;
    GLfloat  avgBaseColor;

    baseColor = ctx->BaseColor[side];
    avgBaseColor = baseColor[0] + baseColor[1] + baseColor[2];
    avgBaseColor /= 3.0f;

    sumA = (GLint)baseColor[3];

    for (j = 0; j < (GLint)n; j++)
    {
        GLfloat sumI;
        GLfloat nx, ny, nz;
        gl_light* light;

        if (side == 0)
        {
            nx = normal[j][0];
            ny = normal[j][1];
            nz = normal[j][2];
        }
        else
        {
            nx = -normal[j][0];
            ny = -normal[j][1];
            nz = -normal[j][2];
        }

        sumI = avgBaseColor;

        for (light = ctx->ActiveLight; light != NULL; light = light->next)
        {
            GLfloat n_dot_VP;

            n_dot_VP = nx * light->VP_inf_norm[0]
                     + ny * light->VP_inf_norm[1]
                     + nz * light->VP_inf_norm[2];

            if (n_dot_VP > 0.0f)
            {
                n_dot_VP *= 255.0f;
                sumI += n_dot_VP;
            }
        }

        color[j][0] = FAST_TO_INT(MIN2(sumI, 255.0f));
        color[j][1] = color[j][0];
        color[j][2] = color[j][0];
        color[j][3] = sumA;
    }
}

/*
 * optimized shader.  technically this shouldn't be used unless the context decides
 * the lighting parameters are within range, but Homeworld figures "what the hell"
 */
void gl_fast_color_shade_vertices(
        GLcontext* ctx,
		GLuint side,
        GLuint n,
		GLfloat vertex[][4],
		GLfloat normal[][3],
		GLubyte color[][4]
      )
{
    GLint j;
    GLint sumA;
    GLfloat baseColor[4];

    baseColor[0] = ctx->BaseColor[side][0];
    baseColor[1] = ctx->BaseColor[side][1];
    baseColor[2] = ctx->BaseColor[side][2];

    sumA = (GLint)ctx->BaseColor[side][3];

    for (j = 0; j < (GLint)n; j++)
    {
        GLfloat sumR, sumG, sumB;
        GLfloat nx, ny, nz;
        gl_light* light;

        if (side == 0)
        {
            nx = normal[j][0];
            ny = normal[j][1];
            nz = normal[j][2];
        }
        else
        {
            nx = -normal[j][0];
            ny = -normal[j][1];
            nz = -normal[j][2];
        }

        sumR = baseColor[0];
        sumG = baseColor[1];
        sumB = baseColor[2];

        for (light = ctx->ActiveLight; light != NULL; light = light->next)
        {
            GLfloat n_dot_VP;

            n_dot_VP = nx * light->VP_inf_norm[0]
                     + ny * light->VP_inf_norm[1]
                     + nz * light->VP_inf_norm[2];

            if (n_dot_VP > 0.0f)
            {
                GLfloat* lightMatDiffuse = light->MatDiffuse[side];
                n_dot_VP *= 255.0f;
                sumR += n_dot_VP * lightMatDiffuse[0];
                sumG += n_dot_VP * lightMatDiffuse[1];
                sumB += n_dot_VP * lightMatDiffuse[2];
            }
        }

        color[j][0] = FAST_TO_INT(MIN2(sumR, 255.0f));
        color[j][1] = FAST_TO_INT(MIN2(sumG, 255.0f));
        color[j][2] = FAST_TO_INT(MIN2(sumB, 255.0f));
        color[j][3] = sumA;
    }
}

/*
 * Input: side - 0 front, 1 back
 *        n - number of verts to process
 *        vertex - array of vertex positions in eye coordinates
 *        normal - array of surface normal vectors
 * Output: color - array of resulting colors
 */
void gl_color_shade_vertices(
        GLcontext* ctx,
		GLuint side,
        GLuint n,
		GLfloat vertex[][4],
		GLfloat normal[][3],
		GLubyte color[][4]
      )
{
    GLint j;
    GLfloat rscale, gscale, bscale, ascale;
    GLfloat baseR, baseG, baseB, baseA;
    GLint sumA;
    gl_light* light;
    gl_material* mat;

    rscale = ctx->Buffer.rscale;
    gscale = ctx->Buffer.gscale;
    bscale = ctx->Buffer.bscale;
    ascale = ctx->Buffer.ascale;

    mat = &ctx->Material[side];

    baseR = ctx->Ambient[0] * mat->Ambient[0];
    baseG = ctx->Ambient[1] * mat->Ambient[1];
    baseB = ctx->Ambient[2] * mat->Ambient[2];
    baseA = mat->Diffuse[3];

    sumA = (GLint)(CLAMP(baseA, 0.0f, 1.0f) * ascale);

    for (j = 0; j < (GLint)n; j++)
    {
	    GLfloat sumR, sumG, sumB;
	    GLfloat nx, ny, nz;

	    if (side == 0)
        {
	        nx = normal[j][0];
	        ny = normal[j][1];
	        nz = normal[j][2];
	    }
        else
        {
    	    nx = -normal[j][0];
	        ny = -normal[j][1];
	        nz = -normal[j][2];
	    }

	    sumR = baseR;
	    sumG = baseG;
	    sumB = baseB;

	    for (light = ctx->ActiveLight; light != NULL; light = light->next)
        {
	        GLfloat ambientR, ambientG, ambientB;
	        GLfloat VPx, VPy, VPz;
	        GLfloat n_dot_VP, n_dot_h;
            GLfloat attenuation;

	        if (light->Position[3] == 0.0f)
            {
		        VPx = light->VP_inf_norm[0];
    		    VPy = light->VP_inf_norm[1];
	    	    VPz = light->VP_inf_norm[2];
                attenuation = 1.0f;
	        }
            else
            {
                GLfloat d;
                VPx = light->Position[0] - vertex[j][0];
                VPy = light->Position[1] - vertex[j][1];
                VPz = light->Position[2] - vertex[j][2];
                d = (GLfloat)fsqrt(VPx*VPx + VPy*VPy + VPz*VPz);
                if (d > 0.0001f)
                {
                    GLfloat invd = 1.0f / d;
                    VPx *= invd;
                    VPy *= invd;
                    VPz *= invd;
                }
                attenuation = 1.0f /
                    (light->ConstantAttenuation
                     + d * (light->LinearAttenuation
                     + d * light->QuadraticAttenuation));
	        }

	        ambientR = mat->Ambient[0] * light->Ambient[0];
	        ambientG = mat->Ambient[1] * light->Ambient[1];
	        ambientB = mat->Ambient[2] * light->Ambient[2];

	        n_dot_VP = nx * VPx + ny * VPy + nz * VPz;

	        if (n_dot_VP <= 0.0f)
            {
		        sumR += attenuation * ambientR;
		        sumG += attenuation * ambientG;
		        sumB += attenuation * ambientB;
	        }
            else
            {
		        GLfloat diffuseR, diffuseG, diffuseB;
		        GLfloat specularR, specularG, specularB;
		        GLfloat h_x, h_y, h_z;

		        diffuseR = n_dot_VP * mat->Diffuse[0] * light->Diffuse[0];
		        diffuseG = n_dot_VP * mat->Diffuse[1] * light->Diffuse[1];
		        diffuseB = n_dot_VP * mat->Diffuse[2] * light->Diffuse[2];

		        /* !LocalViewer */
		        h_x = VPx;
		        h_y = VPy;
		        h_z = VPz + 1.0f;

		        n_dot_h = nx*h_x + ny*h_y + nz*h_z;

#if 0
		        if (n_dot_h <= 0.0f)       //HACK to avoid specular
#endif
                {
		            specularR = 0.0f;
		            specularG = 0.0f;
		            specularB = 0.0f;
		        }
#if 0
                else
                {
		            GLfloat spec_coef, n_dot_h;
		            n_dot_h = n_dot_h /
                              (GLfloat)fsqrt(h_x*h_x + h_y*h_y + h_z*h_z);
		            /* FIXME: fast pow table */
		            spec_coef = (GLfloat)pow((GLdouble)n_dot_h,
                                             mat->Shininess);

		            if (spec_coef < 1.0e-10)
                    {
			            specularR = 0.0f;
			            specularG = 0.0f;
			            specularB = 0.0f;
		            }
                    else
                    {
			            specularR = spec_coef * mat->Specular[0] * light->Specular[0];
			            specularG = spec_coef * mat->Specular[1] * light->Specular[1];
			            specularB = spec_coef * mat->Specular[2] * light->Specular[2];
		            }
		        }
#endif
		        sumR += attenuation * (ambientR + diffuseR + specularR);
		        sumG += attenuation * (ambientG + diffuseG + specularG);
		        sumB += attenuation * (ambientB + diffuseB + specularB);
	        }
	    }

#if 0
	    color[j][0] = (GLint)(CLAMP(sumR, 0.0f, 1.0f) * rscale);
	    color[j][1] = (GLint)(CLAMP(sumG, 0.0f, 1.0f) * gscale);
	    color[j][2] = (GLint)(CLAMP(sumB, 0.0f, 1.0f) * bscale);
#else
        color[j][0] = FAST_TO_INT(CLAMP(sumR, 0.0f, 1.0f) * rscale);
        color[j][1] = FAST_TO_INT(CLAMP(sumG, 0.0f, 1.0f) * gscale);
        color[j][2] = FAST_TO_INT(CLAMP(sumB, 0.0f, 1.0f) * bscale);
#endif
	    color[j][3] = sumA;
    }
}

/*
 * Input: ctx - the context
 *        n - number of vertices to transform
 *        vObj - array [n][4] of object coordinates
 * In/Out vEye - array [n][4] of eye coordinates
 */
void transform_points3(GLcontext* ctx, GLuint n,
	                   GLfloat vObj[][4], GLfloat vEye[][4])
{
    switch (ctx->ModelViewMatrixType)
    {
    case MATRIX_GENERAL:
    {
        /* general matrix */
        const GLfloat* m = ctx->ModelViewMatrix;
        GLuint i;
        for (i = 0; i < n; i++)
        {
	        GLfloat ox = vObj[i][0], oy = vObj[i][1], oz = vObj[i][2];
	        vEye[i][0] = m[0] * ox + m[4] * oy + m[8]  * oz + m[12];
	        vEye[i][1] = m[1] * ox + m[5] * oy + m[9]  * oz + m[13];
	        vEye[i][2] = m[2] * ox + m[6] * oy + m[10] * oz + m[14];
	        vEye[i][3] = m[3] * ox + m[7] * oy + m[11] * oz + m[15];
        }
        break;
    }
    case MATRIX_IDENTITY:
    {
        /* identity matrix */
#if C_MATH
        GLuint i;
        for (i = 0; i < n; i++)
        {
            vEye[i][0] = vObj[i][0];
            vEye[i][1] = vObj[i][1];
            vEye[i][2] = vObj[i][2];
            vEye[i][3] = 1.0f;
        }
#else
        MEMCPY(vEye, vObj, n*4*sizeof(GLfloat));
#endif
        break;
    }
    case MATRIX_2D:
    {
        /* 2d matrix */
        GLfloat const* m = ctx->ModelViewMatrix;
        GLuint i;
        for (i = 0; i < n; i++)
        {
            GLfloat ox = vObj[i][0], oy = vObj[i][1], oz = vObj[i][2];
            vEye[i][0] = m[0] * ox + m[4] * oy + m[12];
            vEye[i][1] = m[1] * ox + m[5] * oy + m[13];
            vEye[i][2] = oz;
            vEye[i][3] = 1.0f;
        }
        break;
    }
    case MATRIX_2D_NO_ROT:
    {
        /* 2d, no rotation */
        GLfloat const* m = ctx->ModelViewMatrix;
        GLuint i;
        for (i = 0; i < n; i++)
        {
            GLfloat ox = vObj[i][0], oy = vObj[i][1], oz = vObj[i][2];
            vEye[i][0] = m[0] * ox + m[12];
            vEye[i][1] = m[5] * oy + m[13];
            vEye[i][2] = oz;
            vEye[i][3] = 1.0f;
        }
        break;
    }
    case MATRIX_3D:
    {
        /* 3d matrix */
#if C_MATH
        GLfloat const* m = ctx->ModelViewMatrix;
        GLuint i;
        for (i = 0; i < n; i++)
        {
    	    GLfloat ox = vObj[i][0], oy = vObj[i][1], oz = vObj[i][2];
	        vEye[i][0] = m[0] * ox + m[4] * oy +  m[8] * oz + m[12];
	        vEye[i][1] = m[1] * ox + m[5] * oy +  m[9] * oz + m[13];
	        vEye[i][2] = m[2] * ox + m[6] * oy + m[10] * oz + m[14];
	        vEye[i][3] = 1.0f;
        }
#else
        if (ctx->CpuKatmai && (n > KATMAI_THRESH_3D))
        {
            gl_intrin_3dtransform(&vEye[0][0], &vObj[0][0], ctx->ModelViewMatrix, n);
        }
        else
        {
            gl_megafast_affine_transform(
                &vEye[0][0], &vObj[0][0], ctx->ModelViewMatrix, n);
        }
#endif
        break;
    }
    default:
        /* surprised if this happens (MATRIX_GENERAL) */
        gl_problem(ctx, "invalid matrix type in transform_points3()");
        return;
    }
}

void cliptest(
    GLcontext* ctx,
    GLuint n, GLfloat vClip[][4],
    GLubyte clipMask[],
    GLubyte* orMask, GLubyte* andMask)
{
    asm_cliptest(n, (GLfloat*)vClip, clipMask, orMask, andMask);
}

/*
 * Input: ctx - the context
 *        n - number of vertices
 *        vEye - array [n][4] of Eye coordinates
 * Output: vClip - array [n][4] of Clip coordinates
 *        clipMask - array [n] of clip masks
 */
void project_and_cliptest(
            GLcontext* ctx,
			GLuint n, GLfloat vEye[][4],
			GLfloat vClip[][4], GLubyte clipMask[],
			GLubyte* orMask, GLubyte* andMask)
{
    GLubyte tmpOrMask = *orMask;
    GLubyte tmpAndMask = *andMask;

    switch (ctx->ProjectionMatrixType)
    {
    case MATRIX_GENERAL:
    {
#if C_MATH
        /* general matrix */
        GLfloat const* m = ctx->ProjectionMatrix;
        GLuint i;
        for (i = 0; i < n; i++)
        {
	        GLfloat ex = vEye[i][0], ey = vEye[i][1];
	        GLfloat ez = vEye[i][2], ew = vEye[i][3];
	        GLfloat cx = m[0] * ex + m[4] * ey + m[8]  * ez + m[12] * ew;
	        GLfloat cy = m[1] * ex + m[5] * ey + m[9]  * ez + m[13] * ew;
	        GLfloat cz = m[2] * ex + m[6] * ey + m[10] * ez + m[14] * ew;
	        GLfloat cw = m[3] * ex + m[7] * ey + m[11] * ez + m[15] * ew;
	        GLubyte mask = 0;
	        vClip[i][0] = cx;
	        vClip[i][1] = cy;
	        vClip[i][2] = cz;
	        vClip[i][3] = cw;
	        if (cx >  cw)       mask |= CLIP_RIGHT_BIT;
	        else if (cx < -cw)  mask |= CLIP_LEFT_BIT;
	        if (cy >  cw)       mask |= CLIP_TOP_BIT;
	        else if (cy < -cw)  mask |= CLIP_BOTTOM_BIT;
	        if (cz >  cw)       mask |= CLIP_FAR_BIT;
	        else if (cz < -cw)  mask |= CLIP_NEAR_BIT;
	        if (mask)
            {
	            clipMask[i] |= mask;
	            tmpOrMask |= mask;
	        }
	        tmpAndMask &= mask;
        }
        break;
#else
        asm_project_and_cliptest_general(
            n,
            (GLfloat*)vClip, ctx->ProjectionMatrix, (GLfloat*)vEye,
            clipMask, orMask, andMask);
        return;
#endif
    }
    case MATRIX_IDENTITY:
    {
#if C_MATH
        GLuint i;
        for (i = 0; i < n; i++)
        {
            GLfloat cx = vClip[i][0] = vEye[i][0];
            GLfloat cy = vClip[i][1] = vEye[i][1];
            GLfloat cz = vClip[i][2] = vEye[i][2];
            GLfloat cw = vClip[i][3] = vEye[i][3];
            GLubyte mask = 0;
            if (cx >  cw)       mask |= CLIP_RIGHT_BIT;
            else if (cx < -cw)  mask |= CLIP_LEFT_BIT;
            if (cy >  cw)       mask |= CLIP_TOP_BIT;
            else if (cy < -cw)  mask |= CLIP_BOTTOM_BIT;
            if (cz >  cw)       mask |= CLIP_FAR_BIT;
            else if (cz < -cw)  mask |= CLIP_NEAR_BIT;
            if (mask)
            {
                clipMask[i] |= mask;
                tmpOrMask |= mask;
            }
            tmpAndMask &= mask;
        }
        break;
#else
        asm_project_and_cliptest_identity(
            n,
            (GLfloat*)vClip, ctx->ProjectionMatrix, (GLfloat*)vEye,
            clipMask, orMask, andMask);
        return;
#endif
    }
    case MATRIX_ORTHO:
    {
        GLfloat const* m = ctx->ProjectionMatrix;
        GLuint i;
        for (i = 0; i < n; i++)
        {
            GLfloat ex = vEye[i][0], ey = vEye[i][1];
            GLfloat ez = vEye[i][2], ew = vEye[i][3];
            GLfloat cx = m[0]  * ex + m[12] * ew;
            GLfloat cy = m[5]  * ey + m[13] * ew;
            GLfloat cz = m[10] * ez + m[14] * ew;
            GLfloat cw = ew;
            GLubyte mask = 0;
            vClip[i][0] = cx;
            vClip[i][1] = cy;
            vClip[i][2] = cz;
            vClip[i][3] = cw;
            if (cx >  cw)       mask |= CLIP_RIGHT_BIT;
            else if (cx < -cw)  mask |= CLIP_LEFT_BIT;
            if (cy >  cw)       mask |= CLIP_TOP_BIT;
            else if (cy < -cw)  mask |= CLIP_BOTTOM_BIT;
            if (cz >  cw)       mask |= CLIP_FAR_BIT;
            else if (cz < -cw)  mask |= CLIP_NEAR_BIT;
            if (mask)
            {
                clipMask[i] |= mask;
                tmpOrMask |= mask;
            }
            tmpAndMask &= mask;
        }
        break;
    }
    case MATRIX_PERSPECTIVE:
    {
#if C_MATH
        GLfloat const* m = ctx->ProjectionMatrix;
        GLuint i;
        for (i = 0; i < n; i++)
        {
            GLfloat ex = vEye[i][0], ey = vEye[i][1];
            GLfloat ez = vEye[i][2], ew = vEye[i][3];
            GLfloat cx = m[0] * ex + m[8] * ez;
            GLfloat cy = m[5] * ey + m[9] * ez;
            GLfloat cz = m[10] * ez + m[14] * ew;
            GLfloat cw = -ez;
            GLubyte mask = 0;
            vClip[i][0] = cx;
            vClip[i][1] = cy;
            vClip[i][2] = cz;
            vClip[i][3] = cw;
            if (cx >  cw)       mask |= CLIP_RIGHT_BIT;
            else if (cx < -cw)  mask |= CLIP_LEFT_BIT;
            if (cy >  cw)       mask |= CLIP_TOP_BIT;
            else if (cy < -cw)  mask |= CLIP_BOTTOM_BIT;
            if (cz >  cw)       mask |= CLIP_FAR_BIT;
            else if (cz < -cw)  mask |= CLIP_NEAR_BIT;
            if (mask)
            {
                clipMask[i] |= mask;
                tmpOrMask |= mask;
            }
            tmpAndMask &= mask;
        }
        break;
#else
/*        if (ctx->CpuKatmai && (n > KATMAI_THRESH_PER))
        {
            intrin_project_and_cliptest_perspective(
                n,
                (GLfloat*)vClip, ctx->ProjectionMatrix, (GLfloat*)vEye,
                clipMask, orMask, andMask);
        }
        else*/
        {
            asm_project_and_cliptest_perspective(
                n,
                (GLfloat*)vClip, ctx->ProjectionMatrix, (GLfloat*)vEye,
                clipMask, orMask, andMask);
        }
        return;
#endif
    }
    default:
        gl_problem(ctx, "invalid matrix type in project_and_cliptest");
        return;
    }

    *orMask = tmpOrMask;
    *andMask = tmpAndMask;
}

/*
 * Input: ctx - the context
 *        n - number of vertices to transform
 *        vClip - array [n] of input vertices
 *        clipMask - array [n] of vertex clip masks.
 *                   NULL == no clipped verts
 */
void viewport_map_vertices(
            GLcontext* ctx,
			GLuint n, GLfloat vClip[][4],
			GLubyte const clipMask[], GLfloat vWin[][3])
{
    GLuint i;
    GLfloat sz, tz;
    GLfloat sx = ctx->Viewport.Sx;
    GLfloat tx = ctx->Viewport.Tx;
    GLfloat sy = ctx->Viewport.Sy;
    GLfloat ty = ctx->Viewport.Ty;

    if (ctx->ScaleDepthValues)
    {
        sz = ctx->Viewport.Sz;
        tz = ctx->Viewport.Tz;
    }
    else
    {
        sz = 0.5f;
        tz = 0.0f;
    }

    if ((!ctx->RasterizeOnly) &&
        (ctx->ModelViewMatrixType != MATRIX_GENERAL) &&
        (ctx->ProjectionMatrixType == MATRIX_ORTHO ||
         ctx->ProjectionMatrixType == MATRIX_IDENTITY))
    {
        // don't need to divide by w
        if (clipMask)
        {
            /* one or more vertices are clipped */
            for (i = 0; i < n; i++)
            {
                if (clipMask[i] == 0)
                {
                    vWin[i][0] = vClip[i][0] * sx + tx;
                    vWin[i][1] = vClip[i][1] * sy + ty;
                    vWin[i][2] = vClip[i][2] * sz + tz;
                }
            }
        }
        else
        {
            /* no vertices are clipped */
            for (i = 0; i < n; i++)
            {
                vWin[i][0] = vClip[i][0] * sx + tx;
                vWin[i][1] = vClip[i][1] * sy + ty;
                vWin[i][2] = vClip[i][2] * sz + tz;
            }
        }
    }
    else
    {
        /* need to divide by w */
        if (clipMask)
        {
        	/* one or more vertices are clipped */
    	    for (i = 0; i < n; i++)
            {
                if (clipMask[i] == 0)
                {
#if PARANOID_W
        		    if (vClip[i][3] != 0.0f)
#endif
                    {
    		            GLfloat wInv = 1.0f / vClip[i][3];
		                vWin[i][0] = vClip[i][0] * wInv * sx + tx;
		                vWin[i][1] = vClip[i][1] * wInv * sy + ty;
		                vWin[i][2] = vClip[i][2] * wInv * sz + tz;
		            }
#if PARANOID_W
                    else
                    {
        		        vWin[i][0] = 0.0f;
		                vWin[i][1] = 0.0f;
		                vWin[i][2] = 0.0f;
		            }
#endif
                }
	        }
        }
        else
        {
	        /* no vertices are clipped */
	        for (i = 0; i < n; i++)
            {
#if PARANOID_W
                if (vClip[i][3] != 0.0f)
#endif
                {
    		        GLfloat wInv = 1.0f / vClip[i][3];
		            vWin[i][0] = vClip[i][0] * wInv * sx + tx;
		            vWin[i][1] = vClip[i][1] * wInv * sy + ty;
		            vWin[i][2] = vClip[i][2] * wInv * sz + tz;
                }
#if PARANOID_W
                else
                {
    		        vWin[i][0] = 0.0f;
		            vWin[i][1] = 0.0f;
		            vWin[i][2] = 0.0f;
                }
#endif
	        }
        }
    }
}

/*
 * LightingAdjust fade alpha
 */
static void gl_adjust_color(GLcontext* ctx, GLuint n, GLubyte color[][4])
{
    vertex_buffer* VB = ctx->VB;
    GLuint i;
    GLfloat adjust;
    GLint badjust;

    adjust = 1.0f - ctx->LightingAdjust;
    badjust = (GLubyte)(adjust * 255.0f);

    for (i = 0; i < n; i++)
    {
        color[i][3] = (GLubyte)(((GLint)color[i][3] * badjust) >> 8);
    }
}

/*
 * LightingAdjust fade colour to black
 */
static void gl_adjust_color_to_black(GLcontext* ctx, GLuint n, GLubyte color[][4])
{
    vertex_buffer* VB = ctx->VB;
    GLuint i;
    GLfloat adjust;
    GLint badjust;

    GLubyte r, g, b;

    r = ctx->ClearColorByte[0];
    g = ctx->ClearColorByte[1];
    b = ctx->ClearColorByte[2];

    adjust = 1.0f - ctx->LightingAdjust;
    badjust = (GLubyte)(adjust * 255.0f);

    for (i = 0; i < n; i++)
    {
        color[i][0] = CLAMP((GLubyte)(((GLint)color[i][0] * badjust) >> 8), r, 255);
        color[i][1] = CLAMP((GLubyte)(((GLint)color[i][1] * badjust) >> 8), g, 255);
        color[i][2] = CLAMP((GLubyte)(((GLint)color[i][2] * badjust) >> 8), b, 255);
    }
}

/*
 * call appropriate subsidiary shader
 */
void shade_vertices(GLcontext* ctx)
{
    vertex_buffer* VB = ctx->VB;

    if (ctx->NewMask & NEW_LIGHTING)
    {
        gl_update_lighting(ctx);
    }

    if (ctx->SpecularRender)
    {
        switch ((GLint)ctx->SpecularRender)
        {
        case 1:
            gl_spechack_shade_vertices(ctx, 0, VB->Count - VB->Start,
                                       VB->Eye + VB->Start, VB->Normal + VB->Start,
                                       VB->Color + VB->Start);
            break;
        case 2:
            gl_spechack2_shade_vertices(ctx, 0, VB->Count - VB->Start,
                                        VB->Eye + VB->Start, VB->Normal + VB->Start,
                                        VB->Color + VB->Start);
            break;
        case 3:
            gl_spechack3_shade_vertices(ctx, 0, VB->Count - VB->Start,
                                        VB->Eye + VB->Start, VB->Normal + VB->Start,
                                        VB->Color + VB->Start);
            break;
        }
    }
    else if (ctx->UsingLitPalette)
    {
        gl_illum_shade_vertices(ctx, 0, VB->Count - VB->Start,
                                VB->Eye + VB->Start,
                                VB->Normal + VB->Start,
                                VB->Fcolor + VB->Start);
        if (ctx->TwoSide)
        {
            gl_illum_shade_vertices(ctx, 1, VB->Count - VB->Start,
                                    VB->Eye + VB->Start,
                                    VB->Normal + VB->Start,
                                    VB->Bcolor + VB->Start);
        }
    }
    else if (ctx->FastLighting)
    {
        gl_fast_color_shade_vertices(ctx, 0,
                                     VB->Count - VB->Start,
                                     VB->Eye + VB->Start,
                                     VB->Normal + VB->Start,
                                     VB->Fcolor + VB->Start);
        if (ctx->TwoSide)
        {
            gl_fast_color_shade_vertices(ctx, 1,
                                         VB->Count - VB->Start,
                                         VB->Eye + VB->Start,
                                         VB->Normal + VB->Start,
                                         VB->Bcolor + VB->Start);
        }
    }
    else
    {
        gl_color_shade_vertices(ctx, 0,
                                VB->Count - VB->Start,
                                VB->Eye + VB->Start,
                                VB->Normal + VB->Start,
                                VB->Fcolor + VB->Start);
        if (ctx->TwoSide)
        {
            gl_color_shade_vertices(ctx, 0,
                                    VB->Count - VB->Start,
                                    VB->Eye + VB->Start,
                                    VB->Normal + VB->Start,
                                    VB->Bcolor + VB->Start);
        }
    }
}

/*
 * call subsidiary xform funcs or do it ourselves if we have to normalize
 */
void gl_xform_normals_3fv(GLuint n, GLfloat v[][3], GLfloat const m[16],
			              GLfloat u[][3], GLboolean normalize, GLboolean rescale)
{
    GLuint i;
    GLfloat m0 = m[0], m4 = m[4], m8 = m[8];
    GLfloat m1 = m[1], m5 = m[5], m9 = m[9];
    GLfloat m2 = m[2], m6 = m[6], m10 = m[10];

    if (normalize)
    {
        for (i = 0; i < n; i++)
        {
            GLdouble tx, ty, tz;
            {
                GLfloat ux = u[i][0], uy = u[i][1], uz = u[i][2];
                tx = ux*m0 + uy*m1 + uz*m2;
                ty = ux*m4 + uy*m5 + uz*m6;
                tz = ux*m8 + uy*m9 + uz*m10;
            }
            {
                GLdouble len, scale;
                len = fsqrt(tx*tx + ty*ty + tz*tz);
                scale = (len > 1E-30) ? (1.0 / len) : 1.0;
                v[i][0] = (GLfloat)(tx * scale);
                v[i][1] = (GLfloat)(ty * scale);
                v[i][2] = (GLfloat)(tz * scale);
            }
        }
    }
    else if (rescale)
    {
        GLfloat ux, uy, uz;
        GLfloat tx, ty, tz;

        GLfloat mscale = fsqrt(m2*m2 + m6*m6 + m10*m10);
        mscale = (mscale > 1E-30f) ? (1.0f / mscale) : 1.0f;
#if C_MATH
        for (i = 0; i < n; i++)
        {
            ux = u[i][0];
            uy = u[i][1];
            uz = u[i][2];
            tx = ux*m0 + uy*m1 + uz*m2;
            ty = ux*m4 + uy*m5 + uz*m6;
            tz = ux*m8 + uy*m9 + uz*m10;

            v[i][0] = tx * mscale;
            v[i][1] = ty * mscale;
            v[i][2] = tz * mscale;
        }
#else
        if (mscale == 1.0f)
        {
            gl_wicked_fast_normal_xform(&v[0][0], &u[0][0], (GLfloat*)m, n);
        }
        else
        {
            gl_fairly_fast_scaled_normal_xform(&v[0][0], &u[0][0], (GLfloat*)m, n, mscale);
        }
#endif
    }
    else if (gl_get_context_ext()->ModelViewMatrixType == MATRIX_IDENTITY)
    {
        //normals are assumed to be already transformed
        return;
    }
    else
    {
#if C_MATH
        for (i = 0; i < n; i++)
        {
            GLfloat ux = u[i][0], uy = u[i][1], uz = u[i][2];
            v[i][0] = ux*m0 + uy*m1 + uz*m2;
            v[i][1] = ux*m4 + uy*m5 + uz*m6;
            v[i][2] = ux*m8 + uy*m9 + uz*m10;
        }
#else
        gl_wicked_fast_normal_xform(
            &v[0][0], &u[0][0], (GLfloat*)m, n);
#endif
    }
}

/*
 * clip vertices against user clipping planes (eyespace)
 */
static GLuint gl_userclip_vertices(
    GLcontext* ctx, GLuint n, GLfloat vEye[][4], GLubyte clipMask[])
{
    GLboolean anyClipped = GL_FALSE;
    GLuint p;

    for (p = 0; p < MAX_CLIP_PLANES; p++)
    {
        if (ctx->ClipEnabled[p])
        {
            GLfloat a = ctx->ClipEquation[p][0];
            GLfloat b = ctx->ClipEquation[p][1];
            GLfloat c = ctx->ClipEquation[p][2];
            GLfloat d = ctx->ClipEquation[p][3];
            GLboolean allClipped = GL_TRUE;
            GLuint i;

            for (i = 0; i < n; i++)
            {
                GLfloat dot = vEye[i][0]*a + vEye[i][1]*b
                            + vEye[i][2]*c + vEye[i][3]*d;
                if (dot < EPSILON)
                {
                    clipMask[i] = CLIP_USER_BIT;
                    anyClipped = GL_TRUE;
                }
                else
                {
                    allClipped = GL_FALSE;
                }
            }
            if (allClipped)
            {
                return CLIP_ALL;
            }
        }
    }

    return anyClipped ? CLIP_SOME : CLIP_NONE;
}

void gl_reset_vb(GLcontext* ctx, GLboolean allDone);
void gl_render_vb(GLcontext* ctx, GLboolean allDone);
void gl_transform_vb_part2(GLcontext*, GLboolean);

/*
 * the transformation stage is divided into 2 funcs so that vertex buffers
 * can easily be pre-transformed
 */

void gl_transform_vb_part1(GLcontext* ctx, GLboolean allDone)
{
    vertex_buffer* VB = ctx->VB;

    if (VB->Count == 0)
    {
        //empty vertex list
	    return;
    }

    //update matrices
    if (ctx->NewMask & NEW_MODELVIEW)
    {
        gl_update_modelview();
    }
    if (ctx->NewMask & NEW_PROJECTION)
    {
        gl_update_projection();
    }

    //transform object -> eye
    if (!ctx->RasterizeOnly)
    {
        transform_points3(ctx, VB->Count - VB->Start,
    		              VB->Obj + VB->Start, VB->Eye + VB->Start);
    }

    //transform normals
    if (ctx->Lighting)
    {
        if (ctx->NewMask & NEW_MODELVIEWINV)
        {
            //invert modelview
            gl_invert_modelview();
        }
        gl_xform_normals_3fv(VB->Count - VB->Start,
                             VB->Normal + VB->Start, ctx->ModelViewInv,
                             VB->Normal + VB->Start,
                             ctx->Normalize, ctx->RescaleNormal);
    }

    //complete the process
    gl_transform_vb_part2(ctx, allDone);
}

void gl_transform_vb_part2(GLcontext* ctx, GLboolean allDone)
{
    GLboolean blendoff = GL_FALSE;
    vertex_buffer* VB = ctx->VB;

#if 0
    if (VB->Count == 0)
    {
	    return;
    }
#endif

    //eyespace clipping
    if (ctx->UserClip)
    {
        GLuint result = gl_userclip_vertices(
            ctx, VB->Count - VB->Start, VB->Eye + VB->Start,
            VB->ClipMask + VB->Start);
        if (result == CLIP_ALL)
        {
            VB->ClipOrMask = CLIP_ALL_BITS;
            gl_reset_vb(ctx, allDone);
            return;
        }
        else if (result == CLIP_SOME)
        {
            VB->ClipOrMask = CLIP_USER_BIT;
        }
        else
        {
            VB->ClipAndMask = 0;
        }
    }

    //project eye -> clip
    if (ctx->RasterizeOnly)
    {
        cliptest(ctx, VB->Count - VB->Start, VB->Clip + VB->Start,
                 VB->ClipMask + VB->Start, &VB->ClipOrMask, &VB->ClipAndMask);
    }
    else
    {
        project_and_cliptest(ctx, VB->Count - VB->Start, VB->Eye + VB->Start,
    			 VB->Clip + VB->Start, VB->ClipMask + VB->Start,
    			 &VB->ClipOrMask, &VB->ClipAndMask);
    }

    if (VB->ClipAndMask)
    {
        //every vertex is clipped
        VB->ClipOrMask = CLIP_ALL_BITS;
	    gl_reset_vb(ctx, allDone);
    	return;
    }

    //light vertices
    if (ctx->Lighting)
    {
	    shade_vertices(ctx);
    }

    //fog vertices
    if (ctx->Fog)
    {
        if (ctx->DriverFuncs.fog_vertices != NULL)
        {
            ctx->DriverFuncs.fog_vertices(ctx);
        }
    }

    //apply LightingAdjust fading
    if (ctx->LightingAdjust != 0.0f)
    {
        if (devices[activeDevice].fastBlend)
        {
            gl_adjust_color(ctx, VB->Count - VB->Start, VB->Fcolor + VB->Start);
            if (ctx->TwoSide)
            {
                gl_adjust_color(ctx, VB->Count - VB->Start, VB->Bcolor + VB->Start);
            }
            blendoff = !ctx->Blend;
        }
        else
        {
            gl_adjust_color_to_black(ctx, VB->Count - VB->Start, VB->Fcolor + VB->Start);
            if (ctx->TwoSide)
            {
                gl_adjust_color_to_black(ctx, VB->Count - VB->Start, VB->Bcolor + VB->Start);
            }
        }
    }

    //transform/project clip -> window
    viewport_map_vertices(
            ctx, VB->Count - VB->Start, VB->Clip + VB->Start,
            VB->ClipOrMask ? VB->ClipMask + VB->Start : NULL,
            VB->Win + VB->Start);

    if (blendoff)
    {
        ctx->Blend = GL_TRUE;
        gl_update_raster(ctx);
    }

    //render
    gl_render_vb(ctx, allDone);

    if (blendoff)
    {
        ctx->Blend = GL_FALSE;
        gl_update_raster(ctx);
    }
}

void general_points(GLcontext* ctx, GLuint first, GLuint last)
{
    vertex_buffer* VB = ctx->VB;
    GLuint i;
    GLint isize;

    if (ctx->DriverFuncs.set_monocolor == NULL ||
        ctx->DriverFuncs.draw_pixel == NULL)
    {
        return;
    }

    isize = (GLint)(CLAMP(ctx->PointSize, 0.1f, 40.0f) + 0.5f);
    if (isize < 1)
        isize = 1;

    for (i = first; i <= last; i++)
    {
        if (VB->ClipMask[i] == 0 || ctx->PointHack)
        {
            GLint x, y;
            GLdepth z;
            GLint x0, x1, y0, y1;
            GLint ix, iy;

            x = FAST_TO_INT(VB->Win[i][0]);
            y = FAST_TO_INT(VB->Win[i][1]);
            z = FAST_TO_INT(VB->Win[i][2]);

            if (isize & 1)
            {
                x0 = x - isize/2;
                x1 = x + isize/2;
                y0 = y - isize/2;
                y1 = y + isize/2;
            }
            else
            {
                x0 = FAST_TO_INT(x + 0.5f) - isize/2;
                x1 = x0 + isize-1;
                y0 = FAST_TO_INT(y + 0.5f) - isize/2;
                y1 = y0 + isize-1;
            }

            for (iy = y0; iy <= y1; iy++)
            {
                for (ix = x0; ix <= x1; ix++)
                {
                    ctx->DriverFuncs.set_monocolor(
                        ctx, VB->Color[i][0], VB->Color[i][1],
                             VB->Color[i][2], VB->Color[i][3]);
                    ctx->DriverFuncs.draw_pixel(ix, iy, z);
                }
            }
        }
    }
}

void render_points(GLcontext* ctx, GLuint first, GLuint last)
{
    vertex_buffer* VB = ctx->VB;
    GLuint i;

    if (last == first)
    {
        //FIXME: why is this hack necessary?
        GLfloat p[4];
        for (i = 0; i < 4; i++)
        {
            p[i] = VB->Clip[first][i];
        }
        if (gl_viewclip_point(p) == 0)
        {
            return;
        }
        else
        {
            VB->ClipMask[first] = 0;
        }
    }

    if (ctx->DriverFuncs.draw_point != NULL)
    {
        ctx->DriverFuncs.draw_point(first, last);
        return;
    }

    if (ctx->PointSize != 1.0f)
    {
        general_points(ctx, first, last);
        return;
    }

    for (i = first; i <= last; i++)
    {
        if (VB->ClipMask[i] == 0 || ctx->PointHack)
        {
            GLint x, y;
            GLdepth z;
            GLint red, green, blue;

            x = FAST_TO_INT(VB->Win[i][0]);
            y = FAST_TO_INT(VB->Win[i][1]);
            z = FAST_TO_INT(VB->Win[i][2]);

            red   = VB->Color[i][0];
            green = VB->Color[i][1];
            blue  = VB->Color[i][2];

            if (ctx->DriverFuncs.set_monocolor != NULL)
                ctx->DriverFuncs.set_monocolor(ctx, red, green, blue, VB->Color[i][3]);
            if (ctx->DriverFuncs.draw_pixel != NULL)
                ctx->DriverFuncs.draw_pixel(x, y, z);
        }
    }
}

void render_line(GLcontext* ctx,
                 GLuint vert0, GLuint vert1,
                 GLuint pvert)
{
    ctx->LineCount++;

#if LINES_DISABLEABLE
    if (ctx->LinesEnabled && ctx->DriverFuncs.draw_line != NULL)
#else
    if (ctx->DriverFuncs.draw_line != NULL)
#endif
    {
        ctx->DriverFuncs.draw_line(vert0, vert1, pvert);
    }
}

GLfloat polygon_area(vertex_buffer const* VB,
                     GLuint n, GLuint const vlist[])
{
    GLfloat area = 0.0f;
    GLuint i, j0, j1;
    GLfloat const (*win)[3] = VB->Win;

    for (i = 0; i < n; i++)
    {
        j0 = vlist[i];
        j1 = vlist[(i+1)%n];
        area += (win[j0][0] - win[j1][0]) * (win[j0][1] + win[j1][1]);
    }
    return area/* * 0.5f*/;
}

void unfilled_polygon(GLcontext* ctx,
                      GLuint n, GLuint vlist[],
                      GLuint pv, GLuint facing)
{
    vertex_buffer* VB = ctx->VB;
    GLuint i, j0, j1;

    for (i = 0; i < n; i++)
    {
        j0 = (i == 0) ? vlist[n-1] : vlist[i-1];
        j1 = vlist[i];
        render_line(ctx, j0, j1, pv);
    }
}

void render_clipped_line(GLcontext* ctx, GLuint v1, GLuint v2)
{
    GLfloat ndc_x, ndc_y, ndc_z;
    GLuint provoking_vertex;
    vertex_buffer* VB = ctx->VB;

    provoking_vertex = v2;

    VB->Free = VB_MAX;

    if (gl_viewclip_line(ctx, &v1, &v2) == 0)
    {
        return;
    }

    if (VB->Clip[v1][3] != 0.0f)
    {
        GLfloat wInv = 1.0f / VB->Clip[v1][3];
        ndc_x = VB->Clip[v1][0] * wInv;
        ndc_y = VB->Clip[v1][1] * wInv;
        ndc_z = VB->Clip[v1][2] * wInv;
    }
    else
    {
        ndc_x = ndc_y = ndc_z = 0.0f;
    }

    VB->Win[v1][0] = ndc_x * ctx->Viewport.Sx + ctx->Viewport.Tx;
    VB->Win[v1][1] = ndc_y * ctx->Viewport.Sy + ctx->Viewport.Ty;
    if (ctx->ScaleDepthValues)
    {
        VB->Win[v1][2] = ndc_z * ctx->Viewport.Sz + ctx->Viewport.Tz;
    }
    else
    {
        VB->Win[v1][2] = ndc_z * 0.5f;
    }

    if (VB->Clip[v2][3] != 0.0f)
    {
        GLfloat wInv = 1.0f / VB->Clip[v2][3];
        ndc_x = VB->Clip[v2][0] * wInv;
        ndc_y = VB->Clip[v2][1] * wInv;
        ndc_z = VB->Clip[v2][2] * wInv;
    }
    else
    {
        ndc_x = ndc_y = ndc_z = 0.0f;
    }

    VB->Win[v2][0] = ndc_x * ctx->Viewport.Sx + ctx->Viewport.Tx;
    VB->Win[v2][1] = ndc_y * ctx->Viewport.Sy + ctx->Viewport.Ty;
    if (ctx->ScaleDepthValues)
    {
        VB->Win[v2][2] = ndc_z * ctx->Viewport.Sz + ctx->Viewport.Tz;
    }
    else
    {
        VB->Win[v2][2] = ndc_z * 0.5f;
    }

    render_line(ctx, v1, v2, provoking_vertex);
}

void render_clipped_polygon(GLcontext* ctx, GLuint n, GLuint vlist[])
{
    GLuint pv;
    vertex_buffer* VB = ctx->VB;
    GLfloat (*win)[3] = VB->Win;
    GLuint facing;
    GLfloat area;

    pv = (ctx->Primitive == GL_POLYGON) ? vlist[0] : vlist[n-1];

    if (n == 3 && ctx->DriverFuncs.draw_clipped_triangle != NULL)
    {
        ctx->DriverFuncs.draw_clipped_triangle(vlist, pv);
        return;
    }

    VB->Free = VB_MAX;

    if (ctx->TexEnabled)
    {
        ctx->ClipMask |= CLIP_TEXTURE_BIT;
    }

    if (ctx->UserClip)
    {
        GLfloat* proj = ctx->ProjectionMatrix;
        GLuint i;
        n = gl_userclip_polygon(ctx, n, vlist);
        if (n < 3)
        {
            return;
        }
        for (i = 0; i < n; i++)
        {
            GLuint j = vlist[i];
            TRANSFORM_POINT(VB->Clip[j], proj, VB->Eye[j]);
        }
    }

    n = gl_viewclip_polygon(ctx, n, vlist);
    ctx->ClipMask &= ~CLIP_TEXTURE_BIT;
    if (n < 3)
    {
        return;
    }

    {
        GLfloat sx = ctx->Viewport.Sx;
        GLfloat tx = ctx->Viewport.Tx;
        GLfloat sy = ctx->Viewport.Sy;
        GLfloat ty = ctx->Viewport.Ty;
        GLfloat sz = ctx->Viewport.Sz;
        GLfloat tz = ctx->Viewport.Tz;
        GLuint i;
        for (i = 0; i < n; i++)
        {
            GLuint j = vlist[i];
#if PARANOID_W
            if (VB->Clip[j][3] != 0.0f)
#endif
            {
                GLfloat wInv = 1.0f / VB->Clip[j][3];
                win[j][0] = VB->Clip[j][0] * wInv * sx + tx;
                win[j][1] = VB->Clip[j][1] * wInv * sy + ty;
                if (ctx->ScaleDepthValues)
                {
                    win[j][2] = VB->Clip[j][2] * wInv * sz + tz;
                }
                else
                {
                    win[j][2] = VB->Clip[j][2] * wInv * 0.5f;
                }
            }
#if PARANOID_W
            else
            {
                win[j][0] = win[j][1] = win[j][2] = 0.0f;
            }
#endif
        }
    }

    area = polygon_area(VB, n, vlist);
    if (area == 0.0f)
    {
        return;
    }

    facing = (area > 0.0f);
    if (ctx->CullFace)
    {
//        if (ctx->CullFaceMode == GL_FRONT_AND_BACK)
//            return;
//        if (ctx->CullFaceMode == GL_FRONT)
//            facing = !facing;
        if (!facing)
        {
            g_CulledPolys += n - 2;
            return;     //culled
        }
    }

    if (ctx->TwoSide && (!facing)) VB->Color = VB->Bcolor;

    if (ctx->PolygonMode == GL_FILL)
    {
#if DRIVER_TRIANGLE_FAN
        if (ctx->DriverFuncs.draw_triangle_fan != NULL)
        {
            g_NumPolys += n;
            ctx->DriverFuncs.draw_triangle_fan(n, vlist, pv);
        }
        else
#endif
        {
            GLuint vl[3], i, j0;
            j0 = vlist[0];
            for (i = 2; i < n; i++)
            {
                vl[0] = j0;
                vl[1] = vlist[i-1];
                vl[2] = vlist[i];
                CALL_TRIANGLE(ctx, vl, pv);
            }
        }
    }
    else
    {
        GLuint i, j0 = vlist[0];
        for (i = 2; i < n; i++)
        {
            render_clipped_line(ctx, j0, vlist[i-1]);
            render_clipped_line(ctx, vlist[i-1], vlist[i]);
            render_clipped_line(ctx, vlist[i], j0);
        }
    }

    if (ctx->TwoSide && (!facing)) VB->Color = VB->Fcolor;
}

void render_triangle(
    GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
    vertex_buffer* VB = ctx->VB;
    GLfloat (*win)[3] = VB->Win;
    GLfloat ex = win[v1][0] - win[v0][0];
    GLfloat ey = win[v1][1] - win[v0][1];
    GLfloat fx = win[v2][0] - win[v0][0];
    GLfloat fy = win[v2][1] - win[v0][1];
    GLfloat c = ex*fy - ey*fx;
    GLuint facing;

    if (c == 0.0f)
        return;

    facing = (c > 0.0f);
    if (ctx->CullFace)
    {
//        if (ctx->CullFaceMode == GL_FRONT_AND_BACK)
//            return;
//        if (ctx->CullFaceMode == GL_FRONT)
//            facing = !facing;
        if (!facing)
        {
            g_CulledPolys++;
            return;     //culled
        }
    }

    if (ctx->TwoSide && (!facing)) VB->Color = VB->Bcolor;

    if (ctx->PolygonMode == GL_FILL)
    {
        GLuint vl[3];
        vl[0] = v0;
        vl[1] = v1;
        vl[2] = v2;
        CALL_TRIANGLE(ctx, vl, pv)
    }
    else
    {
        render_clipped_line(ctx, v0, v1);
        render_clipped_line(ctx, v1, v2);
        render_clipped_line(ctx, v2, v0);
    }

    if (ctx->TwoSide && (!facing)) VB->Color = VB->Fcolor;
}

void render_quad(GLcontext* ctx,
                 GLuint v0, GLuint v1, GLuint v2, GLuint v3,
                 GLuint pv)
{
    vertex_buffer* VB = ctx->VB;
    GLfloat (*win)[3] = VB->Win;

    GLfloat ex = win[v2][0] - win[v0][0];
    GLfloat ey = win[v2][1] - win[v0][1];
    GLfloat fx = win[v3][0] - win[v1][0];
    GLfloat fy = win[v3][1] - win[v1][1];
    GLfloat c = ex*fy - ey*fx;
    GLuint facing;

    if (c == 0.0f)
        return;

    facing = (c > 0.0f);
    if (ctx->CullFace)
    {
//        if (ctx->CullFaceMode == GL_FRONT_AND_BACK)
//            return;
//        if (ctx->CullFaceMode == GL_FRONT)
//            facing = !facing;
        if (!facing)
        {
            g_CulledPolys += 2;
            return;     //culled
        }
    }

    if (ctx->TwoSide && (!facing)) VB->Color = VB->Bcolor;

    if (ctx->PolygonMode == GL_FILL)
    {
        GLuint vl[4];
        vl[0] = v0;
        vl[1] = v1;
        vl[2] = v2;
        vl[3] = v3;
        CALL_QUAD(ctx, vl, pv);
    }
    else
    {
        render_clipped_line(ctx, v0, v1);
        render_clipped_line(ctx, v1, v2);
        render_clipped_line(ctx, v2, v3);
        render_clipped_line(ctx, v3, v0);
    }

    if (ctx->TwoSide && (!facing)) VB->Color = VB->Fcolor;
}

void gl_render_vb(GLcontext* ctx, GLboolean allDone)
{
    vertex_buffer* VB = ctx->VB;
    GLuint vlist[VB_SIZE];

    if (ctx->RequireLocking && !ctx->ExclusiveLock)
    {
        LOCK_BUFFER(ctx);
        ctx->FrameBuffer = GET_FRAMEBUFFER(ctx);
    }

    switch (ctx->Primitive)
    {
    case GL_POLYGON:
        if (VB->Count > 2)
        {
            GLuint i;
            for (i = 0; i < VB->Count; i++)
            {
                vlist[i] = i;
            }
            if (VB->ClipOrMask)
            {
                render_clipped_polygon(ctx, VB->Count, vlist);
            }
            else
            {
                render_clipped_polygon(ctx, VB->Count, vlist);
            }
        }
        break;

    case GL_QUADS:
        if (VB->ClipOrMask)
        {
            GLuint i;
            for (i = 3; i < VB->Count; i += 4)
            {
                if (VB->ClipMask[i-3] | VB->ClipMask[i-2] |
                    VB->ClipMask[i-1] | VB->ClipMask[i])
                {
                    vlist[0] = i-3;
                    vlist[1] = i-2;
                    vlist[2] = i-1;
                    vlist[3] = i;
                    render_clipped_polygon(ctx, 4, vlist);
                }
                else
                {
                    render_quad(ctx, i-3, i-2, i-1, i, i);
                }
            }
        }
        else
        {
            GLuint i;
            for (i = 3; i < VB->Count; i += 4)
            {
                render_quad(ctx, i-3, i-2, i-1, i, i);
            }
        }
        break;

    case GL_QUAD_STRIP:
        if (VB->ClipOrMask)
        {
            GLuint i;
            for (i = 3; i < VB->Count; i += 2)
            {
                if (VB->ClipMask[i-2] | VB->ClipMask[i-3] |
                    VB->ClipMask[i-1] | VB->ClipMask[i])
                {
                    vlist[0] = i-1;
                    vlist[1] = i-3;
                    vlist[2] = i-2;
                    vlist[3] = i;
                    render_clipped_polygon(ctx, 4, vlist);
                }
                else
                {
                    render_quad(ctx, i-3, i-2, i, i-1, i);
                }
            }
        }
        else
        {
            GLuint i;
            for (i = 3; i < VB->Count; i += 2)
            {
                render_quad(ctx, i-3, i-2, i, i-1, i);
            }
        }
        break;

    case GL_TRIANGLE_FAN:
        if (VB->ClipOrMask)
        {
            GLuint i;
            for (i = 2; i < VB->Count; i++)
            {
                if (VB->ClipMask[0] | VB->ClipMask[i-1] | VB->ClipMask[i])
                {
                    vlist[0] = 0;
                    vlist[1] = i-1;
                    vlist[2] = i;
                    render_clipped_polygon(ctx, 3, vlist);
                }
                else
                {
                    render_triangle(ctx, 0, i-1, i, 2);
                }
            }
        }
        else
        {
#if DRIVER_TRIANGLE_FAN
            if (ctx->DriverFuncs.draw_triangle_fan != NULL)
            {
                GLuint i, vl[256];
                if (VB->Count < 256)
                {
                    for (i = 0; i < VB->Count; i++)
                    {
                        vl[i] = i;
                    }
                    ctx->DriverFuncs.draw_triangle_fan(VB->Count, vl, 2);
                }
                else
                {
                    gl_error(ctx, GL_INVALID_VALUE, "gl_render_vb triangle_fan overflow");
                }
            }
            else
#endif
            {
                GLuint i;
                for (i = 2; i < VB->Count; i++)
                {
                    render_triangle(ctx, 0, i-1, i, 2);
                }
            }
        }
        break;

    case GL_TRIANGLE_STRIP:
        if (VB->ClipOrMask)
        {
            GLuint i;
            for (i = 2; i < VB->Count; i++)
            {
                if (VB->ClipMask[i-2] | VB->ClipMask[i-1] | VB->ClipMask[i])
                {
                    if (i&1)
                    {
                        vlist[0] = i-1;
                        vlist[1] = i-2;
                        vlist[2] = i-0;
                        render_clipped_polygon(ctx, 3, vlist);
                    }
                    else
                    {
                        vlist[0] = i-2;
                        vlist[1] = i-1;
                        vlist[2] = i;
                        render_clipped_polygon(ctx, 3, vlist);
                    }
                }
                else
                {
                    if (i&1)
                    {
                        render_triangle(ctx, i, i-1, i-2, i);
                    }
                    else
                    {
                        render_triangle(ctx, i-2, i-1, i, i);
                    }
                }
            }
        }
        else
        {
#if DRIVER_TRIANGLE_STRIP
            if (ctx->DriverFuncs.draw_triangle_strip != NULL)
            {
                GLuint i, vl[64];
                for (i = 0; i < VB->Count; i++)
                {
                    vl[i] = i;
                }
                ctx->DriverFuncs.draw_triangle_strip(VB->Count, vl, 2);
            }
            else
#endif
            {
                GLuint i;
                for (i = 2; i < VB->Count; i++)
                {
                    if (i&1)
                    {
                        render_triangle(ctx, i, i-1, i-2, i);
                    }
                    else
                    {
                        render_triangle(ctx, i-2, i-1, i, i);
                    }
                }
            }
        }
        break;

    case GL_TRIANGLES:
        if (VB->ClipOrMask)
        {
            GLuint i;
            for (i = 2; i < VB->Count; i += 3)
            {
                if (VB->ClipMask[i-2] | VB->ClipMask[i-1] | VB->ClipMask[i])
                {
                    vlist[0] = i-2;
                    vlist[1] = i-1;
                    vlist[2] = i;
                    render_clipped_polygon(ctx, 3, vlist);
                }
                else
                {
                    render_triangle(ctx, i-2, i-1, i, i);
                }
            }
        }
        else
        {
            GLuint i;
#if DRIVER_TRIANGLE_ARRAY
            if (ctx->DriverFuncs.draw_triangle_array != NULL)
            {
                for (i = 0; i < VB->Count; i++)
                {
                    vlist[i] = i;
                }
                ctx->DriverFuncs.draw_triangle_array(VB->Count, vlist, 0);
            }
            else
#endif
            {
                for (i = 2; i < VB->Count; i += 3)
                {
                    render_triangle(ctx, i-2, i-1, i, i);
                }
            }
        }
        break;

    case GL_POINTS:
        render_points(ctx, VB->Start, VB->Count - 1);
        break;

    case GL_LINES:
        if (VB->ClipOrMask)
        {
            GLuint i;
            for (i = 1; i < VB->Count; i += 2)
            {
                if (VB->ClipMask[i-1] | VB->ClipMask[i])
                {
                    render_clipped_line(ctx, i-1, i);
                }
                else
                {
                    render_line(ctx, i-1, i, i);
                }
                ctx->StippleCounter = 0;
            }
        }
        else
        {
            GLuint i;
            for (i = 1; i < VB->Count; i += 2)
            {
                render_line(ctx, i-1, i, i);
                ctx->StippleCounter = 0;
            }
        }
        break;

    case GL_LINE_STRIP:
        if (VB->ClipOrMask)
        {
            GLuint i;
            for (i = 1; i < VB->Count; i++)
            {
                if (VB->ClipMask[i-1] | VB->ClipMask[i])
                {
                    render_clipped_line(ctx, i-1, i);
                }
                else
                {
                    render_line(ctx, i-1, i, i);
                }
            }
        }
        else
        {
            GLuint i;
            for (i = 1; i < VB->Count; i++)
            {
                render_line(ctx, i-1, i, i);
            }
        }
        break;

    case GL_LINE_LOOP:
    {
        GLuint i;
        i = (VB->Start == 0) ? 1 : 2;
        while (i < VB->Count)
        {
            if (VB->ClipMask[i-1] | VB->ClipMask[i])
            {
                render_clipped_line(ctx, i-1, i);
            }
            else
            {
                render_line(ctx, i-1, i, i);
            }
            i++;
        }
        break;
    }

    default:
        gl_problem(ctx, "unsupported type in gl_render_vb");
        return;
    }

    gl_reset_vb(ctx, allDone);
    if (ctx->RequireLocking && !ctx->ExclusiveLock)
    {
        UNLOCK_BUFFER(ctx);
    }
}

void gl_reset_vb(GLcontext* ctx, GLboolean allDone)
{
    vertex_buffer* VB = ctx->VB;

    if (ctx->Primitive == GL_LINE_LOOP && allDone)
    {
        if (VB->ClipMask[VB->Count-1] | VB->ClipMask[0])
        {
            render_clipped_line(ctx, VB->Count-1, 0);
        }
        else
        {
            render_line(ctx, VB->Count-1, 0, 0);
        }
    }

    if (VB->ClipOrMask)
    {
        MEMSET(VB->ClipMask + VB->Start, 0,
               (VB->Count - VB->Start) * sizeof(VB->ClipMask[0]));
    }

#if !FANCY_RESET

    VB->Count = 0;
    VB->ClipOrMask = 0;
    VB->ClipAndMask = CLIP_ALL_BITS;

#else //FANCY_RESET

    switch (ctx->Primitive)
    {
    case GL_POLYGON:
        VB->Count = 0;
        VB->ClipOrMask = 0;
        VB->ClipAndMask = CLIP_ALL_BITS;
        break;

    case GL_QUADS:
//        ASSERT(VB->Start == 0);
        VB->Count = 0;
        VB->ClipOrMask = 0;
        VB->ClipAndMask = CLIP_ALL_BITS;
        break;

    case GL_QUAD_STRIP:
        if (allDone)
        {
            VB->Count = 0;
            VB->ClipOrMask = 0;
            VB->ClipAndMask = CLIP_ALL_BITS;
        }
        else
        {
            gl_error(ctx, GL_INVALID_VALUE, "gl_reset_vb quad_strip overflow");
        }
        break;

    case GL_POINTS:
//        ASSERT(VB->Start == 0);
        VB->Count = 0;
        VB->ClipOrMask = 0;
        VB->ClipAndMask = CLIP_ALL_BITS;
        break;

    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
        if (allDone)
        {
            VB->Count = 0;
            VB->ClipOrMask = 0;
            VB->ClipAndMask = CLIP_ALL_BITS;
        }
        else
        {
            gl_error(ctx, GL_INVALID_VALUE, "gl_reset_vb line_loop overflow");
        }
        break;

    case GL_TRIANGLES:
//        ASSERT(VB->Start == 0);
        VB->Count = 0;
        VB->ClipOrMask = 0;
        VB->ClipAndMask = CLIP_ALL_BITS;
        break;

    case GL_TRIANGLE_FAN:
        if (allDone)
        {
            VB->Count = 0;
            VB->ClipOrMask = 0;
            VB->ClipAndMask = CLIP_ALL_BITS;
        }
        else
        {
            gl_error(ctx, GL_INVALID_VALUE, "gl_reset_vb triangle_fan overflow");
        }
        break;

    case GL_TRIANGLE_STRIP:
        if (allDone)
        {
            VB->Count = 0;
            VB->ClipOrMask = 0;
            VB->ClipAndMask = CLIP_ALL_BITS;
        }
        else
        {
            gl_error(ctx, GL_INVALID_VALUE, "gl_reset_vb triangle_strip overflow");
        }
        break;
    }

#endif //!FANCY_RESET

    VB->Start = VB->Count;
}
