#ifndef _SMsgDirG2QueryBase_H
#define _SMsgDirG2QueryBase_H

// SMsgDirG2QueryBase.h

// Base class derived from Smallmessage and KeyedBased that provides common definitions
// for DirServer query requests.  Simply adds base msg functionality and key fields
// from its base classes.

// *NOTE* this class is implemented inline.


#include "STRING"
#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/dir/SMsgDirG2KeyedBase.h"


namespace WONMsg {

class SMsgDirG2QueryBase : public SmallMessage, public SMsgDirG2KeyedBase
{
public:
    // Default ctor
    explicit SMsgDirG2QueryBase(KeyType theType);

    // SmallMessage ctor
    explicit SMsgDirG2QueryBase(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgDirG2QueryBase(const SMsgDirG2QueryBase& theMsgR);

    // Destructor
    virtual ~SMsgDirG2QueryBase(void);

    // Assignment
    SMsgDirG2QueryBase& operator=(const SMsgDirG2QueryBase& theMsgR);

    // Virtual Duplicate from SmallMessage
    // Pure virtual - must be overridden!
    virtual TRawMsg* Duplicate(void) const = 0;

protected:

private:
};


// Inlines

inline
SMsgDirG2QueryBase::SMsgDirG2QueryBase(KeyType theType) :
    SmallMessage(),
    SMsgDirG2KeyedBase(theType)
{}


inline
SMsgDirG2QueryBase::SMsgDirG2QueryBase(const SmallMessage& theMsgR) :
    SmallMessage(theMsgR),
    SMsgDirG2KeyedBase(KT_DIRECTORY)
{}

inline
SMsgDirG2QueryBase::SMsgDirG2QueryBase(const SMsgDirG2QueryBase& theMsgR) :
    SmallMessage(theMsgR),
    SMsgDirG2KeyedBase(theMsgR)
{}

inline
SMsgDirG2QueryBase::~SMsgDirG2QueryBase(void)
{}

inline
SMsgDirG2QueryBase&
SMsgDirG2QueryBase::operator=(const SMsgDirG2QueryBase& theMsgR)
{
    SmallMessage::operator=(theMsgR);
    SMsgDirG2KeyedBase::operator=(theMsgR);
    return *this;
}

};  // Namespace WONMsg

#endif