#ifndef _SMsgDirG2UpdateExtendBase_H
#define _SMsgDirG2UpdateExtendBase_H

// SMsgDirG2UpdateExtendBase.h

// Base class derived from UpdateBase that provides common definitions for extended
// updates. Adds list of data objects.  Also logical extended flag (is update an
// extended update).

// Derived class should set the mExtended member as needed to specify whether update
// is extended or not.  This should be done in constructors and Unpack.  Note that
// mExtended defaults to false.

// Adds a PackExtended() and UnpackExtended() methods to pack/unpack the data object
// get types.  These methods should be called be derived class Pack/Unpack at the
// appropriate point to pack/unpack extended data if needed.  Note that these methods
// are NoOps if message is not extended (mExtended == false).


#include "STRING"
#include "common/DataObject.h"
#include "msg/dir/SMsgDirG2UpdateBase.h"


namespace WONMsg {

class SMsgDirG2UpdateExtendBase : public SMsgDirG2UpdateBase
{
public:
    // Default ctor
    explicit SMsgDirG2UpdateExtendBase(KeyType theType, bool isExtended=false);

    // SmallMessage ctor
    explicit SMsgDirG2UpdateExtendBase(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgDirG2UpdateExtendBase(const SMsgDirG2UpdateExtendBase& theMsgR);

    // Destructor
    virtual ~SMsgDirG2UpdateExtendBase(void);

    // Assignment
    SMsgDirG2UpdateExtendBase& operator=(const SMsgDirG2UpdateExtendBase& theMsgR);

    // Virtual Duplicate from SmallMessage
    // Pure virtual - must be overridden!
    virtual TRawMsg* Duplicate(void) const = 0;

    // Extended access (read only)
    bool IsExtended() const;

    // Data Object access
    const WONCommon::DataObjectTypeSet& GetDataObjects() const;
    void SetDataObjects(const WONCommon::DataObjectTypeSet& theSetR);
    void AddDataObject(const WONCommon::DataObject& theObjR);

protected:
    bool                         mExtended;      // Is update extended?
    WONCommon::DataObjectTypeSet mDataObjects;  // Set of data objects for extended updates

    // Pack data objects into raw buffer (call in Pack()).
    // Is a NoOp if mExtended is false.
    virtual void PackExtended();

    // Unpack data objects from raw buffer (call in Unpack()).
    // Is a NoOp if mExtended is false.
    virtual void UnpackExtended();

private:
};


// Inlines
inline bool
SMsgDirG2UpdateExtendBase::IsExtended() const
{ return mExtended; }

inline const WONCommon::DataObjectTypeSet&
SMsgDirG2UpdateExtendBase::GetDataObjects() const
{ return mDataObjects; }

inline void
SMsgDirG2UpdateExtendBase::SetDataObjects(const WONCommon::DataObjectTypeSet& theSetR)
{ mDataObjects = theSetR; }

inline void
SMsgDirG2UpdateExtendBase::AddDataObject(const WONCommon::DataObject& theObjR)
{ mDataObjects.insert(theObjR); }


};  // Namespace WONMsg

#endif