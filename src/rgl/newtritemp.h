/*void triangle(GLcontext* ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)*/
{
    typedef struct
    {
        GLint v0, v1;       // Y(v0) < Y(v1)
        GLfloat dx;         // X(v1) - X(v0)
        GLfloat dy;         // Y(v1) - Y(v0)
        GLfixed fdxdy;      // dx/dy in fixed-point
        GLfixed fsx;        // first sample point x coord
        GLfixed fsy;        // first sample point y coord
        GLfloat adjy;       // adjust from v[0]->fy to fsy, scaled
        GLint lines;        // number of lines to be sampled on this edge
        GLfixed fx0;        // fixed pt X of lower endpoint
    } EdgeT;

    vertex_buffer* VB = ctx->VB;
    EdgeT eMaj, eTop, eBot;
    GLfloat oneOverArea;
    int vMin, vMid, vMax;   // vertex indices: Y(vMin) <= Y(vMid) <= Y(vMax)

    // find the order of the 3 vertices along the Y axis
    {
        GLfloat y0 = VB->Win[v0][1];
        GLfloat y1 = VB->Win[v1][1];
        GLfloat y2 = VB->Win[v2][1];

        if (y0 <= y1)
        {
	        if (y1 <= y2)
            {
	            vMin = v0; vMid = v1; vMax = v2;   // y0<=y1<=y2
	        }
	        else if (y2 <= y0)
            {
	            vMin = v2; vMid = v0; vMax = v1;   // y2<=y0<=y1
	        }
	        else
            {
	            vMin = v0; vMid = v2; vMax = v1;   // y0<=y2<=y1
	        }
        }
        else
        {
	        if (y0 <= y2)
            {
	            vMin = v1; vMid = v0; vMax = v2;   // y1<=y0<=y2
	        }
	        else if (y2 <= y1)
            {
	            vMin = v2; vMid = v1; vMax = v0;   // y2<=y1<=y0
	        }
	        else
            {
	            vMin = v1; vMid = v2; vMax = v0;   // y1<=y2<=y0
	        }
        }
    }

    // vertex/edge relationship
    eMaj.v0 = vMin; eMaj.v1 = vMax;     //TODO: .v1's not needed
    eTop.v0 = vMid; eTop.v1 = vMax;
    eBot.v0 = vMin; eBot.v1 = vMid;

    // compute deltas for each edge:  vertex[v1] - vertex[v0]
    eMaj.dx = VB->Win[vMax][0] - VB->Win[vMin][0];
    eMaj.dy = VB->Win[vMax][1] - VB->Win[vMin][1];
    eTop.dx = VB->Win[vMax][0] - VB->Win[vMid][0];
    eTop.dy = VB->Win[vMax][1] - VB->Win[vMid][1];
    eBot.dx = VB->Win[vMid][0] - VB->Win[vMin][0];
    eBot.dy = VB->Win[vMid][1] - VB->Win[vMin][1];

    // compute oneOverArea
    {
        GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;
        if (area > -0.05f && area < 0.05f)
        {
            return; // very small; CULLED
        }
        oneOverArea = 1.0f / area;
    }

    // Edge setup.  For a triangle strip these could be reused...
    {
        // fixed point Y coordinates
        GLfixed vMin_fx = FloatToFixed(VB->Win[vMin][0] + 0.5f);
        GLfixed vMin_fy = FloatToFixed(VB->Win[vMin][1] - 0.5f);
        GLfixed vMid_fx = FloatToFixed(VB->Win[vMid][0] + 0.5f);
        GLfixed vMid_fy = FloatToFixed(VB->Win[vMid][1] - 0.5f);
        GLfixed vMax_fy = FloatToFixed(VB->Win[vMax][1] - 0.5f);

        eMaj.fsy = FixedCeil(vMin_fy);
        eMaj.lines = FixedToInt(vMax_fy + FIXED_ONE - FIXED_EPSILON - eMaj.fsy);
        if (eMaj.lines > 0)
        {
            GLfloat dxdy = eMaj.dx / eMaj.dy;
            eMaj.fdxdy = SignedFloatToFixed(dxdy);
            eMaj.adjy = (GLfloat)(eMaj.fsy - vMin_fy); // SCALED!
            eMaj.fx0 = vMin_fx;
            eMaj.fsx = eMaj.fx0 + (GLfixed)(eMaj.adjy * dxdy);
        }
        else
        {
            return; // CULLED
        }

        eTop.fsy = FixedCeil(vMid_fy);
        eTop.lines = FixedToInt(vMax_fy + FIXED_ONE - FIXED_EPSILON - eTop.fsy);
        if (eTop.lines > 0)
        {
            GLfloat dxdy = eTop.dx / eTop.dy;
            eTop.fdxdy = SignedFloatToFixed(dxdy);
            eTop.adjy = (GLfloat)(eTop.fsy - vMid_fy); // SCALED!
            eTop.fx0 = vMid_fx;
            eTop.fsx = eTop.fx0 + (GLfixed)(eTop.adjy * dxdy);
        }

        eBot.fsy = FixedCeil(vMin_fy);
        eBot.lines = FixedToInt(vMid_fy + FIXED_ONE - FIXED_EPSILON - eBot.fsy);
        if (eBot.lines > 0)
        {
            GLfloat dxdy = eBot.dx / eBot.dy;
            eBot.fdxdy = SignedFloatToFixed(dxdy);
            eBot.adjy = (GLfloat)(eBot.fsy - vMin_fy); // SCALED!
            eBot.fx0 = vMin_fx;
            eBot.fsx = eBot.fx0 + (GLfixed)(eBot.adjy * dxdy);
        }
    }

    {
        GLint ltor;     // true if scanning left-to-right
#if INTERP_Z
        GLfloat dzdx, dzdy;  GLfixed fdzdx;
#endif
#if INTERP_I
        GLfloat didx, didy;  GLfixed fdidx;
#endif
#if INTERP_RGB
        GLfloat drdx, drdy;  GLfixed fdrdx;
        GLfloat dgdx, dgdy;  GLfixed fdgdx;
        GLfloat dbdx, dbdy;  GLfixed fdbdx;
#endif
#if INTERP_ALPHA
        GLfloat dadx, dady;  GLfixed fdadx;
#endif
#if INTERP_ST
        GLfloat dsdx, dsdy;  GLfixed fdsdx;
        GLfloat dtdx, dtdy;  GLfixed fdtdx;
#endif
#if INTERP_STW
        GLfloat dsdx, dsdy;
        GLfloat dtdx, dtdy;
        GLfloat dwdx, dwdy;
#endif
#if INTERP_STW_AFFINE
        GLint Subdivisions, WidthModLength;
        GLint Counter;
        GLfloat OneOverWLeft, SOverWLeft, TOverWLeft;
        GLfloat dOneOverWdX, dSOverWdX, dTOverWdX;
        GLfloat dOneOverWdXAff, dSOverWdXAff, dTOverWdXAff;
        GLfloat OneOverWRight, SOverWRight, TOverWRight;
        GLfloat WLeft, SLeft, TLeft;
        GLfloat WRight, SRight, TRight;
        GLfixed S, T, DeltaS, DeltaT;
#endif
        /*
         * Execute user-supplied setup code
         */
#ifdef SETUP_CODE
        SETUP_CODE
#endif

        ltor = (oneOverArea < 0.0F);

        // compute d?/dx and d?/dy derivatives
#if INTERP_Z
        {
            GLfloat eMaj_dz, eBot_dz;
            eMaj_dz = VB->Win[vMax][2] - VB->Win[vMin][2];
            eBot_dz = VB->Win[vMid][2] - VB->Win[vMin][2];
            dzdx = oneOverArea * (eMaj_dz * eBot.dy - eMaj.dy * eBot_dz);
            if (dzdx > DEPTH_SCALE || dzdx < -DEPTH_SCALE)
            {
                // probably a sliver triangle
                dzdx = 0.0f;
                dzdy = 0.0f;
            }
            else
            {
                dzdy = oneOverArea * (eMaj.dx * eBot_dz - eMaj_dz * eBot.dx);
            }
#if DEPTH_BITS == 16
            fdzdx = SignedFloatToFixed(dzdx);
#else
            fdzdx = FAST_TO_INT(dzdx); //GLint
#endif
        }
#endif
#if INTERP_I
        {
            GLfloat eMaj_di, eBot_di;
            eMaj_di = (GLfloat)((GLint)VB->Color[vMax][0] - (GLint)VB->Color[vMin][0]);
            eBot_di = (GLfloat)((GLint)VB->Color[vMid][0] - (GLint)VB->Color[vMin][0]);
            didx = oneOverArea * (eMaj_di * eBot.dy - eMaj.dy * eBot_di);
            fdidx = SignedFloatToFixed(didx);
            didy = oneOverArea * (eMaj.dx * eBot_di - eMaj_di * eBot.dx);
        }
#endif
#if INTERP_RGB
        {
            GLfloat eMaj_dr, eBot_dr;
            eMaj_dr = (GLfloat)((GLint)VB->Color[vMax][0] - (GLint)VB->Color[vMin][0]);
            eBot_dr = (GLfloat)((GLint)VB->Color[vMid][0] - (GLint)VB->Color[vMin][0]);
            drdx = oneOverArea * (eMaj_dr * eBot.dy - eMaj.dy * eBot_dr);
            fdrdx = SignedFloatToFixed(drdx);
            drdy = oneOverArea * (eMaj.dx * eBot_dr - eMaj_dr * eBot.dx);
        }
        {
            GLfloat eMaj_dg, eBot_dg;
            eMaj_dg = (GLfloat)((GLint)VB->Color[vMax][1] - (GLint)VB->Color[vMin][1]);
	        eBot_dg = (GLfloat)((GLint)VB->Color[vMid][1] - (GLint)VB->Color[vMin][1]);
            dgdx = oneOverArea * (eMaj_dg * eBot.dy - eMaj.dy * eBot_dg);
            fdgdx = SignedFloatToFixed(dgdx);
            dgdy = oneOverArea * (eMaj.dx * eBot_dg - eMaj_dg * eBot.dx);
        }
        {
            GLfloat eMaj_db, eBot_db;
            eMaj_db = (GLfloat)((GLint)VB->Color[vMax][2] - (GLint)VB->Color[vMin][2]);
            eBot_db = (GLfloat)((GLint)VB->Color[vMid][2] - (GLint)VB->Color[vMin][2]);
            dbdx = oneOverArea * (eMaj_db * eBot.dy - eMaj.dy * eBot_db);
            fdbdx = SignedFloatToFixed(dbdx);
	        dbdy = oneOverArea * (eMaj.dx * eBot_db - eMaj_db * eBot.dx);
        }
#endif
#if INTERP_ALPHA
        {
            GLfloat eMaj_da, eBot_da;
            eMaj_da = (GLfloat)((GLint)VB->Color[vMax][3] - (GLint)VB->Color[vMin][3]);
            eBot_da = (GLfloat)((GLint)VB->Color[vMid][3] - (GLint)VB->Color[vMin][3]);
            dadx = oneOverArea * (eMaj_da * eBot.dy - eMaj.dy * eBot_da);
            fdadx = SignedFloatToFixed(dadx);
            dady = oneOverArea * (eMaj.dx * eBot_da - eMaj_da * eBot.dx);
        }
#endif
#if INTERP_ST
        {
            GLfloat eMaj_ds, eBot_ds;
            eMaj_ds = (VB->TexCoord[vMax][0] - VB->TexCoord[vMin][0]) * S_SCALE;
            eBot_ds = (VB->TexCoord[vMid][0] - VB->TexCoord[vMin][0]) * S_SCALE;
            dsdx = oneOverArea * (eMaj_ds * eBot.dy - eMaj.dy * eBot_ds);
            fdsdx = SignedFloatToFixed(dsdx);
            dsdy = oneOverArea * (eMaj.dx * eBot_ds - eMaj_ds * eBot.dx);
        }
        {
            GLfloat eMaj_dt, eBot_dt;
            eMaj_dt = (VB->TexCoord[vMax][1] - VB->TexCoord[vMin][1]) * T_SCALE;
            eBot_dt = (VB->TexCoord[vMid][1] - VB->TexCoord[vMin][1]) * T_SCALE;
            dtdx = oneOverArea * (eMaj_dt * eBot.dy - eMaj.dy * eBot_dt);
            fdtdx = SignedFloatToFixed(dtdx);
            dtdy = oneOverArea * (eMaj.dx * eBot_dt - eMaj_dt * eBot.dx);
        }
#endif
#if INTERP_STW
        {
            GLfloat wMax = 1.0f / VB->Clip[vMax][3];
            GLfloat wMin = 1.0f / VB->Clip[vMin][3];
            GLfloat wMid = 1.0f / VB->Clip[vMid][3];
            GLfloat eMaj_dw, eBot_dw;
            GLfloat eMaj_ds, eBot_ds;
            GLfloat eMaj_dt, eBot_dt;

            eMaj_dw = wMax - wMin;
            eBot_dw = wMid - wMin;
            dwdx = oneOverArea * (eMaj_dw * eBot.dy - eMaj.dy * eBot_dw);
            dwdy = oneOverArea * (eMaj.dx * eBot_dw - eMaj_dw * eBot.dx);

            eMaj_ds = VB->TexCoord[vMax][0]*wMax - VB->TexCoord[vMin][0]*wMin;
            eBot_ds = VB->TexCoord[vMid][0]*wMid - VB->TexCoord[vMin][0]*wMin;
            dsdx = oneOverArea * (eMaj_ds * eBot.dy - eMaj.dy * eBot_ds);
            dsdy = oneOverArea * (eMaj.dx * eBot_ds - eMaj_ds * eBot.dx);

            eMaj_dt = VB->TexCoord[vMax][1]*wMax - VB->TexCoord[vMin][1]*wMin;
            eBot_dt = VB->TexCoord[vMid][1]*wMid - VB->TexCoord[vMin][1]*wMin;
            dtdx = oneOverArea * (eMaj_dt * eBot.dy - eMaj.dy * eBot_dt);
            dtdy = oneOverArea * (eMaj.dx * eBot_dt - eMaj_dt * eBot.dx);
        }
#endif

        {
            int subTriangle;
            GLfixed fx, fxLeftEdge, fxRightEdge, fdxLeftEdge, fdxRightEdge;
            GLfixed fdxOuter;
            int idxOuter;
            float dxOuter;
            GLfixed fError, fdError;
            float adjx, adjy;
            GLfixed fy;
            int iy;
#ifdef PIXEL_ADDRESS
            PIXEL_TYPE* pRow;
            int dPRowOuter, dPRowInner;
#endif
#if INTERP_Z
            GLdepth* zRow;
            int dZRowOuter, dZRowInner; // offset in bytes
            GLfixed fz, fdzOuter, fdzInner;
#endif
#if INTERP_I
            GLfixed fi, fdiOuter, fdiInner;
#endif
#if INTERP_RGB
            GLfixed fr, fdrOuter, fdrInner;
            GLfixed fg, fdgOuter, fdgInner;
            GLfixed fb, fdbOuter, fdbInner;
#endif
#if INTERP_ALPHA
            GLfixed fa, fdaOuter, fdaInner;
#endif
#if INTERP_ST
            GLfixed fs, fdsOuter, fdsInner;
            GLfixed ft, fdtOuter, fdtInner;
#endif
#if INTERP_STW
            GLfloat sLeft, dsOuter, dsInner;
            GLfloat tLeft, dtOuter, dtInner;
            GLfloat wLeft, dwOuter, dwInner;
#endif

            for (subTriangle = 0; subTriangle <= 1; subTriangle++)
            {
                EdgeT* eLeft;
                EdgeT* eRight;
                int setupLeft, setupRight;
                int lines;

                if (subTriangle == 0)
                {
                    // bottom half
                    if (ltor)
                    {
                        eLeft = &eMaj;
                        eRight = &eBot;
                        lines = eRight->lines;
                        setupLeft = 1;
                        setupRight = 1;
                    }
                    else
                    {
                        eLeft = &eBot;
                        eRight = &eMaj;
                        lines = eLeft->lines;
                        setupLeft = 1;
                        setupRight = 1;
                    }
                }
                else
                {
                    // top half
                    if (ltor)
                    {
                        eLeft = &eMaj;
                        eRight = &eTop;
                        lines = eRight->lines;
                        setupLeft = 0;
                        setupRight = 1;
                    }
                    else
                    {
                        eLeft = &eTop;
                        eRight = &eMaj;
                        lines = eLeft->lines;
                        setupLeft = 1;
                        setupRight = 0;
                    }
                    if (lines == 0) return;
                }

                if (setupLeft && eLeft->lines > 0)
                {
                    GLint vLower;
                    GLfixed fsx = eLeft->fsx;
                    fx = FixedCeil(fsx);
                    fError = fx - fsx - FIXED_ONE;
                    fxLeftEdge = fsx - FIXED_EPSILON;
                    fdxLeftEdge = eLeft->fdxdy;
                    fdxOuter = FixedFloor(fdxLeftEdge - FIXED_EPSILON);
                    fdError = fdxOuter - fdxLeftEdge + FIXED_ONE;
                    idxOuter = FixedToInt(fdxOuter);
                    dxOuter = (float)idxOuter;

                    fy = eLeft->fsy;
                    iy = FixedToInt(fy);

                    adjx = (float)(fx - eLeft->fx0); // SCALED!
                    adjy = eLeft->adjy; // SCALED!

                    vLower = eLeft->v0;

#ifdef PIXEL_ADDRESS
                    {
                        pRow = PIXEL_ADDRESS(FixedToInt(fxLeftEdge), iy);
                        dPRowOuter = -((int)BYTES_PER_ROW) + idxOuter * sizeof(PIXEL_TYPE);
                    }
#endif

#if INTERP_Z
                    {
                        GLfloat z0, tmp;
                        z0 = VB->Win[vLower][2];
#if DEPTH_BITS == 16
                        // interpolate fixed-pt values
                        tmp = (z0 * FIXED_SCALE + dzdx * adjx + dzdy * adjy) + FIXED_HALF;
                        if (tmp < MAX_GLUINT/2)
                        {
                            fz = (GLfixed)tmp;
                        }
                        else
                        {
                            fz = MAX_GLUINT/2;
                        }
                        fdzOuter = SignedFloatToFixed(dzdy + dxOuter * dzdx);
#else
                        // interpolate depth values exactly
                        fz = FAST_TO_INT(z0 + dzdx*FixedToFloat(adjx) + dzdy*FixedToFloat(adjy)); //GLint
                        fdzOuter = FAST_TO_INT(dzdy + dxOuter * dzdx); //GLint
#endif
                        zRow = CTX_Z_ADDRESS(ctx, FixedToInt(fxLeftEdge), iy);
                        dZRowOuter = (ctx->Buffer.Width + idxOuter) * sizeof(GLdepth);
                    }
#endif
#if INTERP_I
                    fi = (GLfixed)(IntToFixed(VB->Color[vLower][0]) + didx * adjx + didy * adjy) + FIXED_HALF;
                    fdiOuter = SignedFloatToFixed(didy + dxOuter * didx);
#endif
#if INTERP_RGB
                    fr = (GLfixed)(IntToFixed(VB->Color[vLower][0]) + drdx * adjx + drdy * adjy) + FIXED_HALF;
                    fdrOuter = SignedFloatToFixed(drdy + dxOuter * drdx);

                    fg = (GLfixed)(IntToFixed(VB->Color[vLower][1]) + dgdx * adjx + dgdy * adjy) + FIXED_HALF;
                    fdgOuter = SignedFloatToFixed(dgdy + dxOuter * dgdx);

                    fb = (GLfixed)(IntToFixed(VB->Color[vLower][2]) + dbdx * adjx + dbdy * adjy) + FIXED_HALF;
                    fdbOuter = SignedFloatToFixed(dbdy + dxOuter * dbdx);
#endif
#if INTERP_ALPHA
                    fa = (GLfixed)(IntToFixed(VB->Color[vLower][3]) + dadx * adjx + dady * adjy) + FIXED_HALF;
                    fdaOuter = SignedFloatToFixed(dady + dxOuter * dadx);
#endif
#if INTERP_ST
                    {
                        GLfloat s0, t0;
                        s0 = VB->TexCoord[vLower][0] * S_SCALE;
                        fs = (GLfixed)(s0 * FIXED_SCALE + dsdx * adjx + dsdy * adjy) + FIXED_HALF;
                        fdsOuter = SignedFloatToFixed(dsdy + dxOuter * dsdx);
                        t0 = VB->TexCoord[vLower][1] * T_SCALE;
                        ft = (GLfixed)(t0 * FIXED_SCALE + dtdx * adjx + dtdy * adjy) + FIXED_HALF;
                        fdtOuter = SignedFloatToFixed(dtdy + dxOuter * dtdx);
                    }
#endif
#if INTERP_STW
                    {
                        GLfloat w0 = 1.0f / VB->Clip[vLower][3];
                        GLfloat s0, t0;
                        wLeft = w0 + (dwdx * adjx + dwdy * adjy) * (1.0f/FIXED_SCALE);
		                dwOuter = dwdy + dxOuter * dwdx;
                        s0 = VB->TexCoord[vLower][0] * w0;
                        sLeft = s0 + (dsdx * adjx + dsdy * adjy) * (1.0f/FIXED_SCALE);
                        dsOuter = dsdy + dxOuter * dsdx;
                        t0 = VB->TexCoord[vLower][1] * w0;
                        tLeft = t0 + (dtdx * adjx + dtdy * adjy) * (1.0f/FIXED_SCALE);
                        dtOuter = dtdy + dxOuter * dtdx;
                    }
#endif
                } //if setupLeft


                if (setupRight && eRight->lines > 0)
                {
                    fxRightEdge = eRight->fsx - FIXED_EPSILON;
                    fdxRightEdge = eRight->fdxdy;
                }

                if (lines == 0)
                {
                    continue;
                }


                // Rasterize setup
#ifdef PIXEL_ADDRESS
                dPRowInner = dPRowOuter + sizeof(PIXEL_TYPE);
#endif
#if INTERP_Z
                dZRowInner = dZRowOuter + sizeof(GLdepth);
                fdzInner = fdzOuter + fdzdx;
#endif
#if INTERP_I
                fdiInner = fdiOuter + fdidx;
#endif
#if INTERP_RGB
                fdrInner = fdrOuter + fdrdx;
                fdgInner = fdgOuter + fdgdx;
                fdbInner = fdbOuter + fdbdx;
#endif
#if INTERP_ALPHA
                fdaInner = fdaOuter + fdadx;
#endif
#if INTERP_ST
                fdsInner = fdsOuter + fdsdx;
                fdtInner = fdtOuter + fdtdx;
#endif
#if INTERP_STW
	            dwInner = dwOuter + dwdx;
	            dsInner = dsOuter + dsdx;
	            dtInner = dtOuter + dtdx;
#endif

                while (lines > 0)
                {
                    // initialize the span interpolants to the leftmost value
                    // ff = fixed-pt fragment
#if INTERP_Z
                    GLfixed ffz = fz | g_DepthMask;
                    //GLdepth *zp = zRow;
#endif
#if INTERP_I
                    GLfixed ffi = fi;
#endif
#if INTERP_RGB
                    GLfixed ffr = fr, ffg = fg, ffb = fb;
#endif
#if INTERP_ALPHA
                    GLfixed ffa = fa;
#endif
#if INTERP_ST
                    GLfixed ffs = fs, fft = ft;
#endif
#if INTERP_STW
                    GLfloat ss = sLeft, tt = tLeft, ww = wLeft;
#endif
                    GLint left = FixedToInt(fxLeftEdge);
                    GLint right = FixedToInt(fxRightEdge);

#if INTERP_I
                    {
                        GLfixed ffiend = ffi + (right-left-1)*fdidx;
                        if (ffiend < 0) ffi -= ffiend;
                        if (ffi < 0) ffi = 0;
                    }
#endif
#if INTERP_RGB
                    {
                        // need this to accomodate round-off errors
                        GLfixed ffrend = ffr + (right-left-1)*fdrdx;
                        GLfixed ffgend = ffg + (right-left-1)*fdgdx;
                        GLfixed ffbend = ffb + (right-left-1)*fdbdx;
                        if (ffrend < 0) ffr -= ffrend;
                        if (ffgend < 0) ffg -= ffgend;
                        if (ffbend < 0) ffb -= ffbend;
                        if (ffr < 0) ffr = 0;
                        if (ffg < 0) ffg = 0;
                        if (ffb < 0) ffb = 0;
                    }
#endif
#if INTERP_ALPHA
                    {
                        GLfixed ffaend = ffa + (right-left-1)*fdadx;
                        if (ffaend < 0) ffa -= ffaend;
                        if (ffa < 0) ffa = 0;
                    }
#endif

                    if (ctx->ScissorTest &&
                        (iy < ctx->ScissorY || iy >= (ctx->ScissorY + ctx->ScissorHeight)))
                    {
                        goto SKIP_SPAN;
                    }

                    if (ctx->Speedy && rglGetSkip(iy))
                    {
                        goto SKIP_SPAN;
                    }

#if INTERP_STW_AFFINE

                    if ((right - left) > 0)
                    {
                        GLint i, Width;

                        Width = right - left;

                        OneOverWLeft = ww;
                        SOverWLeft = ss * S_SCALE;
                        TOverWLeft = tt * T_SCALE;

                        dOneOverWdX = dwdx;
                        dSOverWdX = dsdx * S_SCALE;
                        dTOverWdX = dtdx * T_SCALE;

                        dOneOverWdXAff = dOneOverWdX * AFFINE_LENGTH;
                        dSOverWdXAff = dSOverWdX * AFFINE_LENGTH;
                        dTOverWdXAff = dTOverWdX * AFFINE_LENGTH;

                        OneOverWRight = OneOverWLeft + dOneOverWdXAff;
                        SOverWRight = SOverWLeft + dSOverWdXAff;
                        TOverWRight = TOverWLeft + dTOverWdXAff;

                        WLeft = 1.0f / OneOverWLeft;
                        SLeft = WLeft * SOverWLeft;
                        TLeft = WLeft * TOverWLeft;

                        i = 0;

                        Subdivisions = Width / AFFINE_LENGTH;
                        WidthModLength = Width & (AFFINE_LENGTH-1);

                        if (!WidthModLength)
                        {
                            Subdivisions--;
                            WidthModLength = AFFINE_LENGTH;
                        }

                        while (Subdivisions-- > 0)
                        {
                            WRight = 1.0f / OneOverWRight;
                            SRight = WRight * SOverWRight;
                            TRight = WRight * TOverWRight;

                            S = FAST_TO_FIXED(SLeft);
                            T = FAST_TO_FIXED(TLeft);
                            DeltaS = FAST_TO_FIXED(SRight - SLeft) >> AFFINE_SHIFT;
                            DeltaT = FAST_TO_FIXED(TRight - TLeft) >> AFFINE_SHIFT;

                            AFFINE_LOOP0

                            SLeft = SRight;
                            TLeft = TRight;

                            OneOverWRight += dOneOverWdXAff;
                            SOverWRight += dSOverWdXAff;
                            TOverWRight += dTOverWdXAff;
                        }

                        if (WidthModLength)
                        {
                            OneOverWRight -= dOneOverWdXAff;
                            SOverWRight -= dSOverWdXAff;
                            TOverWRight -= dTOverWdXAff;

                            WidthModLength--;

                            OneOverWRight += dOneOverWdX * WidthModLength;
                            SOverWRight += dSOverWdX * WidthModLength;
                            TOverWRight += dTOverWdX * WidthModLength;

                            WRight = 1.0f / OneOverWRight;
                            SRight = WRight * SOverWRight;
                            TRight = WRight * TOverWRight;

                            S = FAST_TO_FIXED(SLeft);
                            T = FAST_TO_FIXED(TLeft);

                            if (WidthModLength)
                            {
                                DeltaS = FAST_TO_FIXED(SRight - SLeft) / WidthModLength;
                                DeltaT = FAST_TO_FIXED(TRight - TLeft) / WidthModLength;
                            }

                            AFFINE_LOOP1
                        }

#ifdef AFTER_LOOP
                        AFTER_LOOP
#endif
                    }

#else //!INTERP_STW_AFFINE

                    INNER_LOOP(left, right, iy);
#endif


            SKIP_SPAN:
                    iy++;
                    lines--;

                    fxLeftEdge += fdxLeftEdge;
                    fxRightEdge += fdxRightEdge;


                    fError += fdError;
                    if (fError >= 0)
                    {
                        fError -= FIXED_ONE;
#ifdef PIXEL_ADDRESS
                        pRow = (PIXEL_TYPE*)((GLubyte*)pRow + dPRowOuter);
#endif
#if INTERP_Z
                        zRow = (GLdepth*)((GLubyte*)zRow + dZRowOuter);
                        fz += fdzOuter;
#endif
#if INTERP_I
                        fi += fdiOuter;
#endif
#if INTERP_RGB
                        fr += fdrOuter; fg += fdgOuter; fb += fdbOuter;
#endif
#if INTERP_ALPHA
                        fa += fdaOuter;
#endif
#if INTERP_ST
                        fs += fdsOuter; ft += fdtOuter;
#endif
#if INTERP_STW
		                sLeft += dsOuter;
		                tLeft += dtOuter;
		                wLeft += dwOuter;
#endif
                    }
                    else
                    {
#ifdef PIXEL_ADDRESS
                        pRow = (PIXEL_TYPE*)((GLubyte*)pRow + dPRowInner);
#endif
#if INTERP_Z
                        zRow = (GLdepth*)((GLubyte*)zRow + dZRowInner);
                        fz += fdzInner;
#endif
#if INTERP_I
                        fi += fdiInner;
#endif
#if INTERP_RGB
                        fr += fdrInner; fg += fdgInner; fb += fdbInner;
#endif
#if INTERP_ALPHA
                        fa += fdaInner;
#endif
#if INTERP_ST
                        fs += fdsInner; ft += fdtInner;
#endif
#if INTERP_STW
                        sLeft += dsInner;
                        tLeft += dtInner;
                        wLeft += dwInner;
#endif
                    }
                } //while lines>0

            } // for subTriangle

        }
    }
}

#undef SETUP_CODE
#undef INNER_LOOP

#undef PIXEL_TYPE
#undef BYTES_PER_ROW
#undef PIXEL_ADDRESS

#undef INTERP_Z
#undef INTERP_I
#undef INTERP_RGB
#undef INTERP_ALPHA
#undef INTERP_ST
#undef INTERP_STW
#undef INTERP_STW_AFFINE

#undef AFFINE_LOOP0
#undef AFFINE_LOOP1
#undef AFTER_LOOP

#undef AFFINE_LENGTH
#undef AFFINE_SHIFT

#undef S_SCALE
#undef T_SCALE
