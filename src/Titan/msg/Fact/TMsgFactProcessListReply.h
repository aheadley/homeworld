#if !defined(TMsgFactProcessListReply_H)
#define TMsgFactProcessListReply_H

// TMsgFactProcessListReply.h

// Message that is used to return a list of process configuration names from the Factory Server


#include "msg/TMessage.h"
#include "SET"


namespace WONMsg {

    typedef std::set<std::string> FACT_SERV_PROCESS_SET;


class TMsgFactProcessListReply : public TMessage {

public:
    // Default ctor
    TMsgFactProcessListReply(void);

    // TMessage ctor
    explicit TMsgFactProcessListReply(const TMessage& theMsgR);

    // Copy ctor
    TMsgFactProcessListReply(const TMsgFactProcessListReply& theMsgR);

    // Destructor
    virtual ~TMsgFactProcessListReply(void);

    // Assignment
    TMsgFactProcessListReply& operator=(const TMsgFactProcessListReply& theMsgR);

    // Virtual Duplicate from TMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    short GetStatus(void) const;
    const FACT_SERV_PROCESS_SET& GetProcessSet(void) const;

    virtual void SetStatus(short theStatus);
    virtual void SetProcessSet(const FACT_SERV_PROCESS_SET& theProcessSet);

protected:
    short                 mStatus;
    FACT_SERV_PROCESS_SET mProcessSet;

};


// Inlines
inline TRawMsg* TMsgFactProcessListReply::Duplicate(void) const
{ return new TMsgFactProcessListReply(*this); }

inline short TMsgFactProcessListReply::GetStatus(void) const
{ return mStatus; }

inline const FACT_SERV_PROCESS_SET& TMsgFactProcessListReply::GetProcessSet(void) const
{ return mProcessSet; }

inline void TMsgFactProcessListReply::SetStatus(short theStatus)
{ mStatus = theStatus; }

inline void TMsgFactProcessListReply::SetProcessSet(const FACT_SERV_PROCESS_SET& theProcessSet)
{ mProcessSet = theProcessSet; }

};  // Namespace WONMsg

#endif