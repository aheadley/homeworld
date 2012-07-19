#if !defined(SMsgFactStartProcessUnicode_H)
#define SMsgFactStartProcessUnicode_H

// SMsgFactStartProcessUnicode.h

// Message that is used to start a process via the Factory Server


#include "SMsgFactStartProcessBase.h"
#include "SET"


namespace WONMsg {

class SMsgFactStartProcessUnicode : public SMsgFactStartProcessBase {

public:
    // Default ctor
    SMsgFactStartProcessUnicode(void);

    // TMessage ctor
    explicit SMsgFactStartProcessUnicode(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgFactStartProcessUnicode(const SMsgFactStartProcessUnicode& theMsgR);

    // Destructor
    virtual ~SMsgFactStartProcessUnicode(void);

    // Assignment
    SMsgFactStartProcessUnicode& operator=(const SMsgFactStartProcessUnicode& theMsgR);

    // Virtual Duplicate from TMessage
    virtual TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

protected:
    virtual void PackCommandLine();
    virtual void UnpackCommandLine();
};


// Inlines
inline TRawMsg* SMsgFactStartProcessUnicode::Duplicate(void) const
{ return new SMsgFactStartProcessUnicode(*this); }

};  // Namespace WONMsg

#endif