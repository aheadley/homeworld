/*=============================================================================
    Name    : kgl.c
    Purpose : entrypoints to the (r)GL functions, and GL initialization

    Created 12/??/1997 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "kgl.h"
#include "kgl_macros.h"
#include "maths.h"
#include "clip.h"
#include "asm.h"

#define CALL_VERTEX(x,y,z) CC->DriverFuncs.vertex(x,y,z)

static char DEFAULT_RENDERER[8];
#define TEXTURE_LOG 0

#define SHARED_PALETTES 1
#define NO_PALETTES     0

#define LOG_PROBLEMS    0

/*-----------------------------------------------------------------------------
    data
-----------------------------------------------------------------------------*/


//use malloc / free if WIN_ALLOC is defined
//#undef WIN_ALLOC
#define WIN_ALLOC

//poly stat variables
GLuint g_NumPolys = 0;
GLuint g_CulledPolys = 0;

//useful to determine if we're already shut down
static GLboolean gl_is_shutdown = GL_FALSE;

//a screenshot'll go here
static GLubyte* sbuf = NULL;

//GL_TRUE if the context has been initialized, &c
static GLboolean gl_have_initialized = GL_FALSE;

#define MAX_DEVICES 4

GLint activeDevice = 0;
GLint nDevices = 0;
device_t* devices;
char* deviceToSelect = NULL;

//function pointers for memory de/allocation
static MemAllocFunc gAllocFunc = NULL;
static MemFreeFunc  gFreeFunc  = NULL;

//screen y lookup table
GLint scrMult[MAX_HEIGHT];
GLint zMult[MAX_HEIGHT];
GLint scrMultByte[MAX_HEIGHT];

//GetString(..) strings
#if NO_PALETTES || !SHARED_PALETTES
GLubyte STR_EXTENSIONS[] = "xxxxxxxx GL_RGL_rgl_feature GL_EXT_rescale_normal GL_EXT_paletted_texture GL_RGL_lit_texture_palette";
#else
GLubyte STR_EXTENSIONS[] = "xxxxxxxx GL_RGL_rgl_feature GL_EXT_rescale_normal GL_EXT_paletted_texture GL_EXT_shared_texture_palette GL_RGL_lit_texture_palette";
#endif

GLubyte STR_VENDOR[]     = "Relic Entertainment, Inc. [1.1a]";
GLubyte STR_RENDERER[]   = "rgl   ";    //3 spaces
GLubyte STR_VERSION[]    = "1.1   ";    //3 spaces
GLubyte STR_NOTHING[]    = "[unrecognized string]";

//**THE CONTEXT**  (the only context)
static GLcontext* CC = NULL;

//the latest error goes here
char gl_error_string[128];
GLenum gl_error_num = GL_NO_ERROR;

//our table of texture objects
hashtable* _texobjs = NULL;

//frame counter
GLuint gl_frames = 0;

//identity matrix
GLfloat Identity[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

//simply return CC, the ONLY context we've got
#define gl_get_context() CC

//number of bytes allocated
static GLint gl_allocated = 0;

static GLboolean IsFullscreen = GL_TRUE;
static GLboolean IsTruecolor = GL_FALSE;
static GLboolean IsSlow = GL_FALSE;

static void (*gl_ArrayElement)(GLint i) = NULL;

static void gl_ArrayElement_cnt_3(GLint i);
static void gl_ArrayElement_ct_3(GLint i);
static void gl_ArrayElement_cn_3(GLint i);
static void gl_ArrayElement_c_3(GLint i);
static void gl_ArrayElement_ct(GLint i);
static void gl_ArrayElement_c(GLint i);

static void (*gl_Vertex3f)(GLfloat x, GLfloat y, GLfloat z) = NULL;
static void (*gl_Vertex3fv)(GLfloat const* v) = NULL;
static void (*gl_Vertex4fv)(GLfloat const* v) = NULL;

static void gl_Vertex3f_cnt(GLfloat x, GLfloat y, GLfloat z);
static void gl_Vertex3f_cn(GLfloat x, GLfloat y, GLfloat z);
static void gl_Vertex3f_ct(GLfloat x, GLfloat y, GLfloat z);
static void gl_Vertex3f_c(GLfloat x, GLfloat y, GLfloat z);
static void gl_Vertex3f_driver(GLfloat x, GLfloat y, GLfloat z);

static void gl_Vertex3fv_cnt(GLfloat const*);
static void gl_Vertex3fv_cn(GLfloat const*);
static void gl_Vertex3fv_ct(GLfloat const*);
static void gl_Vertex3fv_c(GLfloat const*);
static void gl_Vertex3fv_driver(GLfloat const*);

static void gl_Vertex4fv_cnt(GLfloat const*);
static void gl_Vertex4fv_cn(GLfloat const*);
static void gl_Vertex4fv_ct(GLfloat const*);
static void gl_Vertex4fv_c(GLfloat const*);

/*-----------------------------------------------------------------------------
    functions
-----------------------------------------------------------------------------*/


#define GET_SCRATCH(CTX) (CTX->DriverFuncs.get_scratch != NULL) ? CTX->DriverFuncs.get_scratch(CTX) : NULL

void gl_shutdown(void);

DLL GLboolean rglGetFullscreen(void) { return IsFullscreen; }
DLL GLboolean rglGetTruecolor(void) { return IsTruecolor; }
DLL GLboolean rglGetSlow(void) { return IsSlow; }

DLL void rglSetRendererString(char* s)
{
    STR_RENDERER[0] = s[0];
    STR_RENDERER[1] = s[1];
    STR_RENDERER[2] = s[2];
}

DLL void rglSetExtensionString(char* s)
{
    GLint i;

    for (i = 0; i < strlen(s); i++)
    {
        STR_EXTENSIONS[i] = s[i];
    }
}


/*-----------------------------------------------------------------------------
    Name        : gl_Allocate
    Description : allocates memory and returns a pointer to it
    Inputs      : size - number of bytes to (attempt to) allocate
    Outputs     :
    Return      : pointer to the new memory, or NULL
----------------------------------------------------------------------------*/
void* gl_Allocate(GLint size)
{
    gl_allocated += size;
#ifdef WIN_ALLOC
    return malloc(size);
#else
    return gAllocFunc(size, "RGL alloc", 8);    //8 == non volatile memory
#endif
}

void* hash_Allocate(GLint size)
{
    gl_allocated += size;
    return gAllocFunc(size, "hash alloc", 4);   //4 == small block heap
}

void hash_Free(void* data)
{
    gFreeFunc(data);
}

/*-----------------------------------------------------------------------------
    Name        : gl_Free
    Description : frees memory
    Inputs      : data - the memory to free
    Outputs     : the memory is freed, or there's a seg fault
    Return      :
----------------------------------------------------------------------------*/
void gl_Free(void* data)
{
#ifdef WIN_ALLOC
    free(data);
#else
    gFreeFunc(data);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : gl_get_context_ext
    Description : returns the context to an external module
    Inputs      :
    Outputs     :
    Return      : the context
----------------------------------------------------------------------------*/
DLL GLcontext* gl_get_context_ext()
{
    return CC;
}

/*-----------------------------------------------------------------------------
    Name        : gl_set_context
    Description : sets the current context, which is also the only context
    Inputs      : cc - the context
    Outputs     : CC is set to cc
    Return      :
----------------------------------------------------------------------------*/
void gl_set_context(GLcontext* cc)
{
    CC = cc;
}

/*-----------------------------------------------------------------------------
    Name        : gl_new_device
    Description : fills in a device structure
    Inputs      : dev - the device structure
                  name - name (handle) of the device
                  aliases - space-separated list of aliases
                  fastAlpha, fastBlend, litPalette - TRUE or FALSE
    Outputs     : dev->name and dev->aliases are allocated,
                  dev is otherwise initialized
    Return      :
----------------------------------------------------------------------------*/
void gl_new_device(device_t* dev, char* name, char* aliases,
                   GLboolean fastAlpha, GLboolean fastBlend,
                   GLboolean litPalette)
{
    GLint len;

    dev->fastAlpha = fastAlpha;
    dev->fastBlend = fastBlend;
    dev->litPalette = litPalette;

    len = strlen(name);
    dev->name = (char*)malloc(len+1);
    //ASSERT(dev->name != NULL)
    strcpy(dev->name, name);

    len = strlen(aliases);
    dev->aliases = (char*)malloc(len+1);
    //ASSERT(dev->name != NULL)
    strcpy(dev->aliases, aliases);
}

/*-----------------------------------------------------------------------------
    Name        : gl_parse_devices
    Description : parses the device file (rparams.dat)
    Inputs      :
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
static GLboolean gl_parse_devices(void)
{
    char line[128];
    GLboolean inDrivers;
    FILE* in;

    in = fopen("rparams.dat", "rt");
    if (in == NULL)
    {
        return GL_FALSE;
    }

    inDrivers = GL_FALSE;

    for (;;)
    {
        if (fgets(line, 128, in) == NULL)
        {
            break;
        }

        if (line[0] == ';')
        {
            continue;
        }

        if (line[0] == '[')
        {
            if (strstr(line, "drivers") != NULL)
            {
                inDrivers = GL_TRUE;
                continue;
            }
        }

        if (inDrivers)
        {
            char dll[16], name[16], aliases[128];
            char alpha, blend, prelit;

            if (strstr(line, "default") != NULL)
            {
                sscanf(line, "%s %s", dll, name);
                strcpy(DEFAULT_RENDERER, name);
                continue;
            }

            if (strstr(line, "dll ") == NULL)
            {
                continue;
            }

            sscanf(line, "%s %s %c %c %c %s",
                   dll, name, &alpha, &blend, &prelit, aliases);

            gl_new_device(&devices[nDevices], name, aliases,
                          (GLboolean)(alpha == 'A'),
                          (GLboolean)(blend == 'B'),
                          (GLboolean)(prelit == 'L'));
            nDevices++;
        }
    }

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : gl_load_devices
    Description : prepare the device list
    Inputs      :
    Outputs     : device list (devices) is initialized
    Return      : TRUE (success) or FALSE (failure)
----------------------------------------------------------------------------*/
GLboolean gl_load_devices()
{
    devices = (device_t*)malloc(MAX_DEVICES * sizeof(device_t));
    //ASSERT(devices != NULL)
    MEMSET(devices, 0, MAX_DEVICES * sizeof(device_t));

//    if (!gl_parse_devices())
    {
        nDevices = 2;//3;
        gl_new_device(&devices[0], "sw", "software_kgl", GL_FALSE, GL_FALSE, GL_TRUE);
        gl_new_device(&devices[1], "d3d", "d3d_direct3d_directaxe_ms", GL_TRUE, GL_TRUE, GL_FALSE);
//        gl_new_device(&devices[2], "fx", "glide_glide2_glide2x_3dfx_accel", GL_TRUE, GL_TRUE, GL_FALSE);
        strcpy(DEFAULT_RENDERER, "sw");
    }

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : gl_free_devices
    Description : frees memory allocated to the device list
    Inputs      :
    Outputs     : device list (devices) is freed
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean gl_free_devices()
{
    GLint index;
    device_t* dev;

    for (index = 0, dev = devices; index < nDevices; index++, dev++)
    {
        if (dev->name != NULL)
        {
            free(dev->name);
        }
        if (dev->aliases != NULL)
        {
            free(dev->aliases);
        }
    }

    free(devices);
    devices = NULL;

    nDevices = 0;

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : gl_select_device
    Description : make a device the active device
    Inputs      : index - position in device list
    Outputs     : activeDevice reflects the currently selected device
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean gl_select_device(GLint index)
{
    if (index >= 0 && index < nDevices)
    {
        activeDevice = index;
        return GL_TRUE;
    }
    else
    {
        return GL_FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : gl_select_named_device
    Description : make a device the active device, if it exists
    Inputs      : name - name of the device to select
    Outputs     : activeDevice reflects the currently selected device
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean gl_select_named_device(char* name)
{
    GLint index;

    for (index = 0; index < nDevices; index++)
    {
        if (strcmp(devices[index].name, name) == 0)
        {
            return gl_select_device(index);
        }
    }

    return GL_FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : gl_next_device
    Description : selects the next device in the device list
    Inputs      :
    Outputs     : activeDevice
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean gl_next_device()
{
    return gl_select_device((activeDevice + 1) % nDevices);
}

/*-----------------------------------------------------------------------------
    Name        : gl_prev_device
    Description : selects the previous device in the device list
    Inputs      :
    Outputs     : activeDevice
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean gl_prev_device()
{
    return gl_select_device((activeDevice == 0) ? nDevices - 1 : activeDevice--);
}

/*-----------------------------------------------------------------------------
    Name        : gl_update_vertexfunc
    Description : decides which glVertex3f to use
    Inputs      :
    Outputs     :
    Return      :
    Notes       : FIXME - colours need to be copied always due to non-GL standard
                  lighting models
----------------------------------------------------------------------------*/
static void gl_update_vertexfunc()
{
    GLcontext* ctx = CC;

    if (ctx->DriverTransforms)
    {
        gl_Vertex3f = gl_Vertex3f_driver;
        gl_Vertex3fv = gl_Vertex3fv_driver;
        if (ctx->VertexSize == 3)
        {
            gl_ArrayElement = gl_ArrayElement_cnt_3;
        }
        else
        {
            gl_ArrayElement = gl_ArrayElement_ct;
        }
        return;
    }

    if (ctx->Lighting)
    {
        if (ctx->TexEnabled)
        {
            gl_Vertex3f = gl_Vertex3f_cnt;
            gl_Vertex3fv = gl_Vertex3fv_cnt;
            gl_Vertex4fv = gl_Vertex4fv_cnt;
            if (ctx->VertexSize == 3)
            {
                gl_ArrayElement = gl_ArrayElement_cnt_3;
            }
            else
            {
                gl_ArrayElement = gl_ArrayElement_ct;
            }
        }
        else
        {
            gl_Vertex3f = gl_Vertex3f_cn;
            gl_Vertex3fv = gl_Vertex3fv_cn;
            gl_Vertex4fv = gl_Vertex4fv_cn;
            if (ctx->VertexSize == 3)
            {
                gl_ArrayElement = gl_ArrayElement_cn_3;
            }
            else
            {
                gl_ArrayElement = gl_ArrayElement_c;
            }
        }
    }
    else
    {
        if (ctx->TexEnabled)
        {
            gl_Vertex3f = gl_Vertex3f_ct;
            gl_Vertex3fv = gl_Vertex3fv_ct;
            gl_Vertex4fv = gl_Vertex4fv_ct;
            if (ctx->VertexSize == 3)
            {
                gl_ArrayElement = gl_ArrayElement_ct_3;
            }
            else
            {
                gl_ArrayElement = gl_ArrayElement_ct;
            }
        }
        else
        {
            gl_Vertex3f = gl_Vertex3f_c;
            gl_Vertex3fv = gl_Vertex3fv_c;
            gl_Vertex4fv = gl_Vertex4fv_c;
            if (ctx->VertexSize == 3)
            {
                gl_ArrayElement = gl_ArrayElement_c_3;
            }
            else
            {
                gl_ArrayElement = gl_ArrayElement_c;
            }
        }
    }
}

static void gl_Vertex3f_driver(GLfloat x, GLfloat y, GLfloat z)
{
    CALL_VERTEX(x, y, z);
}

static void gl_Vertex3fv_driver(GLfloat const* v)
{
    CALL_VERTEX(v[0], v[1], v[2]);
}

static void gl_Vertex3f_cnt(GLfloat x, GLfloat y, GLfloat z)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = x;
    VB->Obj[count][1] = y;
    VB->Obj[count][2] = z;
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->Normal[count][0] = ctx->Current.Normal[0];
    VB->Normal[count][1] = ctx->Current.Normal[1];
    VB->Normal[count][2] = ctx->Current.Normal[2];

    VB->TexCoord[count][0] = ctx->Current.TexCoord[0];
    VB->TexCoord[count][1] = ctx->Current.TexCoord[1];

    count++;
    VB->Count = count;
}

static void gl_Vertex3f_cn(GLfloat x, GLfloat y, GLfloat z)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = x;
    VB->Obj[count][1] = y;
    VB->Obj[count][2] = z;
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->Normal[count][0] = ctx->Current.Normal[0];
    VB->Normal[count][1] = ctx->Current.Normal[1];
    VB->Normal[count][2] = ctx->Current.Normal[2];

    count++;
    VB->Count = count;
}

static void gl_Vertex3f_ct(GLfloat x, GLfloat y, GLfloat z)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = x;
    VB->Obj[count][1] = y;
    VB->Obj[count][2] = z;
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->TexCoord[count][0] = ctx->Current.TexCoord[0];
    VB->TexCoord[count][1] = ctx->Current.TexCoord[1];

    count++;
    VB->Count = count;
}

static void gl_Vertex3f_c(GLfloat x, GLfloat y, GLfloat z)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = x;
    VB->Obj[count][1] = y;
    VB->Obj[count][2] = z;
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    count++;
    VB->Count = count;
}

static void gl_Vertex3fv_cnt(GLfloat const* v)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = v[0];
    VB->Obj[count][1] = v[1];
    VB->Obj[count][2] = v[2];
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->Normal[count][0] = ctx->Current.Normal[0];
    VB->Normal[count][1] = ctx->Current.Normal[1];
    VB->Normal[count][2] = ctx->Current.Normal[2];

    VB->TexCoord[count][0] = ctx->Current.TexCoord[0];
    VB->TexCoord[count][1] = ctx->Current.TexCoord[1];

    count++;
    VB->Count = count;
}

static void gl_Vertex3fv_cn(GLfloat const* v)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = v[0];
    VB->Obj[count][1] = v[1];
    VB->Obj[count][2] = v[2];
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->Normal[count][0] = ctx->Current.Normal[0];
    VB->Normal[count][1] = ctx->Current.Normal[1];
    VB->Normal[count][2] = ctx->Current.Normal[2];

    count++;
    VB->Count = count;
}

static void gl_Vertex3fv_ct(GLfloat const* v)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = v[0];
    VB->Obj[count][1] = v[1];
    VB->Obj[count][2] = v[2];
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->TexCoord[count][0] = ctx->Current.TexCoord[0];
    VB->TexCoord[count][1] = ctx->Current.TexCoord[1];

    count++;
    VB->Count = count;
}

static void gl_Vertex3fv_c(GLfloat const* v)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = v[0];
    VB->Obj[count][1] = v[1];
    VB->Obj[count][2] = v[2];
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    count++;
    VB->Count = count;
}

static void gl_Vertex4fv_cnt(GLfloat const* v)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Clip[count][0] = v[0];
    VB->Clip[count][1] = v[1];
    VB->Clip[count][2] = v[2];
    VB->Clip[count][3] = v[3];

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->Normal[count][0] = ctx->Current.Normal[0];
    VB->Normal[count][1] = ctx->Current.Normal[1];
    VB->Normal[count][2] = ctx->Current.Normal[2];

    VB->TexCoord[count][0] = ctx->Current.TexCoord[0];
    VB->TexCoord[count][1] = ctx->Current.TexCoord[1];

    count++;
    VB->Count = count;
}

static void gl_Vertex4fv_cn(GLfloat const* v)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Clip[count][0] = v[0];
    VB->Clip[count][1] = v[1];
    VB->Clip[count][2] = v[2];
    VB->Clip[count][3] = v[3];

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->Normal[count][0] = ctx->Current.Normal[0];
    VB->Normal[count][1] = ctx->Current.Normal[1];
    VB->Normal[count][2] = ctx->Current.Normal[2];

    count++;
    VB->Count = count;
}

static void gl_Vertex4fv_ct(GLfloat const* v)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Clip[count][0] = v[0];
    VB->Clip[count][1] = v[1];
    VB->Clip[count][2] = v[2];
    VB->Clip[count][3] = v[3];

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->TexCoord[count][0] = ctx->Current.TexCoord[0];
    VB->TexCoord[count][1] = ctx->Current.TexCoord[1];

    count++;
    VB->Count = count;
}

static void gl_Vertex4fv_c(GLfloat const* v)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Clip[count][0] = v[0];
    VB->Clip[count][1] = v[1];
    VB->Clip[count][2] = v[2];
    VB->Clip[count][3] = v[3];

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    count++;
    VB->Count = count;
}

/*-----------------------------------------------------------------------------
    Name        : gl_lock_framebuffer
    Description : locks the driver's framebuffer if necessary, and not already so
    Inputs      :
    Outputs     : ctx->FrameBuffer == framebuffer
    Return      :
----------------------------------------------------------------------------*/
static void gl_lock_framebuffer()
{
    if (CC->RequireLocking && !CC->ExclusiveLock)
    {
        LOCK_BUFFER(CC);
        CC->FrameBuffer = GET_FRAMEBUFFER(CC);
    }
}

/*-----------------------------------------------------------------------------
    Name        : gl_unlock_framebuffer
    Description : unlocks the driver's framebuffer if an exclusive lock is not held
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void gl_unlock_framebuffer()
{
    if (CC->RequireLocking && !CC->ExclusiveLock)
    {
        UNLOCK_BUFFER(CC);
    }
}

/*-----------------------------------------------------------------------------
    Name        : gl_update_raster
    Description : call driver functions to update rasterization state
    Inputs      : ctx - the context
    Outputs     :
    Return      :
    State       : clear NEW_RASTER
----------------------------------------------------------------------------*/
void gl_update_raster(GLcontext* ctx)
{
    ctx->NewMask &= ~(NEW_RASTER);

    //call driver setup_* funcs
    if (ctx->DriverFuncs.setup_raster != NULL)
    {
        ctx->DriverFuncs.setup_raster(ctx);
    }
    if (ctx->DriverFuncs.setup_triangle != NULL)
    {
        ctx->DriverFuncs.setup_triangle(ctx);
    }
    if (ctx->DriverFuncs.setup_line != NULL)
    {
        ctx->DriverFuncs.setup_line(ctx);
    }
    if (ctx->DriverFuncs.setup_point != NULL)
    {
        ctx->DriverFuncs.setup_point(ctx);
    }
}

