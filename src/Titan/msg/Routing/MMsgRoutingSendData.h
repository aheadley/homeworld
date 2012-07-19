#if !defined(MMsgRoutingSendData_H)
#define MMsgRoutingSendData_H

// MMsgRoutingSendData.h

#include "common/won.h"
#include "AddresseeList.h"

namespace WONMsg {

class MMsgRoutingSendData : public RoutingServerMessage, public AddresseeList {
public:
    // Default ctor
    MMsgRoutingSendData(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingSendData(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingSendData(const MMsgRoutingSendData& theMsgR);

    // Destructor
    virtual ~MMsgRoutingSendData(void);

    // Assignment
    MMsgRoutingSendData& operator=(const MMsgRoutingSendData& theMsgR);

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
inline TRawMsg* MMsgRoutingSendData::Duplicate(void) const
    { return new MMsgRoutingSendData(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingSendData_H