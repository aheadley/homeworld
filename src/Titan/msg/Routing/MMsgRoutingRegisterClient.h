#if !defined(MMsgRoutingRegisterClient_H)
#define MMsgRoutingRegisterClient_H

// MMsgRoutingRegisterClient.h

#include "RoutingServerMessage.h"
#include "MMsgRoutingStatusReply.h"

namespace WONMsg {

//
// MMsgRoutingRegisterClient
//
class MMsgRoutingRegisterClient : public RoutingServerMessage {
public:
	enum { REGISTERFLAG_BECOMEHOST      = 0x01, 
	       REGISTERFLAG_BECOMESPECTATOR = 0x02,
		   REGISTERFLAG_SETUPCHAT       = 0x04 };

    // Default ctor
    MMsgRoutingRegisterClient(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingRegisterClient(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingRegisterClient(const MMsgRoutingRegisterClient& theMsgR);

    // Destructor
    virtual ~MMsgRoutingRegisterClient(void);

    // Assignment
    MMsgRoutingRegisterClient& operator=(const MMsgRoutingRegisterClient& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	const ClientName& GetClientName() const             { return mClientName; }
	const Password&   GetPassword() const               { return mPassword; }
	bool              TryingToBecomeHost() const        { return mTryingToBecomeHost; }
	bool              BecomeSpectator() const           { return mBecomeSpectator; }
	bool              SetupChat() const                 { return mSetupChat; }

	void SetClientName(const ClientName& theClientName) { mClientName = theClientName; }
	void SetPassword(const Password& thePassword)       { mPassword = thePassword; }
	void SetTryingToBecomeHost(bool tryingToBecomeHost) { mTryingToBecomeHost = tryingToBecomeHost; }
	void SetBecomeSpectator(bool becomeSpectator)       { mBecomeSpectator = becomeSpectator; }
	void SetSetupChat(bool setupChat)                   { mSetupChat = setupChat; }
private:
	ClientName mClientName;
	Password   mPassword;
	bool       mTryingToBecomeHost;
	bool       mBecomeSpectator;
	bool       mSetupChat;
};

//
// MMsgRoutingRegisterClientReply
//
class MMsgRoutingRegisterClientReply : public MMsgRoutingStatusReply {
public:
    // Default ctor
    MMsgRoutingRegisterClientReply(void);

    // RoutingServerMessage ctor
    explicit MMsgRoutingRegisterClientReply(const RoutingServerMessage& theMsgR);

    // Copy ctor
    MMsgRoutingRegisterClientReply(const MMsgRoutingRegisterClientReply& theMsgR);

    // Destructor
    virtual ~MMsgRoutingRegisterClientReply(void);

    // Assignment
    MMsgRoutingRegisterClientReply& operator=(const MMsgRoutingRegisterClientReply& theMsgR);

    // Virtual Duplicate from RoutingServerMessage
    virtual TRawMsg* Duplicate(void) const;

	// Debug output
	virtual void Dump(std::ostream& os) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    virtual void* Pack(void); 
    virtual void  Unpack(void);

    // Member access
	ClientId            GetClientId() const                 { return mClientId; }
	const ClientName&   GetHostName() const                 { return mHostName; }
	const std::wstring& GetHostComment() const              { return mHostComment; }

	void SetClientId(ClientId theClientId)                  { mClientId = theClientId; }
	void SetHostName(const ClientName& theHostName)         { mHostName = theHostName; }
	void SetHostComment(const std::wstring& theHostComment) { mHostComment = theHostComment; }
private:
	ClientId     mClientId;
	ClientName   mHostName;
	std::wstring mHostComment;
};


// Inlines
inline TRawMsg* MMsgRoutingRegisterClient::Duplicate(void) const
    { return new MMsgRoutingRegisterClient(*this); }
inline TRawMsg* MMsgRoutingRegisterClientReply::Duplicate(void) const
    { return new MMsgRoutingRegisterClientReply(*this); }

};  // Namespace WONMsg

#endif // MMsgRoutingRegisterClient_H