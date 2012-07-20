
#include <string.h>
#include "Types.h"
#include "Debug.h"
#include "Memory.h"
#include "Queue.h"

#define QUEUE_WRAPAROUND_FLAG   0xffffffffL

/*-----------------------------------------------------------------------------
    Name        : ResetQueue
    Description : Resets the Queue
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ResetQueue(Queue *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->num = 0;
    queue->totalsize = 0;
    queue->totaltotalsize = 0;
    queue->overruns = 0;
}

/*-----------------------------------------------------------------------------
    Name        : InitQueue
    Description : Initializes the Queue
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void InitQueue(Queue *queue,udword buffersize)
{
    queue->mutex = SDL_CreateMutex();
    dbgAssert(queue->mutex != NULL);
    queue->buffer = memAlloc(buffersize,"qbuffer",NonVolatile);
    queue->buffersize = buffersize;
    ResetQueue(queue);
}

/*-----------------------------------------------------------------------------
    Name        : CloseQueue
    Description : Closes the Queue
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseQueue(Queue *queue)
{
    SDL_DestroyMutex(queue->mutex);
    queue->mutex = NULL;
    memFree(queue->buffer);
    queue->buffer = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : LockQueue
    Description : Locks the Queue for exclusive access
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void LockQueue(Queue *queue)
{
    int result = SDL_mutexP(queue->mutex);
    dbgAssert(result != -1);
}

/*-----------------------------------------------------------------------------
    Name        : UnLockQueue
    Description : Unlocks the queue, so other tasks can access it
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void UnLockQueue(Queue *queue)
{
    int result = SDL_mutexV(queue->mutex);
    dbgAssert(result != -1);
}

/*-----------------------------------------------------------------------------
    Name        : Enqueue
    Description : Enqueue's data
    Inputs      : packet, sizeofPacket
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void HWEnqueue(Queue *queue,ubyte *packet,udword sizeofPacket)
{
    ubyte *writeto;
    udword sizeinQ = sizeof(udword) + sizeofPacket;

    dbgAssert(sizeofPacket > 0);

    if ((queue->totalsize+sizeinQ) > queue->buffersize)
    {
        queue->overruns++;
        return;
    }

    if ((queue->head+sizeinQ) > queue->buffersize)
    {
        // we are going over end of buffer, so wrap around
        writeto = &queue->buffer[queue->head];
        *((udword *)writeto) = QUEUE_WRAPAROUND_FLAG;           // flag to indicate this is an invalid Q entry, and user should wrap to beginning
        queue->head = 0;
    }

    writeto = &queue->buffer[queue->head];
    *((udword *)writeto)++ = sizeofPacket;
    queue->head += sizeinQ;
    if ((queue->head+sizeof(udword)) > queue->buffersize)
    {
        queue->head = 0;
    }

    queue->totalsize += sizeinQ;
    queue->totaltotalsize += sizeinQ;

    memcpy(writeto,packet,sizeofPacket);
    queue->num++;
}

/*-----------------------------------------------------------------------------
    Name        : Dequeue
    Description : Dequeue's data
    Inputs      :
    Outputs     : packet pointer
    Return      : size of data returned
----------------------------------------------------------------------------*/
udword HWDequeue(Queue *queue,ubyte **packet)
{
    ubyte *readfrom;
    udword sizeofPacket;
    udword sizeinQ;

    if (queue->num == 0)
    {
        return 0;
    }

    readfrom = &queue->buffer[queue->tail];
    if (*((udword *)readfrom) == QUEUE_WRAPAROUND_FLAG)
    {
        readfrom = &queue->buffer[0];
        queue->tail = 0;
    }

    sizeofPacket = *((udword *)readfrom)++;
    *packet = readfrom;
    sizeinQ = sizeof(udword) + sizeofPacket;
    queue->tail += sizeinQ;
    if ((queue->tail+sizeof(udword)) > queue->buffersize)
    {
        queue->tail = 0;
    }
    queue->totalsize -= sizeinQ;

    queue->num--;

    return sizeofPacket;
}

/*-----------------------------------------------------------------------------
    Name        : Peekqueue
    Description : Peek at the next data in the queue without actually dequeueing
    Inputs      :
    Outputs     : packet pointer
    Return      : size of data returned
----------------------------------------------------------------------------*/
udword Peekqueue(Queue *queue,ubyte **packet)
{
    ubyte *readfrom;
    udword sizeofPacket;
    //udword sizeinQ;

    if (queue->num == 0)
    {
        return 0;
    }

    readfrom = &queue->buffer[queue->tail];
    if (*((udword *)readfrom) == QUEUE_WRAPAROUND_FLAG)
    {
        readfrom = &queue->buffer[0];
    }

    sizeofPacket = *((udword *)readfrom)++;
    *packet = readfrom;

    return sizeofPacket;
}

