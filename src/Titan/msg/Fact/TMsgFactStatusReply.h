#if !defined(TMsgFactStatusReply_H)
#define TMsgFactStatusReply_H

// TMsgFactStatusReply.h

// Message that is used to reply to a start/stop process via the Factory Server


#include "msg/TMessage.h"
#include "SET"


namespace WONMsg {

    typedef std::set<unsigned short> FACT_SERV_PORT_RANGE_SET;

class TMsgFactStatusReply : public TMessage {

public:
    // Default ctor
    TMsgFactStatusReply(void);

    // TMessage ctor
    explicit TMsgFactStatusReply(const TMessage& theMsgR);

    // Copy ctor
    TMsgFactStatusReply(const TMsgFactStatusReply& theMsgR);

    // Destructor
    virtual ~TMsgFactStatusReply(void);

    // Assignment
    TMsgFactStatusReply& operator=(const TMsgFactStatusReply& theMsgR);

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
inline TRawMsg* TMsgFactStatusReply::Duplicate(void) const
{ return new TMsgFactStatusReply(*this); }

inline short TMsgFactStatusReply::GetProcessStatus(void) const
{ return mProcessStatus; }

inline const FACT_SERV_PORT_RANGE_SET& TMsgFactStatusReply::GetProcessPortIDSet(void) const
{ return mProcessPortIDSet; }

inline void TMsgFactStatusReply::SetProcessStatus(short theProcessStatus)
{ mProcessStatus = theProcessStatus; }

inline void TMsgFactStatusReply::SetProcessPortIDSet(const FACT_SERV_PORT_RANGE_SET& theProcessPortIDSet)
{ mProcessPortIDSet = theProcessPortIDSet; }

};  // Namespace WONMsg

#endif