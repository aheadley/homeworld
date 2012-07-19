/*=============================================================================
    Name    : fxtmg.c
    Purpose : 3Dfx texture management functions for rgl

    Created 1/5/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <limits.h>
#include "kgl.h"
#include "fxdrv.h"

/* 2MB - 1 */
#define TWOMEGS 0x1FFFFF

extern FxI32 TEXTURE_ALIGN;

static GLcontext* ctx;

static int logbase2(int n)
{
    GLint i = 1;
    GLint log2 = 0;

    if (n < 0)
        return -1;

    while (n > i)
    {
        i *= 2;
        log2++;
    }

    return (i == n) ? log2 : -1;
}

/*-----------------------------------------------------------------------------
    Name        : tmTexInfo
    Description : provides info for converting GL texture objects into
                  3Dfx (glide) format
    Inputs      : w, h - width, height
                  lodlevel - calc'd glide lod
                  ar - resultant glide aspect ratio
                  s/tscale - s/t coord scalar
                  w/hscale - width/height expansion ratios
    Outputs     :
    Return      : GL_TRUE on success, GL_FALSE on failure
----------------------------------------------------------------------------*/
int tmTexInfo(int w, int h, GrLOD_t* lodlevel, GrAspectRatio_t* ar,
              float* sscale, float* tscale, int* wscale, int* hscale)
{
    static GrLOD_t lod[9] = {GR_LOD_256, GR_LOD_128, GR_LOD_64, GR_LOD_32,
                             GR_LOD_16, GR_LOD_8, GR_LOD_4, GR_LOD_2, GR_LOD_1};
    int logw, logh, ws, hs;
    GrLOD_t l;
    GrAspectRatio_t aspectratio;
    float s, t;

    logw = logbase2(w);
    logh = logbase2(h);

    switch (logw - logh)
    {
    case 0:
        aspectratio = GR_ASPECT_1x1;
        l = lod[8 - logw];
        s = t = 256.0f;
        ws = hs = 1;
        break;
    case 1:
        aspectratio = GR_ASPECT_2x1;
        l = lod[8 - logw];
        s = 256.0f;
        t = 128.0f;
        ws = 1;
        hs = 1;
        break;
    case 2:
        aspectratio = GR_ASPECT_4x1;
        l = lod[8 - logw];
        s = 256.0f;
        t = 64.0f;
        ws = 1;
        hs = 1;
        break;
    case 3:
        aspectratio = GR_ASPECT_8x1;
        l = lod[8 - logw];
        s = 256.0f;
        t = 32.0f;
        ws = 1;
        hs = 1;
        break;
    case 4:
        aspectratio = GR_ASPECT_8x1;
        l = lod[8 - logw];
        s = 256.0f;
        t = 32.0f;
        ws = 1;
        hs = 2;
        break;
    case 5:
        aspectratio = GR_ASPECT_8x1;
        l = lod[8 - logw];
        s = 256.0f;
        t = 32.0f;
        ws = 1;
        hs = 4;
        break;
    case 6:
        aspectratio = GR_ASPECT_8x1;
        l = lod[8 - logw];
        s = 256.0f;
        t = 32.0f;
        ws = 1;
        hs = 8;
        break;
    case 7:
        aspectratio = GR_ASPECT_8x1;
        l = lod[8 - logw];
        s = 256.0f;
        t = 32.0f;
        ws = 1;
        hs = 16;
        break;
    case 8:
        aspectratio = GR_ASPECT_8x1;
        l = lod[8 - logw];
        s = 256.0f;
        t = 32.0f;
        ws = 1;
        hs = 32;
        break;
    case -1:
        aspectratio = GR_ASPECT_1x2;
        l = lod[8 - logh];
        s = 128.0f;
        t = 256.0f;
        ws = 1;
        hs = 1;
        break;
    case -2:
        aspectratio = GR_ASPECT_1x4;
        l = lod[8 - logh];
        s = 64.0f;
        t = 256.0f;
        ws = 1;
        hs = 1;
        break;
    case -3:
        aspectratio = GR_ASPECT_1x8;
        l = lod[8 - logh];
        s = 32.0f;
        t = 256.0f;
        ws = 1;
        hs = 1;
        break;
    case -4:
        aspectratio = GR_ASPECT_1x8;
        l = lod[8 - logh];
        s = 32.0f;
        t = 256.0f;
        ws = 2;
        hs = 1;
        break;
    case -5:
        aspectratio = GR_ASPECT_1x8;
        l = lod[8 - logh];
        s = 32.0f;
        t = 256.0f;
        ws = 4;
        hs = 1;
        break;
    case -6:
        aspectratio = GR_ASPECT_1x8;
        l = lod[8 - logh];
        s = 32.0f;
        t = 256.0f;
        ws = 8;
        hs = 1;
        break;
    case -7:
        aspectratio = GR_ASPECT_1x8;
        l = lod[8 - logh];
        s = 32.0f;
        t = 256.0f;
        ws = 16;
        hs = 1;
        break;
    case -8:
        aspectratio = GR_ASPECT_1x8;
        l = lod[8 - logh];
        s = 32.0f;
        t = 256.0f;
        ws = 32;
        hs = 1;
        break;
    default:
        return GL_FALSE;
    }

    if (lodlevel)
        (*lodlevel) = l;

    if (ar)
        (*ar) = aspectratio;

    if (sscale)
        (*sscale) = s;

    if (tscale)
        (*tscale) = t;

    if (wscale)
        (*wscale) = ws;

    if (hscale)
        (*hscale) = hs;

    return GL_TRUE;
}