/*-----------------------------------------------------------------------------
    Name        : gl_update_lighting
    Description : updates lighting state, precomputes useful stuff
    Inputs      : ctx - the context
    Outputs     :
    Return      :
    State       : clear NEW_LIGHTING
----------------------------------------------------------------------------*/
void gl_update_lighting(GLcontext* ctx)
{
    GLint i, side, sides;
    GLfloat brightness;
    gl_light* light;

    if (!ctx->Lighting)
            return;

    brightness = ctx->GammaAdjust * 127.0f;

    ctx->ActiveLight = NULL;
    for (i = 0; i < MAX_LIGHTS; i++)
    {
        if (ctx->Light[i].Enabled)
        {
            if (ctx->ActiveLight == NULL)
            {
                ctx->ActiveLight = &ctx->Light[i];
                ctx->ActiveLight->next = NULL;
            }
            else
            {
                ctx->Light[i].next = ctx->ActiveLight;
                ctx->ActiveLight = &ctx->Light[i];
            }
        }
    }

    ctx->NewMask &= ~(NEW_LIGHTING);

    if (ctx->TwoSide)
    {
        ctx->ClipMask = CLIP_COLOR_BITS;
    }
    else
    {
        ctx->ClipMask = CLIP_FCOLOR_BIT;
    }

    sides = (ctx->TwoSide) ? 2 : 1;

    for (side = 0; side < sides; side++)
    {
        ctx->BaseColor[side][0] = ctx->Ambient[0] * ctx->Material[side].Ambient[0];
        ctx->BaseColor[side][1] = ctx->Ambient[1] * ctx->Material[side].Ambient[1];
        ctx->BaseColor[side][2] = ctx->Ambient[2] * ctx->Material[side].Ambient[2];
        ctx->BaseColor[side][3] = CLAMP(ctx->Material[side].Diffuse[3], 0.0f, 1.0f);
    }

    for (light = ctx->ActiveLight; light != NULL; light = light->next)
    {
        v3_copy(light->VP_inf_norm, light->Position);
        v3_normalize(light->VP_inf_norm);
        for (side = 0; side < sides; side++)
        {
            gl_material* mat = &ctx->Material[side];
            //add ambient components
            ctx->BaseColor[side][0] += light->Ambient[0] * mat->Ambient[0];
            ctx->BaseColor[side][1] += light->Ambient[1] * mat->Ambient[1];
            ctx->BaseColor[side][2] += light->Ambient[2] * mat->Ambient[2];
            //light's diffuse with mat's
            light->MatDiffuse[side][0] = light->Diffuse[0] * mat->Diffuse[0];
            light->MatDiffuse[side][1] = light->Diffuse[1] * mat->Diffuse[1];
            light->MatDiffuse[side][2] = light->Diffuse[2] * mat->Diffuse[2];
#if 0
            //light's ambient with mat's
            light->MatAmbient[side][0] = light->Ambient[0] * mat->Ambient[0];
            light->MatAmbient[side][1] = light->Ambient[1] * mat->Ambient[1];
            light->MatAmbient[side][2] = light->Ambient[2] * mat->Ambient[2];
#endif
#if 0
            //light's specular with mat's
            light->MatSpecular[side][0] = light->Specular[0] * mat->Specular[0];
            light->MatSpecular[side][1] = light->Specular[1] * mat->Specular[1];
            light->MatSpecular[side][2] = light->Specular[2] * mat->Specular[2];
#endif
        }
    }

    for (i = 0; i < 4; i++)
    {
        ctx->BaseColor[0][i] *= 255.0f;
        ctx->BaseColor[1][i] *= 255.0f;
    }
    for (i = 0; i < 3; i++)
    {
        ctx->BaseColor[0][i] += brightness;
        ctx->BaseColor[1][i] += brightness;
    }

    ctx->FastLighting = GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : glFrustum
    Description : sets up a viewing frustum
    Inputs      : [as per spec]
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glFrustum(GLdouble left, GLdouble right,
                       GLdouble bottom, GLdouble top,
                       GLdouble zNear, GLdouble zFar)
{
    GLfloat x, y, a, b, c, d;
    GLfloat m[16];
    GLcontext* ctx = CC;

    if (zNear <= 0.0 || zFar <= 0.0)
    {
        gl_error(ctx, GL_INVALID_VALUE, "glFrustum(near or far)");
    }

    x = (GLfloat)((2.0*zNear) / (right - left));
    y = (GLfloat)((2.0*zNear) / (top - bottom));
    a = (GLfloat)((right + left) / (right - left));
    b = (GLfloat)((top + bottom) / (top - bottom));
    c = (GLfloat)(-(zFar + zNear) / (zFar - zNear));
    d = (GLfloat)(-(2.0 * zFar * zNear) / (zFar - zNear));

#define M(row,col) m[(4*col)+row]
    M(0,0) = x;    M(0,1) = 0.0f; M(0,2) = a;     M(0,3) = 0.0f;
    M(1,0) = 0.0f; M(1,1) = y;    M(1,2) = b;     M(1,3) = 0.0f;
    M(2,0) = 0.0f; M(2,1) = 0.0f; M(2,2) = c;     M(2,3) = d;
    M(3,0) = 0.0f; M(3,1) = 0.0f; M(3,2) = -1.0f; M(3,3) = 0.0f;
#undef M

    glMultMatrixf(m);
}

/*-----------------------------------------------------------------------------
    Name        : gl_error
    Description : update the error variables
    Inputs      : ctx - the context
                  err - the enumeration of the error
                  s - the error string
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void gl_error(GLcontext* ctx, GLenum err, char* s)
{
    strncpy(gl_error_string, s, 63);
    gl_error_num = err;
}

/*-----------------------------------------------------------------------------
    Name        : gl_new_problemfile
    Description : creates a new problem file for errors that don't go to gl_error
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gl_new_problemfile()
{
#if LOG_PROBLEMS
    FILE* problems = fopen("problems.dat", "wt");
    if (problems != NULL)
    {
        fclose(problems);
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : gl_problem
    Description : log a new problem
    Inputs      : ctx - the context
                  s - the problem string
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void gl_problem(GLcontext* ctx, char* s)
{
#if LOG_PROBLEMS
    FILE* problems = fopen("problems.dat", "at");
    if (problems != NULL)
    {
        fprintf(problems, "GL problem: %s\n", s);
        fclose(problems);
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : glMatrixMode
    Description : set the current matrix mode
    Inputs      : mode - only MODELVIEW or PROJECTION supported
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glMatrixMode(GLenum mode)
{
    switch (mode)
    {
    case GL_MODELVIEW:
    case GL_PROJECTION:
        CC->MatrixMode = mode;
        break;
    default:
        gl_error(CC, GL_INVALID_ENUM, "glMatrixMode");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glPushMatrix
    Description : push a matrix onto the stack.  MODELVIEW, PROJECTION states only
    Inputs      :
    Outputs     :
    Return      :
    State       : NEW_MODELVIEW, NEW_MODELVIEWINV, NEW_PROJECTION
----------------------------------------------------------------------------*/
DLL void API glPushMatrix()
{
    GLcontext* ctx = CC;

    switch (ctx->MatrixMode)
    {
    case GL_MODELVIEW:
        if (ctx->ModelViewStackDepth >= MAX_MODELVIEW_STACK_DEPTH - 1)
        {
            gl_error(ctx, GL_STACK_OVERFLOW, "glPushMatrix modelview overflow");
            return;
        }
        MAT4_COPY(ctx->ModelViewStack[ctx->ModelViewStackDepth], ctx->ModelViewMatrix);
        ctx->ModelViewStackDepth++;
        ctx->NewMask |= NEW_MODELVIEW | NEW_MODELVIEWINV;
        break;
    case GL_PROJECTION:
        if (ctx->ProjectionStackDepth >= MAX_PROJECTION_STACK_DEPTH)
        {
            gl_error(ctx, GL_STACK_OVERFLOW, "glPushMatrix projection overflow");
            return;
        }
        MAT4_COPY(ctx->ProjectionStack[ctx->ProjectionStackDepth], ctx->ProjectionMatrix);
        ctx->ProjectionStackDepth++;
        ctx->NewMask |= NEW_PROJECTION;

        /* do nothing with near/far vals */

        break;
    default:
        gl_problem(ctx, "Bad matrix mode in glPushMatrix");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glPopMatrix
    Description : pop a matrix from the stack.  MODELVIEW, PROJECTION states only
    Inputs      :
    Outputs     :
    Return      :
    State       : NEW_MODELVIEW, NEW_MODELVIEWINV, NEW_PROJECTION
----------------------------------------------------------------------------*/
DLL void API glPopMatrix()
{
    GLcontext* ctx = CC;

    switch (ctx->MatrixMode)
    {
    case GL_MODELVIEW:
        if (ctx->ModelViewStackDepth == 0)
        {
            gl_error(ctx, GL_STACK_UNDERFLOW, "glPopMatrix modelview underflow");
            return;
        }
        ctx->ModelViewStackDepth--;
        MAT4_COPY(ctx->ModelViewMatrix, ctx->ModelViewStack[ctx->ModelViewStackDepth]);
        ctx->NewMask |= NEW_MODELVIEW | NEW_MODELVIEWINV;
        break;
    case GL_PROJECTION:
        if (ctx->ProjectionStackDepth == 0)
        {
            gl_error(ctx, GL_STACK_UNDERFLOW, "glPopMatrix projection underflow");
            return;
        }
        ctx->ProjectionStackDepth--;
        MAT4_COPY(ctx->ProjectionMatrix, ctx->ProjectionStack[ctx->ProjectionStackDepth]);
        ctx->NewMask |= NEW_PROJECTION;

        /* ignore near/far vals */

        break;
    default:
        gl_problem(ctx, "Bad matrix mode in gl_PopMatrix");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glPushAttrib
    Description : a very limited implementation of the spec, but point / line
                  sizes are saved
    Inputs      : attrib - GL_POINT_BIT or GL_LINE_BIT
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glPushAttrib(GLenum attrib)
{
    GLcontext* ctx = CC;

    if (ctx->AttribStackDepth >= MAX_ATTRIB_STACK_DEPTH - 1)
    {
        gl_error(ctx, GL_STACK_OVERFLOW, "glPushAttrib overflow");
        return;
    }

    switch (attrib)
    {
    case GL_POINT_BIT:
        ctx->AttribStack[ctx->AttribStackDepth].type = GL_POINT_BIT;
        ctx->AttribStack[ctx->AttribStackDepth].val.fvalue = ctx->PointSize;
        break;

    case GL_LINE_BIT:
        ctx->AttribStack[ctx->AttribStackDepth].type = GL_LINE_BIT;
        ctx->AttribStack[ctx->AttribStackDepth].val.fvalue = ctx->LineWidth;
        break;
    default:
        return;
    }

    ctx->AttribStackDepth++;
}

/*-----------------------------------------------------------------------------
    Name        : glPopAttrib
    Description : a very limited implementation of the spec, but point / line
                  sizes are restored
    Inputs      :
    Outputs     : the POINT or LINE state may be updated
    Return      :
    State       : NEW_RASTER
----------------------------------------------------------------------------*/
DLL void API glPopAttrib()
{
    GLcontext* ctx = CC;

    if (ctx->AttribStackDepth == 0)
    {
        gl_error(ctx, GL_STACK_UNDERFLOW, "glPopAttrib");
        return;
    }
    ctx->AttribStackDepth--;

    switch (ctx->AttribStack[ctx->AttribStackDepth].type)
    {
    case GL_POINT_BIT:
        ctx->PointSize = ctx->AttribStack[ctx->AttribStackDepth].val.fvalue;
        break;

    case GL_LINE_BIT:
        ctx->LineWidth = ctx->AttribStack[ctx->AttribStackDepth].val.fvalue;
        break;
    }

    ctx->NewMask |= NEW_RASTER;
}

/*-----------------------------------------------------------------------------
    Name        : glLoadIdentity
    Description : load an identity matrix
    Inputs      :
    Outputs     : an identity matrix is loaded as current MODELVIEW or PROJECTION
    Return      :
    State       : NEW_MODELVIEW, NEW_MODELVIEWINV, NEW_PROJECTION
----------------------------------------------------------------------------*/
DLL void API glLoadIdentity()
{
    GLcontext* ctx = CC;
    switch (ctx->MatrixMode)
    {
    case GL_MODELVIEW:
        MAT4_COPY(ctx->ModelViewMatrix, Identity);
        ctx->NewMask |= NEW_MODELVIEW | NEW_MODELVIEWINV;
        break;
    case GL_PROJECTION:
        MAT4_COPY(ctx->ProjectionMatrix, Identity);
        ctx->NewMask |= NEW_PROJECTION;
        break;
    default:
        gl_problem(ctx, "Bad matrix mode in glLoadIdentity");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glScalef
    Description :
    Inputs      : x, y, z - scale factors
    Outputs     : current matrix is scaled
    Return      :
    State       : NEW_MODELVIEW, NEW_MODELVIEWINV, NEW_PROJECTION
----------------------------------------------------------------------------*/
DLL void API glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    GLcontext* ctx = CC;
    GLfloat* m;

    switch (ctx->MatrixMode)
    {
    case GL_MODELVIEW:
        m = ctx->ModelViewMatrix;
        ctx->NewMask |= NEW_MODELVIEW | NEW_MODELVIEWINV;
        break;
    case GL_PROJECTION:
        m = ctx->ProjectionMatrix;
        ctx->NewMask |= NEW_PROJECTION;
        break;
    default:
        gl_problem(ctx, "bad matrix mode in glScalef");
        return;
    }
    m[0] *= x;  m[4] *= y;  m[8]  *= z;
    m[1] *= x;  m[5] *= y;  m[9]  *= z;
    m[2] *= x;  m[6] *= y;  m[10] *= z;
    m[3] *= x;  m[7] *= y;  m[11] *= z;
}

/*-----------------------------------------------------------------------------
    Name        : glLoadMatrixf
    Description : directly load a matrix
    Inputs      : m - the matrix to load
    Outputs     : the matrix is replaced
    Return      :
    State       : NEW_MODELVIEW, NEW_MODELVIEWINV, NEW_PROJECTION
----------------------------------------------------------------------------*/
DLL void API glLoadMatrixf(GLfloat const* m)
{
    GLcontext* ctx = CC;
    switch (ctx->MatrixMode)
    {
    case GL_MODELVIEW:
        MAT4_COPY(ctx->ModelViewMatrix, m);
        ctx->NewMask |= NEW_MODELVIEW | NEW_MODELVIEWINV;
        break;
    case GL_PROJECTION:
        MAT4_COPY(ctx->ProjectionMatrix, m);
        ctx->NewMask |= NEW_PROJECTION;
        break;
    default:
        gl_problem(ctx, "Bad matrix mode in glLoadMatrixf");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glMultMatrixd
    Description : converts to a matrix of GLfloat's then calls glMultMatrixf
    Inputs      : m - the matrix
    Outputs     : current matrix is updated with result
    Return      :
----------------------------------------------------------------------------*/
DLL void API glMultMatrixd(GLdouble const* m)
{
#if 0
    GLcontext* ctx = CC;
    GLdouble   mat[16];
    GLint i;
    switch (ctx->MatrixMode)
    {
    case GL_MODELVIEW:
        for (i = 0; i < 16; i++)
        {
            mat[i] = (GLdouble)ctx->ModelViewMatrix[i];
        }
        mat4_multd(mat, mat, m);
        for (i = 0; i < 16; i++)
        {
            ctx->ModelViewMatrix[i] = (GLfloat)mat[i];
        }
        ctx->NewMask |= NEW_MODELVIEW | NEW_MODELVIEWINV;
        break;
    case GL_PROJECTION:
        for (i = 0; i < 16; i++)
        {
            mat[i] = (GLdouble)ctx->ProjectionMatrix[i];
        }
        mat4_multd(mat, mat, m);
        for (i = 0; i < 16; i++)
        {
            ctx->ProjectionMatrix[i] = (GLfloat)mat[i];
        }
        ctx->NewMask |= NEW_PROJECTION;
        break;
    default:
        gl_problem(ctx, "Bad matrix mode in glMultMatrixd");
    }
#else
    int i;
    GLfloat mf[16];
    for (i = 0; i < 16; i++)
        mf[i] = (GLfloat)m[i];
    glMultMatrixf(mf);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : glMultMatrixf
    Description : multiply current matrix by given matrix and store the result
    Inputs      : m - the matrix
    Outputs     : current matrix is updated with result
    Return      :
    State       : NEW_MODELVIEW, NEW_MODELVIEWINV, NEW_PROJECTION
----------------------------------------------------------------------------*/
DLL void API glMultMatrixf(GLfloat const* m)
{
    GLcontext* ctx = CC;
    switch (ctx->MatrixMode)
    {
    case GL_MODELVIEW:
        mat4_mult(ctx->ModelViewMatrix, ctx->ModelViewMatrix, m);
        ctx->NewMask |= NEW_MODELVIEW | NEW_MODELVIEWINV;
        break;
    case GL_PROJECTION:
        mat4_mult(ctx->ProjectionMatrix, ctx->ProjectionMatrix, m);
        ctx->NewMask |= NEW_PROJECTION;
        break;
    default:
        gl_problem(ctx, "Bad matrix mode in glMultMatrixf");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glRotatef
    Description : create a rotation matrix then multiply against current matrix
                  w/ glMultMatrixf
    Inputs      : angle - angle in degrees
                  x, y, z - axis vectors
    Outputs     : matrix is updated
    Return      :
----------------------------------------------------------------------------*/
DLL void API glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat m[16];
    GLfloat axis[3];
    V3_SET(axis, x,y,z);
    mat4_rotation(m, axis, angle);
    glMultMatrixf(m);
}

/*-----------------------------------------------------------------------------
    Name        : glTranslatef
    Description : create a translation matrix then multiply against current matrix
    Inputs      : x, y, z - components of translation
    Outputs     : matrix is updated
    Return      :
    State       : NEW_MODELVIEW, NEW_MODELVIEWINV, NEW_PROJECTION
----------------------------------------------------------------------------*/
DLL void API glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    GLcontext* ctx = CC;
    GLfloat* m;

    switch (ctx->MatrixMode)
    {
    case GL_MODELVIEW:
        m = ctx->ModelViewMatrix;
        ctx->NewMask |= NEW_MODELVIEW | NEW_MODELVIEWINV;
        break;
    case GL_PROJECTION:
        m = ctx->ProjectionMatrix;
        ctx->NewMask |= NEW_PROJECTION;
        break;
    default:
        gl_problem(ctx, "Bad matrix mode in glTranslatef");
        return;
    }

    m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
    m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
    m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
    m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];
}

/*-----------------------------------------------------------------------------
    Name        : glTranslated
    Description : call glTranslatef with converted parameters
    Inputs      : x, y, z - components of translation
    Outputs     : matrix is updated
    Return      :
----------------------------------------------------------------------------*/
DLL void API glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

/*-----------------------------------------------------------------------------
    Name        : glViewport
    Description : sets up the viewport transformation (clip -> 2D)
    Inputs      : [as per spec]
    Outputs     : ctx->Viewport is updated
    Return      :
----------------------------------------------------------------------------*/
DLL void API glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    GLcontext* ctx = CC;

    if (width < 0 || height < 0)
    {
        gl_error(ctx, GL_INVALID_VALUE, "glViewport");
        return;
    }

    width  = CLAMP(width,  1, MAX_WIDTH);
    height = CLAMP(height, 1, MAX_HEIGHT);

    ctx->Viewport.X = x;
    ctx->Viewport.Width = width;
    ctx->Viewport.Y = y;
    ctx->Viewport.Height = height;

    ctx->Viewport.Sx = (GLfloat)width / 2.0f;
    ctx->Viewport.Tx = ctx->Viewport.Sx + x;
    ctx->Viewport.Sy = (GLfloat)height / 2.0f;
    ctx->Viewport.Ty = ctx->Viewport.Sy + y;
}

/*-----------------------------------------------------------------------------
    Name        : glCullFace
    Description : sets the current mode for face culling
    Inputs      : mode - FRONT, BACK, FRONT_AND_BACK
    Outputs     : ctx->CullFaceMode is updated
    Return      :
----------------------------------------------------------------------------*/
DLL void API glCullFace(GLenum mode)
{
    GLcontext* ctx = CC;

    if (mode != GL_FRONT && mode != GL_BACK && mode != GL_FRONT_AND_BACK)
    {
        gl_error(ctx, GL_INVALID_ENUM, "glCullFace");
        return;
    }

    ctx->CullFaceMode = mode;
}

/*-----------------------------------------------------------------------------
    Name        : glShadeModel
    Description : sets the current polygon shading model
    Inputs      : mode - the shading model, FLAT or SMOOTH
    Outputs     : ctx->ShadeModel may be modified
    Return      :
    State       : NEW_RASTER
----------------------------------------------------------------------------*/
DLL void API glShadeModel(GLenum mode)
{
    GLcontext* ctx = CC;
    if (ctx->ShadeModel == mode)
    {
        return;
    }

    switch (mode)
    {
    case GL_FLAT:
    case GL_SMOOTH:
        ctx->ShadeModel = mode;
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glShadeModel");
    }

    ctx->NewMask |= NEW_RASTER;
}

/*-----------------------------------------------------------------------------
    Name        : gl_set_material
    Description :
    Inputs      : bitmask - specifies which parameters are to be updated
                  params - the parameters
    Outputs     : ctx->Material may be updated
    Return      :
    State       : NEW_LIGHTING
----------------------------------------------------------------------------*/
void gl_set_material(GLuint bitmask, GLfloat const* params)
{
    GLcontext* ctx = CC;
    gl_material* mat;
    GLfloat shininess;

    mat = ctx->Material;

    if (bitmask & FRONT_AMBIENT_BIT)
        V4_COPY(mat[0].Ambient, params);
    if (bitmask & BACK_AMBIENT_BIT)
        V4_COPY(mat[1].Ambient, params);
    if (bitmask & FRONT_DIFFUSE_BIT)
        V4_COPY(mat[0].Diffuse, params);
    if (bitmask & BACK_DIFFUSE_BIT)
        V4_COPY(mat[1].Diffuse, params);
    if (bitmask & FRONT_SPECULAR_BIT)
        V4_COPY(mat[0].Specular, params);
    if (bitmask & BACK_SPECULAR_BIT)
        V4_COPY(mat[1].Specular, params);
    if (bitmask & FRONT_SHININESS_BIT)
    {
        shininess = CLAMP(params[0], 0.0f, 128.0f);
        mat[0].Shininess = shininess;
    }
    if (bitmask & BACK_SHININESS_BIT)
    {
        shininess = CLAMP(params[0], 0.0f, 128.0f);
        mat[1].Shininess = shininess;
    }

    ctx->NewMask |= NEW_LIGHTING;
}

/*-----------------------------------------------------------------------------
    Name        : gl_material_bitmask
    Description : sets up a bitmask for gl_set_material
    Inputs      : face - [ignored]
                  pname - which material property to add to the bitmask
    Outputs     :
    Return      : the bitmask
    Deviation   : always sets both FRONT and BACK faces
----------------------------------------------------------------------------*/
GLuint gl_material_bitmask(GLenum face, GLenum pname)
{
    GLuint bitmask = 0;
    switch (pname)
    {
    case GL_AMBIENT:
        bitmask |= FRONT_AMBIENT_BIT | BACK_AMBIENT_BIT;
        break;
    case GL_DIFFUSE:
        bitmask |= FRONT_DIFFUSE_BIT | BACK_DIFFUSE_BIT;
        break;
    case GL_SPECULAR:
        bitmask |= FRONT_SPECULAR_BIT | BACK_SPECULAR_BIT;
        break;
    case GL_SHININESS:
        bitmask |= FRONT_SHININESS_BIT | BACK_SHININESS_BIT;
        break;
    default:
        gl_problem(NULL, "bad param in gl_material_bitmask");
        return 0;
    }

    return bitmask;
}

/*-----------------------------------------------------------------------------
    Name        : glMaterialfv
    Description :
    Inputs      : face - FRONT, BACK, FRONT_AND_BACK
                  pname - the material property to update
                  params - the parameters of the property
    Outputs     : current material is updated
    Return      :
    State       : (NEW_LIGHTING thru gl_set_material)
----------------------------------------------------------------------------*/
DLL void API glMaterialfv(GLenum face, GLenum pname, GLfloat const* params)
{
    GLcontext* ctx = CC;
    GLuint bitmask;

    switch (face)
    {
    case GL_FRONT:
    case GL_BACK:
    case GL_FRONT_AND_BACK:
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glMaterial(face)");
        return;
    }

    switch (pname)
    {
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_SPECULAR:
    case GL_SHININESS:
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glMaterial(pname)");
        return;
    }

    bitmask = gl_material_bitmask(face, pname);

    gl_set_material(bitmask, params);
}

/*-----------------------------------------------------------------------------
    Name        : glDepthFunc
    Description : sets the current depthtest function
    Inputs      : func - LESS, LEQUAL are the only supported depthtest functions
    Outputs     : ctx->DepthFunc may be updated
    Return      :
    State       : NEW_RASTER
----------------------------------------------------------------------------*/
DLL void API glDepthFunc(GLenum func)
{
    GLcontext* ctx = CC;

    if (ctx->DepthFunc == func)
    {
        //avoid redundant state setting
        return;
    }

    switch (func)
    {
    case GL_LEQUAL:
    case GL_LESS:
        ctx->DepthFunc = func;
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glDepthFunc");
    }

    ctx->NewMask |= NEW_RASTER;
}

/*-----------------------------------------------------------------------------
    Name        : glBlendFunc
    Description : sets the current blending factors
    Inputs      : sfactor - source blending factor
                  dfactor - destination blending factor
    Outputs     : ctx->BlendSrc, ctx->BlendDst may be set
    Return      :
    State       : NEW_RASTER
----------------------------------------------------------------------------*/
DLL void API glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    GLcontext* ctx = CC;

    if (ctx->BlendSrc == sfactor && ctx->BlendDst == dfactor)
    {
        //avoid redundant state setting
        return;
    }

    ctx->BlendSrc = sfactor;
    ctx->BlendDst = dfactor;

    ctx->NewMask |= NEW_RASTER;
}

/*-----------------------------------------------------------------------------
    Name        : glAlphaFunc
    Description : sets the current alphatest function
    Inputs      : func - the function
                  ref - reference value
    Outputs     : ctx->AlphaFunc, ctx->AlphaByteRef may be set
    Return      :
    State       : NEW_RASTER
----------------------------------------------------------------------------*/
DLL void API glAlphaFunc(GLenum func, GLclampf ref)
{
    GLcontext* ctx = CC;
    if ((ctx->AlphaFunc == func) &&
        (ctx->AlphaByteRef == FAST_TO_INT(ref*255.0f)))
    {
        //avoid redundant state setting
        return;
    }

    ctx->AlphaFunc = func;
    ctx->AlphaByteRef = FAST_TO_INT(ref*255.0f);

    ctx->NewMask |= NEW_RASTER;
}

/*-----------------------------------------------------------------------------
    Name        : gl_Enable
    Description : enable/disable states in the context, non-redundantly.  this fn
                  is used internally for both Enable and Disable
    Inputs      : ctx - the context
                  cap - capability to enable
                  state - GL_TRUE or GL_FALSE
    Outputs     : the context is updated
    Return      :
    State       : NEW_RASTER
----------------------------------------------------------------------------*/
DLL void gl_Enable(GLenum cap, GLboolean state)
{
#define NEW(X) \
    if (CC->X != state) \
    { \
        CC->NewMask |= NEW_RASTER; \
        CC->X = state; \
    }

    GLcontext* ctx = CC;
    GLboolean updateVert;
    GLint l;

    updateVert = GL_FALSE;

    switch (cap)
    {
    case GL_CLIP_PLANE0:
    case GL_CLIP_PLANE1:
    case GL_CLIP_PLANE2:
    case GL_CLIP_PLANE3:
    case GL_CLIP_PLANE4:
    case GL_CLIP_PLANE5:
        ctx->ClipEnabled[(GLint)cap - (GLint)GL_CLIP_PLANE0] = state;
        ctx->UserClip = GL_FALSE;
        for (l = 0; l < MAX_CLIP_PLANES; l++)
        {
            if (ctx->ClipEnabled[l])
            {
                ctx->UserClip = GL_TRUE;
            }
        }
        break;
    case GL_SHARED_TEXTURE_PALETTE_EXT:
        ctx->UsingSharedPalette = state;
        break;
    case GL_LIT_TEXTURE_PALETTE_EXT:
        ctx->UsingLitPalette = state;
        break;
    case GL_TEXTURE_2D:
        NEW(TexEnabled)
        updateVert = GL_TRUE;
        break;
    case GL_CULL_FACE:
        NEW(CullFace)
        break;
    case GL_NORMALIZE:
        NEW(Normalize)
        break;
    case GL_RESCALE_NORMAL:
        NEW(RescaleNormal);
        break;
    case GL_DEPTH_TEST:
        NEW(DepthTest)
        break;
    case GL_BLEND:
        NEW(Blend)
        break;
    case GL_LIGHTING:
        if (ctx->Lighting != state)
        {
            ctx->NewMask |= NEW_RASTER;
            if (state) ctx->NewMask |= NEW_LIGHTING;
            ctx->Lighting = state;
        }
        updateVert = GL_TRUE;
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        l = (GLint)(cap - GL_LIGHT0);
        NEW(Light[l].Enabled)
//        if (ctx->Speedy) ctx->Light[0].Enabled = GL_FALSE;
        ctx->NewMask |= NEW_LIGHTING;
        break;
    case GL_LINE_STIPPLE:
        NEW(LineStipple)
        break;
    case GL_LINE_SMOOTH:
        NEW(LineSmooth)
        break;
    case GL_POINT_SMOOTH:
        NEW(PointSmooth);
        break;
    case GL_SCISSOR_TEST:
        NEW(ScissorTest)
        break;
    case GL_ALPHA_TEST:
        NEW(AlphaTest)
        break;
    case GL_FOG:
        NEW(Fog);
        break;
    case GL_POLYGON_STIPPLE:
        ctx->PolygonStipple = state;
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "gl_Enable");
    }

    if (updateVert)
    {
        gl_update_vertexfunc();
    }
#undef NEW
}

/*-----------------------------------------------------------------------------
    Name        : glEnable
    Description : enables a capability
    Inputs      : cap - the capability to enable
    Outputs     : context is updated
    Return      :
----------------------------------------------------------------------------*/
DLL void API glEnable(GLenum cap)
{
    gl_Enable(cap, GL_TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : glDisable
    Description : disables a capability
    Inputs      : cap - the capability to disable
    Outputs     : context is updated
    Return      :
----------------------------------------------------------------------------*/
DLL void API glDisable(GLenum cap)
{
    gl_Enable(cap, GL_FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : glIsEnabled
    Description : used to determine a GL capability's state
    Inputs      : cap - the capability
    Outputs     :
    Return      : GL_TRUE or GL_FALSE, reflecting capability's enabled status
----------------------------------------------------------------------------*/
DLL GLboolean API glIsEnabled(GLenum cap)
{
    GLcontext* ctx = CC;

    switch (cap)
    {
    case GL_ALPHA_TEST:
        return ctx->AlphaTest;
    case GL_BLEND:
        return ctx->Blend;
    case GL_CULL_FACE:
        return ctx->CullFace;
    case GL_DEPTH_TEST:
        return ctx->DepthTest;
    case GL_FOG:
        return ctx->Fog;
    case GL_LIGHT0:
        return ctx->Light[0].Enabled;
    case GL_LIGHT1:
        return ctx->Light[1].Enabled;
    case GL_LIGHTING:
        return ctx->Lighting;
    case GL_LINE_SMOOTH:
        return ctx->LineSmooth;
    case GL_LINE_STIPPLE:
        return ctx->LineStipple;
    case GL_NORMALIZE:
        return ctx->Normalize;
    case GL_POINT_SMOOTH:
        return ctx->PointSmooth;
    case GL_POLYGON_SMOOTH:
        return GL_FALSE;
    case GL_POLYGON_STIPPLE:
        return ctx->PolygonStipple;
    case GL_RESCALE_NORMAL:
        return ctx->RescaleNormal;
    case GL_SCISSOR_TEST:
        return ctx->ScissorTest;
    case GL_TEXTURE_2D:
        return ctx->TexEnabled;
    case GL_SHARED_TEXTURE_PALETTE_EXT:
        return ctx->UsingSharedPalette;
    case GL_LIT_TEXTURE_PALETTE_EXT:
        return ctx->UsingLitPalette;
    case GL_CLIP_PLANE0:
    case GL_CLIP_PLANE1:
    case GL_CLIP_PLANE2:
    case GL_CLIP_PLANE3:
    case GL_CLIP_PLANE4:
    case GL_CLIP_PLANE5:
        return (ctx->ClipEnabled[(GLint)cap - (GLint)GL_CLIP_PLANE0]);
    default:
        return GL_FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glLightfv
    Description : set lighting parameters
    Inputs      : light - the light to modify
                  pname - which lighting parameter to modify
                  params - the parameters
    Outputs     : context lighting data is updated
    Return      :
    State       : NEW_LIGHTING
----------------------------------------------------------------------------*/
DLL void API glLightfv(GLenum light, GLenum pname, GLfloat const* params)
{
    GLcontext* ctx = CC;
    GLint l;

    l = (GLint)(light - GL_LIGHT0);
    if (l < 0 || l >= MAX_LIGHTS)
    {
        gl_error(ctx, GL_INVALID_ENUM, "glLight");
        return;
    }

    switch (pname)
    {
    case GL_AMBIENT:
        V4_COPY(ctx->Light[l].Ambient, params);
        break;
    case GL_DIFFUSE:
        V4_COPY(ctx->Light[l].Diffuse, params);
        break;
    case GL_SPECULAR:
        V4_COPY(ctx->Light[l].Specular, params);
        break;
    case GL_POSITION:
        V4_COPY(ctx->Light[l].OPos, params);
        TRANSFORM_POINT(ctx->Light[l].Position, ctx->ModelViewMatrix, params);
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glLight");
        return;
    }

    ctx->NewMask |= NEW_LIGHTING;
}

/*-----------------------------------------------------------------------------
    Name        : gl_clear_colorbuffer
    Description : clears the colorbuffer, relying on subsidiary functions to do
                  the actual work
    Inputs      :
    Outputs     : the colorbuffer is cleared
    Return      :
----------------------------------------------------------------------------*/
void gl_clear_colorbuffer()
{
    GLcontext* ctx = CC;
    if (ctx->DriverFuncs.clear_colorbuffer != NULL)
    {
        ctx->DriverFuncs.clear_colorbuffer(ctx);
    }
}

/*-----------------------------------------------------------------------------
    Name        : glClearDepth
    Description : sets the depth clear value
    Inputs      : d - depth value
    Outputs     : ctx->DepthClear is set
    Return      :
----------------------------------------------------------------------------*/
DLL void API glClearDepth(GLdouble d)
{
    GLcontext* ctx = CC;
    ctx->DepthClear = (GLfloat)d;
}

/*-----------------------------------------------------------------------------
    Name        : gl_clear_depthbuffer
    Description : clears the depthbuffer using driver functions
    Inputs      :
    Outputs     : the depthbuffer is presumably cleared
    Return      :
----------------------------------------------------------------------------*/
void gl_clear_depthbuffer()
{
    GLcontext* ctx = CC;
    if (ctx->DriverFuncs.clear_depthbuffer != NULL)
    {
        ctx->DriverFuncs.clear_depthbuffer(ctx);
    }
}

/*-----------------------------------------------------------------------------
    Name        : glClear
    Description : clears the buffers specified in the supplied bitmask.  will use
                  a driver func to clear the COLOR and DEPTH buffers simultaneously
                  if the func is supplied
    Inputs      : mask - mask of buffer bits to clear
    Outputs     : specified buffers are cleared
    Return      :
    Deviation   : rGL only supports COLOR and DEPTH buffers
----------------------------------------------------------------------------*/
DLL void API glClear(GLbitfield mask)
{
    GLcontext* ctx = CC;
    if ((mask & GL_COLOR_BUFFER_BIT) &&
        (mask & GL_DEPTH_BUFFER_BIT))
    {
        if (ctx->DriverFuncs.clear_both_buffers != NULL)
        {
            ctx->DriverFuncs.clear_both_buffers(ctx);
            return;
        }
    }
    if (mask & GL_COLOR_BUFFER_BIT)
    {
        gl_clear_colorbuffer();
    }
    if (mask & GL_DEPTH_BUFFER_BIT)
    {
        gl_clear_depthbuffer();
    }
}

/*-----------------------------------------------------------------------------
    Name        : glClearColor
    Description : sets the background clear colour
    Inputs      : red, green, blue, alpha - colour components
    Outputs     : ctx->ClearColor is set, driver func clear_color is called if
                  it's supplied
    Return      :
----------------------------------------------------------------------------*/
DLL void API glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    GLcontext* ctx = CC;

    ctx->ClearColor[0] = CLAMP(red,   0.0f, 1.0f);
    ctx->ClearColor[1] = CLAMP(green, 0.0f, 1.0f);
    ctx->ClearColor[2] = CLAMP(blue,  0.0f, 1.0f);
    ctx->ClearColor[3] = CLAMP(alpha, 0.0f, 1.0f);

    ctx->ClearColorByte[0] = (GLubyte)(red * ctx->Buffer.rscale);
    ctx->ClearColorByte[1] = (GLubyte)(green * ctx->Buffer.gscale);
    ctx->ClearColorByte[2] = (GLubyte)(blue * ctx->Buffer.bscale);
    ctx->ClearColorByte[3] = (GLubyte)(alpha * ctx->Buffer.ascale);

    if (ctx->DriverFuncs.clear_color != NULL)
    {
        ctx->DriverFuncs.clear_color(
            ctx->ClearColorByte[0],
            ctx->ClearColorByte[1],
            ctx->ClearColorByte[2],
            ctx->ClearColorByte[3]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : gl_init_material
    Description : initializes a material struct to defaults
    Inputs      : m - the gl_material to initialize
    Outputs     : m is initialized
    Return      :
----------------------------------------------------------------------------*/
void gl_init_material(gl_material* m)
{
    V4_SET(m->Ambient,  0.2f, 0.2f, 0.2f, 1.0f);
    V4_SET(m->Diffuse,  0.8f, 0.8f, 0.8f, 1.0f);
    V4_SET(m->Specular, 0.0f, 0.0f, 0.0f, 1.0f);
    m->Shininess = 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : gl_init_light
    Description : initializes a light struct to defaults
    Inputs      : l - the gl_light to initialize
    Outputs     : l is initialized
    Return      :
----------------------------------------------------------------------------*/
void gl_init_light(gl_light* l)
{
    V4_SET(l->Ambient, 0.0f, 0.0f, 0.0f, 1.0f);
    V4_SET(l->Diffuse, 1.0f, 1.0f, 1.0f, 1.0f);
    V4_SET(l->Specular, 1.0f, 1.0f, 1.0f, 1.0f);
    V4_SET(l->Position, 0.0f, 0.0f, 1.0f, 0.0f);
    l->ConstantAttenuation = 1.0f;
    l->LinearAttenuation = 0.0f;
    l->QuadraticAttenuation = 0.0f;
    l->Enabled = GL_FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : gl_init_viewport
    Description : initializes a viewport struct to defaults
    Inputs      : v - the gl_viewport to initialize
    Outputs     : v is initialized
    Return      :
----------------------------------------------------------------------------*/
void gl_init_viewport(gl_viewport* v)
{
    v->X = 0;
    v->Y = 0;
    v->Width = 0;
    v->Height = 0;
    v->Near = 0.0f;
    v->Far = 1.0f;
    v->Sx = 0.0f;
    v->Tx = 0.0f;
    v->Sy = 0.0f;
    v->Ty = 0.0f;
    v->Sz = 0.5f * DEPTH_SCALE;
    v->Tz = 0.5f * DEPTH_SCALE;
}

/*-----------------------------------------------------------------------------
    Name        : glDepthRange
    Description : set the viewport parameters for depth values
    Inputs      : nearval, farval - extents of the depth components
    Outputs     : ctx->Viewport.Near,Far,Sz,Tz are set
    Return      :
----------------------------------------------------------------------------*/
DLL void API glDepthRange(GLclampd nearval, GLclampd farval)
{
    GLcontext* ctx = CC;
    GLfloat n, f;

    n = (GLfloat)CLAMP(nearval, 0.0, 1.0);
    f = (GLfloat)CLAMP(farval, 0.0, 1.0);

    ctx->Viewport.Near = n;
    ctx->Viewport.Far = f;
    ctx->Viewport.Sz = DEPTH_SCALE * ((f - n) / 2.0f);
    ctx->Viewport.Tz = DEPTH_SCALE * ((f - n) / 2.0f + n);
}

/*-----------------------------------------------------------------------------
    Name        : gl_create_context
    Description : creates (allocates) a new context
    Inputs      :
    Outputs     :
    Return      : returns the new context
----------------------------------------------------------------------------*/
GLcontext* gl_create_context()
{
    GLcontext* ctx = (GLcontext*)malloc(sizeof(GLcontext));
    if (!ctx)
    {
        return NULL;
    }
    MEMSET(ctx, 0, sizeof(GLcontext));

    ctx->D3DReference = 0;

    ctx->DepthBuffer = NULL;
    ctx->FrontFrameBuffer = NULL;
    ctx->BackFrameBuffer = NULL;

    ctx->VB = gl_alloc_vb();
    if (!ctx->VB)
    {
        free(ctx);
        return NULL;
    }
    ctx->VB->Color = ctx->VB->Fcolor;
    return ctx;
}

/*-----------------------------------------------------------------------------
    Name        : is_identity
    Description : determines whether supplied matrix is identity
    Inputs      : m - the matrix to test
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean is_identity(GLfloat const* m)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        if (m[i] != Identity[i])
        {
            return GL_FALSE;
        }
    }
    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : gl_classify_modelview
    Description : classifies the current modelview matrix as IDENTITY, 2D_NO_ROT,
                  2D, 3D, or GENERAL
    Inputs      :
    Outputs     : ctx->ModelViewMatrixType is set accordingly
    Return      :
----------------------------------------------------------------------------*/
void gl_classify_modelview()
{
    GLcontext* ctx = CC;
    GLint t;
    GLfloat const* m = ctx->ModelViewMatrix;

    if (is_identity(m))
    {
        t = MATRIX_IDENTITY;
    }
    else if (m[4] == 0.0f && m[8] == 0.0f
          && m[1] == 0.0f && m[9] == 0.0f
          && m[2] == 0.0f && m[6] == 0.0f && m[10] == 1.0f && m[14] == 0.0f
          && m[3] == 0.0f && m[7] == 0.0f && m[11] == 0.0f && m[15] == 1.0f)
    {
        t = MATRIX_2D_NO_ROT;
    }
    else if (m[8] == 0.0f && m[9] == 0.0f
          && m[2] == 0.0f && m[6] == 0.0f && m[10] == 1.0f && m[14] == 0.0f
          && m[3] == 0.0f && m[7] == 0.0f && m[11] == 0.0f && m[15] == 1.0f)
    {
        t = MATRIX_2D;
    }
    else if (m[3] == 0.0f && m[7] == 0.0f && m[11] == 0.0f && m[15] == 1.0f)
    {
        t = MATRIX_3D;
    }
    else
    {
        t = MATRIX_GENERAL;
    }

    ctx->ModelViewMatrixType = t;
}

/*-----------------------------------------------------------------------------
    Name        : gl_classify_projection
    Description : classifies the current projection matrix as IDENTITY, ORTHO,
                  PERSPECTIVE, or GENERAL
    Inputs      :
    Outputs     : ctx->ProjectionMatrixType is set accordingly
    Return      :
----------------------------------------------------------------------------*/
void gl_classify_projection()
{
    GLcontext* ctx = CC;
    GLint t;
    GLfloat const* m = ctx->ProjectionMatrix;

    if (is_identity(m))
    {
        t = MATRIX_IDENTITY;
    }
    else if (m[4] == 0.0f && m[8] == 0.0f
          && m[1] == 0.0f && m[9] == 0.0f
          && m[2] == 0.0f && m[6] == 0.0f
          && m[3] == 0.0f && m[7] == 0.0f && m[11] == 0.0f && m[15] == 1.0f)
    {
        t = MATRIX_ORTHO;
    }
    else if (m[4] == 0.0f && m[12] == 0.0f
          && m[1] == 0.0f && m[13] == 0.0f
          && m[2] == 0.0f && m[6] == 0.0f
          && m[3] == 0.0f && m[7] == 0.0f && m[11] == -1.0f && m[15] == 0.0f)
    {
        t = MATRIX_PERSPECTIVE;
    }
    else
    {
        t = MATRIX_GENERAL;
    }

    ctx->ProjectionMatrixType = t;
}

/*-----------------------------------------------------------------------------
    Name        : rglGetTexobj
    Description : returns the structure associated with a GL texture name
    Inputs      : name - texture object handle to return the structure of
    Outputs     :
    Return      : the gl_texture_object structure
----------------------------------------------------------------------------*/
DLL gl_texture_object* rglGetTexobj(GLuint name)
{
    return (gl_texture_object*)hashLookup(_texobjs, name);
}

/*-----------------------------------------------------------------------------
    Name        : rglGetMaxTexobj
    Description : returns the highest currently in-use texture object in the GL
    Inputs      :
    Outputs     :
    Return      : texture name of maximum value
----------------------------------------------------------------------------*/
DLL GLuint rglGetMaxTexobj(void)
{
    return _texobjs->maxkey;
}

DLL hashtable* rglGetTexobjs(void)
{
    return _texobjs;
}

/*-----------------------------------------------------------------------------
    Name        : gl_init_texobj
    Description : initializes a texture object structure
    Inputs      : n - texture name
                  texobj - texture structure
    Outputs     : texobj is initialized
    Return      :
----------------------------------------------------------------------------*/
void gl_init_texobj(GLuint n, gl_texture_object* texobj)
{
    texobj->Format = GL_NONE;
    texobj->Width = texobj->Height = 0;
    texobj->WidthLog2 = texobj->HeightLog2 = 0;
    texobj->MaxLog2 = 0;
    texobj->Data = NULL;
    texobj->Data2 = NULL;
    texobj->Name = n;
    texobj->Priority = 1.0f;
    texobj->WrapS = GL_REPEAT;
    texobj->WrapT = GL_REPEAT;
    texobj->Mag = GL_NEAREST;
    texobj->Min = GL_NEAREST;
    texobj->created = GL_FALSE;
    texobj->DriverData = NULL;
    texobj->Palette = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : gl_init_context
    Description : initialize a context.  various other initialization funcs are
                  also called
    Inputs      : ctx - the context
                  depthbuffer, frontframebuffer, backframebuffer - buffers
    Outputs     : the context is initialized
    Return      :
----------------------------------------------------------------------------*/
void gl_init_context(GLcontext* MYctx,
                             GLdepth* depthbuffer,
                             GLubyte* frontframebuffer,
                             GLubyte* backframebuffer)
{
    GLint i;

    gl_is_shutdown = GL_FALSE;

    gl_new_problemfile();

    gl_have_initialized = GL_TRUE;

    gl_load_devices();
    (void)gl_select_named_device(DEFAULT_RENDERER);

    _texobjs = hashNewTable(gl_Allocate, gl_Free);
//    _texobjs = hashNewTable(hash_Allocate, hash_Free);

    CC->Speedy = GL_FALSE;

    CC->RasterizeOnly = GL_FALSE;

    {
        GLuint cputype;
        GLboolean cpummx, cpukatmai;

        cputype = get_cputype();
        cpummx = get_cpummx();
        cpukatmai = 0;//get_cpukatmai();
        CC->CpuType = cputype;
        CC->CpuMMX = cpummx;
        CC->CpuKatmai = cpukatmai;
    }

    if (CC->CpuKatmai)
    {
        STR_VERSION[3] = 'X';
        STR_VERSION[4] = 'M';
        STR_VERSION[5] = 'M';
    }
    else if (CC->CpuMMX)
    {
        STR_VERSION[3] = 'M';
        STR_VERSION[4] = 'M';
        STR_VERSION[5] = 'X';
    }

    CC->AmRendering = GL_TRUE;
    CC->SpecularRender = GL_FALSE;
    CC->SpecularDefault[0] = 11.0f;
    CC->SpecularDefault[1] = 2.0f;
    CC->SpecularDefault[2] = 4.0f;
    for (i = 0; i < 3; i++)
    {
        CC->SpecularExponent[i] = CC->SpecularDefault[i];
    }

    CC->AllocFunc = gl_Allocate;
    CC->FreeFunc  = gl_Free;

    CC->scrMult = scrMult;
    CC->zMult = zMult;
    CC->scrMultByte = scrMultByte;

    CC->ClipMask = CLIP_FCOLOR_BIT;
    CC->NewMask  = NEW_ALL;

    CC->StippleCounter = 0;

    V4_SET(CC->Current.Color, 0, 0, 0, (GLubyte)CC->Buffer.ascale);
    V3_SET(CC->Current.Normal, 0.0f, 0.0f, 0.0f);

    MAT4_COPY(CC->ModelViewMatrix, Identity);
    for (i = 0; i < MAX_MODELVIEW_STACK_DEPTH; i++)
    {
        MAT4_COPY(CC->ModelViewStack[i], Identity);
    }
    CC->ModelViewStackDepth = 0;
    CC->ModelViewMatrixType = MATRIX_IDENTITY;

    MAT4_COPY(CC->ProjectionMatrix, Identity);
    for (i = 0; i < MAX_PROJECTION_STACK_DEPTH; i++)
    {
        MAT4_COPY(CC->ProjectionStack[i], Identity);
    }
    CC->ProjectionStackDepth = 0;
    CC->ProjectionMatrixType = MATRIX_IDENTITY;

    CC->MatrixMode = GL_MODELVIEW;

    for (i = 0; i < MAX_ATTRIB_STACK_DEPTH; i++)
    {
        CC->AttribStack[i].type  = GL_NONE;
    }
    CC->AttribStackDepth = 0;

    CC->CullFace = GL_FALSE;
    CC->Normalize = GL_FALSE;
    CC->RescaleNormal = GL_FALSE;
    CC->Lighting = GL_FALSE;
    CC->Blend = GL_FALSE;
    CC->DepthTest = GL_FALSE;
    CC->DepthWrite = GL_TRUE;
    CC->ColorWrite = GL_TRUE;
    CC->LineStipple = GL_FALSE;
    CC->LineSmooth = GL_FALSE;
    CC->PointSmooth = GL_FALSE;
    CC->Bias = GL_FALSE;
    CC->AlphaTest = GL_FALSE;
    CC->TwoSide = GL_FALSE;
    CC->PolygonStipple = GL_FALSE;
    CC->PerspectiveCorrect = GL_TRUE;
    CC->PolygonMode = GL_FILL;

    CC->Fog = GL_FALSE;
    CC->FogMode = GL_LINEAR;
    CC->FogDensity = 0.3f;
    V4_SET(CC->FogColor, 0.0f, 1.0f, 0.0f, 1.0f);

    gl_Vertex3f  = gl_Vertex3f_cnt;
    gl_Vertex3fv = gl_Vertex3fv_cnt;

    CC->AlphaByteRef = 0;

    CC->FloatBias[0] = 0.0f;
    CC->FloatBias[1] = 0.0f;
    CC->FloatBias[2] = 0.0f;

    CC->ScissorTest = GL_FALSE;
    CC->ScissorX = 0;
    CC->ScissorY = 0;
    CC->ScissorWidth = 640;
    CC->ScissorHeight = 480;

    CC->UserClip = GL_FALSE;
    for (i = 0; i < MAX_CLIP_PLANES; i++)
    {
        CC->ClipEnabled[i] = GL_FALSE;
        V4_SET(CC->ClipEquation[i], 0.0f, 0.0f, 0.0f, 0.0f);
    }

    CC->ShadeModel = GL_SMOOTH;
    V4_SET(CC->Ambient, 0.2f, 0.2f, 0.2f, 1.0f);
    for (i = 0; i < MAX_LIGHTS; i++)
            gl_init_light(&CC->Light[i]);
    gl_init_material(&CC->Material[0]);
    gl_init_material(&CC->Material[1]);

    CC->Current.TexCoord[0] = 0.0f;
    CC->Current.TexCoord[1] = 0.0f;
    CC->TexEnabled = GL_FALSE;
    CC->TexEnvMode = GL_MODULATE;
    CC->TexBoundObject = NULL;
    CC->UsingSharedPalette = GL_FALSE;
    CC->UsingLitPalette = GL_FALSE;

    CC->DepthBuffer = depthbuffer;
    CC->DepthFunc = GL_LESS;
    CC->DepthClear = 1.0f;

    CC->FrontFrameBuffer = frontframebuffer;
    CC->BackFrameBuffer = backframebuffer;
    CC->FrameBuffer = CC->FrontFrameBuffer;
    CC->DrawBuffer = GL_FRONT;
    CC->ReadBuffer = GL_FRONT;

    gl_init_viewport(&CC->Viewport);

    CC->CullFaceMode = GL_BACK;

    CC->BlendSrc = GL_SRC_ALPHA;
    CC->BlendDst = GL_ONE_MINUS_SRC_ALPHA;
    V4_SET(CC->ClearColor, 0.0f, 0.0f, 0.0f, 0.0f);

    CC->PointSize = 1.0f;
    CC->LineWidth = 1.0f;

    CC->Primitive = GL_NEVER;

    CC->VertexArray = NULL;
    CC->VertexSize  = 4;
    CC->VertexFormat = GL_NEVER;

    CC->DriverTransforms = GL_FALSE;
    CC->DriverCtx = NULL;
    //FIXME: this doesn't force a driver to be aware
    //of things it possibly should be
    MEMSET(&CC->DriverFuncs, 0, sizeof(gl_driver_funcs));

    CC->ExclusiveLock = GL_FALSE;

    CC->EffectPoint = GL_FALSE;
    CC->PointHack = GL_FALSE;

    CC->SansDepth = GL_FALSE;

    CC->GammaAdjust = 0.15f;
    CC->LightingAdjust = 0.0f;

    CC->LinesEnabled = GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : gl_alloc_vb
    Description : allocates a new vertex buffer
    Inputs      :
    Outputs     :
    Return      : the newly allocated vertex buffer
----------------------------------------------------------------------------*/
vertex_buffer* gl_alloc_vb()
{
    vertex_buffer* vb;
    vb = (vertex_buffer*)gl_Allocate(sizeof(vertex_buffer));
    if (vb)
    {
            GLuint i;
            for (i = 0; i < VB_SIZE; i++)
        {
                vb->ClipMask[i] = 0;
                vb->Obj[i][3] = 1.0f;
            }
            vb->ClipOrMask = 0;
            vb->ClipAndMask = CLIP_ALL_BITS;
    }
    return vb;
}

/*-----------------------------------------------------------------------------
    Name        : glFlush
    Description : flush render buffers.  possibly take a screenshot, too
    Inputs      :
    Outputs     : maybe a screenshot is saved, buffer swap occurs
    Return      :
----------------------------------------------------------------------------*/
DLL void API glFlush()
{
    GLcontext* ctx = CC;
    GLubyte* framebuf;

    g_NumPolys = 0;
    g_CulledPolys = 0;
    gl_frames++;

    if (ctx->DriverFuncs.flush != NULL)
    {
        ctx->DriverFuncs.flush();
    }
    else
    {
        gl_problem(ctx, "glFlush(DR.flush)");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glGetFrameCountEXT
    Description : returns the frame counter
    Inputs      :
    Outputs     :
    Return      : frame counter
----------------------------------------------------------------------------*/
DLL GLuint glGetFrameCountEXT(void)
{
    return gl_frames;
}

/*-----------------------------------------------------------------------------
    Name        : rglGetSkip
    Description : determine raster no/skip condition
    Inputs      : y - current raster
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
DLL GLboolean rglGetSkip(GLint y)
{
    if (gl_frames & 1)
    {
        if (y & 1)
        {
            return GL_TRUE;
        }
    }
    else if (!(y & 1))
    {
        return GL_TRUE;
    }

    return GL_FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : rauxInitDisplayMode
    Description : this function does nothing
    Inputs      : flags - ???
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void rauxInitDisplayMode(GLuint flags)
{
    //nothing here
}

/*-----------------------------------------------------------------------------
    Name        : gl_driver_init
    Description : initialize a rasterization driver
    Inputs      :
    Outputs     :
    Return      : TRUE (success) or FALSE (failure)
----------------------------------------------------------------------------*/
static GLboolean gl_driver_init(GLboolean reload)
{
    GLcontext* ctx = CC;
    static GLboolean (*init_driver)();
    void* lib;
    char fname[64];

    ctx->NewMask |= NEW_RASTER;

    STR_RENDERER[0] = 'r';
    STR_RENDERER[1] = 'g';
    STR_RENDERER[2] = 'l';

    if (deviceToSelect != NULL)
    {
        if (!gl_select_named_device(deviceToSelect))
        {
#if 1
            return GL_FALSE;
#else
            gl_select_named_device("sw");
            reload = GL_TRUE;
#endif
        }
        deviceToSelect = NULL;
    }

    if (reload)
    {
#ifdef _WIN32
        strcpy(fname, "rgl");
#else
        strcpy(fname, "librgl");
#endif
        strcat(fname, devices[activeDevice].name);
#ifndef _WIN32
        strcat(fname, ".so");
#endif

#ifdef _WIN32
        lib = GetModuleHandle(fname);
        if (lib == NULL)
        {
            lib = LoadLibrary(fname);
        }
#else
        lib = dlopen(fname, RTLD_GLOBAL | RTLD_NOW);
#endif
        if (lib == NULL)
        {
            strcat(fname, " : couldn't load driver");
            gl_error(ctx, GL_INVALID_VALUE, fname);
            return GL_FALSE;
        }

#ifdef _WIN32
        init_driver = (GLboolean(*)())GetProcAddress(lib, "init_driver");
#else
        init_driver = (GLboolean(*)())dlsym(lib, "init_driver");
#endif
        if (init_driver == NULL)
        {
            return GL_FALSE;
        }
    }

    ctx->ScaleDepthValues = GL_TRUE;

    if (!init_driver(ctx))
    {
        return GL_FALSE;
    }

    STR_RENDERER[3] = devices[activeDevice].name[0];
    STR_RENDERER[4] = devices[activeDevice].name[1];
    STR_RENDERER[5] = ' ';

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : gl_driver_postinit
    Description : post-initialization of a driver module
    Inputs      : ctx - the context
    Outputs     :
    Return      : TRUE (success) or FALSE (failure)
----------------------------------------------------------------------------*/
static GLboolean gl_driver_postinit(GLcontext* ctx)
{
    if (ctx->DriverFuncs.post_init_driver != NULL)
    {
        if (!ctx->DriverFuncs.post_init_driver(ctx))
        {
            //FIXME: fatal error
            gl_error(ctx, GL_INVALID_OPERATION, "gl_driver_postinit(DriverFuncs)");
            return GL_FALSE;
        }
    }

    if (ctx->DriverFuncs.allocate_colorbuffer != NULL)
    {
        ctx->DriverFuncs.allocate_colorbuffer(ctx);
    }
    if (ctx->DriverFuncs.allocate_depthbuffer != NULL)
    {
        ctx->DriverFuncs.allocate_depthbuffer(ctx);
    }

    if (ctx->DriverFuncs.clear_color != NULL)
    {
        ctx->DriverFuncs.clear_color(
            ctx->ClearColorByte[0],
            ctx->ClearColorByte[1],
            ctx->ClearColorByte[2],
            ctx->ClearColorByte[3]);
    }

    if (ctx->DriverFuncs.scissor != NULL)
    {
        ctx->DriverFuncs.scissor(
            ctx->ScissorX, ctx->ScissorY-1,
            ctx->ScissorWidth-1, ctx->ScissorHeight);
    }

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : _rgl_init
    Description : calls gl_driver_init(), sets up dependent variables in the context,
                  calls gl_driver_postinit()
    Inputs      : width, height, depth - screen dimensions / colour depth
    Outputs     : large portions of the context are adjusted
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
static GLboolean _rgl_init(GLboolean reload, GLuint width, GLuint height, GLuint depth)
{
    GLcontext* ctx = CC;
    GLuint i, pitch, mult;

    ctx->Buffer.Width  = ctx->ScissorWidth  = width;
    ctx->Buffer.Height = ctx->ScissorHeight = height;
    ctx->Buffer.Depth  = depth;

    if (!gl_driver_init(reload))
    {
        return GL_FALSE;
    }

    ctx->DriverFuncs.driver_caps(ctx);
    if (ctx->Buffer.Depth == 15)
    {
        ctx->Buffer.Depth = 16;
    }

    switch (ctx->Buffer.PixelType)
    {
    case GL_RGB555:
    case GL_RGB565:
        ctx->Buffer.rscale = 255.0f;
        ctx->Buffer.gscale = 255.0f;
        ctx->Buffer.bscale = 255.0f;
        ctx->Buffer.ascale = 255.0f;
        ctx->Buffer.maxr = 255;
        ctx->Buffer.maxg = 255;
        ctx->Buffer.maxb = 255;
        ctx->Buffer.maxa = 255;
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "whoa, can't handle this bitdepth");
        return GL_FALSE;
    }
    mult = ctx->Buffer.Depth >> 3;
    pitch = ctx->Buffer.Pitch / mult;
    for (i = 0; i < height; i++)
    {
        scrMult[i] = i*pitch;
        scrMultByte[i] = mult*((height-1)-i)*pitch;
        zMult[i] = i*width;
    }
    for (i = height; i < MAX_HEIGHT; i++)
    {
        scrMult[i] = (height-(height-1))*pitch;
        scrMultByte[i] = mult*(height-(height-1))*pitch;
        zMult[i] = (height-(height-1))*width;
    }
    ctx->Buffer.ByteMult = mult;

    if (sbuf != NULL)
    {
        free(sbuf);
    }
//    sbuf = (GLubyte*)malloc(3*ctx->Buffer.Width*ctx->Buffer.Height);

    glViewport(0, 0, width, height);

    if (!gl_driver_postinit(CC))
    {
        //FIXME: fatal error
        gl_error(ctx, GL_INVALID_OPERATION, "_rgl_init(gl_driver_postinit)");
        return GL_FALSE;
    }

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : rauxInitPosition
    Description : set the size and location of the GL's render buffer
    Inputs      : x, y - location
                  width, height - dimension
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL int rauxInitPosition(GLuint x, GLuint y, GLuint width, GLuint height, GLuint depth)
{
    GLdepth* db = NULL;
    GLubyte* ffb = NULL;
    GLubyte* bfb = NULL;
    DLL void rglDeleteWindow(GLint);

    GLcontext* ctx = gl_create_context();

    gl_set_context(ctx);
    gl_init_context(ctx, db, ffb, bfb);

    ctx->ScissorWidth = width;
    ctx->ScissorHeight = height;

    init_sqrt_tab();

    if (!_rgl_init(GL_TRUE, width, height, depth))
    {
        rglDeleteWindow(0);
        return 0;
    }
    else
    {
        return 1;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glBegin
    Description : begin a new geometric primitive
    Inputs      : p - primitive to begin
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glBegin(GLenum p)
{
    GLcontext* ctx = CC;

    if (ctx->Primitive != GL_NEVER)
    {
        gl_error(ctx, GL_INVALID_OPERATION, "glBegin");
        return;
    }

    if (ctx->DriverTransforms)
    {
        if (ctx->NewMask & NEW_MODELVIEW)
        {
            gl_update_modelview();
        }
        if (ctx->NewMask & NEW_PROJECTION)
        {
            gl_update_projection();
        }
        ctx->DriverFuncs.begin(ctx, p);
    }
    else if (ctx->DriverFuncs.begin != NULL)
    {
        ctx->DriverFuncs.begin(ctx, p);
    }

    ctx->Primitive = p;
    ctx->VB->Start = ctx->VB->Count = 0;

    switch (p)
    {
    case GL_POLYGON:
    case GL_POINTS:
    case GL_LINES:
    case GL_LINE_LOOP:
    case GL_LINE_STRIP:
    case GL_TRIANGLES:
    case GL_TRIANGLE_FAN:
    case GL_TRIANGLE_STRIP:
    case GL_QUAD_STRIP:
    case GL_QUADS:
        ctx->StippleCounter = 0;
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glBegin");
        ctx->Primitive = GL_NEVER;
    }
}

//maximum number of vertices that have come thru glEnd
static GLuint _vbcount_max = 0;

/*-----------------------------------------------------------------------------
    Name        : glEnd
    Description : end a geometric primitive, sending it thru the pipeline
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glEnd()
{
    GLcontext* ctx = CC;

    if (ctx->DriverTransforms)
    {
        if (ctx->NewMask & NEW_RASTER)
        {
            gl_update_raster(ctx);
        }
        ctx->DriverFuncs.end(ctx);
        ctx->Primitive = GL_NEVER;
        ctx->EffectPoint = GL_FALSE;
        return;
    }
    else if (ctx->DriverFuncs.end != NULL)
    {
        ctx->DriverFuncs.end(ctx);
    }

    if (ctx->Primitive == GL_NEVER)
    {
        gl_error(ctx, GL_INVALID_OPERATION, "glEnd");
        return;
    }

    if (ctx->VB->Count > _vbcount_max)
    {
        _vbcount_max = ctx->VB->Count;
    }

    if (ctx->VB->Count > ctx->VB->Start)
    {
        ctx->VB->Free = ctx->VB->Count + 1;
        if (ctx->NewMask & NEW_RASTER)
        {
            gl_update_raster(ctx);
        }
        gl_transform_vb_part1(ctx, GL_TRUE);
    }

    if (ctx->DriverFuncs.flush_batch != NULL)
    {
        ctx->DriverFuncs.flush_batch();
    }

    ctx->Primitive = GL_NEVER;

    ctx->EffectPoint = GL_FALSE;
}

DLL void API glVertex4fv(GLfloat const* v)
{
    gl_Vertex4fv(v);
}

/*-----------------------------------------------------------------------------
    Name        : glVertex3f
    Description : add a new vertex to the vertex buffer.  technically only valid
                  within a Begin / End block, but rGL doesn't complain
    Inputs      : x, y, z - components
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    gl_Vertex3f(x, y, z);
}

/*-----------------------------------------------------------------------------
    Name        : glVertex3fv
    Description : [as glVertex3f]
    Inputs      : v - array [3] of components
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glVertex3fv(GLfloat const* v)
{
    gl_Vertex3fv(v);
}

/*-----------------------------------------------------------------------------
    Name        : glVertex2f
    Description : [as glVertex3f]
    Inputs      : x, y - components.  z is 1.0
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glVertex2f(GLfloat x, GLfloat y)
{
    glVertex3f(x, y, 1.0f);
}

/*-----------------------------------------------------------------------------
    Name        : glVertex2i
    Description : [as glVertex3f]
    Inputs      : x, y - components.  z is 1
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glVertex2i(GLint x, GLint y)
{
    glVertex3f((GLfloat)x, (GLfloat)y, 1.0f);
}

/*-----------------------------------------------------------------------------
    Name        : glNormal3f
    Description : adds a normal to the current vertex in the context
    Inputs      : nx, ny, nz - normal components
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    GLcontext* ctx = CC;
    ctx->Current.Normal[0] = nx;
    ctx->Current.Normal[1] = ny;
    ctx->Current.Normal[2] = nz;
}

/*-----------------------------------------------------------------------------
    Name        : glNormal3fv
    Description : [as glNormal3f]
    Inputs      : n - array [3] of normal components
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glNormal3fv(GLfloat* n)
{
    GLcontext* ctx = CC;
    ctx->Current.Normal[0] = n[0];
    ctx->Current.Normal[1] = n[1];
    ctx->Current.Normal[2] = n[2];
}

/*-----------------------------------------------------------------------------
    Name        : glColor3f
    Description : adds a colour to the current vertex in the context
    Inputs      : r, g, b - colour components
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
    GLcontext* ctx = CC;
    ctx->Current.Color[0] = (GLubyte)(ctx->Buffer.rscale * r);
    ctx->Current.Color[1] = (GLubyte)(ctx->Buffer.gscale * g);
    ctx->Current.Color[2] = (GLubyte)(ctx->Buffer.bscale * b);
    ctx->Current.Color[3] = (GLubyte)(ctx->Buffer.ascale * 1.0f);
}

/*-----------------------------------------------------------------------------
    Name        : glColor4f
    Description : adds a colour to the current vertex in the context
    Inputs      : r, g, b, a - colour components
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    GLcontext* ctx = CC;
    ctx->Current.Color[0] = (GLubyte)(ctx->Buffer.rscale * r);
    ctx->Current.Color[1] = (GLubyte)(ctx->Buffer.gscale * g);
    ctx->Current.Color[2] = (GLubyte)(ctx->Buffer.bscale * b);
    ctx->Current.Color[3] = (GLubyte)(ctx->Buffer.ascale * a);
}

/*-----------------------------------------------------------------------------
    Name        : glColor3ub
    Description : adds a colour to the current vertex in the context
    Inputs      : r, g, b - colour components
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glColor3ub(GLubyte r, GLubyte g, GLubyte b)
{
    GLcontext* ctx = CC;
    ctx->Current.Color[0] = r;
    ctx->Current.Color[1] = g;
    ctx->Current.Color[2] = b;
    ctx->Current.Color[3] = 255;
}

/*-----------------------------------------------------------------------------
    Name        : glColor4ub
    Description : adds a colour to the current vertex in the context
    Inputs      : r, g, b, a - colour components
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    GLcontext* ctx = CC;
    ctx->Current.Color[0] = r;
    ctx->Current.Color[1] = g;
    ctx->Current.Color[2] = b;
    ctx->Current.Color[3] = a;
}

/*-----------------------------------------------------------------------------
    Name        : glTexCoord2f
    Description : adds a texture coordinate to the current vertex in the context
    Inputs      : s, t - texel
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glTexCoord2f(GLfloat s, GLfloat t)
{
    GLcontext* ctx = CC;
    ctx->Current.TexCoord[0] = s;
    ctx->Current.TexCoord[1] = t;
}

static void _insert_texobj(GLuint textureName)
{
    gl_texture_object* tex = (gl_texture_object*)gl_Allocate(sizeof(gl_texture_object));
    gl_init_texobj(textureName, tex);
    hashInsert(_texobjs, textureName, tex);
}

/*-----------------------------------------------------------------------------
    Name        : glGenTextures
    Description : generates texture names
    Inputs      : n - number of names to generate
                  textureNames - generated names go here
    Outputs     : textureNames will contain n names, or all 0 (null name)
    Return      :
----------------------------------------------------------------------------*/
DLL void API glGenTextures(GLsizei n, GLuint* textureNames)
{
    GLcontext* ctx = CC;
    GLuint i;
    GLuint first = hashFindFreeKeyBlock(_texobjs, n);
    if (first == 0)
    {
        MEMSET(textureNames, 0, n*sizeof(GLuint));
        gl_error(ctx, GL_INVALID_VALUE, "glGenTextures(first)");
    }
    else
    {
        for (i = 0; i < (GLuint)n; i++)
        {
            textureNames[i] = first + i;
            _insert_texobj(textureNames[i]);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : glTexParameteri
    Description : set texture parameters of currently bound texture object
    Inputs      : target - the target texture (only TEXTURE_2D supported)
                  pname - the parameter to modify
                  param - what to modify it to
    Outputs     : current texture object is modified
    Return      :
----------------------------------------------------------------------------*/
DLL void API glTexParameteri(GLenum target, GLenum pname, GLenum param)
{
    GLcontext* ctx = CC;

    //ignore target
    gl_texture_object* texobj = ctx->TexBoundObject;
    if (texobj == NULL)
    {
//        gl_error(ctx, GL_INVALID_VALUE, "glTexParameteri(texobj)");
        return;
    }

    if (ctx->DriverFuncs.tex_param != NULL)
    {
        GLfloat params[1];
        params[0] = (GLfloat)param;
        ctx->DriverFuncs.tex_param(pname, params);
    }

    switch (pname)
    {
    case GL_TEXTURE_WRAP_S:
        texobj->WrapS = param;
        break;
    case GL_TEXTURE_WRAP_T:
        texobj->WrapT = param;
        break;
    case GL_TEXTURE_MAG_FILTER:
        texobj->Mag = param;
        break;
    case GL_TEXTURE_MIN_FILTER:
        texobj->Min = param;
        break;
    default:
        gl_error(ctx, GL_INVALID_VALUE, "glTexParameteri(pname)");
        return;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glBindTexture
    Description : binds a texture object to the GL
    Inputs      : target - target texture (only TEXTURE_2D supported)
                  textureName - the texture to bind
    Outputs     : current texture is modified, driver bind_texture function is called
    Return      :
----------------------------------------------------------------------------*/
DLL void API glBindTexture(GLenum target, GLuint textureName)
{
    GLcontext* ctx = CC;

    //totally ignore target (2D textures only)
    gl_texture_object* to;

//    if ((GLint)textureName <= 0)
    if (textureName == 0 || textureName > 0xfffffff0)
    {
        ctx->TexBoundObject = NULL;
        return;
    }

    to = (gl_texture_object*)hashLookup(_texobjs, textureName);
    if (to == NULL)
    {
        gl_texture_object* tex = (gl_texture_object*)gl_Allocate(sizeof(gl_texture_object));
        gl_init_texobj(textureName, tex);
        hashInsert(_texobjs, textureName, tex);
        to = tex;
    }

    if (ctx->TexBoundObject == to)
    {
        //avoid redundant state change
        return;
    }

    ctx->TexBoundObject = to;

    if (ctx->DriverFuncs.bind_texture != NULL)
    {
        ctx->DriverFuncs.bind_texture();
    }
}

/*-----------------------------------------------------------------------------
    Name        : gl_copy_3_to_4
    Description : internal use: copies an RGB texture to an RGBA texture
    Inputs      : d, s - destination, source
                  width, height - dimensions of the texture
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gl_copy_3_to_4(GLubyte* d, GLubyte* s, GLsizei width, GLsizei height)
{
    GLcontext* ctx = CC;
    GLint y, x;
    GLubyte *sp, *dp, alpha;

    alpha = ctx->Buffer.maxa;

    for (y = 0, dp = d; y < height; y++)
    {
        sp = s + 3*y*width;

        for (x = 0; x < width; x++, dp += 4, sp += 3)
        {
            dp[0] = sp[0];
            dp[1] = sp[1];
            dp[2] = sp[2];
            dp[3] = alpha;
        }
    }
}

//return log2 of an unsigned integer
GLuint gl_log2(GLuint n)
{
    switch (n)
    {
    case 2:
        return 1;
    case 4:
        return 2;
    case 8:
        return 3;
    case 16:
        return 4;
    case 32:
        return 5;
    case 64:
        return 6;
    case 128:
        return 7;
    case 256:
        return 8;
    case 512:
        return 9;
    case 1024:
        return 10;
    default:
        return 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : gl_paltex
    Description : subsidiary to glTexImage2D that handles paletted textures
    Inputs      : ctx - the context
                  to - the texture object
                  width, height - texture dimensions
                  pixels - texture data
    Outputs     : to is modified, to->Data is allocated
    Return      :
----------------------------------------------------------------------------*/
void gl_paltex(gl_texture_object* to, GLsizei width, GLsizei height, GLubyte const* pixels)
{
#if NO_PALETTES
    GLubyte* dp;
    GLubyte* sp;
    GLint i, index;

    to->Format = GL_RGB;

    to->Data = (GLubyte*)gl_Allocate(4*width*height);

    //fill the data
    dp = (GLubyte*)to->Data;
    sp = (GLubyte*)pixels;
    for (i = 0; i < width*height; i++, dp += 4, sp++)
    {
        index = 4 * (*sp);
        dp[0] = to->Palette[index + 0];
        dp[1] = to->Palette[index + 1];
        dp[2] = to->Palette[index + 2];
        dp[3] = to->Palette[index + 3];
    }
#else
    GLint size;

    //slight departure from the standard as I see it, but maybe not.
    //"should" be GL_COLOR_INDEX8_EXT
    to->Format = GL_COLOR_INDEX;

    //8bit data
    size = width*height;
    to->Data = (GLubyte*)gl_Allocate(size);
    MEMCPY(to->Data, pixels, size);
#endif

    //other stuff
    to->Width = width;
    to->Height = height;
    to->WidthLog2 = gl_log2(width);
    to->HeightLog2 = gl_log2(height);
    to->WidthMask = width - 1;
    to->HeightMask = height - 1;
    to->created = GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : glTexImage2D
    Description : set texture data in the currently bound texture object
    Inputs      : target - target texture (only TEXTURE_2D supported)
                  level - mipmap level (ignored - assumed to be 0)
                  internalFormat, format - assumed to be identical
                  width, height - texture dimensions
                  border - (not supported)
                  type - data type (assumed to be UNSIGNED_BYTE)
                  pixels - texture data
    Outputs     : the texture is allocated and such.  it's safe to free supplied
                  data (pixels) after this if you like
    Return      :
----------------------------------------------------------------------------*/
DLL void API glTexImage2D(
    GLenum target, GLint level, GLint internalFormat,
    GLsizei width, GLsizei height, GLint border,
    GLenum format, GLenum type,
    GLvoid const* pixels)
{
    GLcontext* ctx = CC;

    //ignore target
    gl_texture_object* to = ctx->TexBoundObject;
    if (to == NULL)
    {
        gl_error(ctx, GL_INVALID_VALUE, "glTexImage2D has no bound texobj");
        return;
    }

    //handle paletted textures separately
    if (internalFormat == GL_COLOR_INDEX || format == GL_COLOR_INDEX)
    {
        gl_paltex(to, width, height, (GLubyte const*)pixels);
        goto TEXIMAGE_DONE;
    }

    //ignore level
    if (internalFormat == GL_RGB || internalFormat == GL_RGBA || internalFormat == GL_RGBA16)
    {
        if (format != GL_RGB &&
            format != GL_RGBA &&
            format != GL_RGBA16)
        {
            gl_error(ctx, GL_INVALID_VALUE, "glTexImage2D(internalFormat)");
            return;
        }
    }
    to->Format = internalFormat;

    //ignore border

    if (type != GL_UNSIGNED_BYTE)
    {
        gl_error(ctx, GL_INVALID_VALUE, "glTexImage2D(type)");
        return;
    }

    if (to->Data != NULL)
    {
        gl_Free(to->Data);
    }

    if (internalFormat == GL_RGBA16)
    {
        to->Data = (GLubyte*)gl_Allocate(2*width*height);
    }
    else
    {
        to->Data = (GLubyte*)gl_Allocate(4*width*height);
    }

    to->Width = width;
    to->Height = height;
    to->WidthLog2 = gl_log2(width);
    to->HeightLog2 = gl_log2(height);
    to->WidthMask = width - 1;
    to->HeightMask = height - 1;
    switch (internalFormat)
    {
    case GL_RGB:
        if (format == GL_RGBA)
        {
            MEMCPY(to->Data, (GLubyte*)pixels, 4*width*height);
        }
        else
        {
            gl_copy_3_to_4(to->Data, (GLubyte*)pixels, width, height);
        }
        break;
    case GL_RGBA16:
        MEMCPY(to->Data, (GLubyte*)pixels, 2*width*height);
        break;
    case GL_RGBA:
        MEMCPY(to->Data, (GLubyte*)pixels, 4*width*height);
        break;
    default:
        gl_error(ctx, GL_INVALID_VALUE, "glTexImage2D(format)");
    }

    to->created = GL_TRUE;

TEXIMAGE_DONE:
    if (ctx->DriverFuncs.tex_img != NULL)
    {
        ctx->DriverFuncs.tex_img(to, 0, to->Format);
    }

#if 0
    if (activeDevice != 0)
    {
        //free memory used by the texture if we're not using rglsw
        gl_Free(to->Data);
        to->Data = NULL;
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : glTexEnvi
    Description : alters the current texture environment mode
    Inputs      : target - ignored (assumed to be TEXTURE_ENV)
                  pname - ignored (assumed to be TEXTURE_ENV_MODE)
                  param - the mode (only MODULATE, REPLACE, DECAL supported)
    Outputs     : ctx->TexEnvMode is updated
    Return      :
    Deviation   : DECAL implementation is non-standard and shouldn't be used
    State       : NEW_RASTER
----------------------------------------------------------------------------*/
DLL void API glTexEnvi(GLenum target, GLenum pname, GLenum param)
{
    GLcontext* ctx = CC;

    //ignore target (assume GL_TEXTURE_ENV)
    //ignore pname  (assume GL_TEXTURE_ENV_MODE)

    if (ctx->TexEnvMode == param)
    {
        //avoid redundant state change
        return;
    }

    if (ctx->DriverFuncs.tex_env != NULL)
    {
        ctx->DriverFuncs.tex_env(param);
    }

    switch (param)
    {
    case GL_MODULATE:
    case GL_REPLACE:
    case GL_DECAL:
        ctx->TexEnvMode = param;
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glTexEnvi(param)");
    }

//    if (ctx->Speedy) ctx->TexEnvMode = GL_REPLACE;

    ctx->NewMask |= NEW_RASTER;
}

/*-----------------------------------------------------------------------------
    Name        : glGetFloatv
    Description : returns GL state
    Inputs      : pname - the state parameter to retrieve (limited support)
                  param - where to put the state data
    Outputs     : param is filled
    Return      :
----------------------------------------------------------------------------*/
DLL void API glGetFloatv(GLenum pname, GLfloat* param)
{
    GLcontext* ctx = CC;

    switch (pname)
    {
    case GL_MODELVIEW_MATRIX:
        MAT4_COPY(param, ctx->ModelViewMatrix);
        break;
    case GL_PROJECTION_MATRIX:
        MAT4_COPY(param, ctx->ProjectionMatrix);
        break;
    case GL_LIGHT_MODEL_AMBIENT:
        V4_COPY(param, ctx->Ambient);
        break;
    case GL_COLOR_CLEAR_VALUE:
        V4_COPY(param, ctx->ClearColor);
        break;
    case GL_CURRENT_COLOR:
        param[0] = (GLfloat)ctx->Current.Color[0] * ONE_OVER_255f;
        param[1] = (GLfloat)ctx->Current.Color[1] * ONE_OVER_255f;
        param[2] = (GLfloat)ctx->Current.Color[2] * ONE_OVER_255f;
        param[3] = (GLfloat)ctx->Current.Color[3] * ONE_OVER_255f;
        break;
    case GL_CURRENT_RASTER_COLOR:
        V4_COPY(param, ctx->Current.RasterColor);
        break;
    case GL_CURRENT_RASTER_POSITION:
        V4_COPY(param, ctx->Current.RasterPos);
        break;
    case GL_CURRENT_TEXTURE_COORDS:
        param[0] = ctx->Current.TexCoord[0];
        param[1] = ctx->Current.TexCoord[1];
        param[2] = 0.0f;
        param[3] = 1.0f;
        break;
    case GL_FOG_COLOR:
        V4_COPY(param, ctx->FogColor);
        break;
    case GL_ALPHA_TEST_REF:
        *param = (GLfloat)ctx->AlphaByteRef * ONE_OVER_255f;
        break;
    case GL_RED_BIAS:
        *param = ctx->FloatBias[0];
        break;
    case GL_GREEN_BIAS:
        *param = ctx->FloatBias[1];
        break;
    case GL_BLUE_BIAS:
        *param = ctx->FloatBias[2];
        break;
    case GL_FOG_DENSITY:
        *param = ctx->FogDensity;
        break;
    case GL_LINE_WIDTH:
        *param = ctx->LineWidth;
        break;
    case GL_LINE_WIDTH_GRANULARITY:
    case GL_POINT_SIZE_GRANULARITY:
        *param = 1.0f / 8.0f;
        break;
    case GL_POINT_SIZE:
        *param = ctx->PointSize;
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glGetFloatv(pname)");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glGetDoublev
    Description : returns GL state
    Inputs      : pname - the state parameter to retrieve (limited support)
                  param - where to put the state data
    Outputs     : param is filled
    Return      :
----------------------------------------------------------------------------*/
DLL void API glGetDoublev(GLenum pname, GLdouble* param)
{
    GLcontext* ctx = CC;
    GLfloat m[16];
    GLint i;

    switch (pname)
    {
    case GL_PROJECTION_MATRIX:
        glGetFloatv(pname, m);
        for (i = 0; i < 16; i++)
        {
            param[i] = (GLdouble)m[i];
        }
        break;
    case GL_MODELVIEW_MATRIX:
        glGetFloatv(pname, m);
        for (i = 0; i < 16; i++)
        {
            param[i] = (GLdouble)m[i];
        }
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glGetDoublev(pname)");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glGetIntegerv
    Description : returns GL state
    Inputs      : pname - the state parameter to retrieve (limited support)
                  param - where to put the state data
    Outputs     : param is filled
    Return      :
----------------------------------------------------------------------------*/
DLL void API glGetIntegerv(GLenum pname, GLint* param)
{
    GLcontext* ctx = CC;

    switch (pname)
    {
    case GL_ALPHA_TEST_FUNC:
        *param = ctx->AlphaFunc;
        break;
    case GL_BLEND_DST:
        *param = ctx->BlendDst;
        break;
    case GL_BLEND_SRC:
        *param = ctx->BlendSrc;
        break;
    case GL_MATRIX_MODE:
        *param = ctx->MatrixMode;
        break;
    case GL_COLOR_MATERIAL_FACE:
        *param = GL_FRONT_AND_BACK;
        break;
    case GL_CULL_FACE_MODE:
        *param = ctx->CullFaceMode;
        break;
    case GL_DEPTH_BITS:
        *param = DEPTH_BITS;
        break;
    case GL_SCISSOR_BOX:
        param[0] = ctx->ScissorX;
        param[1] = ctx->ScissorY - 1;
        param[2] = ctx->ScissorWidth - 1;
        param[3] = ctx->ScissorHeight;
        break;
    case GL_VIEWPORT:
        param[0] = ctx->Viewport.X;
        param[1] = ctx->Viewport.Y;
        param[2] = ctx->Viewport.Width;
        param[3] = ctx->Viewport.Height;
        break;
    case GL_MAX_TEXTURE_SIZE:
        *param = 256*256;
        break;
    case GL_MAX_VIEWPORT_DIMS:
        param[0] = MAX_WIDTH;
        param[1] = MAX_HEIGHT;
        break;
    case GL_PERSPECTIVE_CORRECTION_HINT:
        *param = ctx->PerspectiveCorrect ? GL_NICEST : GL_FASTEST;
        break;
    case GL_POLYGON_MODE:
        *param = ctx->PolygonMode;
        break;
    case GL_SHADE_MODEL:
        *param = ctx->ShadeModel;
        break;
    case GL_SUBPIXEL_BITS:
        *param = FIXED_SHIFT;
        break;
#ifdef GL_TEXTURE_2D_BINDING
    case GL_TEXTURE_2D_BINDING:
        if (ctx->TexBoundObject == NULL)
        {
            *param = 0;
        }
        else
        {
            *param = ctx->TexBoundObject->Name;
        }
        break;
#endif
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glGetIntegerv(pname)");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glGetBooleanv
    Description : returns GL state
    Inputs      : pname - the state parameter to retrieve (limited support)
                  param - where to put the state data
    Outputs     : param is filled
    Return      :
----------------------------------------------------------------------------*/
DLL void API glGetBooleanv(GLenum pname, GLboolean* param)
{
    GLcontext* ctx = CC;
    switch (pname)
    {
    case GL_DEPTH_WRITEMASK:
        *param = ctx->DepthWrite;
        break;
    case GL_LIGHT_MODEL_TWO_SIDE:
        *param = ctx->TwoSide;
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glGetBooleanv(pname)");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glGetError
    Description : returns the last error that the GL encountered
    Inputs      :
    Outputs     :
    Return      : gl_error_num, a GLenum that describes the last error
----------------------------------------------------------------------------*/
DLL GLenum API glGetError()
{
    GLenum err = gl_error_num;
    gl_error_num = GL_NO_ERROR;
    return err;
}

/*-----------------------------------------------------------------------------
    Name        : glGluErrorString
    Description : returns a string describing the last error
    Inputs      : err - ignored
    Outputs     :
    Return      : gl_error_string, the last error string.  should be copied if
                  not used right away because gl_error_string will change
----------------------------------------------------------------------------*/
DLL char* glGluErrorString(GLenum err)
{
    return gl_error_string;
}

/*-----------------------------------------------------------------------------
    Name        : gl_update_modelview
    Description : classify the current modelview matrix
    Inputs      :
    Outputs     : gl_classify_modelview is called
    Return      :
    State       : clear NEW_MODELVIEW
----------------------------------------------------------------------------*/
void gl_update_modelview()
{
    GLcontext* ctx = CC;

    gl_classify_modelview();
    ctx->NewMask &= ~(NEW_MODELVIEW);

    if (ctx->CpuKatmai)
    {
        xmm_update_modelview(ctx);
    }

    if (ctx->DriverFuncs.update_modelview != NULL)
    {
        ctx->DriverFuncs.update_modelview();
    }
}

/*-----------------------------------------------------------------------------
    Name        : gl_invert_modelview
    Description : invert the current modelview matrix
    Inputs      :
    Outputs     : will call gl_update_modelview if necessary.
                  ctx->ModelViewInv is calculated from ctx->ModelViewMatrix
    Return      :
    State       : clear NEW_MODELVIEWINV
----------------------------------------------------------------------------*/
void gl_invert_modelview()
{
    GLcontext* ctx = CC;
    if (ctx->NewMask & NEW_MODELVIEW)
    {
        gl_update_modelview();
    }
    mat4_inverse(ctx->ModelViewInv, ctx->ModelViewMatrix);
    ctx->NewMask &= ~(NEW_MODELVIEWINV);
}

/*-----------------------------------------------------------------------------
    Name        : gl_update_projection
    Description : classify the current projection matrix
    Inputs      :
    Outputs     : gl_classify_projection is called
    Return      :
    State       : clear NEW_PROJECTION
----------------------------------------------------------------------------*/
void gl_update_projection()
{
    GLcontext* ctx = CC;

    gl_classify_projection();
    ctx->NewMask &= ~(NEW_PROJECTION);

    if (ctx->CpuKatmai)
    {
        xmm_update_projection(ctx);
    }

    if (ctx->DriverFuncs.update_projection != NULL)
    {
        ctx->DriverFuncs.update_projection();
    }
}

/*-----------------------------------------------------------------------------
    Name        : glRasterPos2f
    Description : sets the current raster position in the context
    Inputs      : x, y - position
    Outputs     : current rasterposition in the context is set
    Return      :
    Deviation   : provided points are NOT TRANSFORMED, merely copied
----------------------------------------------------------------------------*/
DLL void API glRasterPos2f(GLfloat x, GLfloat y)
{
    GLcontext* ctx = CC;
    GLfloat v[4], eye[4], clip[4], ndc[3], d;
    void gl_color_shade_vertices(
        GLcontext* ctx,
                GLuint side,
        GLuint n,
                GLfloat vertex[][4],
                GLfloat normal[][3],
                GLubyte color[][4]);

    V4_SET(v, x, y, 0.0f, 1.0f);

    TRANSFORM_POINT(eye, ctx->ModelViewMatrix, v);
//    V4_COPY(eye, v);

    if (ctx->Lighting)
    {
        GLfloat eyenorm[3];
        GLubyte color[4];
        if (ctx->NewMask & NEW_MODELVIEWINV)
            gl_invert_modelview();
        TRANSFORM_NORMAL(eyenorm[0], eyenorm[1], eyenorm[2],
                         ctx->Current.Normal, ctx->ModelViewInv);
        gl_color_shade_vertices(ctx, 0, 1, &eye, &eyenorm, &color);
        ctx->Current.RasterColor[0] = (GLfloat)color[0] / ctx->Buffer.rscale;
        ctx->Current.RasterColor[1] = (GLfloat)color[1] / ctx->Buffer.gscale;
        ctx->Current.RasterColor[2] = (GLfloat)color[2] / ctx->Buffer.bscale;
        ctx->Current.RasterColor[3] = (GLfloat)color[3] / ctx->Buffer.ascale;
    }
    else
    {
        GLfloat* rc = ctx->Current.RasterColor;
        rc[0] = (GLfloat)ctx->Current.Color[0] / ctx->Buffer.rscale;
        rc[1] = (GLfloat)ctx->Current.Color[1] / ctx->Buffer.gscale;
        rc[2] = (GLfloat)ctx->Current.Color[2] / ctx->Buffer.bscale;
        rc[3] = (GLfloat)ctx->Current.Color[3] / ctx->Buffer.ascale;
    }

    TRANSFORM_POINT(clip, ctx->ProjectionMatrix, eye);
//    V4_COPY(clip, eye);

#if 0
    //invalid rasterpos ... undefined behaviour
    if (gl_viewclip_point(clip) == 0)
    {
        return;
    }
#endif

//    ASSERT(clip[3] != 0.0f);
    d = 1.0f / clip[3];
    ndc[0] = clip[0] * d;
    ndc[1] = clip[1] * d;
    ndc[2] = clip[2] * d;

    ctx->Current.RasterPos[0] = ndc[0] * ctx->Viewport.Sx + ctx->Viewport.Tx;
    ctx->Current.RasterPos[1] = ndc[1] * ctx->Viewport.Sy + ctx->Viewport.Ty;
    ctx->Current.RasterPos[2] = (ndc[2] * ctx->Viewport.Sz + ctx->Viewport.Tz) / DEPTH_SCALE;
    ctx->Current.RasterPos[3] = clip[3];
}

/*-----------------------------------------------------------------------------
    Name        : glRasterPos2i
    Description : wrapper for glRasterPos2f
    Inputs      : x, y - position
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glRasterPos2i(GLint x, GLint y)
{
    glRasterPos2f((GLfloat)x, (GLfloat)y);
}

/*-----------------------------------------------------------------------------
    Name        : glRasterPos4f
    Description : set all 4 rasterpos components
    Inputs      : x, y, z, w - components
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLcontext* ctx = CC;
    ctx->Current.RasterPos[0] = x;
    ctx->Current.RasterPos[1] = y;
    ctx->Current.RasterPos[2] = z;
    ctx->Current.RasterPos[3] = w;
}

/*-----------------------------------------------------------------------------
    Name        : glBitmap
    Description : renders a monochrome bitmap at the current raster position
    Inputs      : [as per spec]
    Outputs     : a rendered bitmap
    Return      :
----------------------------------------------------------------------------*/
DLL void API glBitmap(GLsizei width, GLsizei height,
                  GLfloat xb0, GLfloat yb0,
                  GLfloat xb1, GLfloat yb1,
                  GLubyte const* bitmap)
{
    GLcontext* ctx = CC;
    ctx->Current.Bitmap = (GLubyte*)bitmap;

    if (ctx->DriverFuncs.draw_bitmap != NULL)
    {
        gl_lock_framebuffer();

        ctx->DriverFuncs.draw_bitmap(
            ctx, width, height, xb0, yb0, xb1, yb1);

        gl_unlock_framebuffer();
    }
    else
    {
        gl_error(ctx, GL_INVALID_VALUE, "glBitmap(draw_bitmap)");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glLineWidth
    Description : sets the width of rendered lines
    Inputs      : width - the desired width of lines
    Outputs     : ctx->LineWidth is modified
    Return      :
    State       : NEW_RASTER
----------------------------------------------------------------------------*/
DLL void API glLineWidth(GLfloat width)
{
    GLcontext* ctx = CC;
    if (ctx->LineWidth != width)
    {
        ctx->LineWidth = width;
        ctx->NewMask |= NEW_RASTER;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glPointSize
    Description : sets the size of rendered points
    Inputs      : size - the desired side of points
    Outputs     : ctx->PointSize is modified
    Return      :
    State       : NEW_RASTER
----------------------------------------------------------------------------*/
DLL void API glPointSize(GLfloat size)
{
    GLcontext* ctx = CC;
    if (ctx->PointSize != size)
    {
        ctx->PointSize = size;
        ctx->NewMask |= NEW_RASTER;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glGetString
    Description : returns an informative string about the GL
    Inputs      : cap - the particular string to return
    Outputs     :
    Return      : requested string
----------------------------------------------------------------------------*/
DLL GLubyte* API glGetString(GLenum cap)
{
    switch (cap)
    {
    case GL_VENDOR:
        return STR_VENDOR;
    case GL_RENDERER:
        return STR_RENDERER;
    case GL_VERSION:
        return STR_VERSION;
    case GL_EXTENSIONS:
        return STR_EXTENSIONS;
    default:
        return STR_NOTHING;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glLightModelf
    Description : [stubbed out]
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glLightModelf(GLenum pname, GLfloat param)
{
    //nothing here
}

/*-----------------------------------------------------------------------------
    Name        : glLineStipple
    Description : sets line stippling parameters (factor, pattern)
    Inputs      : [as per spec]
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glLineStipple(GLint factor, GLushort pattern)
{
    GLcontext* ctx = CC;
    ctx->StippleFactor = CLAMP(factor, 1, 256);
    ctx->StipplePattern = pattern;
}

/*-----------------------------------------------------------------------------
    Name        : glPixelStorei
    Description : [stubbed out]
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glPixelStorei(GLenum pname, GLint param)
{
    //nothing here
}

/*-----------------------------------------------------------------------------
    Name        : glDeleteTextures
    Description : deletes texture objects in the GL, freeing all memory and such
    Inputs      : n - number of textures to delete
                  textures - array of n textures to be deleted
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glDeleteTextures(GLsizei n, GLuint const* textures)
{
    GLcontext* ctx = CC;
    GLsizei i;
    gl_texture_object* tex;

    if (gl_is_shutdown)
    {
        return;
    }

    for (i = 0; i < n; i++)
    {
        tex = hashLookup(_texobjs, textures[i]);
        if (tex == NULL)
            continue;

        hashRemove(_texobjs, textures[i]);

        if (tex->Data != NULL)
        {
            gl_Free(tex->Data);
            tex->Data = NULL;
        }

        if (tex->created && ctx->DriverFuncs.tex_del != NULL)
        {
            ctx->DriverFuncs.tex_del(tex);
        }

        gl_Free(tex);
    }
}

/*-----------------------------------------------------------------------------
    Name        : glScissor
    Description : sets the dimensions of the scissor window
    Inputs      : x, y - location (lower left)
                  width, height - dimension
    Outputs     :
    Return      :
    State       : no state flags are set, implying that a driver is responsible for
                  management of the state of scissor window parameters
----------------------------------------------------------------------------*/
DLL void API glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    GLcontext* ctx = CC;
    ctx->ScissorX = x;
    ctx->ScissorY = y+1;
    ctx->ScissorWidth = width+1;
    ctx->ScissorHeight = height;

    if (ctx->DriverFuncs.scissor != NULL)
    {
        ctx->DriverFuncs.scissor(x, y, width, height);
    }
}

/*-----------------------------------------------------------------------------
    Name        : glLightModelfv
    Description : modify parameters global to the lighting model
    Inputs      : pname - lighting model parameter to modify
                  params - the parameters
    Outputs     : ctx->Ambient[] is updated
    Return      :
    Deviation   : only pname == GL_LIGHT_MODEL_AMBIENT is supported
----------------------------------------------------------------------------*/
DLL void API glLightModelfv(GLenum pname, GLfloat const* params)
{
    GLcontext* ctx = CC;
    GLint i;

    switch (pname)
    {
    case GL_LIGHT_MODEL_AMBIENT:
        for (i = 0; i < 4; i++)
        {
            ctx->Ambient[i] = params[i];
        }
        break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : _rgl_next_renderer
    Description : entry point to switch between sw / hw rendering
    Inputs      :
    Outputs     : large portions of the context are adjusted
    Return      :
----------------------------------------------------------------------------*/
static void _rgl_next_renderer()
{
    GLcontext* ctx = CC;
    GLint prevDevice;

    if (ctx->DriverFuncs.shutdown_driver != NULL)
    {
        ctx->DriverFuncs.shutdown_driver(ctx);
    }

    prevDevice = activeDevice;
    gl_next_device();
    if (!_rgl_init(GL_TRUE, ctx->Buffer.Width, ctx->Buffer.Height, ctx->Buffer.Depth))
    {
        gl_select_device(prevDevice);
        (void)_rgl_init(GL_TRUE, ctx->Buffer.Width, ctx->Buffer.Height, ctx->Buffer.Depth);
        //ASSERT: return TRUE
    }
}

/*-----------------------------------------------------------------------------
    Name        : _rgl_reinit_renderer
    Description : entry point to shutdown / re-initialize a driver dll
    Inputs      :
    Outputs     : context is adjusted
    Return      :
----------------------------------------------------------------------------*/
static void _rgl_reinit_renderer()
{
    GLcontext* ctx = CC;
    if (ctx->DriverFuncs.shutdown_driver != NULL)
    {
        ctx->DriverFuncs.shutdown_driver(ctx);
    }

    (void)_rgl_init(GL_FALSE, ctx->Buffer.Width, ctx->Buffer.Height, ctx->Buffer.Depth);
    //ASSERT: return TRUE
}

/*-----------------------------------------------------------------------------
    Name        : _rgl_activate
    Description : activate the GL (recover from a deactivation - "sleep").
                  calls the driver
    Inputs      :
    Outputs     : ctx->AmRendering = GL_TRUE
    Return      :
----------------------------------------------------------------------------*/
static void _rgl_activate()
{
    GLcontext* ctx = CC;
    if (!gl_have_initialized)
    {
        return;
    }
    if (ctx->DriverFuncs.activate != NULL)
    {
        ctx->DriverFuncs.activate();
    }
    ctx->AmRendering = GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : _rgl_deactivate
    Description : "sleep" the GL.  calls the driver to close windows, restore vid
                  modes, whatever
    Inputs      :
    Outputs     : ctx->AmRendering = GL_FALSE
    Return      :
----------------------------------------------------------------------------*/
static void _rgl_deactivate()
{
    GLcontext* ctx = CC;
    if (!gl_have_initialized)
    {
        return;
    }
    if (ctx->DriverFuncs.deactivate != NULL)
    {
        ctx->DriverFuncs.deactivate();
    }
    ctx->AmRendering = GL_FALSE;
}

static GLuint rgl24BitPixelAt(GLint x, GLint y)
{
    GLcontext* ctx = CC;
    GLubyte* buf;
    GLuint* dbuf;

    if (x < 0 || y < 0 ||
        x >= ctx->Buffer.Width || y >= ctx->Buffer.Height)
    {
        return 0;
    }

    buf = GET_SCRATCH(ctx);
    if (buf == NULL)
    {
        return 0;
    }
    buf += ctx->Buffer.Pitch*y + 3*x;
    dbuf = (GLuint*)buf;
    return *dbuf;
}

static void rglPut24BitPixelAt(GLint x, GLint y, GLuint pixel)
{
    GLcontext* ctx = CC;
    GLubyte* buf;
    GLuint* dbuf;

    if (x < 0 || y < 0 ||
        x >= ctx->Buffer.Width || y >= ctx->Buffer.Height)
    {
        return;
    }

    buf = GET_SCRATCH(ctx);
    if (buf == NULL)
    {
        return;
    }
    buf += ctx->Buffer.Pitch*y + 3*x;
    dbuf = (GLuint*)buf;
    *dbuf = pixel;
}

static GLushort rglPixelAt(GLint x, GLint y)
{
    GLcontext* ctx = CC;
    GLubyte* buf;
    GLushort* sbuf;

    if (x < 0 || y < 0 ||
        x >= ctx->Buffer.Width || y >= ctx->Buffer.Height)
    {
        return 0;
    }

    buf = GET_SCRATCH(ctx);
    if (buf == NULL)
    {
        return 0;
    }
    buf += ctx->Buffer.Pitch*y + 2*x;
    sbuf = (GLushort*)buf;
    return *sbuf;
}

static void rglPutPixelAt(GLint x, GLint y, GLushort pixel)
{
    GLcontext* ctx = CC;
    GLubyte* buf;
    GLushort* sbuf;

    if (x < 0 || y < 0 ||
        x >= ctx->Buffer.Width || y >= ctx->Buffer.Height)
    {
        return;
    }

    buf = GET_SCRATCH(ctx);
    if (buf == NULL)
    {
        return;
    }
    buf += ctx->Buffer.Pitch*y + 2*x;
    sbuf = (GLushort*)buf;
    *sbuf = pixel;
}

static void cursorUnderLock(GLboolean lock)
{
    GLcontext* ctx = CC;
    if (lock)
    {
        if (ctx->RequireLocking && !ctx->ExclusiveLock)
        {
            LOCK_BUFFER(ctx);
            ctx->FrameBuffer = GET_FRAMEBUFFER(ctx);
        }
    }
    else
    {
        if (ctx->RequireLocking && !ctx->ExclusiveLock)
        {
            UNLOCK_BUFFER(ctx);
            ctx->FrameBuffer = NULL;
        }
    }
}

static void restore24BitCursorUnder(GLubyte* data, GLsizei width, GLsizei height, GLint x, GLint y)
{
    GLint sx, sy;
    GLuint* dp;

    cursorUnderLock(GL_TRUE);

    dp = (GLuint*)data;

    for (sy = y; sy < y + height; sy++)
    {
        for (sx = x; sx < x + width; sx++)
        {
            rglPut24BitPixelAt(sx, sy, *dp++);
        }
    }

    cursorUnderLock(GL_FALSE);
}

static void save24BitCursorUnder(GLubyte* data, GLsizei width, GLsizei height, GLint x, GLint y)
{
    GLcontext* ctx = CC;
    GLint sx, sy;
    GLuint* dp;

    cursorUnderLock(GL_TRUE);

    dp = (GLuint*)data;

    for (sy = y; sy < y + height; sy++)
    {
        for (sx = x; sx < x + width; sx++)
        {
            *dp++ = rgl24BitPixelAt(sx, sy);
        }
    }

    cursorUnderLock(GL_FALSE);
}

DLL void rglRestoreCursorUnder(GLubyte* data, GLsizei width, GLsizei height, GLint x, GLint y)
{
    GLcontext* ctx = CC;
    GLint sx, sy;
    GLushort* dp;

    if (ctx->Buffer.Depth > 16)
    {
        restore24BitCursorUnder(data, width, height, x, y);
        return;
    }

    cursorUnderLock(GL_TRUE);

    dp = (GLushort*)data;

    for (sy = y; sy < y + height; sy++)
    {
        for (sx = x; sx < x + width; sx++)
        {
            rglPutPixelAt(sx, sy, *dp++);
        }
    }

    cursorUnderLock(GL_FALSE);
}

DLL void rglSaveCursorUnder(GLubyte* data, GLsizei width, GLsizei height, GLint x, GLint y)
{
    GLcontext* ctx = CC;
    GLint sx, sy;
    GLushort* dp;

    if (ctx->Buffer.Depth > 16)
    {
        save24BitCursorUnder(data, width, height, x, y);
        return;
    }

    cursorUnderLock(GL_TRUE);

    dp = (GLushort*)data;

    for (sy = y; sy < y + height; sy++)
    {
        for (sx = x; sx < x + width; sx++)
        {
            *dp++ = rglPixelAt(sx, sy);
        }
    }

    cursorUnderLock(GL_FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : gl_texture_log
    Description : writes a texture logfile to disk
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gl_texture_log()
{
    hash_t* element;
    gl_texture_object* texobj;
    FILE* tlog;
    char  tlogString[64];
    GLuint i;

    tlog = fopen("textures.analysis", "wt");
    if (tlog == NULL)
    {
        return;
    }
    fprintf(tlog, "maxkey %d\n", _texobjs->maxkey);

    for (i = 0; i < TABLE_SIZE; i++)
    {
        element = _texobjs->table[i];
        while (element != NULL)
        {
            texobj = (gl_texture_object*)element->data;
            if (texobj != NULL)
            {
                switch (texobj->Format)
                {
                case GL_RGB:
                    sprintf(tlogString, "GL_RGB");
                    break;
                case GL_RGBA:
                    sprintf(tlogString, "GL_RGBA");
                    break;
                case GL_COLOR_INDEX:
                    sprintf(tlogString, "GL_COLOR_INDEX");
                    break;
                case GL_RGBA16:
                    sprintf(tlogString, "GL_RGBA16");
                    break;
                case GL_RGBA8:
                    sprintf(tlogString, "GL_RGBA8");
                    break;
                default:
                    sprintf(tlogString, "GL_???");
                }
                fprintf(tlog, "texobj %d : %dx%d %s\n", i, texobj->Width, texobj->Height, tlogString);
            }

            //next element
            element = element->next;
        }
    }

    fclose(tlog);
}

/*-----------------------------------------------------------------------------
    Name        : rglSelectDevice
    Description : select a device by name
    Inputs      : name - name of the device to select
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static char _selDevice[16];
DLL void rglSelectDevice(char* name, char* data)
{
    strncpy(_selDevice, name, 15);
    deviceToSelect = _selDevice;
    rglD3DSetDevice(data);
}

static char _d3dDevice[128];
DLL void rglD3DSetDevice(char* dev)
{
    if (dev == NULL)
    {
        _d3dDevice[0] = '\0';
    }
    else
    {
        strncpy(_d3dDevice, dev, 127);
    }
}

DLL char* rglD3DGetDevice(void)
{
    return _d3dDevice;
}

/*-----------------------------------------------------------------------------
    Name        : rglFeature
    Description : utilize a special feature (extension) of rGL
    Inputs      : feature - the feature
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL int rglFeature(unsigned int feature)
{
    GLcontext* ctx = CC;
    extern GLboolean WINDOWED;
    extern GLboolean SLOW;

    if (gl_is_shutdown)
    {
        return 0;
    }

    switch (feature)
    {
    case RGL_SKIP_RASTER:
        ctx->Speedy = GL_TRUE;
        break;
    case RGL_NOSKIP_RASTER:
        ctx->Speedy = GL_FALSE;
        break;
    case RGL_SPEEDY:
//        ctx->Speedy = !ctx->Speedy;
//        ctx->UsingLitPalette = !ctx->UsingLitPalette;
        if (ctx->DriverFuncs.super_clear != NULL)
        {
            ctx->DriverFuncs.super_clear();
        }
        break;
    case RGL_FULLSCREEN:
        IsFullscreen = GL_TRUE;
        break;
    case RGL_WINDOWED:
        IsFullscreen = GL_FALSE;
        break;
    case RGL_TRUECOLOR:
        IsTruecolor = GL_TRUE;
        break;
    case RGL_HICOLOR:
        IsTruecolor = GL_FALSE;
        break;
    case RGL_SLOWBLT:
        IsSlow = GL_TRUE;
        break;
    case RGL_FASTBLT:
        IsSlow = GL_FALSE;
        break;
    case RGL_NEXT_RENDERER:
        _rgl_next_renderer();
        break;
    case RGL_D3D_SHUFFLE:
        ctx->D3DReference++;
        if (ctx->D3DReference > 1)
        {
            ctx->D3DReference = 0;
        }
        _rgl_reinit_renderer();
        break;
    case RGL_REINIT_RENDERER:
        _rgl_reinit_renderer();
        break;
    case RGL_ACCELERATED:
        deviceToSelect = "fx";
        break;
    case RGL_ACTIVATE:
        _rgl_activate();
        break;
    case RGL_DEACTIVATE:
        _rgl_deactivate();
        break;
    case RGL_LOCK:
        if (ctx->RequireLocking && !ctx->ExclusiveLock)
        {
            LOCK_BUFFER(ctx);
            ctx->FrameBuffer = GET_FRAMEBUFFER(ctx);
            ctx->ExclusiveLock = GL_TRUE;
        }
        break;
    case RGL_UNLOCK:
        if (ctx->RequireLocking && ctx->ExclusiveLock)
        {
            UNLOCK_BUFFER(ctx);
            ctx->ExclusiveLock = GL_FALSE;
        }
        break;
    case RGL_SCREENSHOT:
//        gl_new_screenshot(SHOT_SINGLE);
        break;
    case RGL_MULTISHOT_START:
//        gl_new_screenshot(SHOT_MULTI);
//        gl_begin_multi();
        break;
    case RGL_MULTISHOT_END:
//        _screenshot = GL_FALSE;
//        _incshotseq();
//        gl_end_multi();
        break;
    case RGL_EFFECTPOINT:
        ctx->EffectPoint = GL_TRUE;
        break;
    case RGL_SPECULAR_RENDER:
        ctx->SpecularRender = GL_TRUE;
        break;
    case RGL_SPECULAR2_RENDER:
        ctx->SpecularRender = (GLboolean)2;
        break;
    case RGL_SPECULAR3_RENDER:
        ctx->SpecularRender = (GLboolean)3;
        break;
    case RGL_NORMAL_RENDER:
        ctx->SpecularRender = GL_FALSE;
        break;
    case RGL_SANSDEPTH:
        ctx->SansDepth = GL_TRUE;
        break;
    case RGL_NORMDEPTH:
        ctx->SansDepth = GL_FALSE;
        break;
    case RGL_GAMMA_UP:
        if (ctx->DriverFuncs.gamma_up != NULL)
        {
            ctx->DriverFuncs.gamma_up();
        }
        break;
    case RGL_GAMMA_DN:
        if (ctx->DriverFuncs.gamma_dn != NULL)
        {
            ctx->DriverFuncs.gamma_dn();
        }
        break;
    case RGL_CHROMAKEY_ON:
        if (ctx->DriverFuncs.chromakey != NULL)
        {
            ctx->DriverFuncs.chromakey(0,0,0, GL_TRUE);
            ctx->NewMask |= NEW_RASTER;
        }
        break;
    case RGL_CHROMAKEY_OFF:
        if (ctx->DriverFuncs.chromakey != NULL)
        {
            ctx->DriverFuncs.chromakey(0,0,0, GL_FALSE);
            ctx->NewMask |= NEW_RASTER;
        }
        break;
    case RGL_MAPPOINT:
        ctx->PointHack = !ctx->PointHack;
        break;
    case RGL_FASTBIND_ON:
        if (ctx->DriverFuncs.fastbind_set != NULL)
        {
            ctx->DriverFuncs.fastbind_set(GL_TRUE);
        }
        break;
    case RGL_FASTBIND_OFF:
        if (ctx->DriverFuncs.fastbind_set != NULL)
        {
            ctx->DriverFuncs.fastbind_set(GL_FALSE);
        }
        break;
    case RGL_RESET_LINECOUNTER:
        ctx->LineCount = 0;
        break;
    case RGL_GET_LINECOUNTER:
        return ctx->LineCount;
    case RGL_FEATURE_NOLINES:
#if LINES_DISABLEABLE
        ctx->LinesEnabled = GL_FALSE;
#endif
        break;
    case RGL_FEATURE_LINES:
#if LINES_DISABLEABLE
        ctx->LinesEnabled = GL_TRUE;
#endif
        break;
    case RGL_2D_QUADS:
        if (ctx->Buffer.Depth < 24 && !devices[activeDevice].fastBlend)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    case RGL_RENDER_2D_QUAD:
        //nothing here
        break;
    case RGL_SAVEBUFFER_ON:
        if (ctx->DriverFuncs.set_save_state != NULL)
        {
            ctx->DriverFuncs.set_save_state(1);
        }
        break;
    case RGL_SAVEBUFFER_OFF:
        if (ctx->DriverFuncs.set_save_state != NULL)
        {
            ctx->DriverFuncs.set_save_state(0);
        }
        break;
    case RGL_TEXTURE_LOG:
        gl_texture_log();
        break;
    case RGL_SHUTDOWN:
        gl_shutdown();
        break;
    case RGL_BROKEN_MIXED_DEPTHTEST:
    case RGL_COLOROP_ADD:
        if (ctx->DriverFuncs.feature_exists != NULL)
        {
            return ctx->DriverFuncs.feature_exists(feature);
        }
        else
        {
            return GL_FALSE;
        }
    case RGL_D3D_FULLSCENE:
        if (ctx->DriverFuncs.feature_exists != NULL)
        {
            return ctx->DriverFuncs.feature_exists(feature);
        }
        else
        {
            return GL_FALSE;
        }
    case GL_SHARED_TEXTURE_PALETTE_EXT:
        if (ctx->DriverFuncs.feature_exists != NULL)
        {
            return ctx->DriverFuncs.feature_exists(feature);
        }
        else
        {
            return GL_TRUE;
        }
    default:
        gl_problem(ctx, "rglFeature(feature)");
    }

    return 0;
}

/*-----------------------------------------------------------------------------
    Name        : rglIsFast
    Description : determines if a given "feature" of the renderer is "fast", ie.
                  hardware assisted or not
    Inputs      : feature - RGL_FEATURE_ALPHA, RGL_FEATURE_BLEND
    Outputs     :
    Return      : 0 (no) or not-0 (yes)
----------------------------------------------------------------------------*/
DLL unsigned char rglIsFast(unsigned int feature)
{
    if (devices == NULL)
    {
        return 0;
    }

    switch (feature)
    {
    case RGL_FEATURE_ALPHA:
        return devices[activeDevice].fastAlpha;

    case RGL_FEATURE_BLEND:
        return devices[activeDevice].fastBlend;

    default:
        return 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glDepthMask
    Description : set the depthbuffer writing flag
    Inputs      : flag - TRUE or FALSE
    Outputs     : ctx->DepthWrite set to flag
    Return      :
    State       : NEW_RASTER
----------------------------------------------------------------------------*/
DLL void API glDepthMask(GLboolean flag)
{
    GLcontext* ctx = CC;
    if (ctx->DepthWrite != flag)
    {
        ctx->DepthWrite = flag;
        ctx->NewMask |= NEW_RASTER;
    }
}

/*-----------------------------------------------------------------------------
    Name        : glColorMask
    Description : set the colorbuffer writing flag.  as per spec independent flags
                  for each component are specified, but this impl treats them as one
    Inputs      : red, green, blue - TRUE or FALSE
    Outputs     :
    Return      :
    Deviation   : individual components OR'd and treated as one
    State       : NEW_RASTER
----------------------------------------------------------------------------*/
DLL void API glColorMask(GLboolean red, GLboolean green, GLboolean blue)
{
    GLcontext* ctx = CC;
    GLboolean  flag;

    flag = (red || green || blue) ? GL_TRUE : GL_FALSE;
    if (ctx->ColorWrite != flag)
    {
        ctx->ColorWrite = flag;
        ctx->NewMask |= NEW_RASTER;
    }
}

typedef struct c4ub_v3f_s
{
    GLubyte c[4];
    GLfloat v[3];
} c4ub_v3f;

/*-----------------------------------------------------------------------------
    Name        : glDrawElements
    Description : [as per spec, but really limited]
    Inputs      : [as per spec]
    Outputs     :
    Return      :
    Deviation   : massively limited impl
----------------------------------------------------------------------------*/
DLL void API glDrawElements(GLenum mode, GLsizei count, GLenum type, GLvoid const* indices)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB;
    GLuint* indexList;
    c4ub_v3f* array;
    c4ub_v3f* pVert;
    GLint i;

    if (ctx->VertexArray == NULL)
    {
        gl_error(ctx, GL_INVALID_VALUE, "glDrawElements(VertexArray)");
        return;
    }

#if 0
    if (ctx->VertexFormat == GL_C4UB_V3F &&
        mode == GL_TRIANGLES &&
        ctx->DriverFuncs.draw_triangle_elements != NULL)
    {
        if (ctx->NewMask & NEW_RASTER)
        {
            gl_update_raster(ctx);
        }
        glBegin(mode);
        glEnd();
        ctx->DriverFuncs.draw_triangle_elements(count, (GLsizei)type, indices);
        return;
    }
#endif

    VB = ctx->VB;
    indexList = (GLuint*)indices;
    array = (c4ub_v3f*)ctx->VertexArray;

    glBegin(mode);

    switch (ctx->VertexFormat)
    {
    case GL_C4UB_V3F:
        if (ctx->DriverTransforms)
        {
            for (i = 0; i < count; i++)
            {
                pVert = &array[indexList[i]];
                V4_COPY(ctx->Current.Color, pVert->c);
                CALL_VERTEX(pVert->v[0], pVert->v[1], pVert->v[2]);
            }
        }
        else
        {
            for (i = 0; i < count; i++)
            {
                pVert = &array[indexList[i]];
                V4_COPY(VB->Color[i], pVert->c);
                V3_COPY(VB->Obj[i], pVert->v);
            }
        }
        break;

    default:
        glEnd();
        gl_error(ctx, GL_INVALID_VALUE, "glDrawElements(VertexFormat)");
        return;
    }

    VB->Count = count;
    glEnd();
}

/*-----------------------------------------------------------------------------
    Name        : glDrawArrays
    Description : super limited impl, only supports one type of structure that
                  gets rendered as points
    Inputs      : mode - ignored
                  first - starting point in the array
                  count - number of elements
    Outputs     :
    Return      :
    Deviation   : massively limited impl
----------------------------------------------------------------------------*/
DLL void API glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    GLcontext* ctx = CC;
    GLint i;
    vertex_buffer* VB = ctx->VB;
    GLuint vcount = 0;
    GLboolean smoothed = (ctx->ShadeModel == GL_SMOOTH);
    GLboolean depthed = ctx->DepthTest;
    GLboolean depthwrited = ctx->DepthWrite;

    if (ctx->VertexArray == NULL)
    {
        gl_error(ctx, GL_INVALID_VALUE, "glDrawArrays(VertexArray)");
        return;
    }

    ctx->ShadeModel = GL_SMOOTH;
    ctx->DepthTest = GL_FALSE;
    ctx->DepthWrite = GL_FALSE;
    gl_update_raster(ctx);

    if (ctx->VertexFormat == GL_C3F_V3F)
    {
        GLfloat* r = (GLfloat*)ctx->VertexArray;
        r += 6*sizeof(GLfloat)*first;

        glBegin(mode);
        for (i = first; i < count; i++)
        {
            V4_SET(VB->Obj[vcount], r[3], r[4], r[5], 1.0f);
            VB->Color[vcount][0] = (GLubyte)(ctx->Buffer.rscale * r[0]);
            VB->Color[vcount][1] = (GLubyte)(ctx->Buffer.gscale * r[1]);
            VB->Color[vcount][2] = (GLubyte)(ctx->Buffer.bscale * r[2]);
            VB->Color[vcount][3] = (GLubyte)(ctx->Buffer.ascale * 1.0f);

            vcount++;
            VB->Count = vcount;
            r += 6;
        }
        glEnd();
    }
    else if (ctx->VertexFormat == GL_C4UB_V3F)
    {
        c4ub_v3f* ptr = (c4ub_v3f*)ctx->VertexArray;
        if (ctx->DriverTransforms && ctx->DriverFuncs.draw_c4ub_v3f)
        {
            glBegin(mode);
            glEnd();
            ctx->DriverFuncs.draw_c4ub_v3f(mode, first, count);
        }
        else
        {
            ptr += first * sizeof(c4ub_v3f);

            glBegin(mode);
            if (ctx->DriverTransforms)
            {
                for (i = first; i < count; i++, ptr++)
                {
                    ctx->Current.Color[0] = ptr->c[0];
                    ctx->Current.Color[1] = ptr->c[1];
                    ctx->Current.Color[2] = ptr->c[2];
                    ctx->Current.Color[3] = ptr->c[3];
                    CALL_VERTEX(ptr->v[0], ptr->v[1], ptr->v[2]);
                }
            }
            else
            {
                for (i = first; i < count; i++, vcount++, ptr++)
                {
                    V4_SET(VB->Obj[vcount], ptr->v[0], ptr->v[1], ptr->v[2], 1.0f);
                    VB->Color[vcount][0] = ptr->c[0];
                    VB->Color[vcount][1] = ptr->c[1];
                    VB->Color[vcount][2] = ptr->c[2];
                    VB->Color[vcount][3] = ptr->c[3];
                }
                VB->Count = vcount;
            }
            glEnd();
        }
    }
    else
    {
        gl_error(ctx, GL_INVALID_VALUE, "glDrawArrays(VertexFormat)");
    }

    if (smoothed)
        ctx->ShadeModel = GL_SMOOTH;
    if (depthed)
        ctx->DepthTest = GL_TRUE;
    if (depthwrited)
        ctx->DepthWrite = GL_TRUE;
    gl_update_raster(ctx);
}

/*-----------------------------------------------------------------------------
    Name        : glInterleavedArrays
    Description : super limited impl
    Inputs      : format & stride are checked for validity w/in this limited impl,
                  pointer gets used
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glInterleavedArrays(GLenum format, GLsizei stride, GLvoid const* pointer)
{
    GLcontext* ctx = CC;

    if ((format != GL_C3F_V3F && format != GL_C4UB_V3F) || stride != 0)
    {
        gl_error(ctx, GL_INVALID_VALUE, "glInterLeavedArrays(format or stride)");
        return;
    }

    ctx->VertexArray  = (GLvoid*)pointer;
    ctx->VertexFormat = format;
}

/*-----------------------------------------------------------------------------
    Name        : glVertexPointer
    Description : super limited impl
    Inputs      : size - 3 or 4
                  type - assumed to be GL_FLOAT
                  stride - assumed to be 4*sizeof(GLfloat)
                  pointer - this actually gets used
    Outputs     :
    Return      :
    Note        : don't mix this with InterleavedArrays
----------------------------------------------------------------------------*/
DLL void API glVertexPointer(GLint size, GLenum type, GLsizei stride, GLvoid const* pointer)
{
    GLcontext* ctx = CC;
    ctx->VertexArray = (GLvoid*)pointer;
    ctx->VertexSize  = size;
}

DLL void API glEnableClientState(GLenum array)
{
    //nothing here
}

DLL void API glDisableClientState(GLenum array)
{
    //nothing here
}

/*-----------------------------------------------------------------------------
    Name        : glArrayElement
    Description : only supports VertexPointer
    Inputs      : i - index of the element
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glArrayElement(GLint i)
{
    gl_ArrayElement(i);
}

static void gl_ArrayElement_cnt_3(GLint i)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLfloat (*array)[4] = ctx->VertexArray;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = array[i][0];
    VB->Obj[count][1] = array[i][1];
    VB->Obj[count][2] = array[i][2];
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->Normal[count][0] = ctx->Current.Normal[0];
    VB->Normal[count][1] = ctx->Current.Normal[1];
    VB->Normal[count][2] = ctx->Current.Normal[2];

    VB->TexCoord[count][0] = ctx->Current.TexCoord[0];
    VB->TexCoord[count][1] = ctx->Current.TexCoord[1];

    count++;
    VB->Count = count;
}

static void gl_ArrayElement_ct_3(GLint i)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLfloat (*array)[4] = ctx->VertexArray;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = array[i][0];
    VB->Obj[count][1] = array[i][1];
    VB->Obj[count][2] = array[i][2];
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->TexCoord[count][0] = ctx->Current.TexCoord[0];
    VB->TexCoord[count][1] = ctx->Current.TexCoord[1];

    count++;
    VB->Count = count;
}

static void gl_ArrayElement_cn_3(GLint i)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLfloat (*array)[4] = ctx->VertexArray;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = array[i][0];
    VB->Obj[count][1] = array[i][1];
    VB->Obj[count][2] = array[i][2];
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->Normal[count][0] = ctx->Current.Normal[0];
    VB->Normal[count][1] = ctx->Current.Normal[1];
    VB->Normal[count][2] = ctx->Current.Normal[2];

    count++;
    VB->Count = count;
}

static void gl_ArrayElement_c_3(GLint i)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLfloat (*array)[4] = ctx->VertexArray;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Obj[count][0] = array[i][0];
    VB->Obj[count][1] = array[i][1];
    VB->Obj[count][2] = array[i][2];
    VB->Obj[count][3] = 1.0f;

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    count++;
    VB->Count = count;
}

static void gl_ArrayElement_ct(GLint i)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLfloat (*array)[4] = ctx->VertexArray;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Clip[count][0] = array[i][0];
    VB->Clip[count][1] = array[i][1];
    VB->Clip[count][2] = array[i][2];
    VB->Clip[count][3] = array[i][3];

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    VB->TexCoord[count][0] = ctx->Current.TexCoord[0];
    VB->TexCoord[count][1] = ctx->Current.TexCoord[1];

    count++;
    VB->Count = count;
}

static void gl_ArrayElement_c(GLint i)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLfloat (*array)[4] = ctx->VertexArray;
    GLuint count = VB->Count;
    GLuint* sp;
    GLuint* dp;

    VB->Clip[count][0] = array[i][0];
    VB->Clip[count][1] = array[i][1];
    VB->Clip[count][2] = array[i][2];
    VB->Clip[count][3] = array[i][3];

    sp = (GLuint*)ctx->Current.Color;
    dp = (GLuint*)VB->Color[count];
    *dp = *sp;

    count++;
    VB->Count = count;
}

/*-----------------------------------------------------------------------------
    Name        : rglSetAllocs
    Description : sets memory allocation function pointers
    Inputs      : allocFunc - "malloc"
                  freeFunc - "free"
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void rglSetAllocs(MemAllocFunc allocFunc, MemFreeFunc freeFunc)
{
    gAllocFunc = allocFunc;
    gFreeFunc  = freeFunc;
}

/*-----------------------------------------------------------------------------
    Name        : glLitColorTableEXT
    Description : binds a shared pre-lit colortable to the context
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glLitColorTableEXT(
    GLenum target, GLenum internalformat, GLsizei length,
    GLenum format, GLenum type, GLvoid const* palette)
{
    GLcontext* ctx = CC;

    if (target != GL_TEXTURE_2D)
    {
        GLint* bits = (GLint*)palette;

        if (devices[activeDevice].litPalette)
        {
            switch (ctx->Buffer.PixelType)
            {
            case GL_RGB555:
                *bits = 15;
                break;
            case GL_RGB565:
                *bits = 16;
                break;
            default:
                *bits = 24;
            }
        }
        else
        {
            *bits = 0;
        }
        return;
    }

    //ASSUME: levels == 16
    //ignore target
    //ignore internalformat
    //ignore length
    //ignore type

    ctx->SharedIllumPalettes = (GLushort*)palette;
}

/*-----------------------------------------------------------------------------
    Name        : glColorTableEXT
    Description : binds a colortable (palette) to the currently bound texture object
    Inputs      : [as per spec, but only palette is used]
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glColorTableEXT(
    GLenum target, GLenum internalformat, GLsizei length,
    GLenum format, GLenum type, GLvoid const* palette)
{
    GLcontext* ctx = CC;
    GLubyte* pal = (GLubyte*)palette;

    gl_texture_object* tex = ctx->TexBoundObject;

    //ignore target
    //ignore internalformat
    //ignore length
    //ignore format
    //ignore type

    if (ctx->UsingSharedPalette)
    {
        GLubyte* ppal;
        GLint    i;

        ctx->SharedPalette = (GLubyte*)palette;

        if (!devices[activeDevice].fastBlend)
        {
            if (ctx->Buffer.PixelType == GL_RGB565)
            {
                for (i = 0, ppal = ctx->SharedPalette; i < 256; i++, ppal += 4)
                {
                    ctx->SharedPalette16[i] = FORM_RGB565(ppal[0], ppal[1], ppal[2]);
                }
            }
            else
            {
                for (i = 0, ppal = ctx->SharedPalette; i < 256; i++, ppal += 4)
                {
                    ctx->SharedPalette16[i] = FORM_RGB555(ppal[0], ppal[1], ppal[2]);
                }
            }
#if 0
            for (i = 0, ppal = ctx->SharedPalette; i < 256; i++, ppal += 4)
            {
                ctx->SharedPalette444[i] = FORM_RGB444(ppal[0] >> 4, ppal[1] >> 4, ppal[2] >> 4);
            }
#endif
        }
    }
    else
    {
        if (tex != NULL)
        {
            tex->Palette = (GLubyte*)pal;
        }
    }

    if (ctx->DriverFuncs.tex_palette != NULL)
    {
        ctx->DriverFuncs.tex_palette(tex);
    }
}

/*-----------------------------------------------------------------------------
    Name        : glColorTable
    Description : alias for glColorTableEXT() (for consistency, should be
                  removed once we're sure all references to glColorTable() are
                  cleaned out)
    Inputs      : [as per spec, but only palette is used]
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glColorTable(
    GLenum target, GLenum internalformat, GLsizei length,
    GLenum format, GLenum type, GLvoid const* palette)
{ glColorTableEXT(target, internalformat, length, format, type, palette); }

/*-----------------------------------------------------------------------------
    Name        : glLitColorTable
    Description : alias for glColorTableEXT() (like glColorTable(), it's for
                  consistency and should eventually be removed)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glLitColorTable(
    GLenum target, GLenum internalformat, GLsizei length,
    GLenum format, GLenum type, GLvoid const* palette)
{ glLitColorTableEXT(target, internalformat, length, format, type, palette); }

/*-----------------------------------------------------------------------------
    Name        : glPixelTransferf
    Description : pixel transfer control
    Inputs      : pname - pixel parameter to update
                  param - the value to update with
    Outputs     :
    Return      :
    State       : NEW_RASTER
    Deviation   : only support RED_BIAS, GREEN_BIAS, BLUE_BIAS
----------------------------------------------------------------------------*/
DLL void API glPixelTransferf(GLenum pname, GLfloat param)
{
    GLcontext* ctx = CC;
    param = CLAMP(param, 0.0f, 1.0f);

    switch (pname)
    {
    case GL_RED_BIAS:
        ctx->FloatBias[0] = param;
        ctx->ByteBias[0] = FAST_TO_INT(param*255.0f);
        break;
    case GL_GREEN_BIAS:
        ctx->FloatBias[1] = param;
        ctx->ByteBias[1] = FAST_TO_INT(param*255.0f);
        break;
    case GL_BLUE_BIAS:
        ctx->FloatBias[2] = param;
        ctx->ByteBias[2] = FAST_TO_INT(param*255.0f);
        break;
    default:
        gl_error(ctx, GL_INVALID_ENUM, "glPixelTransfer(pname)");
        return;
    }

    if (ctx->FloatBias[0] == 0.0f &&
        ctx->FloatBias[1] == 0.0f &&
        ctx->FloatBias[2] == 0.0f)
    {
        ctx->Bias = GL_FALSE;
    }
    else
    {
        ctx->Bias = GL_TRUE;
    }

    ctx->NewMask |= NEW_RASTER;
}

/*-----------------------------------------------------------------------------
    Name        : gl_shutdown
    Description : shut down the GL, calling driver shutdown func and whatnot
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gl_shutdown()
{
    GLcontext* ctx = CC;
    GLuint i;
    hash_t* element;
    gl_texture_object* texobj;

    gl_is_shutdown = GL_TRUE;

    if (sbuf != NULL)
    {
        free(sbuf);
        sbuf = NULL;
    }

#if TEXTURE_LOG
    gl_texture_log();
#endif

    if (ctx->DriverFuncs.shutdown_driver != NULL)
    {
        ctx->DriverFuncs.shutdown_driver(ctx);
    }

    for (i = 0; i < TABLE_SIZE; i++)
    {
        element = _texobjs->table[i];
        while (element != NULL)
        {
            texobj = (gl_texture_object*)element->data;
            if (texobj != NULL)
            {
                if (texobj->Data != NULL)
                {
                    gl_Free(texobj->Data);
                }
                if (texobj->created && ctx->DriverFuncs.tex_del != NULL)
                {
                    ctx->DriverFuncs.tex_del(texobj);
                }

                gl_Free(texobj);
            }

            //next chained element
            element = element->next;
        }
    }

    hashDeleteTable(_texobjs);

    gl_free_devices();

    free(CC);

#if 0
    {
        FILE* out = fopen("mem.dat", "wt");
        if (out == NULL)
        {
            return;
        }
        fprintf(out, "%dK bytes allocated\n", gl_allocated >> 10);
        fprintf(out, "VB->Count max %d\n", _vbcount_max);
        fclose(out);
    }
#endif
}

#define V3_SCALE(V,S) \
        { \
            V[0] *= S; \
            V[1] *= S; \
            V[2] *= S; \
        }

#define V3_ADD(C,A,B) \
        { \
            C[0] = A[0] + B[0]; \
            C[1] = A[1] + B[1]; \
            C[2] = A[2] + B[2]; \
        }

typedef GLfloat recttype[4];

//helper for clip testing
static GLboolean _lineinside(recttype* rectpos, GLuint v1, GLuint v2)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;

    if (VB->ClipMask[v1] | VB->ClipMask[v2])
    {
        GLuint vv1 = v1;
        GLuint vv2 = v2;
        VB->Count = 2;
        VB->Free = 3;
        return gl_viewclip_line(ctx, &vv1, &vv2);
    }
    else
    {
        return GL_TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rglIsClipped
    Description : determines whether a bounding box is clipped by the frustum
    Inputs      : collrectoffset - gshaw's bbox array
                  uplength, rightlength, forwardlength - dimensions
    Outputs     :
    Return      : TRUE if totally outside of frustum, FALSE otherwise
    Note        : transform_points3 NO LONGER UPDATES MATRIX TYPES !!!
----------------------------------------------------------------------------*/
DLL unsigned char rglIsClipped(
    GLfloat* collrectoffset,
    GLfloat uplength, GLfloat rightlength, GLfloat forwardlength)
{
    GLcontext* ctx = CC;
    vertex_buffer* VB = ctx->VB;
    GLboolean result;
    GLint i;

    GLfloat upvector[3] = {1.0f, 0.0f, 0.0f};
    GLfloat rightvector[3] = {0.0f, 1.0f, 0.0f};
    GLfloat forwardvector[3] = {0.0f, 0.0f, 1.0f};

    recttype* rectpos = VB->Obj;

    void transform_points3(GLcontext* ctx, GLuint n, GLfloat vObj[][4], GLfloat vEye[][4]);
    void project_and_cliptest(GLcontext* ctx, GLuint n, GLfloat vEye[][4], GLfloat vClip[][4], GLubyte clipMask[], GLubyte* orMask, GLubyte* andMask);
    void gl_reset_vb(GLcontext* ctx, GLboolean allDone);

    for (i = 0; i < 3; i++)
    {
        rectpos[0][i] = collrectoffset[i];
    }

    V3_SCALE(upvector, uplength);
    V3_SCALE(rightvector, rightlength);
    V3_SCALE(forwardvector, forwardlength);

    for (i = 0; i < 8; i++)
    {
        rectpos[i][3] = 1.0f;
    }

    V3_ADD(rectpos[1], rectpos[0], rightvector);
    V3_ADD(rectpos[2], rectpos[1], forwardvector);
    V3_ADD(rectpos[3], rectpos[0], forwardvector);

    V3_ADD(rectpos[4], rectpos[0], upvector);
    V3_ADD(rectpos[5], rectpos[1], upvector);
    V3_ADD(rectpos[6], rectpos[2], upvector);
    V3_ADD(rectpos[7], rectpos[3], upvector);

    VB->ClipOrMask = 0;
    VB->ClipAndMask = CLIP_ALL_BITS;
    VB->Count = 8;
    VB->Start = 0;

    transform_points3(ctx, 8, VB->Obj, VB->Eye);
    project_and_cliptest(ctx, 8, VB->Eye, VB->Clip, VB->ClipMask,
                         &VB->ClipOrMask, &VB->ClipAndMask);
#if 0
    result = VB->ClipAndMask ? GL_TRUE : GL_FALSE;
#else

    result = GL_TRUE;
    if (VB->ClipOrMask)
    {
#define LINETEST(v1,v2) if (_lineinside(rectpos, v1, v2)) { result = GL_FALSE; goto PASSED; }
        GLuint v1, v2;
        LINETEST(0,1);
        LINETEST(1,2);
        LINETEST(2,3);
        LINETEST(3,0);
        LINETEST(4,5);
        LINETEST(5,6);
        LINETEST(6,7);
        LINETEST(7,4);
        LINETEST(0,4);
        LINETEST(1,5);
        LINETEST(2,6);
        LINETEST(3,7);
#undef LINETEST
    }
    else
    {
        result = GL_FALSE;
    }
PASSED:
#endif
    gl_reset_vb(ctx, GL_TRUE);
    return (unsigned char)result;
}

/*-----------------------------------------------------------------------------
    Name        : rglDrawLightVectors
    Description : renders vectors corresponding to the direction of a GL light
    Inputs      : lightno - which light
                  position - where to start the vector
    Outputs     : a light vector gets rendered
    Return      :
----------------------------------------------------------------------------*/
DLL void rglDrawLightVectors(GLuint lightno, GLfloat* position)
{
    GLcontext* ctx = CC;
    GLfloat pos[3], v[3];
    gl_light* light = &ctx->Light[lightno];

    v3_copy(pos, light->OPos);      //light direction/position
    v3_normalize(pos);              //unitize
    V3_SCALE(pos, 100.0f);          //make larger -ve'ly
    V3_ADD(pos, pos, position);     //offset by ship position

    glBegin(GL_LINES);
    glVertex3f(position[0], position[1], position[2]);
    glVertex3f(pos[0], pos[1], pos[2]);
    glEnd();
}

/*-----------------------------------------------------------------------------
    Name        : glDrawPixels
    Description : draw a pixel map at current raster position (2D)
    Inputs      : width, height - dimensions
                  format - map type
                  type - eg. UNSIGNED_BYTE, FLOAT, &c
                  pixels - pixel data
    Outputs     :
    Return      :
    Deviation   : an rGL driver is only required to support UNSIGNED_BYTE type
----------------------------------------------------------------------------*/
DLL void API glDrawPixels(
    GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid const* pixels)
{
    GLboolean animatic;
    GLcontext* ctx = CC;
    ctx->Current.Bitmap = (GLubyte*)pixels;

    if ((format == GL_RGB || format == GL_RGBA16) && width == 640 && height == 480)
    {
        animatic = GL_TRUE;
    }
    else
    {
        animatic = GL_FALSE;
    }

    if (ctx->DriverFuncs.draw_pixels != NULL)
    {
        if (ctx->NewMask & NEW_RASTER)
        {
            gl_update_raster(ctx);
        }

        if (!animatic) gl_lock_framebuffer();

        ctx->DriverFuncs.draw_pixels(ctx, width, height, format, type);

        if (!animatic) gl_unlock_framebuffer();
    }
}

/*-----------------------------------------------------------------------------
    Name        : glReadPixels
    Description : read a pixel map from the framebuffer into supplied buffer
    Inputs      : width, height - dimensions
                  format - map type
                  type - UNSIGNED_BYTE
                  pixels - pixel data goes here
    Outputs     : pixels is filled
    Return      :
    Deviation   : an rGL driver is only required to support UNSIGNED_BYTE type
                  and RGBA format, and so far need only support a full screen read
----------------------------------------------------------------------------*/
DLL void API glReadPixels(
    GLint x, GLint y, GLsizei width, GLsizei height,
    GLenum format, GLenum type, GLvoid* pixels)
{
    GLcontext* ctx = CC;
    ctx->Current.Bitmap = (GLubyte*)pixels;

    if (ctx->DriverFuncs.read_pixels != NULL)
    {
        gl_lock_framebuffer();

        ctx->DriverFuncs.read_pixels(ctx, x, y, width, height, format, type);

        gl_unlock_framebuffer();
    }
}

/*-----------------------------------------------------------------------------
    Name        : glFogi
    Description : set fog parameters
    Inputs      : pname - parameter to set
                  param - value to set parameter to
    Outputs     :
    Return      :
    Deviation   : only pname == GL_FOG_MODE is supported
----------------------------------------------------------------------------*/
DLL void API glFogi(GLenum pname, GLint param)
{
    GLcontext* ctx = CC;
    if (pname == GL_FOG_MODE)
    {
        ctx->FogMode = param;
    }

    ctx->NewMask |= NEW_RASTER;
}

/*-----------------------------------------------------------------------------
    Name        : glFogf
    Description : set fog parameters
    Inputs      : pname - parameter to st
                  param - value to set parameter to
    Outputs     :
    Return      :
    Deviation   : only pname == GL_FOG_DENSITY is supported
----------------------------------------------------------------------------*/
DLL void API glFogf(GLenum pname, GLfloat param)
{
    GLcontext* ctx = CC;
    if (pname == GL_FOG_DENSITY)
    {
        ctx->FogDensity = param;
    }

    ctx->NewMask |= NEW_RASTER;
}

/*-----------------------------------------------------------------------------
    Name        : glFogfv
    Description : set fog parameters
    Inputs      : pname - parameter to set
                  params - value(s) to set parameter to
    Outputs     :
    Return      :
    Deviation   : only pname == GL_FOG_COLOR is supported
----------------------------------------------------------------------------*/
DLL void API glFogfv(GLenum pname, GLfloat const* params)
{
    GLcontext* ctx = CC;
    if (pname == GL_FOG_COLOR)
    {
        V4_COPY(ctx->FogColor, params);
    }

    ctx->NewMask |= NEW_RASTER;
}

/*-----------------------------------------------------------------------------
    Name        : glOrtho
    Description : create an orthographic projection matrix & multiply w/ matrix
                  currently loaded & active
    Inputs      : [as per spec]
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                 GLdouble nearval, GLdouble farval)
{
    GLfloat x, y, z;
    GLfloat tx, ty, tz;
    GLfloat m[16];

    x = 2.0 / (right - left);
    y = 2.0 / (top - bottom);
    z = -2.0 / (farval - nearval);
    tx = -(right + left) / (right - left);
    ty = -(top + bottom) / (top - bottom);
    tz = -(farval + nearval) / (farval - nearval);

#define M(row,col) m[col*4 + row]
    M(0,0) = x;    M(0,1) = 0.0F; M(0,2) = 0.0F; M(0,3) = tx;
    M(1,0) = 0.0F; M(1,1) = y;    M(1,2) = 0.0F; M(1,3) = ty;
    M(2,0) = 0.0F; M(2,1) = 0.0F; M(2,2) = z;    M(2,3) = tz;
    M(3,0) = 0.0F; M(3,1) = 0.0F; M(3,2) = 0.0F; M(3,3) = 1.0F;
#undef M

    glMultMatrixf(m);
}

/*-----------------------------------------------------------------------------
    Name        : rglNumPolys
    Description : returns g_NumPolys, the number of triangles rendered since the
                  last Flush.  tris that are culled are not included in this stat
    Inputs      :
    Outputs     :
    Return      : g_NumPolys
----------------------------------------------------------------------------*/
DLL GLuint rglNumPolys()
{
    return g_NumPolys;
}

/*-----------------------------------------------------------------------------
    Name        : rglCulledPolys
    Description : returns g_CulledPolys, the number of triangles culled since the
                  last Flush
    Inputs      :
    Outputs     :
    Return      : g_CulledPolys
----------------------------------------------------------------------------*/
DLL GLuint rglCulledPolys()
{
    return g_CulledPolys;
}

/*-----------------------------------------------------------------------------
    Name        : rglAnotherPoly
    Description : externally accessible way to increment g_NumPolys
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void rglAnotherPoly()
{
    g_NumPolys++;
}

/*-----------------------------------------------------------------------------
    Name        : rglSpecExp
    Description : set the exponent for alpha falloff w/ SPECHACK3 shader
    Inputs      : index - [0..2] which exponent to modify
                  exp - the exponent.  if exp == -1.0f, reset to default
    Outputs     : ctx->SpecularExponent is set to exp
    Return      :
----------------------------------------------------------------------------*/
DLL void rglSpecExp(GLint index, GLfloat exp)
{
    GLcontext* ctx = CC;
    if (index < 0 || index > 2)
    {
        gl_error(ctx, GL_INVALID_VALUE, "rglSpecExp(index)");
        return;
    }

    ctx->SpecularExponent[index] = (exp == -1.0f) ? ctx->SpecularDefault[index] : exp;
}

/*-----------------------------------------------------------------------------
    Name        : rglLightingAdjust
    Description : hook for fading (w/ alpha or -> black) the colour of vertices
    Inputs      : adj - [0..1] the adjustment.  0 means no adjustment
    Outputs     : ctx->LightingAdjust is modified
    Return      :
----------------------------------------------------------------------------*/
DLL void rglLightingAdjust(GLfloat adj)
{
    GLcontext* ctx = CC;
    if (adj < 0.0f || adj > 1.0f)
    {
        gl_error(ctx, GL_INVALID_VALUE, "rglLightingAdjust(adj)");
        return;
    }

    ctx->LightingAdjust = adj;
}

/*-----------------------------------------------------------------------------
    Name        : glLightModeli
    Description : control lighting model parameters
    Inputs      : pname - the parameter to control
                  param - the value of the parameter
    Outputs     :
    Return      :
    Deviation   : only support GL_LIGHT_MODEL_TWO_SIDE
    State       : NEW_LIGHTING
----------------------------------------------------------------------------*/
DLL void API glLightModeli(GLenum pname, GLint param)
{
    GLcontext* ctx = CC;
    switch (pname)
    {
    case GL_LIGHT_MODEL_TWO_SIDE:
        if (ctx->TwoSide != (GLboolean)param)
        {
            ctx->TwoSide = (GLboolean)param;
            ctx->NewMask |= NEW_LIGHTING;
        }
        break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rglEnable
    Description : enable an rGL-specific capability
    Inputs      : cap - the capability to enable
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void rglEnable(GLint cap)
{
    GLcontext* ctx = CC;
    switch (cap)
    {
    case RGL_RASTERIZE_ONLY:
        ctx->RasterizeOnly = GL_TRUE;
        break;

    case RGL_D3D_FULLSCENE:
        if (ctx->DriverFuncs.fullscene != NULL)
        {
            ctx->DriverFuncs.fullscene(GL_TRUE);
        }
        break;

    default:
        gl_error(ctx, GL_INVALID_ENUM, "rglEnable(cap)");
    }
}

/*-----------------------------------------------------------------------------
    Name        : rglDisable
    Description : disable an rGL-specific capability
    Inputs      : cap - the capability to disable
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void rglDisable(GLint cap)
{
    GLcontext* ctx = CC;
    switch (cap)
    {
    case RGL_RASTERIZE_ONLY:
        ctx->RasterizeOnly = GL_FALSE;
        break;

    case RGL_D3D_FULLSCENE:
        if (ctx->DriverFuncs.fullscene != NULL)
        {
            ctx->DriverFuncs.fullscene(GL_FALSE);
        }
        break;

    default:
        gl_error(ctx, GL_INVALID_ENUM, "rglDisable(cap)");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glSuperClear
    Description : really, really clear the colorbuffer (if a driver has support)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void glSuperClear()
{
    GLcontext* ctx = CC;
    if (ctx->DriverFuncs.super_clear != NULL)
    {
        ctx->DriverFuncs.super_clear();
    }
}

/*-----------------------------------------------------------------------------
    Name        : glHint
    Description : provide hints to the GL
    Inputs      : target - which hint
                  mode - the hint itself
    Outputs     :
    Return      :
    Deviation   : only PERSPECTIVE_CORRECTION_HINT is recognized.  FASTEST sets
                  non-perspective correct texturing, anything else == perspective
                  correct (a driver may not respect these hints)
----------------------------------------------------------------------------*/
DLL void API glHint(GLenum target, GLenum mode)
{
    GLcontext* ctx = CC;
    switch (target)
    {
    case GL_PERSPECTIVE_CORRECTION_HINT:
        if (mode == GL_FASTEST)
        {
            ctx->PerspectiveCorrect = GL_FALSE;
        }
        else
        {
            ctx->PerspectiveCorrect = GL_TRUE;
        }
        break;

    default:
        gl_error(ctx, GL_INVALID_VALUE, "glHint(target)");
    }
}

/*-----------------------------------------------------------------------------
    Name        : rglBackground
    Description : render a bitmap into the framebuffer via a driver function that
                  may not be provided.  background assumed to be 640x480; it's the
                  driver's duty to scale / crop / whatever if the framebuffer differs
    Inputs      : pixels - the RGB UNSIGNED_BYTE pixel data
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void rglBackground(GLubyte* pixels)
{
#if 0
    GLcontext* ctx = CC;
    if (ctx->DriverFuncs.draw_background != NULL)
    {
        ctx->DriverFuncs.draw_background(pixels);
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : glPolygonMode
    Description : set either filled or line (outline) polygons
    Inputs      :
    Outputs     :
    Return      :
    Deviation   : face param ignored (mode is applied to both sides of the polygon).
                  only support FILL and LINE (no POINT) mode
----------------------------------------------------------------------------*/
DLL void API glPolygonMode(GLenum face, GLenum mode)
{
    GLcontext* ctx = CC;

    switch (mode)
    {
    case GL_FILL:
    case GL_LINE:
        ctx->PolygonMode = mode;
        break;
    default:
        gl_error(ctx, GL_INVALID_VALUE, "glPolygonMode(mode)");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glClipPlane
    Description : set one of the user-defined (eye-space) clipping planes
    Inputs      : plane - GL_CLIP_PLANEn (n == 0..5)
                  equation - A B C D, coefficients of the plane equation
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glClipPlane(GLenum plane, GLdouble const* equation)
{
    GLcontext* ctx = CC;
    GLfloat fequation[4];
    GLint p;

    for (p = 0; p < 4; p++)
    {
        fequation[p] = (GLfloat)equation[p];
    }

    p = (GLint)plane - (GLint)GL_CLIP_PLANE0;
    if (p < 0 || p >= MAX_CLIP_PLANES)
    {
        gl_error(ctx, GL_INVALID_ENUM, "glClipPlane");
    }

    if (ctx->NewMask & NEW_MODELVIEWINV)
    {
        gl_invert_modelview();
    }

    v4_project(ctx->ClipEquation[p], fequation, ctx->ModelViewInv);
}

/*-----------------------------------------------------------------------------
    Name        : glGetTexLevelParameteriv
    Description : returns texture parameter values
    Inputs      : [as per spec]
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glGetTexLevelParameteriv(
    GLenum target, GLint level, GLenum pname, GLint* params)
{
    GLcontext* ctx = CC;
    gl_texture_object* tex = ctx->TexBoundObject;

    if (tex != NULL && tex->created)
    {
        switch (pname)
        {
        case GL_TEXTURE_WIDTH:
            *params = tex->Width;
            break;

        case GL_TEXTURE_HEIGHT:
            *params = tex->Height;
            break;

        case GL_TEXTURE_INTERNAL_FORMAT:
            *params = tex->Format;
            break;

        default:
            gl_error(ctx, GL_INVALID_ENUM, "glGetTexLevelParameteriv(pname)");
        }
    }
    else
    {
        gl_error(ctx, GL_INVALID_VALUE, "glGetTexLevelParameteriv");
    }
}

/*-----------------------------------------------------------------------------
    Name        : glGetTexImage
    Description : return a texture image
    Inputs      : [as per spec, but almost totally ignored]
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DLL void API glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels)
{
    GLcontext* ctx = CC;
    gl_texture_object* tex = ctx->TexBoundObject;

    if (tex != NULL && tex->created)
    {
        if (tex->Format == GL_RGBA)
        {
            MEMCPY(pixels, tex->Data, 4*tex->Width*tex->Height);
        }
        else if (tex->Format == GL_COLOR_INDEX)
        {
            MEMCPY(pixels, tex->Data, tex->Width*tex->Height);
        }
        else
        {
            gl_error(ctx, GL_INVALID_ENUM, "glGetTexImage");
        }
    }
    else
    {
        gl_error(ctx, GL_INVALID_VALUE, "glGetTexImage");
    }
}


/*=============================================================================
    stubs
=============================================================================*/

DLL void API glMap1f(
    GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order,
    GLfloat const* points) {}

DLL void API glMap2f(
    GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
    GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat const* points) {}

DLL void API glEvalMesh1(GLenum mode, GLint i1, GLint i2) {}

DLL void API glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) {}

DLL void API glMapGrid1f(GLint un, GLfloat u1, GLfloat u2) {}

DLL void API glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2) {}

DLL void API glEvalPoint2(GLint i, GLint j) {}

DLL void API glEvalPoint1(GLint i) {}

DLL void API glEvalCoord2f(GLfloat u, GLfloat v) {}

DLL void API glEvalCoord1f(GLfloat u) {}

DLL void API glTexImage1D(
    GLenum target, GLint level, GLint internalformat, GLsizei width,
    GLint border, GLenum format, GLenum type, GLvoid const* pixels) {}

DLL void API glDrawBuffer(GLenum mode) {}
DLL void API glReadBuffer(GLenum mode) {}

DLL void API glClearIndex(GLfloat c) {}

// -------------------

typedef void (*pROC)();

struct __extensions__
{
    pROC  proc;
    char  *name;
};

static GLint CreateWindow0, CreateWindow1, CreateWindow2;

DLL void rglCreateWindow(GLint a, GLint b, GLint c)
{
    CreateWindow0 = a;
    CreateWindow1 = b;
    CreateWindow2 = c;
}

DLL GLint rglCreate0(void) { return CreateWindow0; }
DLL GLint rglCreate1(void) { return CreateWindow1; }
DLL GLint rglCreate2(void) { return CreateWindow2; }

DLL void rglDeleteWindow(GLint a)
{
    GLcontext* ctx = CC;
    if (ctx->DriverFuncs.delete_window != NULL)
    {
        ctx->DriverFuncs.delete_window(a);
    }
    gl_shutdown();
}

DLL void* rglGetFramebuffer(GLint* pitch)
{
    GLcontext* ctx = CC;

    if (ctx->DriverFuncs.get_animaticbuffer != NULL)
    {
        return (void*)ctx->DriverFuncs.get_animaticbuffer(pitch);
    }
    else
    {
        if (pitch == NULL)
        {
            gl_unlock_framebuffer();
            return NULL;
        }
        else
        {
            gl_lock_framebuffer();
            *pitch = CC->Buffer.Pitch;
            return (void*)CC->FrameBuffer;
        }
    }
}

DLL void rglDrawPitchedPixels(
    GLint x0, GLint y0, GLint x1, GLint y1,
    GLsizei width, GLsizei height, GLsizei pitch,
    GLvoid const* pixels)
{
    GLcontext* ctx = CC;
    if (ctx->DriverFuncs.draw_pitched_pixels != NULL)
    {
        ctx->DriverFuncs.draw_pitched_pixels(x0, y0, x1, y1,
                                             width, height, pitch,
                                             pixels);
    }
}

static struct __extensions__ ext[] =
{
    { (pROC)glColorTableEXT, "glColorTableEXT" },
    { (pROC)glLitColorTable, "glLitColorTableEXT" },
    { (pROC)rglFeature, "rglFeature" },
    { (pROC)rglSpecExp, "rglSpecExp" },
    { (pROC)rglLightingAdjust, "rglLightingAdjust" },
    { (pROC)rglSaveCursorUnder, "rglSaveCursorUnder" },
    { (pROC)rglRestoreCursorUnder, "rglRestoreCursorUnder" },
    { (pROC)rglIsFast, "rglIsFast" },
    { (pROC)rglCreateWindow, "rglCreateWindow" },
    { (pROC)rglDeleteWindow, "rglDeleteWindow" },
    { (pROC)rglIsClipped, "rglIsClipped" },
    { (pROC)rglNumPolys, "rglNumPolys" },
    { (pROC)rglCulledPolys, "rglCulledPolys" },
    { (pROC)rglBackground, "rglBackground" },
    { (pROC)rglSetAllocs, "rglSetAllocs" },
    { (pROC)glSuperClear, "rglSuperClear" },
    { (pROC)rglEnable, "rglEnable" },
    { (pROC)rglDisable, "rglDisable" },
    { (pROC)rglListSpec, "rglListSpec" },
    { (pROC)rglList, "rglList" },
    { (pROC)rglNormal, "rglNormal" },
    { (pROC)rglTriangle, "rglTriangle" },
    { (pROC)rglTexturedTriangle, "rglTexturedTriangle" },
    { (pROC)rglSmoothTriangle, "rglSmoothTriangle" },
    { (pROC)rglSmoothTexturedTriangle, "rglSmoothTexturedTriangle" },
    { (pROC)rglMeshRender, "rglMeshRender" },
    { (pROC)rauxInitPosition, "rauxInitPosition" },
    { (pROC)rglSelectDevice, "rglSelectDevice" },
    { (pROC)rglD3DSetDevice, "rglD3DSetDevice" },
    { (pROC)rglD3DGetDevice, "rglD3DGetDevice" },
    { (pROC)rglGetFramebuffer, "rglGetFramebuffer" },
    { (pROC)rglDrawPitchedPixels, "rglDrawPitchedPixels" }
};

static int qt_ext = sizeof(ext) / sizeof(ext[0]);

pROC rglGetProcAddress(char* lpszProc)
{
    int i;

    for (i = 0; i < qt_ext; i++)
    {
        if (!strcmp(lpszProc, ext[i].name))
        {
            return (pROC)ext[i].proc;
        }
    }

    return NULL;
}
