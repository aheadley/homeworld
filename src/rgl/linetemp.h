/*void line(GLcontext* ctx, GLuint vert0, GLuint vert1, GLuint pvert)*/
{
    vertex_buffer* VB = ctx->VB;
    GLint x0 = (GLint)VB->Win[vert0][0], x1 = (GLint)VB->Win[vert1][0];
    GLint y0 = (GLint)VB->Win[vert0][1], y1 = (GLint)VB->Win[vert1][1];
    GLint dx, dy;
#if INTERP_XY
    GLint xstep, ystep;
#endif
#if INTERP_Z
    GLint z0, z1, dz, zPtrXstep, zPtrYstep;
    GLdepth* zPtr;
#endif
#if INTERP_RGB
    GLfixed r0 = IntToFixed(VB->Color[vert0][0]);
    GLfixed dr = IntToFixed(VB->Color[vert1][0]) - r0;
    GLfixed g0 = IntToFixed(VB->Color[vert0][1]);
    GLfixed dg = IntToFixed(VB->Color[vert1][1]) - g0;
    GLfixed b0 = IntToFixed(VB->Color[vert0][2]);
    GLfixed db = IntToFixed(VB->Color[vert1][2]) - b0;
#endif
#if INTERP_ALPHA
    GLfixed a0 = IntToFixed(VB->Color[vert0][3]);
    GLfixed da = IntToFixed(VB->Color[vert1][3]) - a0;
#endif
#ifdef PIXEL_ADDRESS
    PIXEL_TYPE* pixelPtr;
    GLint pixelXstep, pixelYstep;
#endif

#if WIDE
    GLint width, min, max;
    width = (GLint)CLAMP(ctx->Line.Width, MIN_LINE_WIDTH, MAX_LINE_WIDTH);
    min = -width / 2;
    max = min + width - 1;
#endif

/*
 * Despite being clipped to the view volume, the line's window coordinates
 * may just lie outside the window bounds.  That is, if the legal window
 * coordinates are [0,W-1][0,H-1], it's possible for x==W and/or y==H.
 * This quick and dirty code nudges the endpoints inside the window if
 * necessary.
 */
#if CLIP_HACK
    {
        GLint w = ctx->Buffer.Width;
        GLint h = ctx->Buffer.Height;
        if ((x0 == w) | (x1 == w))
        {
            if ((x0 == w) & (x1 == w))
            {
                return;
            }
            x0 -= x0==w;
            x1 -= x1==w;
        }
        if ((y0 == h) | (y1 == h))
        {
            if ((y0 == h) & (y1 == h))
            {
                return;
            }
            y0 -= y0==h;
            y1 -= y1==h;
        }
    }
#endif
    dx = x1 - x0;
    dy = y1 - y0;
    if (dx == 0 && dy == 0)
    {
        return;
    }

    /*
     * Setup
     */
#ifdef SETUP_CODE
    SETUP_CODE
#endif

#if INTERP_Z
    zPtr = CTX_Z_ADDRESS(ctx, x0, y0);
#if DEPTH_BITS==16
    z0 = FloatToFixed(VB->Win[vert0][2]);
    z1 = FloatToFixed(VB->Win[vert1][2]);
#else
    z0 = (int)VB->Win[vert0][2] | g_DepthMask;
    z1 = (int)VB->Win[vert1][2] | g_DepthMask;
#endif
#endif
#ifdef PIXEL_ADDRESS
    pixelPtr = (PIXEL_TYPE*)PIXEL_ADDRESS(x0, y0);
#endif

    if (dx < 0)
    {
        dx = -dx;   /* make positive */
#if INTERP_XY
        xstep = -1;
#endif
#ifdef INTERP_Z
        zPtrXstep = -((GLint)sizeof(GLdepth));
#endif
#ifdef PIXEL_ADDRESS
        pixelXstep = -((GLint)sizeof(PIXEL_TYPE));
#endif
    }
    else
    {
#if INTERP_XY
        xstep = 1;
#endif
#if INTERP_Z
        zPtrXstep = sizeof(GLdepth);
#endif
#ifdef PIXEL_ADDRESS
        pixelXstep = sizeof(PIXEL_TYPE);
#endif
    }

    if (dy < 0)
    {
        dy = -dy;   /* make positive */
#if INTERP_XY
        ystep = -1;
#endif
#if INTERP_Z
        zPtrYstep = -((GLint)ctx->Buffer.Width) * sizeof(GLdepth);
#endif
#ifdef PIXEL_ADDRESS
        pixelYstep = BYTES_PER_ROW;
#endif
    }
    else
    {
#if INTERP_XY
        ystep = 1;
#endif
#if INTERP_Z
        zPtrYstep = ctx->Buffer.Width * sizeof(GLdepth);
#endif
#ifdef PIXEL_ADDRESS
        pixelYstep = -((GLint)BYTES_PER_ROW);
#endif
    }

    /*
     * Draw
     */

    if (dx > dy)
    {
        /*
         * X-major line
         */
        GLint i;
        GLint errorInc = dy + dy;
        GLint error = errorInc - dx;
        GLint errorDec = error - dx;
#if INTERP_Z
        dz = (z1-z0) / dx;
#endif
#if INTERP_RGB
        dr /= dx;   /* convert from whole line delta to per-pixel delta */
        dg /= dx;
        db /= dx;
#endif
#if INTERP_ALPHA
        da /= dx;
#endif

        for (i = 0; i < dx; i++)
        {
#if STIPPLE
            GLushort m;
            m = 1 << ((ctx->StippleCounter/ctx->StippleFactor) & 0xf);
            if (ctx->StipplePattern & m)
            {
#endif
#if INTERP_Z
#if DEPTH_BITS==16
                GLdepth Z = FixedToInt(z0);
#else
                GLdepth Z = z0;
#endif
#endif
#if WIDE
                GLint yy;
                GLint ymin = y0 + min;
                GLint ymax = y0 + max;
                for (yy = ymin; yy <= ymax; yy++)
                {
                    PLOT(x0, yy);
                }
#else
#ifdef XMAJOR_PLOT
                XMAJOR_PLOT(x0, y0);
#else
                PLOT(x0, y0);
#endif
#endif /*WIDE*/
#if STIPPLE
            }
	        ctx->StippleCounter++;
#endif
#if INTERP_XY
            x0 += xstep;
#endif
#if INTERP_Z
            zPtr = (GLdepth*)((GLubyte*)zPtr + zPtrXstep);
            z0 += dz;
#endif
#if INTERP_RGB
            r0 += dr;
            g0 += dg;
            b0 += db;
#endif
#if INTERP_ALPHA
            a0 += da;
#endif
#ifdef PIXEL_ADDRESS
            pixelPtr = (PIXEL_TYPE*)((GLubyte*)pixelPtr + pixelXstep);
#endif
            if (error < 0)
            {
                error += errorInc;
            }
            else
            {
                error += errorDec;
#if INTERP_XY
                y0 += ystep;
#endif
#if INTERP_Z
                zPtr = (GLdepth*)((GLubyte*)zPtr + zPtrYstep);
#endif
#ifdef PIXEL_ADDRESS
                pixelPtr = (PIXEL_TYPE*)((GLubyte*)pixelPtr + pixelYstep);
#endif
            }
        }
    }
    else
    {
        /*
         * Y-major line
         */
        GLint i;
        GLint errorInc = dx + dx;
        GLint error = errorInc - dy;
        GLint errorDec = error - dy;
#if INTERP_Z
        dz = (z1-z0) / dy;
#endif
#if INTERP_RGB
        dr /= dy;   /* convert from whole line delta to per-pixel delta */
        dg /= dy;
        db /= dy;
#endif
#if INTERP_ALPHA
        da /= dy;
#endif

        for (i = 0; i < dy; i++)
        {
#if STIPPLE
            GLushort m;
            m = 1 << ((ctx->StippleCounter/ctx->StippleFactor) & 0xf);
            if (ctx->StipplePattern & m)
            {
#endif
#if INTERP_Z
#if DEPTH_BITS==16
                GLdepth Z = FixedToInt(z0);
#else
                GLdepth Z = z0;
#endif
#endif
#if WIDE
                GLint xx;
                GLint xmin = x0 + min;
                GLint xmax = x0 + max;
                for (xx = xmin; xx <= xmax; xx++)
                {
                    PLOT(xx, y0);
                }
#else
#ifdef YMAJOR_PLOT
                YMAJOR_PLOT(x0, y0);
#else
                PLOT(x0, y0);
#endif
#endif /*WIDE*/
#if STIPPLE
            }
	        ctx->StippleCounter++;
#endif
#if INTERP_XY
            y0 += ystep;
#endif
#if INTERP_Z
            zPtr = (GLdepth*)((GLubyte*)zPtr + zPtrYstep);
            z0 += dz;
#endif
#if INTERP_RGB
            r0 += dr;
            g0 += dg;
            b0 += db;
#endif
#if INTERP_ALPHA
            a0 += da;
#endif
#ifdef PIXEL_ADDRESS
            pixelPtr = (PIXEL_TYPE*)((GLubyte*)pixelPtr + pixelYstep);
#endif
            if (error < 0)
            {
                error += errorInc;
            }
            else
            {
                error += errorDec;
#if INTERP_XY
                x0 += xstep;
#endif
#if INTERP_Z
                zPtr = (GLdepth*)((GLubyte*)zPtr + zPtrXstep);
#endif
#ifdef PIXEL_ADDRESS
                pixelPtr = (PIXEL_TYPE*)((GLubyte*)pixelPtr + pixelXstep);
#endif
            }
        }
    }
}

#undef INTERP_XY
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ALPHA
#undef INTERP_INDEX
#undef PIXEL_ADDRESS
#undef PIXEL_TYPE
#undef BYTES_PER_ROW
#undef SETUP_CODE
#undef PLOT
#undef XMAJOR_PLOT
#undef YMAJOR_PLOT
#undef CLIP_HACK
#undef STIPPLE
#undef WIDE
