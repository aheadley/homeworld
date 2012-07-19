#ifndef _DirServerEntry_H
#define _DirServerEntry_H

// DirServerEntry

// Defines a Directory Server Entry component used in the DirContentsReply
// and PeerSynch messages of the Directory Server.

// Note that this class is implemented inline.


#include "STRING"
#include "LIST"

// In the WONMsg namespace
namespace WONMsg {

struct DirServerEntry
{
    // Constants
    enum {
        EntryTypeUnknown   = '\0',
        EntryTypeDirectory = 'D',
        EntryTypeService   = 'S'
    };

    // These members apply to all entries
    unsigned char  mType;          // Directory or Service?
    std::wstring   mPath;          // Path to parent dir
    std::wstring   mName;          // Name
    std::wstring   mDisplayName;   // Display name (may be null)
    long           mCreated;       // Create time as time_t
    long           mLifespan;      // Lifespan in seconds

    // These members are only defined for types EntryTypeDirectory
    bool           mVisible;       // Is visible?

    // These members are only defined for types EntryTypeService
    std::wstring   mVersion;       // Version (may be null)
    std::wstring   mProtoName;     // Protocol name (may be null)
    std::wstring   mProtoVersion;  // Protocol version (may be null)
    std::wstring   mNetAddress;    // Network address (IP:port)
    unsigned short mBlobLen;       // Length of blob (may be 0)
    unsigned char* mBlob;          // Blob (may be null)

    // Constructors / Destructor
    DirServerEntry();
    DirServerEntry(const DirServerEntry& theEntry);
    ~DirServerEntry();

    // Operators
    DirServerEntry& operator=(const DirServerEntry& theKey);

    // Fetch full path (path + name)
    const std::wstring GetFullPath() const;

    // Update blob and blobLen
    void SetBlob(const void* theData, unsigned short theLen);

    // Compute the size (in bytes) to add entry to a TMessage
    unsigned long ComputeSize() const;

    // Compute the size (in bytes) to add a wstring to a TMessage
    static unsigned long WStringSize(const wstring& theStr);
};

// Vector Def
// Changed to a list for efficiency.  Kept the name for compatibility
typedef std::list<DirServerEntry> DirServerEntryVector;


// Inlines
inline
DirServerEntry::DirServerEntry() :
    mType(DirServerEntry::EntryTypeUnknown),
    mPath(),
    mName(),
    mDisplayName(),
    mCreated(0),
    mLifespan(0),
    mVisible(true),
    mVersion(),
    mProtoName(),
    mProtoVersion(),
    mNetAddress(),
    mBlobLen(0),
    mBlob(NULL)
{}

inline
DirServerEntry::DirServerEntry(const DirServerEntry& theEntry) :
    mType(theEntry.mType),
    mPath(theEntry.mPath),
    mName(theEntry.mName),
    mDisplayName(theEntry.mDisplayName),
    mCreated(theEntry.mCreated),
    mLifespan(theEntry.mLifespan),
    mVisible(theEntry.mVisible),
    mVersion(theEntry.mVersion),
    mProtoName(theEntry.mProtoName),
    mProtoVersion(theEntry.mProtoVersion),
    mNetAddress(theEntry.mNetAddress),
    mBlobLen(0),
    mBlob(NULL)
{
    SetBlob(theEntry.mBlob, theEntry.mBlobLen);
}

inline
DirServerEntry::~DirServerEntry()
{ delete [] mBlob; }

inline DirServerEntry&
DirServerEntry::operator=(const DirServerEntry& theEntry)
{
    if (this != &theEntry)
    {
        mType         = theEntry.mType;
        mPath         = theEntry.mPath;
        mName         = theEntry.mName;
        mDisplayName  = theEntry.mDisplayName;
        mCreated      = theEntry.mCreated;
        mLifespan     = theEntry.mLifespan;
        mVisible      = theEntry.mVisible;
        mVersion      = theEntry.mVersion;
        mProtoName    = theEntry.mProtoName;
        mProtoVersion = theEntry.mProtoVersion;
        mNetAddress   = theEntry.mNetAddress;
        SetBlob(theEntry.mBlob, theEntry.mBlobLen);
    }
    return *this;
}

inline const std::wstring
DirServerEntry::GetFullPath() const
{
    wstring aRet(mPath);
    if ((! aRet.empty()) && (aRet[aRet.size()-1] != L'/'))
        aRet += L'/';
    aRet += mName;
    return aRet;
}

inline void
DirServerEntry::SetBlob(const void* theData, unsigned short theLen)
{
    delete mBlob;  mBlob = NULL;
    mBlobLen = theLen;
    if (mBlobLen > 0)
    {
        mBlob = new unsigned char [mBlobLen];
        memcpy(mBlob, theData, mBlobLen);
    }
}

inline unsigned long
DirServerEntry::WStringSize(const wstring& theStr)
{
    return (sizeof(unsigned short) + theStr.size() * sizeof(wchar_t));
}

inline unsigned long
DirServerEntry::ComputeSize() const
{
    unsigned long aSize = sizeof(mType) + WStringSize(mPath) +
                          WStringSize(mName) + WStringSize(mDisplayName) +
                          sizeof(mCreated) + sizeof(mLifespan);

    if (mType == EntryTypeDirectory)
        aSize += 1;  //mVisible stored as byte

    else if (mType == EntryTypeService)
        aSize += WStringSize(mVersion) + WStringSize(mProtoName) +
                 WStringSize(mProtoVersion) + WStringSize(mNetAddress) +
                 sizeof(mBlobLen) + mBlobLen;

    return aSize;
}


};  // Namespace WONMsg

#endif