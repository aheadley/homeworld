#ifndef _TMsgDirStatusReply_H
#define _TMsgDirStatusReply_H

// TMsgDirStatusReply.h

// Directory Generic Reply class.  Returns the status of a request made
// to the Directory Server.


#include "STRING"
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgDirStatusReply : public TMessage
{
public:
    // Default ctor
    TMsgDirStatusReply(void);

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgDirStatusReply(const TMessage& theMsgR);

    // Copy ctor
    TMsgDirStatusReply(const TMsgDirStatusReply& theMsgR);

    // Destructor
    ~TMsgDirStatusReply(void);

    // Assignment
    TMsgDirStatusReply& operator=(const TMsgDirStatusReply& theMsgR);

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
TMsgDirStatusReply::Duplicate(void) const
{ return new TMsgDirStatusReply(*this); }

inline ServerStatus
TMsgDirStatusReply::GetStatus(void) const
{ return mStatus; }

inline void
TMsgDirStatusReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }


};  // Namespace WONMsg

#endif