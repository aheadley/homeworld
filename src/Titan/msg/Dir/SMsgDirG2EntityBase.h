#ifndef _SMsgDirG2EntityBase_H
#define _SMsgDirG2EntityBase_H

// SMsgDirG2EntityBase.h

// Base class derived from SmallMessage that adds the capability to pack and unpack
// DirEntities.  Adds a container of data object types that may be used to specify
// which data object of the entity to pack.
// Does not override Pack/Unpack.

// Adds pure virtual GetFlags() method.  This method must be overridden to return
// the set of get flags for packing the entity.

// Adds a PackEntity() and UnpackEntity() methods to pack/unpack an entity.
// These methods should be called by the derived class Pack/Unpack.


#include "STRING"
#include "common/DataObject.h"
#include "msg/TMessage.h"
#include "DirEntity.h"


namespace WONMsg {

class SMsgDirG2EntityBase : public SmallMessage
{
public:
    // Default ctor
    SMsgDirG2EntityBase(void);

    // SmallMessage ctor
    explicit SMsgDirG2EntityBase(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgDirG2EntityBase(const SMsgDirG2EntityBase& theMsgR);

    // Destructor
    virtual ~SMsgDirG2EntityBase(void);

    // Assignment
    SMsgDirG2EntityBase& operator=(const SMsgDirG2EntityBase& theMsgR);

    // Virtual Duplicate from SmallMessage
    // Pure virtual - must be overridden!
    virtual TRawMsg* Duplicate(void) const = 0;

    // Fetch get flags used for Pack/Unpack entities
    // Pure virtual - must be overridden!
    virtual unsigned long GetFlags() const = 0;

    // Data Types access
    const WONCommon::DataObjectTypeSet& GetDataTypes() const;
    void SetDataTypes(const WONCommon::DataObjectTypeSet& theSetR);

protected:
    WONCommon::DataObjectTypeSet mDataTypes;  // Set of data object types to add

    // Pack entity into raw buffer (call in Pack())
    virtual void PackEntity(const DirEntity& theEntityR);

    // Unpack entity from raw buffer (call in Unpack())
    virtual void UnpackEntity(DirEntity& theEntityR);

private:
};


// Inlines
inline const WONCommon::DataObjectTypeSet&
SMsgDirG2EntityBase::GetDataTypes() const
{ return mDataTypes; }

inline void
SMsgDirG2EntityBase::SetDataTypes(const WONCommon::DataObjectTypeSet& theSetR)
{ mDataTypes = theSetR; }


};  // Namespace WONMsg

#endif