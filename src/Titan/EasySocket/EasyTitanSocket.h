/******************************************************************************/
/*                                                                            */
/*                                 EASYTITANSOCKET.H                          */
/*                               WONMisc Socket Class                         */
/*                                   Include File                             */
/*                                                                            */
/******************************************************************************/
           
/*

  Class:             EasyTitanSocket

  Description:       This class implements methods for easilly sending and 
                     receiving Titan messages.

  Author:            Brian Rothstein
  Last Modified:     28 Sept 98

  Base Classes:      EasySocket

  Contained Classes: none

  Friend Classes:    none

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

  Contained Data Types: none

  Private Data Members: none

  Protected Data Members: None

  Public Data Members: None

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

 	Constructors:
    EasyTitanSocket(SocketType theType)
      This is the main constructor that you want to use since most of the
      time you know what kind of socket you want when you declare it.  The
      constructor doesn't do much.  It doesn't actually get a new socket
      descriptor.  It just initializes the type so that when a connect or
      sendto is performed, the correct kind of socket can be opened.

    EasyTitanSocket()
      The default constructor sets the socket's type to NO_TYPE.  Before 
      using the socket you must call the setType method.  This constructor
      is useful if you don't initially know what kind of socket you need.

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

  Private Methods: none

  Protected Methods: None

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

  Public Methods:
    ES_ErrorType sendTMessage(TMessage *theMsg, int theTotalTime = 1000);
    ES_ErrorType recvTMessage(TMessage *theMsg, int theTotalTime = 1000);
      Send and receive Titan messages.  These functions provide convenient
      ways of sending and receiving Titan messages on connected sockets.
      They handle packing and unpacking of the message and receiving the
      header and assuring that the header types match and that the total
      length is received.  If a base class TMessage is passes to the
      recvTMessage method, then no check is performed for matching types
      and unpack is not called.

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

  Accessors: none
*/



#ifndef __EASYTITANSOCKET_H__
#define __EASYTITANSOCKET_H__

#include "EasySocket.h"
#include "msg/tmessage.h"
#include "auth/Auth1PublicKeyBlock.h"
#include "auth/Auth1Certificate.h"
#include "auth/cryptflags.h"
#include "crypt/EGPrivateKey.h"
#include "crypt/EGPublicKey.h"
#include "crypt/BFSymmetricKey.h"

using namespace WONMsg;

namespace WONMisc {

struct TagInfo
{
	TagInfo() : mTag(-1), mTagDuration(-1) {}
	long mTag;
	short mTagDuration;
};

class EasyTitanSocket : public EasySocket {
protected:
	unsigned long mLength, mCurLen;
	char *mBuffer;
	unsigned char mLengthFieldSize, mTCPLengthFieldSize;

	WONAuth::Auth1PublicKeyBlock mPubKeyBlock;
	WONAuth::Auth1Certificate mCertificate;
	WONCrypt::EGPrivateKey mPrivateKey;
	WONCrypt::BFSymmetricKey mSessionKey;
	bool mIsSequenced, mIsEncrypted, mIsSessioned;
	unsigned short mInSeq, mOutSeq, mSessionId;

	void GetServiceAndMessageTypes(const BaseMessage* theMsgP, unsigned long* theServiceTypeP, unsigned long* theMessageTypeP);
	ES_ErrorType recvLength(unsigned long theHeaderLength, int theTotalTime);
	ES_ErrorType recvMessage(unsigned long theHeaderLength, int theTotalTime);

	ES_ErrorType ExtractMsg(BaseMessage **theMsgPP, int theOffset, TagInfo &theTag);
	ES_ErrorType ExtractTag(BaseMessage **theMsgPP, int theOffset, TagInfo &theTag);

	void init();

public:
	EasyTitanSocket();
	EasyTitanSocket(SocketType theType);

	virtual ~EasyTitanSocket();

	void setType(SocketType theType);

	void SetLengthFieldSize(unsigned char theLengthFieldSize);
	unsigned char GetLengthFieldSize() const;

	virtual ES_ErrorType sendMessage(BaseMessage *theMsg, int theTotalTime = 1000);
	virtual ES_ErrorType sendMessage(BaseMessage *theMsg, const TagInfo &theTag, int theTotalTime = 1000);

	virtual ES_ErrorType recvMessage(BaseMessage *theMsg, int theTotalTime = 5000);
	virtual ES_ErrorType recvMessage(BaseMessage **theMsgPP, int theTotalTime = 5000);
	virtual ES_ErrorType recvMessage(BaseMessage **theMsgPP, TagInfo &theTag, int theTotalTime = 5000);

	ES_ErrorType GetCertificate(const string &theUserName, const string &theCommunity, const string &thePassword, int theTotalTime = 15000);
	ES_ErrorType CreateAccount(const string &theUserName, const string &theCommunity, const string &thePassword, int theTotalTime = 15000);
	ES_ErrorType ChangePassword(const string &theUserName, const string &theCommunity, const string &thePassword, const string &theNewPassword, int theTotalTime = 15000);

	ES_ErrorType DoLogin(bool createAccount, const string &theUserName, const string &theCommunity,
									  const string &theNickNameKey, const string &thePassword,
									  const string &theNewPassword, const string &theNickName,
									  int theTotalTime = 10000);

	ES_ErrorType Authenticate(WONAuth::AuthenticationMode theAuthMode, WONAuth::EncryptionMode theEncryptMode, 
		unsigned short theEncryptFlags, int theTotalTime=10000);

	void ResetAuth();

	const WONCrypt::BFSymmetricKey& GetSessionKey() const;

};

}; //end namespace WONMisc
#endif