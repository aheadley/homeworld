#ifndef _TMsgCommStatusReply_H
#define _TMsgCommStatusReply_H

// TMsgCommStatusReply.h

// Common Generic Reply class.  Returns a status (short) value.


#include "STRING"
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgCommStatusReply : public TMessage
{
public:
    // Default ctor
    TMsgCommStatusReply(void);

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgCommStatusReply(const TMessage& theMsgR);

    // Copy ctor
    TMsgCommStatusReply(const TMsgCommStatusReply& theMsgR);

    // Destructor
    ~TMsgCommStatusReply(void);

    // Assignment
    TMsgCommStatusReply& operator=(const TMsgCommStatusReply& theMsgR);

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
TMsgCommStatusReply::Duplicate(void) const
{ return new TMsgCommStatusReply(*this); }

inline ServerStatus
TMsgCommStatusReply::GetStatus(void) const
{ return mStatus; }

inline void
TMsgCommStatusReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }


};  // Namespace WONMsg

#endif