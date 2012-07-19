#ifndef _SMsgDirG2QueryGetBase_H
#define _SMsgDirG2QueryGetBase_H

// SMsgDirG2QueryGetBase.h

// Base class derived from SMsgDirG2QueryExtendBase that provides common definitions
// for get queries. Adds fields for GetFlags.

// Derived class should set the mExtended member from SMsgDirG2QueryExtendBase as
// needed to specify whether query is extended or not.  This should be done in
// constructors and Unpack.  Note that mExtended defaults to false.

// Note that the Pack and Unpack() methods append/write the getflags.  This implies
// that all classes derived from QueryGetBase MUST have getflags as the first field
// after their message header.

// Note: SMsgDirg2QueryGetbase does not override the pure virtual Duplicate() method
// from SMsgDirG2QueryExtendBase.  Derrived classes must override this method.


#include "STRING"
#include "common/DataObject.h"
#include "msg/TMessage.h"
#include "SMsgDirG2QueryExtendBase.h"


namespace WONMsg {

class SMsgDirG2QueryGetBase : public SMsgDirG2QueryExtendBase
{
public:
    // Default ctor
    explicit SMsgDirG2QueryGetBase(KeyType theType, bool isExtended=false);

    // SmallMessage ctor
    explicit SMsgDirG2QueryGetBase(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgDirG2QueryGetBase(const SMsgDirG2QueryGetBase& theMsgR);

    // Destructor
    virtual ~SMsgDirG2QueryGetBase(void);

    // Assignment
    SMsgDirG2QueryGetBase& operator=(const SMsgDirG2QueryGetBase& theMsgR);

    // Pack and Unpack the message
    // Pack and Unpack will only affect mFlags.  mEntriesPerReply MUST be
    // packed/unpacked explicitly be derived classes as needed.
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Flags for returned fields
    unsigned long GetFlags() const;
    void          SetFlags(unsigned long theFlags);
    
    // EntriesPerReply access
    unsigned short GetEntitiesPerReply() const;
    void           SetEntitiesPerReply(unsigned short theCt);
    
protected:
    unsigned long  mFlags;             // Get Flags
    unsigned short mEntitiesPerReply;  // Max entities per reply

private:
};


// Inlines
inline unsigned long
SMsgDirG2QueryGetBase::GetFlags() const
{ return mFlags; }

inline void
SMsgDirG2QueryGetBase::SetFlags(unsigned long theFlags)
{ mFlags = theFlags; }

inline unsigned short
SMsgDirG2QueryGetBase::GetEntitiesPerReply() const
{ return mEntitiesPerReply; }

inline void
SMsgDirG2QueryGetBase::SetEntitiesPerReply(unsigned short theCt)
{ mEntitiesPerReply = theCt; }

};  // Namespace WONMsg

#endif