#ifndef _SMsgDirG2StatusReply_H
#define _SMsgDirG2StatusReply_H

// SMsgDirG2StatusReply.h

// Directory generation 2 Generic Reply class.  Returns the status of a
// request made to the Directory Server.


#include "STRING"
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class SMsgDirG2StatusReply : public SmallMessage
{
public:
    // Default ctor
    SMsgDirG2StatusReply(ServerStatus theStatus=WONMsg::StatusCommon_Success);

    // TMessage ctor - will throw if SmallMessage type is not of this type
    explicit SMsgDirG2StatusReply(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgDirG2StatusReply(const SMsgDirG2StatusReply& theMsgR);

    // Destructor
    ~SMsgDirG2StatusReply(void);

    // Assignment
    SMsgDirG2StatusReply& operator=(const SMsgDirG2StatusReply& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack(void);
    void  Unpack(void);

    // Status access
    ServerStatus GetStatus(void) const;
    void         SetStatus(ServerStatus theStatus);

    // NetAddress access (for replies to AddService(Ex) only)
    const WONCommon::RawBuffer& GetNetAddress(void) const;
    void                        SetNetAddress(const WONCommon::RawBuffer& theAddr);

private:
    ServerStatus         mStatus;   // Request status
    WONCommon::RawBuffer mNetAddr;  // Service net address.  (AddService(Ex) only)
};


// Inlines
inline TRawMsg*
SMsgDirG2StatusReply::Duplicate(void) const
{ return new SMsgDirG2StatusReply(*this); }

inline ServerStatus
SMsgDirG2StatusReply::GetStatus(void) const
{ return mStatus; }

inline void
SMsgDirG2StatusReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline const WONCommon::RawBuffer&
SMsgDirG2StatusReply::GetNetAddress(void) const
{ return mNetAddr; }

inline void
SMsgDirG2StatusReply::SetNetAddress(const WONCommon::RawBuffer& theAddr)
{ mNetAddr = theAddr; }


};  // Namespace WONMsg

#endif