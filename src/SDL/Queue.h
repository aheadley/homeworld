
#ifndef ___QUEUE_H
#define ___QUEUE_H

#include "SDL.h"
#include "Types.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    //void *mutex;
    SDL_mutex *mutex;
    ubyte *buffer;
    udword buffersize;
    udword head;
    udword tail;
    udword num;
    udword totalsize;
    udword totaltotalsize;
    udword overruns;
} Queue;

/*=============================================================================
    Functions:
=============================================================================*/

void InitQueue(Queue *queue,udword buffersize);
void CloseQueue(Queue *queue);
void ResetQueue(Queue *queue);
void HWEnqueue(Queue *queue,ubyte *packet,udword sizeofPacket);
udword HWDequeue(Queue *queue,ubyte **packet);
udword Peekqueue(Queue *queue,ubyte **packet);
void LockQueue(Queue *queue);
void UnLockQueue(Queue *queue);

/*=============================================================================
    Macros:
=============================================================================*/

#define queueNumberEntries(q) ((q).num)
#define queueNumberOverruns(q) ((q).overruns)

#endif

