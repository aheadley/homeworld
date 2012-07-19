#ifndef _SMsgFirewallStatusReply_H
#define _SMsgFirewallStatusReply_H

// SMsgFirewallStatusReply.h

// Common Generic Reply class.  Returns a status (short) value.


#include "STRING"
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class SMsgFirewallStatusReply : public SmallMessage
{
public:
    // Default ctor
    SMsgFirewallStatusReply(void);

    // SmallMessage ctor - will throw if SmallMessage type is not of this type
    explicit SMsgFirewallStatusReply(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgFirewallStatusReply(const SMsgFirewallStatusReply& theMsgR);

    // Destructor
    ~SMsgFirewallStatusReply(void);

    // Assignment
    SMsgFirewallStatusReply& operator=(const SMsgFirewallStatusReply& theMsgR);

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
SMsgFirewallStatusReply::Duplicate(void) const
{ return new SMsgFirewallStatusReply(*this); }

inline ServerStatus
SMsgFirewallStatusReply::GetStatus(void) const
{ return mStatus; }

inline void
SMsgFirewallStatusReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }


};  // Namespace WONMsg

#endif