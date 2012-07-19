#if !defined(SMsgFactStatusReply_H)
#define SMsgFactStatusReply_H

// TMsgFactStatusReply.h

// Message that is used to reply to a start/stop process via the Factory Server


#include "msg/TMessage.h"
#include "SET"


namespace WONMsg {

    typedef std::set<unsigned short> FACT_SERV_PORT_RANGE_SET;

class SMsgFactStatusReply : public SmallMessage {

public:
    // Default ctor
    SMsgFactStatusReply(void);

    // TMessage ctor
    explicit SMsgFactStatusReply(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgFactStatusReply(const SMsgFactStatusReply& theMsgR);

    // Destructor
    virtual ~SMsgFactStatusReply(void);

    // Assignment
    SMsgFactStatusReply& operator=(const SMsgFactStatusReply& theMsgR);

    // Virtual Duplicate from TMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    short GetProcessStatus(void) const;
    const FACT_SERV_PORT_RANGE_SET& GetProcessPortIDSet(void) const;

    virtual void SetProcessStatus(short theProcessStatus);
    virtual void SetProcessPortIDSet(const FACT_SERV_PORT_RANGE_SET& theProcessPortID);

protected:
    short                    mProcessStatus;
    FACT_SERV_PORT_RANGE_SET mProcessPortIDSet;

};


// Inlines
inline TRawMsg* SMsgFactStatusReply::Duplicate(void) const
{ return new SMsgFactStatusReply(*this); }

inline short SMsgFactStatusReply::GetProcessStatus(void) const
{ return mProcessStatus; }

inline const FACT_SERV_PORT_RANGE_SET& SMsgFactStatusReply::GetProcessPortIDSet(void) const
{ return mProcessPortIDSet; }

inline void SMsgFactStatusReply::SetProcessStatus(short theProcessStatus)
{ mProcessStatus = theProcessStatus; }

inline void SMsgFactStatusReply::SetProcessPortIDSet(const FACT_SERV_PORT_RANGE_SET& theProcessPortIDSet)
{ mProcessPortIDSet = theProcessPortIDSet; }

};  // Namespace WONMsg

#endif