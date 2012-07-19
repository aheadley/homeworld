#ifndef _SMsgCommStatusReply_H
#define _SMsgCommStatusReply_H

// SMsgCommStatusReply.h

// Common Generic Reply class.  Returns a status (short) value.


#include "STRING"
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class SMsgCommStatusReply : public SmallMessage
{
public:
    // Default ctor
    SMsgCommStatusReply(void);

    // SmallMessage ctor - will throw if SmallMessage type is not of this type
    explicit SMsgCommStatusReply(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgCommStatusReply(const SMsgCommStatusReply& theMsgR);

    // Destructor
    ~SMsgCommStatusReply(void);

    // Assignment
    SMsgCommStatusReply& operator=(const SMsgCommStatusReply& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack(void);
    void  Unpack(void);

    // Status access
    ServerStatus GetStatus(void) const;
    void         SetStatus(ServerStatus theStatus);

private:
    ServerStatus mStatus;  // Request status
};


// Inlines
inline TRawMsg*
SMsgCommStatusReply::Duplicate(void) const
{ return new SMsgCommStatusReply(*this); }

inline ServerStatus
SMsgCommStatusReply::GetStatus(void) const
{ return mStatus; }

inline void
SMsgCommStatusReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }


};  // Namespace WONMsg

#endif