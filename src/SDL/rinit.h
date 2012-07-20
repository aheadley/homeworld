/*=============================================================================
    Name    : rinit.h
    Purpose : rGL / OpenGL enumeration initialization routines

    Created 1/5/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _RINIT_H
#define _RINIT_H

typedef struct rmode
{
    int width, height;
    int depth;
    struct rmode* next;
} rmode;

#define RIN_TYPE_OPENGL   1
#define RIN_TYPE_DIRECT3D 2
#define RIN_TYPE_SOFTWARE 4

typedef struct rdevice
{
    int type;
    char data[64];
    char name[64];
    unsigned int devcaps;
    unsigned int devcaps2;

    struct rmode* modes;

    struct rdevice* next;
} rdevice;

#ifdef __cplusplus
extern "C" {
#endif

int rinEnumerateDevices(void);
int rinFreeDevices(void);
unsigned int rinDirectXVersion(void);
rdevice* rinGetDeviceList(void);

unsigned int rinDeviceCRC(void);

#ifdef __cplusplus
}
#endif

#endif

