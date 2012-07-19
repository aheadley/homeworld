#if !defined(MMsgRoutingCreateDataObject_H)
#define MMsgRoutingCreateDataObject_H

// MMsgRoutingCreateDataObject.h

#include "MMsgRoutingBaseDataObject.h"

namespace WONMsg {

//
// MMsgRoutingCreateDataObject
//
class MMsgRoutingCreateDataObject : public MMsgRoutingBaseDataObject {
public:
    // Default ctor
    MMsgRoutingCreateDataObject(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingCreateDataObject(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingCreateDataObject(const MMsgRoutingCreateDataObject& theMsgR);

    // Destructor
    virtual ~MMsgRoutingCreateDataObject(void);

    // Assignment
    MMsgRoutingCreateDataObject& operator=(const MMsgRoutingCreateDataObject& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	ClientOrGroupId GetOwnerId() const                           { return mOwnerId; }
	unsigned short GetLifespan() const                           { return mLifespan; }
	const WONCommon::RawBuffer& GetData() const                  { return mData; }

	void SetOwnerId(ClientOrGroupId theOwnerId)                  { mOwnerId = theOwnerId; }
	void SetLifespan(unsigned short theLifespan)                 { mLifespan = theLifespan; }
	void SetData(const WONCommon::RawBuffer& theDataR)           { mData = theDataR; }
	void AppendToData(const WONCommon::RawBuffer& theAppendandR) { mData += theAppendandR; }
private:
	ClientOrGroupId      mOwnerId;
	unsigned short       mLifespan;
	WONCommon::RawBuffer mData;
};

// Inlines
inline TRawMsg* MMsgRoutingCreateDataObject::Duplicate(void) const
    { return new MMsgRoutingCreateDataObject(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingCreateDataObject_H