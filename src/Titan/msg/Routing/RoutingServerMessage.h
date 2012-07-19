#ifndef ROUTINGSERVERMESSAGE_H
#define ROUTINGSERVERMESSAGE_H

#include "common/won.h"
#include "common/OutputOperators.h"
#include "LIST"
#include "msg/TMessage.h"
#include "RoutingServerTypes.h"

namespace WONMsg {

class RoutingServerMessage : public MiniMessage {
public:
    RoutingServerMessage(void);
    RoutingServerMessage(const RoutingServerMessage& theMsgR);
    RoutingServerMessage(const MiniMessage& theMsgR);
    ~RoutingServerMessage(void);

    RoutingServerMessage& operator=(const RoutingServerMessage& theMsgR);

    void AppendClientName(const WONMsg::ClientName& theString);
    void AppendGroupName(const WONMsg::GroupName& theString);
    void AppendPassword(const WONMsg::Password& theString);
    void AppendClientId(ClientId theClientId);
    void AppendGroupId(GroupId theGroupId);
    void AppendClientOrGroupId(ClientOrGroupId theClientOrGroupId);

    void            ReadClientName(WONMsg::ClientName& theBufR) const;
    void            ReadGroupName(WONMsg::GroupName& theBufR) const;
    void            ReadPassword(WONMsg::Password& theBufR) const;
    ClientId        ReadClientId() const;
    GroupId         ReadGroupId() const;
    ClientOrGroupId ReadClientOrGroupId() const;
};

}; // namespace WONMsg

inline ostream& operator<<(ostream& os, const std::list<WONCommon::RawBuffer>& theRawBufferList)
{
    std::list<WONCommon::RawBuffer>::const_iterator itr = theRawBufferList.begin();
    for (; itr != theRawBufferList.end(); itr++)
        os << " * " << *itr;
    return os;
}

#endif // ROUTINGSERVERMESSAGE_H