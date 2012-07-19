#ifndef _TMsgDirChangeService_H
#define _TMsgDirChangeService_H

// TMsgDirChangeService.h

// DirectoryServer Change Entry message.  Defines an entry to change.


#include "STRING"
#include "msg/TMessage.h"
#include "TMsgDirServiceBase.h"

namespace WONMsg {

class TMsgDirChangeService : public TMsgDirServiceBase
{
public:
    // Default ctor
    TMsgDirChangeService(void);

    // TMessage ctor
    explicit TMsgDirChangeService(const TMessage& theMsgR);

    // Copy ctor
    TMsgDirChangeService(const TMsgDirChangeService& theMsgR);

    // Destructor
    ~TMsgDirChangeService(void);

    // Assignment
    TMsgDirChangeService& operator=(const TMsgDirChangeService& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack(void);
    void  Unpack(void);

    // Member access
    unsigned char       GetChangeMask(void) const;
    const std::wstring& GetNewName(void) const;
    const std::wstring& GetNewDisplayName(void) const;
    const std::wstring& GetNewVersion(void) const;
    const std::wstring& GetNewProtoName(void) const;
    const std::wstring& GetNewProtoVersion(void) const;
    const std::wstring& GetNewNetAddress(void) const;
    unsigned long       GetNewLifespan(void) const;
    unsigned short      GetNewBlobLen(void) const;
    const void*         GetNewBlob(void) const;

    void SetChangeMask(unsigned char theMask);
    void SetNewName(const std::wstring& theName);
    void SetNewDisplayName(const std::wstring& theName);
    void SetNewVersion(const std::wstring& theVersion);
    void SetNewProtoName(const std::wstring& theName);
    void SetNewProtoVersion(const std::wstring& theVersion);
    void SetNewNetAddress(const std::wstring& theAddr);
    void SetNewLifespan(unsigned long theLifespan);
    void SetNewBlob(const void* theBlob, unsigned short theLen);

private:
    unsigned char  mChangeMask;
    std::wstring   mNewName;
    std::wstring   mNewDisplayName;
    std::wstring   mNewVersion;
    std::wstring   mNewProtoName;
    std::wstring   mNewProtoVersion;
    std::wstring   mNewNetAddress;
    unsigned long  mNewLifespan;
    unsigned short mNewBlobLen;
    unsigned char* mNewBlob;
};


// Inlines
inline TRawMsg*
TMsgDirChangeService::Duplicate(void) const
{ return new TMsgDirChangeService(*this); }

inline unsigned char
TMsgDirChangeService::GetChangeMask(void) const
{ return mChangeMask; }

inline const std::wstring&
TMsgDirChangeService::GetNewName(void) const
{ return mNewName; }

inline const std::wstring&
TMsgDirChangeService::GetNewDisplayName(void) const
{ return mNewDisplayName; }

inline const std::wstring&
TMsgDirChangeService::GetNewVersion(void) const
{ return mNewVersion; }

inline const std::wstring&
TMsgDirChangeService::GetNewProtoName(void) const
{ return mNewProtoName; }

inline const std::wstring&
TMsgDirChangeService::GetNewProtoVersion(void) const
{ return mNewProtoVersion; }

inline const std::wstring&
TMsgDirChangeService::GetNewNetAddress(void) const
{ return mNewNetAddress; }

inline unsigned long
TMsgDirChangeService::GetNewLifespan(void) const
{ return mNewLifespan; }

inline unsigned short
TMsgDirChangeService::GetNewBlobLen(void) const
{ return mNewBlobLen; }

inline const void*
TMsgDirChangeService::GetNewBlob(void) const
{ return mNewBlob; }

inline void
TMsgDirChangeService::SetChangeMask(unsigned char theMask)
{ mChangeMask = theMask; }

inline void
TMsgDirChangeService::SetNewName(const std::wstring& theName)
{ mNewName = theName; }

inline void
TMsgDirChangeService::SetNewDisplayName(const std::wstring& theName)
{ mNewDisplayName = theName; }

inline void
TMsgDirChangeService::SetNewVersion(const std::wstring& theVersion)
{ mNewVersion = theVersion; }

inline void
TMsgDirChangeService::SetNewProtoName(const std::wstring& theName)
{ mNewProtoName = theName; }

inline void
TMsgDirChangeService::SetNewProtoVersion(const std::wstring& theVersion)
{ mNewProtoVersion = theVersion; }

inline void
TMsgDirChangeService::SetNewNetAddress(const std::wstring& theAddr)
{ mNewNetAddress = theAddr; }

inline void
TMsgDirChangeService::SetNewLifespan(unsigned long theLifespan)
{ mNewLifespan = theLifespan; }

};  // Namespace WONMsg

#endif