#ifndef _TMsgDirPeerDataBase_H
#define _TMsgDirPeerDataBase_H

// TMsgDirPeerDataBase.h

// Base class for directory server messages that contain peer data.  This
// class is not a directory server message itself.  It provides methods to
// pack and unpack a PeerData if needed.

// Note that this message does not implement Pack and Unpack as PeerData is
// always appended to the end of DirServer messages.  It only provides the
// hooks that derived classes may call in their Pack/Unpack methods.


#include "STRING"
#include "msg/TMessage.h"

namespace WONMsg {

class TMsgDirPeerDataBase : public TMessage
{
public:
    // Default ctor
    TMsgDirPeerDataBase(void);

    // TMessage ctor
    explicit TMsgDirPeerDataBase(const TMessage& theMsgR);

    // Copy ctor
    TMsgDirPeerDataBase(const TMsgDirPeerDataBase& theMsgR);

    // Destructor
    virtual ~TMsgDirPeerDataBase(void);

    // Assignment
    TMsgDirPeerDataBase& operator=(const TMsgDirPeerDataBase& theMsgR);

    // Virtual Duplicate from TMessage
    virtual TRawMsg* Duplicate(void) const;

    // Is Peer Data defined?
    bool PeerDataDefined() const;

    // Member access
    const std::string& GetPeerKey  (void) const;
    unsigned long      GetPeerIndex(void) const;
    void               SetPeerKey  (const std::string& theKey);
    void               SetPeerIndex(unsigned long theIndex);

protected:
    std::string   mPeerKey;
    unsigned long mPeerIndex;

    // Peer Data pack/unpack methods
    void PackPeerData();
    void UnpackPeerData();
};


// Inlines
inline TRawMsg*
TMsgDirPeerDataBase::Duplicate(void) const
{ return new TMsgDirPeerDataBase(*this); }

inline bool
TMsgDirPeerDataBase::PeerDataDefined(void) const
{ return (! mPeerKey.empty()); }

inline const std::string&
TMsgDirPeerDataBase::GetPeerKey(void) const
{ return mPeerKey; }

inline void
TMsgDirPeerDataBase::SetPeerKey(const std::string& theKey)
{ mPeerKey = theKey; }

inline unsigned long
TMsgDirPeerDataBase::GetPeerIndex(void) const
{ return mPeerIndex; }

inline void
TMsgDirPeerDataBase::SetPeerIndex(unsigned long theIndex)
{ mPeerIndex = theIndex; }

};  // Namespace WONMsg

#endif