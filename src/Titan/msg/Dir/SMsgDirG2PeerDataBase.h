#ifndef _SMsgDirG2PeerDataBase_H
#define _SMsgDirG2PeerDataBase_H

// SMsgDirG2PeerDataBase.h

// Base class for directory server messages that contain peer data.  This
// class is not a directory server message itself.  It provides methods to
// pack and unpack a PeerData if needed.

// Note that this message does not implement Pack and Unpack as PeerData is
// always appended to the end of DirServer messages.  It only provides the
// hooks that derived classes may call in their Pack/Unpack methods.


#include "STRING"
#include "msg/TMessage.h"

namespace WONMsg {

class SMsgDirG2PeerDataBase : public SmallMessage
{
public:
    // Default ctor
    SMsgDirG2PeerDataBase(void);

    // SmallMessage ctor
    explicit SMsgDirG2PeerDataBase(const SmallMessage& theMsgR);

    // Copy ctor
    SMsgDirG2PeerDataBase(const SMsgDirG2PeerDataBase& theMsgR);

    // Destructor
    virtual ~SMsgDirG2PeerDataBase(void);

    // Assignment
    SMsgDirG2PeerDataBase& operator=(const SMsgDirG2PeerDataBase& theMsgR);

    // Virtual Duplicate from SmallMessage
    // Virtual Duplicate from SmallMessage
    // Pure virtual - must be overridden!
    virtual TRawMsg* Duplicate(void) const = 0;

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
inline bool
SMsgDirG2PeerDataBase::PeerDataDefined(void) const
{ return (! mPeerKey.empty()); }

inline const std::string&
SMsgDirG2PeerDataBase::GetPeerKey(void) const
{ return mPeerKey; }

inline void
SMsgDirG2PeerDataBase::SetPeerKey(const std::string& theKey)
{ mPeerKey = theKey; }

inline unsigned long
SMsgDirG2PeerDataBase::GetPeerIndex(void) const
{ return mPeerIndex; }

inline void
SMsgDirG2PeerDataBase::SetPeerIndex(unsigned long theIndex)
{ mPeerIndex = theIndex; }

};  // Namespace WONMsg

#endif