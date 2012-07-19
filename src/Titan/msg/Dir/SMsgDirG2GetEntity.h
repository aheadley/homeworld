#ifndef _SMsgDirG2GetEntity_H
#define _SMsgDirG2GetEntity_H

// SMsgDirG2GetEntity.h

// DirectoryServer get entity message.  Requests a directory/service and or its contents.
// Fields in reply are configurable.

#include "STRING"
#include "SMsgDirG2QueryGetBase.h"


namespace WONMsg {

class SMsgDirG2GetEntity : public SMsgDirG2QueryGetBase
{
public:
    // Default ctor
    explicit SMsgDirG2GetEntity(KeyType theType=KT_SERVICE, bool isExtended=false);

    // SmallMessage ctor
    explicit SMsgDirG2GetEntity(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgDirG2GetEntity(const SMsgDirG2GetEntity& theMsgR);

    // Destructor
    ~SMsgDirG2GetEntity(void);

    // Assignment
    SMsgDirG2GetEntity& operator=(const SMsgDirG2GetEntity& theMsgR);

    // Virtual Duplicate
    TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack(void);
    void  Unpack(void);

private:
};


// Inlines
inline TRawMsg*
SMsgDirG2GetEntity::Duplicate(void) const
{ return new SMsgDirG2GetEntity(*this); }

};  // Namespace WONMsg

#endif