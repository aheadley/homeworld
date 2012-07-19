#if !defined(MMsgRoutingReadDataObjectReply_H)
#define MMsgRoutingReadDataObjectReply_H

// MMsgRoutingReadDataObjectReply.h

#include "common/won.h"
#include "LIST"
#include <common/DataObject.h>
#include <common/OutputOperators.h>
#include "MMsgRoutingStatusReply.h"

namespace WONMsg {

//
// MMsgRoutingReadDataObjectReply
//
class MMsgRoutingReadDataObjectReply : public MMsgRoutingStatusReply {
public:
    struct DataObjectWithIds : public WONCommon::DataObject {
        DataObjectWithIds() {};
        DataObjectWithIds(const DataType& theDataTypeR) : DataObject(theDataTypeR) {};
        
        ClientOrGroupId mLinkId;
        ClientOrGroupId mOwnerId;
    };
    class LessDataObjectTypeAndLinkId { public: bool operator()(DataObjectWithIds d1, DataObjectWithIds d2) const { int c = d1.Compare(d2, false); return c == 0 ? d1.mLinkId < d2.mLinkId : c < 0; } };
    typedef std::set<DataObjectWithIds, LessDataObjectTypeAndLinkId> DataObjectSet;

    // Default ctor
    MMsgRoutingReadDataObjectReply(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingReadDataObjectReply(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingReadDataObjectReply(const MMsgRoutingReadDataObjectReply& theMsgR);

    // Destructor
    virtual ~MMsgRoutingReadDataObjectReply(void);

    // Assignment
    MMsgRoutingReadDataObjectReply& operator=(const MMsgRoutingReadDataObjectReply& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Debug output
    virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    const DataObjectSet& GetDataObjects() const                 { return mDataObjects; }
    void SetDataObjects(const DataObjectSet& theDataObjectsR)   { mDataObjects = theDataObjectsR; }
    void AddDataObject(const DataObjectWithIds& theDataObjectR) { mDataObjects.insert(theDataObjectR); }
private:
    DataObjectSet mDataObjects;
};

// Inlines
inline TRawMsg* MMsgRoutingReadDataObjectReply::Duplicate(void) const
    { return new MMsgRoutingReadDataObjectReply(*this); }

};  // Namespace WONMsg

inline ostream& operator<<(ostream& os, const WONMsg::MMsgRoutingReadDataObjectReply::DataObjectSet& theObjectSet)
{
    WONMsg::MMsgRoutingReadDataObjectReply::DataObjectSet::const_iterator itr = theObjectSet.begin();
    for (; itr != theObjectSet.end(); itr++)
    {
        os << " * Link: " << itr->mLinkId
           << ", Owner: " << itr->mOwnerId
           << ", Type: "  << itr->GetDataType()
           << ", Data: "  << itr->GetData();
    }
    return os;
}

#endif // MMsgRoutingReadDataObjectReply_H