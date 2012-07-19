#if !defined(MMsgRoutingGetClientList_H)
#define MMsgRoutingGetClientList_H

// MMsgRoutingGetClientList.h

#include "LIST"
#include <climits>
#include "RoutingServerMessage.h"
#include "MMsgRoutingStatusReply.h"

namespace WONMsg {

//
// RoutingServerClientInfoMessage
//
class RoutingServerClientInfoMessage : public RoutingServerMessage {
public:
    // Default ctor
    RoutingServerClientInfoMessage(void);

    // RoutingServerMessage ctor
    explicit RoutingServerClientInfoMessage(const RoutingServerMessage& theMsgR);

    // Copy ctor
    RoutingServerClientInfoMessage(const RoutingServerClientInfoMessage& theMsgR);

    // Destructor
    virtual ~RoutingServerClientInfoMessage(void);

    // Assignment
    RoutingServerClientInfoMessage& operator=(const RoutingServerClientInfoMessage& theMsgR);

    // Debug output
    virtual void Dump(std::ostream& os) const;

    // Member access
    bool AuthInfoRequested() const                       { return mWasAuthInfoRequested; }
    void SetAuthInfoRequested(bool wasAuthInfoRequested) { mWasAuthInfoRequested = wasAuthInfoRequested; }
protected:
    bool mWasAuthInfoRequested;
};

//
// MMsgRoutingGetClientInfo
//
class MMsgRoutingGetClientInfo : public RoutingServerClientInfoMessage {
public:
    // Default ctor
    MMsgRoutingGetClientInfo(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingGetClientInfo(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingGetClientInfo(const MMsgRoutingGetClientInfo& theMsgR);

    // Destructor
    virtual ~MMsgRoutingGetClientInfo(void);

    // Assignment
    MMsgRoutingGetClientInfo& operator=(const MMsgRoutingGetClientInfo& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingGetClientInfo(*this); }

    // Debug output
    virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    ClientId GetClientId() const           { return mClientId; }
    void SetClientId(ClientId theClientId) { mClientId = theClientId; }
private:
    ClientId mClientId;
};

//
// MMsgRoutingGetClientList
//
class MMsgRoutingGetClientList : public RoutingServerClientInfoMessage {
public:
    // Default ctor
    MMsgRoutingGetClientList(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingGetClientList(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingGetClientList(const MMsgRoutingGetClientList& theMsgR);

    // Destructor
    virtual ~MMsgRoutingGetClientList(void);

    // Assignment
    MMsgRoutingGetClientList& operator=(const MMsgRoutingGetClientList& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingGetClientList(*this); }

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);
};

//
// MMsgRoutingGetMembersOfGroup
//
class MMsgRoutingGetMembersOfGroup : public RoutingServerClientInfoMessage {
public:
    // Default ctor
    MMsgRoutingGetMembersOfGroup(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingGetMembersOfGroup(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingGetMembersOfGroup(const MMsgRoutingGetMembersOfGroup& theMsgR);

    // Destructor
    virtual ~MMsgRoutingGetMembersOfGroup(void);

    // Assignment
    MMsgRoutingGetMembersOfGroup& operator=(const MMsgRoutingGetMembersOfGroup& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingGetMembersOfGroup(*this); }

    // Debug output
    virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    GroupId GetGroupId() const             { return mGroupId; }
    void    SetGroupId(GroupId theGroupId) { mGroupId = theGroupId; }
private:
    GroupId mGroupId;
};

//
// MMsgRoutingGetClientListReply
//
class MMsgRoutingGetClientListReply : public MMsgRoutingStatusReply {
public:
    // Client data
    struct ClientData {
        ClientId       mClientId;
        ClientName     mClientName;
        unsigned long  mIPAddress;
        unsigned long  mWONUserId;
        unsigned long  mCommunityId;
        unsigned short mTrustLevel;
    };
    typedef std::list<ClientData> ClientList;
    enum { MAX_CLIENT_LIST_SIZE = USHRT_MAX };

    // Default ctor
    MMsgRoutingGetClientListReply(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingGetClientListReply(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingGetClientListReply(const MMsgRoutingGetClientListReply& theMsgR);

    // Destructor
    virtual ~MMsgRoutingGetClientListReply(void);

    // Assignment
    MMsgRoutingGetClientListReply& operator=(const MMsgRoutingGetClientListReply& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const { return new MMsgRoutingGetClientListReply(*this); }

    // Debug output
    virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void);
    virtual void  Unpack(void);

    // Member access
    unsigned short GetNumSpectators() const                { return mNumSpectators; }
    bool IncludesIPs() const                               { return mIncludesIPs; }
    bool IncludesAuthInfo() const                          { return mIncludesAuthInfo; }
    const ClientList& GetClientList() const                { return mClientList; }

    void SetNumSpectators(unsigned short theNumSpectators) { mNumSpectators = theNumSpectators; }
    void SetIncludesIPs(bool includesIPs)                  { mIncludesIPs = includesIPs; }
    void SetIncludesAuthInfo(bool includesAuthInfo)        { mIncludesAuthInfo = includesAuthInfo; }
    void SetClientList(const ClientList& theClientListR)   { mClientList = theClientListR; }
    void AddClient(const ClientData& theClientDataR)       { mClientList.push_back(theClientDataR); }
private:
    unsigned short mNumSpectators;
    ClientList     mClientList;
    bool           mIncludesIPs;
    bool           mIncludesAuthInfo;
};

};  // Namespace WONMsg

inline ostream& operator<<(ostream& os, const WONMsg::MMsgRoutingGetClientListReply::ClientList& theClientList)
{
    WONMsg::MMsgRoutingGetClientListReply::ClientList::const_iterator itr = theClientList.begin();
    for (; itr != theClientList.end(); itr++)
    {
        os << " * " << itr->mClientId << ","
                    << itr->mClientName << ","
                    << (itr->mIPAddress & 0x000000FF) << "." << ((itr->mIPAddress & 0x0000FF00) >> 8) << "." << ((itr->mIPAddress & 0x00FF0000) >> 16) << "." << ((itr->mIPAddress & 0xFF000000) >> 24) << ","
                    << itr->mWONUserId << ","
                    << itr->mCommunityId << ","
                    << itr->mTrustLevel << endl;
    }
    return os;
}

#endif // MMsgRoutingGetClientList_H