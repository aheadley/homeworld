#ifndef __SMSGTYPESFIREWALL_H__
#define __SMSGTYPESFIREWALL_H__

namespace WONMsg
{
	enum MsgTypeFirewall
	{
		FirewallMsgMin    = 0,
		FirewallDetectMsg = 1,
		FirewallStatusReply = 2,

		// Last Message type.  Don't use
		FirewallMsgMax
	};

};

#endif