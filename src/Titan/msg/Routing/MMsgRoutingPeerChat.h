#if !defined(MMsgRoutingPeerChat_H)
#define MMsgRoutingPeerChat_H

// MMsgRoutingPeerChat.h

#include "MMsgRoutingPeerData.h"
#include "AddresseeList.h"

namespace WONMsg {

// forward declarations
class MMsgRoutingSendChat;

class MMsgRoutingPeerChat : public MMsgRoutingPeerData, public AddresseeList {
public:
    // Default ctor
    MMsgRoutingPeerChat(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingPeerChat(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingPeerChat(const MMsgRoutingPeerChat& theMsgR);

	MMsgRoutingPeerChat(ClientId theSender, const MMsgRoutingSendChat& theSendChatMsgR);

    // Destructor
    virtual ~MMsgRoutingPeerChat(void);

    // Assignment
    MMsgRoutingPeerChat& operator=(const MMsgRoutingPeerChat& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	ChatType GetChatType() const           { return mChatType; }
	void SetChatType(ChatType theChatType) { mChatType = theChatType; }
private:
	ChatType mChatType;
};


// Inlines
inline TRawMsg* MMsgRoutingPeerChat::Duplicate(void) const
    { return new MMsgRoutingPeerChat(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingPeerChat_H