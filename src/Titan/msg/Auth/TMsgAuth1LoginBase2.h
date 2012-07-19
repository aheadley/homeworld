#ifndef _TMsgAuth1LoginBase2_H
#define _TMsgAuth1LoginBase2_H

// TMsgAuth1LoginBase2.h


#include "STRING"
#include "common/won.h"
#include "msg/TMessage.h"

// Forwards from WONSocket
namespace WONMsg {

// TMsgAuth1LoginBase2 - Auth Login base class
//   Key Block ID        (2 bytes)  Must match key block Id in encrypted block
//   Session Block Length(2 bytes)
//   Session Key Block   (variable) Session key Encrypted with AuthPublicKey
//   Data Block          (variable) encrypted with Session Key
class TMsgAuth1LoginBase2 : public TMessage
{
public:

    // Default ctor
    TMsgAuth1LoginBase2();

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgAuth1LoginBase2(const TMessage& theMsgR);

    // Copy ctor
    TMsgAuth1LoginBase2(const TMsgAuth1LoginBase2& theMsgR);

    // Destructor
    ~TMsgAuth1LoginBase2()=0;

    // Assignment
    TMsgAuth1LoginBase2& operator=(const TMsgAuth1LoginBase2& theMsgR);

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack();
    void  Unpack();

    // KeyBlock ID access
    unsigned short GetKeyBlockId() const;
    void           SetKeyBlockId(unsigned short theId);

    void ForceRawBufOwn();

    // Session Key Buffer access
    const unsigned char* GetRawKeyBuf() const;
    unsigned short       GetRawKeyBufLen() const;

    // Data Buffer access
    const unsigned char* GetRawDataBuf() const;
    unsigned short       GetRawDataBufLen() const;

    // Update buffer.  Setting copyBuf to false will cause the specified
    // theBlockP pointer to be stored without copying its contents.  This will
    // improve performance, but theBlockP MUST NOT BE DEALLOCATED while in use
    // by this class.
    void SetRawKeyBuf(const unsigned char* theRawP, unsigned short theLen,
                   bool copyBuf=false);

    void SetRawDataBuf(const unsigned char* theRawP, unsigned short theLen,
                   bool copyBuf=false);

private:

    unsigned short mKeyBlockId;     // Id of AuthServ pub key block used to encrypt

    WONCommon::RawBuffer mRawKey;
    const unsigned char *mRawKeyP;
    unsigned short mRawKeyLen;

    WONCommon::RawBuffer mDataBlock;
    const unsigned char *mRawP;
    unsigned short mRawLen;
};


inline unsigned short
TMsgAuth1LoginBase2::GetKeyBlockId() const
{ return mKeyBlockId; }

inline void
TMsgAuth1LoginBase2::SetKeyBlockId( unsigned short theId )
{ mKeyBlockId = theId; }

inline const unsigned char*
TMsgAuth1LoginBase2::GetRawKeyBuf() const
{ return mRawKeyP; }

inline unsigned short
TMsgAuth1LoginBase2::GetRawKeyBufLen() const
{ return mRawKeyLen; }

inline const unsigned char*
TMsgAuth1LoginBase2::GetRawDataBuf() const
{ return mRawP; }

inline unsigned short
TMsgAuth1LoginBase2::GetRawDataBufLen() const
{ return mRawLen; }

inline void
TMsgAuth1LoginBase2::ForceRawBufOwn(void)
{
    if ((mRawKeyP) && (mRawKeyP != mRawKey.data()))
        {  mRawKey.assign(mRawKeyP, mRawKeyLen);  mRawKeyP = mRawKey.data();  }
    if ((mRawP) && (mRawP != mDataBlock.data()))
        {  mDataBlock.assign(mRawP, mRawLen);  mRawP = mDataBlock.data();  }
}

};  // Namespace WONMsg

#endif