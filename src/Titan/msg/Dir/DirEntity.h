#ifndef _DirEntity_H
#define _DirEntity_H

// DirEntity

// Defines a generic entity used to represent nodes in the DirServer's tree.
// DirEntities may generically represent a directory node or service node.  This
// implies that non all fields of a DirEntity are applicable, the type of the
// DirEntity must be examined to determine which fields are appropriate.

// DirEntity is used internally in many DirServer messages to represent a
// DirServer node.  Depending on the message and it's attributes, the various
// fields of the DirEntity may or may not be filled in.  The context of the
// individual message must be examined in order to determine which fields of
// DirEntity will have meaning.


#include "../../common/won.h"
#include "STRING"
#include "LIST"
#include "../../common/DataObject.h"
#include "DirG2Flags.h"

// In the WONMsg namespace
namespace WONMsg {

// Fowards
class BaseMessage;


struct DirEntity
{
    // Constants
    enum {
        DE_UNKNOWN   = '\0',

        // Entity types
        ET_DIRECTORY = 'D',
        ET_SERVICE   = 'S',

        // Visibility types
        VT_VISIBLE   = 'V',
        VT_INVISIBLE = 'I'
    };

    // Entity type
    unsigned char mType;  // Directory or Service?

    // These members apply to all entities
    std::wstring  mPath;         // Path to parent dir
    std::wstring  mName;         // Name
    std::wstring  mDisplayName;  // Display name (may be null)
    long          mCreated;      // Entity create time as time_t
    long          mTouched;      // Last touched time as time_t
    unsigned long mLifespan;     // Lifespan in seconds
    unsigned long mCRC;          // Entity CRC

    // Data Objects are common to all entity types.  [0..n]
    WONCommon::DataObjectTypeSet mDataObjects;

    // These members are only applicable to entity types ET_DIRECTORY
    unsigned char mVisible;  // Visibility

    // These members are only applicable to entity types ET_SERVICE
    WONCommon::RawBuffer mNetAddress;  // Network address (binary)

    // Constructors / Destructor
    DirEntity();
    DirEntity(const DirEntity& theEntity);
    ~DirEntity();

    // Operators
    DirEntity& operator=(const DirEntity& theKey);

    // Fetch full path (path + name)
    const std::wstring GetFullPath() const;

    // Pack entity into message
    void Pack(BaseMessage& theMsgR, unsigned long theFlags,
              const WONCommon::DataObjectTypeSet& theSetR=gEmptyTypeSet) const;

    // Unpack entity from message
    void Unpack(BaseMessage& theMsgR, unsigned long theFlags);

    // Compute the size (in bytes) to add entry to a Message
    unsigned long ComputeSize(unsigned long theGetFlags,
                              const WONCommon::DataObjectTypeSet& theSetR=gEmptyTypeSet) const;

    // Class Constants
    static const WONCommon::DataObjectTypeSet gEmptyTypeSet;

private:
    // Private Methods
    static void PackDataObjects(BaseMessage& theMsgR, const WONCommon::DataObjectTypeSet& theSetR,
                                unsigned long theFlags);

    static void UnpackDataObjects(BaseMessage& theMsgR, WONCommon::DataObjectTypeSet& theSetR,
                                  unsigned long theFlags);

};

// List of DirEntitys
typedef std::list<DirEntity> DirEntityList;


};  // Namespace WONMsg

#endif