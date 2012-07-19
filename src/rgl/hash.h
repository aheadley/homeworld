/*=============================================================================
    Name    : hash.c
    Purpose : reasonably generic hash table using uints as keys

    Created 1/9/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#ifndef _HASH_H
#define _HASH_H

#define TABLE_SIZE 3001

typedef struct hash_s
{
    GLuint key;
    void*  data;
    struct hash_s* next;
} hash_t;

typedef struct
{
    hash_t* table[TABLE_SIZE];
    GLuint  maxkey;
} hashtable;

hashtable* hashNewTable(void* (*allocFunc)(GLint),
                        void (*freeFunc)(void*));
void  hashDeleteTable(hashtable* table);
void* hashLookup(hashtable const* table, GLuint key);
void  hashInsert(hashtable* table, GLuint key, void* data);
void  hashRemove(hashtable* table, GLuint key);
GLuint hashFindFreeKeyBlock(hashtable* table, GLuint numkeys);

#endif
