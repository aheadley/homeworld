#ifndef _TitanPacketMsg_H
#define _TitanPacketMsg_H


#include "STRING"
#include "msg/TMessage.h"
#include "msg/MServiceTypes.h"

// Forwards from WONSocket
class TitanPacketMsg : public WONMsg::MiniMessage
{
public:
    // Default ctor
    TitanPacketMsg(unsigned char theType, bool encrypted=false);

    // MiniMessage ctor - will throw if MiniMessage type is not MyServiceType
    explicit TitanPacketMsg(const WONMsg::MiniMessage& theMsgR);

    // Copy ctor
    TitanPacketMsg(const TitanPacketMsg& theMsgR);

    // Destructor
    ~TitanPacketMsg(void);

    // Assignment
    TitanPacketMsg& operator=(const TitanPacketMsg& theMsgR);

    // Virtual Duplicate from TMessage
    WONMsg::TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack(void);
    void  Unpack(void);

    // Blob access
    const void*    GetBlob(void) const     { return mBlob; }
    unsigned short GetBlobLen(void) const  { return mBlobLen; }
    const wstring& GetGameName(void) const { return mGameName; }

    // Update the blob
    void SetGameName(const wstring& theGameNameR)            { mGameName = theGameNameR; }
    void SetBlob(const void* theBlob, unsigned short theLen) { mBlob = theBlob; mBlobLen = theLen; }

    enum {
        MyServiceType        = WONMsg::MaxMiniServiceType + 1,
		MyEncryptServiceType = WONMsg::MaxMiniServiceType + 2
    };

private:
    unsigned short mBlobLen;  // Length of data
    const void*    mBlob;
	bool           mIncludeGameName;
    wstring        mGameName;
};


// Inlines
inline WONMsg::TRawMsg* TitanPacketMsg::Duplicate(void) const
{ return new TitanPacketMsg(*this); }

#endif