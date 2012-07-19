#if !defined(MMsgRoutingReconnectClient_H)
#define MMsgRoutingReconnectClient_H

// MMsgRoutingReconnectClient.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingReconnectClient : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingReconnectClient(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingReconnectClient(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingReconnectClient(const MMsgRoutingReconnectClient& theMsgR);

    // Destructor
    virtual ~MMsgRoutingReconnectClient(void);

    // Assignment
    MMsgRoutingReconnectClient& operator=(const MMsgRoutingReconnectClient& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	ClientId GetClientId() const;
	bool WantMissedMessages() const;

	void SetClientId(ClientId theClientId);
	void SetWantMissedMessages(bool wantMissedMessages);
private:
	ClientId mClientId;
	bool     mWantMissedMessages;
};


// Inlines
inline TRawMsg* MMsgRoutingReconnectClient::Duplicate(void) const
    { return new MMsgRoutingReconnectClient(*this); }

inline ClientId MMsgRoutingReconnectClient::GetClientId() const
{ return mClientId; }
inline bool MMsgRoutingReconnectClient::WantMissedMessages() const
{ return mWantMissedMessages; }
inline void MMsgRoutingReconnectClient::SetClientId(ClientId theClientId)
{ mClientId = theClientId; }
inline void MMsgRoutingReconnectClient::SetWantMissedMessages(bool wantMissedMessages)
{ mWantMissedMessages = wantMissedMessages; }

};  // Namespace WONMsg

#endif // MMsgRoutingReconnectClient_H