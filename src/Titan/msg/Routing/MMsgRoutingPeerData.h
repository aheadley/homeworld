#if !defined(MMsgRoutingPeerData_H)
#define MMsgRoutingPeerData_H

// MMsgRoutingPeerData.h

#include "common/won.h"
#include "RoutingServerMessage.h"

namespace WONMsg {

// forward declarations
class MMsgRoutingSendData;
class MMsgRoutingSendDataBroadcast;

class MMsgRoutingPeerData : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingPeerData(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingPeerData(const RoutingServerMessage& theMsgR, bool doUnpack =true);

    // Copy ctor
    MMsgRoutingPeerData(const MMsgRoutingPeerData& theMsgR);

	MMsgRoutingPeerData(ClientId theSender, const MMsgRoutingSendData& theSendDataMsgR);
	MMsgRoutingPeerData(ClientId theSender, const MMsgRoutingSendDataBroadcast& theBroadcastMsgR);

    // Destructor
    virtual ~MMsgRoutingPeerData(void);

    // Assignment
    MMsgRoutingPeerData& operator=(const MMsgRoutingPeerData& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	ClientId GetClientId() const                                 { return mClientId; }
	const WONCommon::RawBuffer& GetData() const                  { return mData; }

	void SetClientId(ClientId theClientId)                       { mClientId = theClientId; }
	void SetData(const WONCommon::RawBuffer& theDataR)           { mData = theDataR; }
	void AppendToData(const WONCommon::RawBuffer& theAppendandR) { mData += theAppendandR; }
protected:
	ClientId             mClientId;
	WONCommon::RawBuffer mData;
};


// Inlines
inline TRawMsg* MMsgRoutingPeerData::Duplicate(void) const
    { return new MMsgRoutingPeerData(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingPeerData_H