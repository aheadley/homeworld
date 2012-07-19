#ifndef ROUTINGSENDCHAT_H
#define ROUTINGSENDCHAT_H

// MMsgRoutingSendChat.h

#include "MMsgRoutingSendData.h"

namespace WONMsg {

class MMsgRoutingSendChat : public MMsgRoutingSendData {
public:
    // Default ctor
    MMsgRoutingSendChat(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingSendChat(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingSendChat(const MMsgRoutingSendChat& theMsgR);

    // Destructor
    virtual ~MMsgRoutingSendChat(void);

    // Assignment
    MMsgRoutingSendChat& operator=(const MMsgRoutingSendChat& theMsgR);

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
inline TRawMsg* MMsgRoutingSendChat::Duplicate(void) const
    { return new MMsgRoutingSendChat(*this); }

};  // Namespace WONMsg

#endif // ROUTINGSENDCHAT_H