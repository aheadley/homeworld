#if !defined(MMsgRoutingDisconnectClient_H)
#define MMsgRoutingDisconnectClient_H

// MMsgRoutingDisconnectClient.h

#include "RoutingServerMessage.h"

namespace WONMsg {

class MMsgRoutingDisconnectClient : public RoutingServerMessage {
public:
    // Default ctor
    MMsgRoutingDisconnectClient(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingDisconnectClient(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingDisconnectClient(const MMsgRoutingDisconnectClient& theMsgR);

    // Destructor
    virtual ~MMsgRoutingDisconnectClient(void);

    // Assignment
    MMsgRoutingDisconnectClient& operator=(const MMsgRoutingDisconnectClient& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	bool IsPermanent() const;
	void SetIsPermanent(bool isPermanent);
private:
	bool mIsPermanent;
};


// Inlines
inline TRawMsg* MMsgRoutingDisconnectClient::Duplicate(void) const
    { return new MMsgRoutingDisconnectClient(*this); }

inline bool MMsgRoutingDisconnectClient::IsPermanent() const
{ return mIsPermanent; }
inline void MMsgRoutingDisconnectClient::SetIsPermanent(bool isPermanent)
{ mIsPermanent = isPermanent; }

};  // Namespace WONMsg

#endif // MMsgRoutingDisconnectClient_H