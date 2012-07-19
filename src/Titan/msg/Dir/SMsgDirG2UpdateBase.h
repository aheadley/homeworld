#ifndef _SMsgDirG2UpdateBase_H
#define _SMsgDirG2UpdateBase_H

// SMsgDirG2UpdateBase.h

// Base class derived from PeerDataBase and KeyedBased that provides common definitions
// for DirServer updates requests.  Simply adds peering and key fields from its base
// classes.

// *NOTE* this class is implemented inline.


#include "STRING"
#include "common/won.h"
#include "msg/dir/SMsgDirG2PeerDataBase.h"
#include "msg/dir/SMsgDirG2KeyedBase.h"


namespace WONMsg {

class SMsgDirG2UpdateBase : public SMsgDirG2PeerDataBase, public SMsgDirG2KeyedBase
{
public:
    // Default ctor
    explicit SMsgDirG2UpdateBase(KeyType theType);

    // SmallMessage ctor
    explicit SMsgDirG2UpdateBase(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgDirG2UpdateBase(const SMsgDirG2UpdateBase& theMsgR);

    // Destructor
    virtual ~SMsgDirG2UpdateBase(void);

    // Assignment
    SMsgDirG2UpdateBase& operator=(const SMsgDirG2UpdateBase& theMsgR);

    // Virtual Duplicate from SmallMessage
    // Pure virtual - must be overridden!
    virtual TRawMsg* Duplicate(void) const = 0;

protected:

private:
};


// Inlines

inline
SMsgDirG2UpdateBase::SMsgDirG2UpdateBase(KeyType theType) :
    SMsgDirG2PeerDataBase(),
    SMsgDirG2KeyedBase(theType)
{}


inline
SMsgDirG2UpdateBase::SMsgDirG2UpdateBase(const SmallMessage& theMsgR) :
    SMsgDirG2PeerDataBase(theMsgR),
    SMsgDirG2KeyedBase(KT_DIRECTORY)
{}

inline
SMsgDirG2UpdateBase::SMsgDirG2UpdateBase(const SMsgDirG2UpdateBase& theMsgR) :
    SMsgDirG2PeerDataBase(theMsgR),
    SMsgDirG2KeyedBase(theMsgR)
{}

inline
SMsgDirG2UpdateBase::~SMsgDirG2UpdateBase(void)
{}

inline
SMsgDirG2UpdateBase&
SMsgDirG2UpdateBase::operator=(const SMsgDirG2UpdateBase& theMsgR)
{
    SMsgDirG2PeerDataBase::operator=(theMsgR);
    SMsgDirG2KeyedBase::operator=(theMsgR);
    return *this;
}

};  // Namespace WONMsg

#endif