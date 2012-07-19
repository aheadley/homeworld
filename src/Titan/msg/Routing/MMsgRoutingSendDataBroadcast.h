#if !defined(MMsgRoutingSendDataBroadcast_H)
#define MMsgRoutingSendDataBroadcast_H

// MMsgRoutingSendDataBroadcast.h

#include "common/won.h"
#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingSendDataBroadcast : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingSendDataBroadcast(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingSendDataBroadcast(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingSendDataBroadcast(const MMsgRoutingSendDataBroadcast& theMsgR);

    // Destructor
    virtual ~MMsgRoutingSendDataBroadcast(void);

    // Assignment
    MMsgRoutingSendDataBroadcast& operator=(const MMsgRoutingSendDataBroadcast& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	bool ShouldSendReply() const                                 { return mShouldSendReply; }
	const WONCommon::RawBuffer& GetData() const                  { return mData; }

	void SetShouldSendReply(bool shouldSendReply)                { mShouldSendReply = shouldSendReply; }
	void SetData(const WONCommon::RawBuffer& theDataR)           { mData = theDataR; }
	void AppendToData(const WONCommon::RawBuffer& theAppendandR) { mData += theAppendandR; }
protected:
	bool                 mShouldSendReply;
	WONCommon::RawBuffer mData;
};


// Inlines
inline TRawMsg* MMsgRoutingSendDataBroadcast::Duplicate(void) const
    { return new MMsgRoutingSendDataBroadcast(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingSendDataBroadcast_H