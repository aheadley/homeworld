#ifndef _TMsgDirGetDirContents_H
#define _TMsgDirGetDirContents_H

// TMsgDirGetDirContents.h

// Directory Get Directory Contents Message class.  Fetches contents of a
// Directory from the Directory Server.

// Directory Get Directory Contents Reply Message class.  Contains data from
// the Directory Server is response to a GetDirContents request.


#include "STRING"
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "DirServerEntry.h"

// Forwards from WONSocket
namespace WONMsg {

class TMsgDirGetDirContents : public TMessage
{
public:
    // Default ctor
    TMsgDirGetDirContents(void);

    // TMessage ctor - will throw if TMessage type is not CommDebugLevel
    explicit TMsgDirGetDirContents(const TMessage& theMsgR);

    // Copy ctor
    TMsgDirGetDirContents(const TMsgDirGetDirContents& theMsgR);

    // Destructor
    ~TMsgDirGetDirContents(void);

    // Assignment
    TMsgDirGetDirContents& operator=(const TMsgDirGetDirContents& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack(void);
    void  Unpack(void);

    // DirectoryPath access
    const std::wstring& GetDirectoryPath(void) const;
    void                SetDirectoryPath(const std::wstring& thePath);

    // Recursive access
    bool GetRecursive() const;
    void SetRecursive(bool theFlag);

private:
    std::wstring mDirectoryPath;  // Path to get
    bool         mRecursive;      // Recursive request?
};


class TMsgDirGetDirContentsReply : public TMessage
{
public:
    // Default ctor
    TMsgDirGetDirContentsReply(void);

    // TMessage ctor - will throw if TMessage type is not of this type
    explicit TMsgDirGetDirContentsReply(const TMessage& theMsgR);

    // Copy ctor
    TMsgDirGetDirContentsReply(const TMsgDirGetDirContentsReply& theMsgR);

    // Destructor
    ~TMsgDirGetDirContentsReply(void);

    // Assignment
    TMsgDirGetDirContentsReply& operator=(const TMsgDirGetDirContentsReply& theMsgR);

    // Virtual Duplicate from TMessage
    TRawMsg* Duplicate(void) const;

    // Pack and Unpack the message
    // Unpack will throw a BadMsgException is message is not of this type
    void* Pack(void);
    void  Unpack(void);

    // Status access
    ServerStatus GetStatus() const;
    void         SetStatus(ServerStatus theStatus);

    // Entry access (const and non-const versions)
    const DirServerEntryVector& Entries() const;
    DirServerEntryVector&       Entries();

    // Get base size of messsage
    unsigned long GetBaseSize() const;

private:
    ServerStatus         mStatus;   // Status of the request
    DirServerEntryVector mEntries;  // Entries returned (may be empty)

    // Private methods
    void PackEntry(const DirServerEntry& theEntry);
    void UnpackEntry(DirServerEntry& theEntry);
};


// Inlines
inline TRawMsg*
TMsgDirGetDirContents::Duplicate(void) const
{ return new TMsgDirGetDirContents(*this); }

inline const std::wstring&
TMsgDirGetDirContents::GetDirectoryPath(void) const
{ return mDirectoryPath; }

inline void
TMsgDirGetDirContents::SetDirectoryPath(const std::wstring& thePath)
{ mDirectoryPath = thePath; }

inline bool
TMsgDirGetDirContents::GetRecursive(void) const
{ return mRecursive; }

inline void
TMsgDirGetDirContents::SetRecursive(bool theFlag)
{ mRecursive = theFlag; }

inline TRawMsg*
TMsgDirGetDirContentsReply::Duplicate(void) const
{ return new TMsgDirGetDirContentsReply(*this); }

inline ServerStatus
TMsgDirGetDirContentsReply::GetStatus() const
{ return mStatus; }

inline void
TMsgDirGetDirContentsReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline const DirServerEntryVector&
TMsgDirGetDirContentsReply::Entries() const
{ return mEntries; }

inline DirServerEntryVector&
TMsgDirGetDirContentsReply::Entries()
{ return mEntries; }


};  // Namespace WONMsg

#endif