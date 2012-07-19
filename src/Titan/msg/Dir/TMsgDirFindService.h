#ifndef _TMsgDirFindService_H
#define _TMsgDirFindService_H

// TMsgDirFindService.h

// Directory Find Service Message class.  Locates a service is a specified
// directory.  Service may have a parital key specified and search may be
// recursive or not.

// Directory Find Service Reply Message class.  Contains data from the
// Directory Server in response to a find service request.


#include "STRING"
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "DirServerEntry.h"
#include "TMsgDirServiceBase.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgDirFindService : public TMsgDirServiceBase
{
public:
    // Default ctor
    TMsgDirFindService(void);

    // TMessage ctor - will throw if TMessage type is not CommDebugLevel
    explicit TMsgDirFindService(const TMessage& theMsgR);

    // Copy ctor
    TMsgDirFindService(const TMsgDirFindService& theMsgR);

    // Destructor
    ~TMsgDirFindService(void);

    // Assignment
    TMsgDirFindService& operator=(const TMsgDirFindService& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack(void);
    void  Unpack(void);

    // FindMask access
    unsigned char GetFindMask(void) const;
    void          SetFindMask(unsigned char theMask);

    // Recursive access
    bool GetRecursive() const;
    void SetRecursive(bool theFlag);

private:
    unsigned char mFindMask;   // Service find mask (SK_MASK values 'or'ed)
    bool          mRecursive;  // Recursive request?
};


class TMsgDirFindServiceReply : public TMessage
{
public:
    // Default ctor
    TMsgDirFindServiceReply(void);

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgDirFindServiceReply(const TMessage& theMsgR);

    // Copy ctor
    TMsgDirFindServiceReply(const TMsgDirFindServiceReply& theMsgR);

    // Destructor
    ~TMsgDirFindServiceReply(void);

    // Assignment
    TMsgDirFindServiceReply& operator=(const TMsgDirFindServiceReply& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException if message is not of this type
    void* Pack(void);
    void  Unpack(void);

    // Status access
    ServerStatus GetStatus() const;
    void         SetStatus(ServerStatus theStatus);

    // Service access (const and non-const versions)
    const DirServerEntry& Service() const;
    DirServerEntry&       Service();

private:
    ServerStatus   mStatus;   // Status of the request
    DirServerEntry mService;  // Service returned (may be empty)
};


// Inlines
inline TRawMsg*
TMsgDirFindService::Duplicate(void) const
{ return new TMsgDirFindService(*this); }

inline unsigned char
TMsgDirFindService::GetFindMask(void) const
{ return mFindMask; }

inline void
TMsgDirFindService::SetFindMask(unsigned char theMask)
{ mFindMask = theMask; }

inline bool
TMsgDirFindService::GetRecursive(void) const
{ return mRecursive; }

inline void
TMsgDirFindService::SetRecursive(bool theFlag)
{ mRecursive = theFlag; }

inline TRawMsg*
TMsgDirFindServiceReply::Duplicate(void) const
{ return new TMsgDirFindServiceReply(*this); }

inline ServerStatus
TMsgDirFindServiceReply::GetStatus() const
{ return mStatus; }

inline void
TMsgDirFindServiceReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline const DirServerEntry&
TMsgDirFindServiceReply::Service() const
{ return mService; }

inline DirServerEntry&
TMsgDirFindServiceReply::Service()
{ return mService; }


};  // Namespace WONMsg

#endif