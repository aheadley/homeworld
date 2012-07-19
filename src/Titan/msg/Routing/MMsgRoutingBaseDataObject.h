#if !defined(MMsgRoutingBaseDataObject_H)
#define MMsgRoutingBaseDataObject_H

// MMsgRoutingBaseDataObject.h

#include "common/won.h"
#include "LIST"
#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingBaseDataObject : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingBaseDataObject(void);

    // RoutingServerMessage ctor
    MMsgRoutingBaseDataObject(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingBaseDataObject(const MMsgRoutingBaseDataObject& theMsgR);

    // Destructor
    virtual ~MMsgRoutingBaseDataObject(void);

    // Assignment
    MMsgRoutingBaseDataObject& operator=(const MMsgRoutingBaseDataObject& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Debug output
    virtual void Dump(std::ostream& os) const;

    // Member access
    ClientOrGroupId GetLinkId() const                                { return mLinkId; }
    const WONCommon::RawBuffer& GetDataType() const                  { return mDataType; }

    void SetLinkId(ClientOrGroupId theLinkId)                        { mLinkId = theLinkId; }
    void SetDataType(const WONCommon::RawBuffer& theDataTypeR)       { mDataType = theDataTypeR; }
    void AppendToDataType(const WONCommon::RawBuffer& theAppendandR) { mDataType += theAppendandR; }
protected:
    ClientOrGroupId      mLinkId;
    WONCommon::RawBuffer mDataType;
};


// Inlines
inline TRawMsg* MMsgRoutingBaseDataObject::Duplicate(void) const
    { return new MMsgRoutingBaseDataObject(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingBaseDataObject_H