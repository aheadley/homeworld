#ifndef __SMSGFIREWALLDETECT_H__
#define __SMSGFIREWALLDETECT_H__

#include "msg/TMessage.h"

namespace WONMsg {

class SMsgFirewallDetect : public SmallMessage {

public:
	SMsgFirewallDetect(void);
	explicit SMsgFirewallDetect(const SmallMessage& theMsgR);
	SMsgFirewallDetect(const SMsgFirewallDetect& theMsgR);

	virtual ~SMsgFirewallDetect(void);

	SMsgFirewallDetect& operator=(const SMsgFirewallDetect& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const { return new SMsgFirewallDetect(*this); }

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	unsigned short GetListenPort(void) const { return mListenPort; }
	void SetListenPort(unsigned short theListenPort) { mListenPort = theListenPort; }

	bool GetWaitForConnect(void) const { return mWaitForConnect; }
	void SetWaitForConnect(bool theVal) { mWaitForConnect = theVal; }

	unsigned long GetMaxConnectTime() { return mMaxConnectTime; }
	void SetMaxConnectTime(unsigned long theVal) { mMaxConnectTime = theVal; }

protected:
	unsigned short mListenPort;
	bool mWaitForConnect;
	unsigned long mMaxConnectTime;

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



};  // Namespace WONMsg

#endif