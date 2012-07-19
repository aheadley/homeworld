/*=============================================================================
    Name    : hash.c
    Purpose : reasonably generic hash table using uints as keys.
              the GL uses a hash for texture objects

    Created 1/9/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <assert.h>
#include "kgl.h"

/* memory de/allocation function pointers */
static void* (*_hashAlloc)() = NULL;
static void (*_hashFree)() = NULL;

/*-----------------------------------------------------------------------------
    Name        : hashNewTable
    Description : creates a new hashtable structure and initializes
                  all entries to NULL, sets maxkey to 0.  also sets
                  the hash.c mem de/alloc function pointers
    Inputs      : allocFunc - memory allocation func, void* malloc(GLint)
                  freeFunc - memory deallocation func, free(void*)
    Outputs     :
    Return      : a freshly allocated hashtable structure
----------------------------------------------------------------------------*/
hashtable* hashNewTable(void* (*allocFunc)(), void (*freeFunc)(void*))
{
    int i;
    hashtable* table;

    _hashAlloc = allocFunc;
    _hashFree = freeFunc;

    table = (hashtable*)_hashAlloc(sizeof(hashtable));

    table->maxkey = 0;
    for (i = 0; i < TABLE_SIZE; i++)
    {
        table->table[i] = NULL;
    }

    return table;
}

/*-----------------------------------------------------------------------------
    Name        : hashDeleteTable
    Description : deletes a hashtable structure, freeing the memory
                  used by the entry blocks but not what the blocks
                  contain (because this is a generic hash)
    Inputs      : table - the hashtable to delete
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void hashDeleteTable(hashtable* table)
{
    GLuint i;
    assert(table);
    for (i = 0; i < TABLE_SIZE; i++)
    {
        hash_t* entry = table->table[i];
        while (entry)
        {
            hash_t* next = entry->next;
            _hashFree(entry);
            entry = next;
        }
    }
    _hashFree(table);
}

/*-----------------------------------------------------------------------------
    Name        : hashLookup
    Description : lookup given key in given hashtable
    Inputs      : table - the hashtable to search
                  key - the key to search for
    Outputs     :
    Return      : the entry corresponding to key, or NULL
----------------------------------------------------------------------------*/
void* hashLookup(hashtable const* table, GLuint key)
{
    GLuint pos;
    hash_t* entry;

    assert(table);

    if (key == 0)
        return NULL;

    pos = key % TABLE_SIZE;
    entry = table->table[pos];
    while (entry)
    {
        if (entry->key == key)
        {
            return entry->data;
        }
        entry = entry->next;
    }
    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : hashInsert
    Description : insert an entry into given hashtable with given key
    Inputs      : table - the hashtable
                  key - the hash key (index)
                  data - the data to be stored
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void hashInsert(hashtable* table, GLuint key, void* data)
{
    GLuint pos;
    hash_t* entry;

    assert(table);

    if (key == 0)
        return;

    if (key > table->maxkey)
        table->maxkey = key;

    pos = key % TABLE_SIZE;
    entry = table->table[pos];
    while (entry)
    {
        if (entry->key == key)
        {
            /* replace entry's data.  user should really avoid this
               and free collisions manually before inserting to avoid
               unreferenced but allocated memory */
	        entry->data = data;
	        return;
        }
        entry = entry->next;
    }

    /* alloc and insert new table entry */
    entry = (hash_t*)_hashAlloc(sizeof(hash_t));
    entry->key = key;
    entry->data = data;
    entry->next = table->table[pos];
    table->table[pos] = entry;
}

/*-----------------------------------------------------------------------------
    Name        : hashRemove
    Description : remove an entry from given hashtable
    Inputs      : table - the hashtable
                  key - the key of the entry to remove
    Outputs     : table is modified to not contain the entry if it
                  was found, and the entry block is freed (but not
                  the data it contains)
    Return      :
----------------------------------------------------------------------------*/
void hashRemove(hashtable* table, GLuint key)
{
    GLuint pos;
    hash_t *entry, *prev;

    assert(table);
    assert(key);

    pos = key % TABLE_SIZE;
    prev = NULL;
    entry = table->table[pos];
    while (entry)
    {
        if (entry->key == key)
        {
            if (prev)
            {
                prev->next = entry->next;
            }
            else
            {
                table->table[pos] = entry->next;
            }
            _hashFree(entry);
	        return;
        }
        prev = entry;
        entry = entry->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : hashFindFreeKeyBlock
    Description : searches thru a hashtable for a block numkeys in
                  length of free keys.  glGenTextures uses this fn,
                  for instance
    Inputs      : table - the hashtable
                  numkeys - length of the block
    Outputs     :
    Return      : the starting key index, or 0 if failure
----------------------------------------------------------------------------*/
GLuint hashFindFreeKeyBlock(hashtable* table, GLuint numkeys)
{
    GLuint maxkey = (GLuint)(~0);
    if (maxkey - numkeys > table->maxkey)
    {
        /* the quick solution */
        return table->maxkey + 1;
    }
    else
    {
        /* the slow solution */
        GLuint freecount = 0;
        GLuint freestart = 0;
        GLuint key;

        for (key = 0; key != maxkey; key++)
        {
            if (hashLookup(table, key))
            {
                /* key already in use */
                freecount = 0;
                freestart = key + 1;
	        }
	        else
            {
	            /* this key not in use, maybe we've found enough */
	            freecount++;
                if (freecount == numkeys)
                {
                    return freestart;
                }
	        }
	    }

        /* cannot allocate a block of numkeys consecutive keys */
        return 0;
    }
}
