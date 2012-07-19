#if !defined(MMsgRoutingPeerDataMultiple_H)
#define MMsgRoutingPeerDataMultiple_H

// MMsgRoutingPeerDataMultiple.h

#include "common/won.h"
#include "LIST"
#include <common/OutputOperators.h>
#include "RoutingServerMessage.h"

namespace WONMsg {

// forward declarations
class MMsgRoutingSendDataMultiple;

class MMsgRoutingPeerDataMultiple : public RoutingServerMessage {
public:
    struct PeerDataMessage {
        ClientId             mClientId;
        WONCommon::RawBuffer mData;

        PeerDataMessage() : mClientId(0) {}
        PeerDataMessage(ClientId theClientId, const WONCommon::RawBuffer& theDataR) : mClientId(theClientId), mData(theDataR) {}
    };
    typedef std::list<PeerDataMessage> MessageList;

    // Default ctor
    MMsgRoutingPeerDataMultiple(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingPeerDataMultiple(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingPeerDataMultiple(const MMsgRoutingPeerDataMultiple& theMsgR);

    MMsgRoutingPeerDataMultiple(ClientId theSender, const MMsgRoutingSendDataMultiple& theSendDataMultipleMsgR);

    // Destructor
    virtual ~MMsgRoutingPeerDataMultiple(void);

    // Assignment
    MMsgRoutingPeerDataMultiple& operator=(const MMsgRoutingPeerDataMultiple& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

    // Debug output
    virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    const MessageList& GetMessageList() const               { return mMessageList; }
    void SetMessageList(const MessageList& theMessageListR) { mMessageList = theMessageListR; }
    void AddMessage(const PeerDataMessage& theMessageR)     { mMessageList.push_back(theMessageR); }
private:
    MessageList mMessageList;
};


// Inlines
inline TRawMsg* MMsgRoutingPeerDataMultiple::Duplicate(void) const
    { return new MMsgRoutingPeerDataMultiple(*this); }

};  // Namespace WONMsg

inline ostream& operator<<(ostream& os, const WONMsg::MMsgRoutingPeerDataMultiple::MessageList& theMessageList)
{
    WONMsg::MMsgRoutingPeerDataMultiple::MessageList::const_iterator itr = theMessageList.begin();
    for (; itr != theMessageList.end(); itr++)
        os << " * Sender: " << itr->mClientId << ", Data: " << itr->mData;
    return os;
}

#endif // MMsgRoutingPeerDataMultiple_H