/* allocate and return a new texmemnode */
static texmemnode* newtexmemnode(FxU32 start, FxU32 end)
{
    texmemnode* tmn;

    if (!(tmn = ctx->AllocFunc(sizeof(texmemnode))))
    {
        //FIXME: bail here
    }

    tmn->next = NULL;
    tmn->tex = NULL;
    tmn->startadr = start;
    tmn->endadr = end;

    return tmn;
}

/*-----------------------------------------------------------------------------
    Name        : tmInit
    Description : initialize the texture management subsystem.  the idea
                  here is to create texmemnodes in the free list that
                  correspond to blocks of useable memory and then break
                  apart and combine these blocks as mem gets alloc'd
    Inputs      : _ctx - the GL's context
                  fx - 3Dfx context
    Outputs     : generates some freenode blocks in the context
    Return      :
----------------------------------------------------------------------------*/
void tmInit(GLcontext* _ctx, fx_context* fx, FxU32 where)
{
    FxU32 start, end;
    FxU32 blockstart, blockend;
    texmemnode* tmn;
    texmemnode* tmntmp;

    LOG("tmInit\n");

    ctx = _ctx;

    fx->tmfree[where] = NULL;
    fx->tmalloc[where] = NULL;

    start = grTexMinAddress(where);
    end = grTexMaxAddress(where);

    blockstart = start;
    while (blockstart <= end)
    {
        LOG("-- 2MEG block iteration --\n");

        if (blockstart + TWOMEGS > end)
            blockend = end;
        else
            blockend = blockstart + TWOMEGS;

        tmn = newtexmemnode(blockstart, blockend);

        if (fx->tmfree[where])
        {
            for (tmntmp = fx->tmfree[where]; tmntmp->next != NULL; tmntmp = tmntmp->next)
                /* nothing */;
            tmntmp->next = tmn;
        }
        else
        {
            fx->tmfree[where] = tmn;
        }

        blockstart += TWOMEGS + 1;
    }

    /* reset the lru counter */
    fx->texbind = 0;
}

/*-----------------------------------------------------------------------------
    Name        : findoldesttmblock
    Description : finds the oldest tmblock (LRU) and returns it.
                  will handle texbind wraparound
    Inputs      : fx - 3Dfx context
                  tmalloc - where to start
                  texbindnumber - comparator
    Outputs     :
    Return      : oldest gl_texture_object
----------------------------------------------------------------------------*/
static gl_texture_object* findoldesttmblock(
        fx_context* fx, texmemnode* tmalloc, GLuint texbindnumber)
{
    GLuint age, oldestage, lasttimeused;
    gl_texture_object* oldesttexobj;

    oldesttexobj = tmalloc->tex;
    oldestage = 0;

    while (tmalloc)
    {
        lasttimeused = ((fx_texobj*)(tmalloc->tex->DriverData))->tmi.lastused;
        if (lasttimeused > texbindnumber)
            age = texbindnumber + (UINT_MAX - lasttimeused + 1);
        else
            age = texbindnumber - lasttimeused;

        if (age >= oldestage)
        {
            oldestage = age;
            oldesttexobj = tmalloc->tex;
        }

        tmalloc = tmalloc->next;
    }

    return oldesttexobj;
}

