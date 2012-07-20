/*=============================================================================
    Name    : hash.c
    Purpose : reasonably generic hash table using uints as keys

    Created 1/9/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "Hash.h"
#include "Memory.h"
#include "Debug.h"

/*-----------------------------------------------------------------------------
    Name        : hashNewTable
    Description : creates a new hashtable structure and initializes
                  all entries to NULL, sets maxkey to 0
    Inputs      : size - number of distinct entries in the hash table
    Outputs     :
    Return      : a freshly allocated hashtable structure
----------------------------------------------------------------------------*/
hashtable* hashNewTable(udword size)
{
    int i;
    hashtable* table;

    table = (hashtable*)memAlloc((size-1) * sizeof(hash_t) + sizeof(hashtable),
                                 "hashtable", NonVolatile);

    table->size = size;
    table->maxkey = 0;

    for (i = 0; i < size; i++)
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
    udword i;
    hash_t* entry;
    hash_t* next;

    dbgAssert(table != NULL);

    for (i = 0; i < table->size; i++)
    {
        entry = table->table[i];
        while (entry)
        {
            next = entry->next;
            memFree(entry);
            entry = next;
        }
    }

    memFree(table);
}

/*-----------------------------------------------------------------------------
    Name        : hashLookup
    Description : lookup given key in given hashtable
    Inputs      : table - the hashtable to search
                  key - the key to search for
    Outputs     :
    Return      : the entry corresponding to key, or NULL
----------------------------------------------------------------------------*/
void* hashLookup(hashtable const* table, udword key)
{
    udword pos;
    hash_t* entry;

    dbgAssert(table != NULL);

    if (key == 0)
        return NULL;

    pos = key % table->size;
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
void hashInsert(hashtable* table, udword key, void* data)
{
    udword pos;
    hash_t* entry;

    dbgAssert(table != NULL);

    if (key == 0)
        return;

    if (key > table->maxkey)
        table->maxkey = key;

    pos = key % table->size;
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
    entry = (hash_t*)memAlloc(sizeof(hash_t), "hash entry", NonVolatile);
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
void hashRemove(hashtable* table, udword key)
{
    udword pos;
    hash_t *entry, *prev;

    dbgAssert(table != NULL);
    dbgAssert(key != 0);

    pos = key % table->size;
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
            memFree(entry);
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
udword hashFindFreeKeyBlock(hashtable* table, udword numkeys)
{
    udword maxkey = (udword)(~0);
    if (maxkey - numkeys > table->maxkey)
    {
        /* the quick solution */
        return table->maxkey + 1;
    }
    else
    {
        /* the slow solution */
        udword freecount = 0;
        udword freestart = 0;
        udword key;

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
