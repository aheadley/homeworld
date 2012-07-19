#ifndef _SMsgDirG2QueryExtendBase_H
#define _SMsgDirG2QueryExtendBase_H

// SMsgDirG2QueryExtendBase.h

// Base class derived from SMsgDirG2QueryBase that provides common definitions for
// extended queries. Adds list of data object get types.  Also logical extended flag
// (is query an extended query).

// Derived class should set the mExtended member as needed to specify whether query
// is extended or not.  This should be done in constructors and Unpack.  Note that
// mExtended defaults to false.

// Adds a PackExtended() and UnpackExtended() methods to pack/unpack the data object
// get types.  These methods should be called be derived class Pack/Unpack at the
// appropriate point to pack/unpack extended data if needed.  Note that these methods
// are NoOps if message is not extended (mExtended == false).


#include "STRING"
#include "common/DataObject.h"
#include "msg/dir/SMsgDirG2QueryBase.h"


namespace WONMsg {

class SMsgDirG2QueryExtendBase : public SMsgDirG2QueryBase
{
public:
    // Default ctor
    explicit SMsgDirG2QueryExtendBase(KeyType theType, bool isExtended=false);

    // SmallMessage ctor
    explicit SMsgDirG2QueryExtendBase(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgDirG2QueryExtendBase(const SMsgDirG2QueryExtendBase& theMsgR);

    // Destructor
    virtual ~SMsgDirG2QueryExtendBase(void);

    // Assignment
    SMsgDirG2QueryExtendBase& operator=(const SMsgDirG2QueryExtendBase& theMsgR);

    // Virtual Duplicate from SmallMessage
    // Pure virtual - must be overridden!
    virtual TRawMsg* Duplicate(void) const = 0;

    // Extended access (read only)
    bool IsExtended() const;

    // Get Data Types access
    const WONCommon::DataObjectTypeSet& GetGetTypes() const;
    void SetGetTypes(const WONCommon::DataObjectTypeSet& theSetR);
    void AddGetType(const WONCommon::DataObject& theTypeR);
    void AddGetType(const WONCommon::DataObject::DataType& theTypeR);
    void AddGetType(const unsigned char* theTypeP, unsigned char theLen);

protected:
    bool                         mExtended;  // Is query extended?
    WONCommon::DataObjectTypeSet mGetTypes;  // Set of data object get types for extended query

    // Pack entities into raw buffer (call in Pack()).
    // Is a NoOp if mExtended is false.
    virtual void PackExtended();

    // Unpack entities from raw buffer (call in Unpack()).
    // Is a NoOp if mExtended is false.
    virtual void UnpackExtended();

private:
};


// Inlines
inline bool
SMsgDirG2QueryExtendBase::IsExtended() const
{ return mExtended; }

inline const WONCommon::DataObjectTypeSet&
SMsgDirG2QueryExtendBase::GetGetTypes() const
{ return mGetTypes; }

inline void
SMsgDirG2QueryExtendBase::SetGetTypes(const WONCommon::DataObjectTypeSet& theSetR)
{ mGetTypes = theSetR; }

inline void
SMsgDirG2QueryExtendBase::AddGetType(const WONCommon::DataObject& theTypeR)
{ mGetTypes.insert(theTypeR); }

inline void
SMsgDirG2QueryExtendBase::AddGetType(const WONCommon::DataObject::DataType& theTypeR)
{ mGetTypes.insert(WONCommon::DataObject(theTypeR)); }

inline void
SMsgDirG2QueryExtendBase::AddGetType(const unsigned char* theTypeP, unsigned char theLen)
{ mGetTypes.insert(WONCommon::DataObject(WONCommon::DataObject::DataType(theTypeP, theLen))); }


};  // Namespace WONMsg

#endif