/*-----------------------------------------------------------------------------
    Name        : freeoldtmblock
    Description : frees the oldest tmblock in the context and unloads
                  its texture if it has one
    Inputs      : fx - 3Dfx context
    Outputs     :
    Return      : GL_TRUE if a block was freed, GL_FALSE otherwise
----------------------------------------------------------------------------*/
static GLboolean freeoldtmblock(fx_context* fx, FxU32 where)
{
    gl_texture_object* oldesttexobj;

    if (!fx->tmalloc[where])
    {
        return GL_FALSE;
    }

    oldesttexobj = findoldesttmblock(fx, fx->tmalloc[where], fx->texbind);

    tmUnloadTexture(fx, oldesttexobj);

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : extracttmfreeblock
    Description : helper fn to gettmblock, which does what's necessary
                  to allocate a block of memory for a texture object.
                  extracttmfreeblock does no LRU stuff
    Inputs      : tmfree - start of free blocks to consider
                  texmemsize - size (bytes) to allocate
                  success - result success indicator
                  startadr - for the start of block address
    Outputs     : success - GL_TRUE or GL_FALSE
                  startadr - starting loc in texmem
    Return      : NULL or a satisfactory texmemnode
----------------------------------------------------------------------------*/
static texmemnode* extracttmfreeblock(
        texmemnode* tmfree, int texmemsize,
        GLboolean* success, FxU32* startadr)
{
    int blocksize;

    if (!tmfree)
    {
        *success = GL_FALSE;
        return NULL;
    }

    blocksize = tmfree->endadr - tmfree->startadr + 1;

    if (blocksize == texmemsize)
    {
        texmemnode* nexttmfree;

        *success = GL_TRUE;
        *startadr = tmfree->startadr;

        nexttmfree = tmfree->next;
        ctx->FreeFunc(tmfree);

        return nexttmfree;
    }

    if (blocksize > texmemsize)
    {
        *success = GL_TRUE;
        *startadr = tmfree->startadr;

        tmfree->startadr += texmemsize;

        return tmfree;
    }

    tmfree->next = extracttmfreeblock(tmfree->next, texmemsize, success, startadr);
    return tmfree;
}

/*-----------------------------------------------------------------------------
    Name        : gettmblock
    Description : allocate a block of memory, freeing the LRU textures
                  if necessary as it goes
    Inputs      : fx - 3Dfx context
                  tex - the texture object to make room for
                  texmemsize - size of the texobj
    Outputs     :
    Return      : newtmalloc, a texmemnode with the given texobj in it
----------------------------------------------------------------------------*/
static texmemnode* gettmblock(
        fx_context* fx, gl_texture_object* tex, int texmemsize, FxU32 where)
{
    texmemnode* newtmfree;
    texmemnode* newtmalloc;
    GLboolean success;
    FxU32 startadr;

    for (;;)
    {
        newtmfree = extracttmfreeblock(fx->tmfree[where], texmemsize, &success, &startadr);

        if (success)
        {
            fx->tmfree[where] = newtmfree;

            if (!(newtmalloc = ctx->AllocFunc(sizeof(texmemnode))))
            {
                //FIXME: bail
            }

            newtmalloc->next = fx->tmalloc[where];
            newtmalloc->startadr = startadr;
            newtmalloc->endadr = startadr + texmemsize - 1;
            newtmalloc->tex = tex;

            fx->tmalloc[where] = newtmalloc;

            return newtmalloc;
        }

        if (!freeoldtmblock(fx, where))
        {
            //FIXME: bail
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : tmLoadTexture
    Description : update tmalloc/tfree structures for a new texture, and
                  load it into card ram
    Inputs      : fx - 3Dfx context
                  tex - the texture object
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tmLoadTexture(fx_context* fx, gl_texture_object* tex, FxU32 where)
{
    fx_texobj* ti = (fx_texobj*)tex->DriverData;
    GrLOD_t lodlevel;
    int i;
    int texmemsize;

    if (!ti->valid)
        return;

    if (ti->tmi.inmemory)
        return;

    texmemsize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &ti->info);
    //extra space for alignment
    texmemsize += /*2**/TEXTURE_ALIGN;
//    texmemsize += 8;

    {
        char buf[64];
        sprintf(buf, "tmLoadTexture, %dx%d (%d bytes)\n",
                tex->Width, tex->Height, texmemsize);
        LOG(buf);
    }

    ti->tmi.whichTMU = where;
    ti->tmi.tm[where] = gettmblock(fx, tex, texmemsize, where);

    tmTexInfo(ti->width, ti->height, &lodlevel, NULL, NULL, NULL, NULL, NULL);

    for (i = 0; i < MAXNUM_MIPMAPLEVELS; i++)
    {
        if (ti->levelsdefined & (1 << i))
        {
            grTexDownloadMipMapLevel(
                ti->tmi.whichTMU, FXALIGN(ti->tmi.tm[where]->startadr), lodlevel,
                ti->info.largeLod, ti->info.aspectRatio,
                ti->info.format, GR_MIPMAPLEVELMASK_BOTH,
                ti->tmi.mipmaplevel[i]);
            lodlevel++;
        }
    }

    ti->tmi.inmemory = GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : tmReloadMipmap
    Description : reload a same-sized texture into the same area of ram
    Inputs      : fx - 3Dfx context
                  tex - the texture object
                  level - mipmap level
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tmReloadMipmap(
    fx_context* fx, gl_texture_object* tex, GLint level, FxU32 where)
{
    fx_texobj* ti = (fx_texobj*)tex->DriverData;
    GrLOD_t lodlevel;

    LOG("tmReloadMipmap\n");

    tmLoadTexture(fx, tex, where);

    tmTexInfo(ti->width, ti->height, &lodlevel, NULL, NULL, NULL, NULL, NULL);

    grTexDownloadMipMapLevel(
        where, FXALIGN(ti->tmi.tm[where]->startadr), lodlevel + level,
        ti->info.largeLod, ti->info.aspectRatio,
        ti->info.format, GR_MIPMAPLEVELMASK_BOTH,
        ti->tmi.mipmaplevel[level]);
}

/* recursively free a tmalloc block */
static texmemnode* freetmallocblock(texmemnode* tmalloc, texmemnode* tmunalloc)
{
    if (!tmalloc)
        return NULL;

    if (tmalloc == tmunalloc)
    {
        texmemnode* newtmalloc;
        newtmalloc = tmalloc->next;
        ctx->FreeFunc(tmalloc);
        return newtmalloc;
    }

    tmalloc->next = freetmallocblock(tmalloc->next, tmunalloc);

    return tmalloc;
}

/*-----------------------------------------------------------------------------
    Name        : addtmfree
    Description : adds a free block to the pool, joining as necessary
                  but remaining wary of 2MEG boundaries
    Inputs      : tmfree - free heap
                  startadr - start address
                  endadr - end address
    Outputs     :
    Return      : the new head of the free pool
----------------------------------------------------------------------------*/
static texmemnode* addtmfree(
        texmemnode* tmfree, FxU32 startadr, FxU32 endadr)
{
    if (!tmfree)
        return newtexmemnode(startadr, endadr);

    if ((endadr+1 == tmfree->startadr) && (tmfree->startadr & TWOMEGS))
    {
        tmfree->startadr = startadr;
        return tmfree;
    }

    if ((startadr - 1 == tmfree->endadr) && (startadr & TWOMEGS))
    {
        tmfree->endadr = endadr;

        if ((tmfree->next && (endadr+1 == tmfree->next->startadr) &&
            (tmfree->next->startadr & TWOMEGS)))
        {
            texmemnode* nexttmfree;
            tmfree->endadr = tmfree->next->endadr;
            nexttmfree = tmfree->next->next;
            ctx->FreeFunc(tmfree->next);
            tmfree->next = nexttmfree;
        }

        return tmfree;
    }

    if (startadr < tmfree->startadr)
    {
        texmemnode* newtmfree;
        newtmfree = newtexmemnode(startadr, endadr);
        newtmfree->next = tmfree;
        return newtmfree;
    }

    tmfree->next = addtmfree(tmfree->next, startadr, endadr);
    return tmfree;
}

/* free a tmalloc block, then add the freespace to the freenodes */
static void freetmblock(fx_context* fx, texmemnode* tmalloc, FxU32 where)
{
    FxU32 startadr, endadr;

    startadr = tmalloc->startadr;
    endadr = tmalloc->endadr;

    fx->tmalloc[where] = freetmallocblock(fx->tmalloc[where], tmalloc);
    fx->tmfree[where] = addtmfree(fx->tmfree[where], startadr, endadr);
}

/* unload a texture from memory & the heap */
void tmUnloadTexture(fx_context* fx, gl_texture_object* tex)
{
    fx_texobj* ti = (fx_texobj*)tex->DriverData;

    if (!ti->valid)
        return;

    if (!ti->tmi.inmemory)
        return;

    {
        char buf[64];
        sprintf(buf, "tmUnloadTexture, %dx%d\n", tex->Width, tex->Height);
        LOG(buf);
    }

    freetmblock(fx, ti->tmi.tm[ti->tmi.whichTMU], ti->tmi.whichTMU);

    ti->tmi.inmemory = GL_FALSE;
}

/* completely free a texture (vram, heap, ram rep) */
void tmFreeTexture(fx_context* fx, gl_texture_object* tex)
{
    fx_texobj* ti = (fx_texobj*)tex->DriverData;
    int i;

    LOG("tmFreeTexture\n");

    tmUnloadTexture(fx, tex);

    for (i = 0; i < MAXNUM_MIPMAPLEVELS; i++)
    {
        if (ti->levelsdefined & (1 << i))
        {
            ctx->FreeFunc(ti->tmi.mipmaplevel[i]);
        }
    }

    ti->valid = GL_FALSE;
}

/* free all the freespace nodes */
void tmFreeFreeNodes(texmemnode* fn)
{
    if (!fn)
        return;

    if (fn->next)
        tmFreeFreeNodes(fn->next);

    ctx->FreeFunc(fn);
}

/* free all the allocatedspace nodes */
void tmFreeAllocNodes(texmemnode* an)
{
    if (!an)
        return;

    if (an->next)
        tmFreeAllocNodes(an->next);

    ctx->FreeFunc(an);
}

/*-----------------------------------------------------------------------------
    Name        : tmClose
    Description : shutdown the texture manager.  free all texture object
                  data first, then all texmemnodes
    Inputs      : fx - 3Dfx context
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tmClose(fx_context* fx, FxU32 where)
{
    unsigned int i, hits = 0;
    gl_texture_object* tex;

    LOG("tmClose\n");

    //now free texobj data
    for (i = 0; i < rglGetMaxTexobj(); i++)
    {
        tex = rglGetTexobj(i);
        if (tex == NULL)
            continue;
        if (tex->created && tex->DriverData != NULL)
        {
            hits++;
            tmFreeTexture(fx, tex);
            ctx->FreeFunc(tex->DriverData);
            tex->DriverData = NULL;
        }
        else
        {
            tex->DriverData = NULL;
        }
    }

    tmFreeFreeNodes(fx->tmfree[where]);
    tmFreeAllocNodes(fx->tmalloc[where]);

    {
        char buf[64];
        sprintf(buf, "<< freed %d objects >>\n", hits);
        LOG(buf);
    }
}

/*-----------------------------------------------------------------------------
    Name        : tmInitTexobjs
    Description : update the driver texture object state to be that of
                  the GL.  useful when swapping renderers on the fly
    Inputs      : fx - 3Dfx context
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tmInitTexobjs(fx_context* fx)
{
    unsigned int i, hits = 0;
    gl_texture_object* tex;
    void texbind(gl_texture_object*);
    void teximg(gl_texture_object*, GLint, GLint);
    void texpalette(gl_texture_object*);

    LOG("tmInitTexobjs\n");

    for (i = 0; i < rglGetMaxTexobj(); i++)
    {
        tex = rglGetTexobj(i);
        if (tex != NULL && tex->created)
        {
            hits++;
            texbind(tex);
            teximg(tex, 0, tex->Format);
            if (tex->Format == GL_COLOR_INDEX)
            {
                texpalette(tex);
            }
        }
    }
    {
        char buf[64];
        sprintf(buf, "<< made %d objects >>\n", hits);
        LOG(buf);
    }
}
