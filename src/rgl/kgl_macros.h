#ifndef _iKGL_MACROS_H
#define _iKGL_MACROS_H

#define CLAMP(X, MIN, MAX)  ((X) < (MIN) ? (MIN) : ((X) > (MAX) ? (MAX) : (X)))
#define MEMCPY memcpy
#define MEMSET memset
#define MIN2(X, Y) ((X) < (Y) ? (X) : (Y))

#ifdef _WIN32
#define DLL __declspec(dllexport)
#define API __stdcall
#else
#define DLL
#define API
#endif

#define LOCK_BUFFER(CTX) \
    if (CTX->DriverFuncs.lock_buffer != NULL) \
    { \
        CTX->DriverFuncs.lock_buffer(CTX); \
    }
#define UNLOCK_BUFFER(CTX) \
    if (CTX->DriverFuncs.unlock_buffer != NULL) \
    { \
        CTX->DriverFuncs.unlock_buffer(CTX); \
    }
#define GET_FRAMEBUFFER(CTX) \
    (CTX->DriverFuncs.get_framebuffer != NULL) \
    ? CTX->DriverFuncs.get_framebuffer(CTX) \
    : NULL

#define PROD8(A,B) (((GLint)(A) * (GLint)(B)) >> 8)
#define PROD5(A,B) (((GLint)(A) * (GLint)(B)) >> 8)

#define FORM_RGB555(R,G,B) \
        (((R) & 0xF8) << 7) | (((G) & 0xF8) << 2) | ((B) >> 3)
#define FORM_RGB565(R,G,B) \
        (((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | ((B) >> 3)
#define FORM_RGB32(R,G,B) \
        ((R << 16) | (G << 8) | B)
#define FORM_BGR32(R,G,B) \
        ((B << 16) | (G << 8) | R)
#define FORM_RGB444(R,G,B) \
        (((R) << 8) | ((G) << 4) | (B))
#define FFORM_RGB444(R,G,B) \
        ((((R) & 0xF0) << 4) | ((G) & 0xF0) | (((B) & 0xF0) >> 4))

#define CTX_FB_ADDRESS1(CC,X,Y) \
    CC->FrameBuffer + CC->scrMultByte[Y] + CC->Buffer.ByteMult*(X)
#define CTX_FB_ADDRESS2(CC,X,Y) \
    (GLushort*)(CC->FrameBuffer + CC->scrMultByte[Y] + CC->Buffer.ByteMult*(X))
#define CTX_FB_ADDRESS4(CC,X,Y) \
    (GLuint*)(CC->FrameBuffer + CC->scrMultByte[Y] + CC->Buffer.ByteMult*(X))

//FloatToInt(CLAMP(f, 0, 1) * 255)
#define IEEE_ONE 0x3f7f0000
#define FLOAT_COLOR_TO_UBYTE_COLOR(b, f) \
    { \
        GLfloat tmp = f + 32768.0f; \
        b = ((*(GLuint*)&f >= IEEE_ONE) \
            ? (*(GLint*)&f < 0) ? (GLubyte)0 : (GLubyte)255 \
            : (GLubyte)*(GLuint*)&tmp); \
    }

#define GET_RED32(D)    (((D) & 0xFF0000) >> 16)
#define GET_GREEN32(D)  (((D) & 0xFF00) >> 8)
#define GET_BLUE32(D)   ((D) & 0xFF)

#define GET_RED32R(D)   ((D) & 0xFF)
#define GET_GREEN32R(D) (((D) & 0xFF00) >> 8)
#define GET_BLUE32R(D)  (((D) & 0xFF0000) >> 16)

#define ONE_OVER_255f (1.0f / 255.0f)

#define V3_COPY(DST, SRC) \
    { \
        DST[0] = SRC[0]; \
        DST[1] = SRC[1]; \
        DST[2] = SRC[2]; \
    }

#define V4_COPY(DST, SRC) \
    { \
        DST[0] = SRC[0]; \
        DST[1] = SRC[1]; \
        DST[2] = SRC[2]; \
        DST[3] = SRC[3]; \
    }

#define MAT4_COPY(DST, SRC) MEMCPY(DST, SRC, 16*sizeof(GLfloat))

#define V3_SET(DST, A, B, C) \
    { \
        DST[0] = A; \
        DST[1] = B; \
        DST[2] = C; \
    }

#define V4_SET(DST, A, B, C, D) \
    { \
        DST[0] = A; \
        DST[1] = B; \
        DST[2] = C; \
        DST[3] = D; \
    }

#define TRANSFORM_POINT(Q, M, P) \
    Q[0] = M[0] * P[0] + M[4] * P[1] + M[8]  * P[2] + M[12] * P[3]; \
    Q[1] = M[1] * P[0] + M[5] * P[1] + M[9]  * P[2] + M[13] * P[3]; \
    Q[2] = M[2] * P[0] + M[6] * P[1] + M[10] * P[2] + M[14] * P[3]; \
    Q[3] = M[3] * P[0] + M[7] * P[1] + M[11] * P[2] + M[15] * P[3];

#define TRANSFORM_NORMAL(NX, NY, NZ, N, MAT) \
    NX = N[0] * MAT[0] + N[1] * MAT[1] + N[2] * MAT[2]; \
    NY = N[0] * MAT[4] + N[1] * MAT[5] + N[2] * MAT[6]; \
    NZ = N[0] * MAT[8] + N[1] * MAT[9] + N[2] * MAT[10]; \

#endif
