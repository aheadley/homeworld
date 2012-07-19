#if !defined(SMsgFactStartProcessBase_H)
#define SMsgFactStartProcessBase_H

// TMsgFactStartProcessBase.h

// Message that is used to start a process via the Factory Server


#include "msg/TMessage.h"
#include "SET"

#include "AllMsgStartProcessBase.h"

namespace WONMsg {

class SMsgFactStartProcessBase : public SmallMessage, public AllMsgStartProcessBase
{

public:
    // Default ctor
    SMsgFactStartProcessBase(void);

    // TMessage ctor
    explicit SMsgFactStartProcessBase(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgFactStartProcessBase(const SMsgFactStartProcessBase& theMsgR);

    // Destructor
    virtual ~SMsgFactStartProcessBase(void);

    // Assignment
    SMsgFactStartProcessBase& operator=(const SMsgFactStartProcessBase& theMsgR);

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

protected:
    virtual void PackCommandLine() =0;
    virtual void UnpackCommandLine() =0;
};

};  // Namespace WONMsg

#endif