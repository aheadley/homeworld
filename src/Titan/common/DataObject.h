#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include "won.h"
#include "LIST"
#include "SET"

namespace WONCommon {

// Forwards
struct DataObjectRep;  // Real data object (ref counted)


class DataObject
{
public:
    // Types
    typedef WONCommon::RawBuffer DataType;
    typedef WONCommon::RawBuffer Data;

    // Constructors
    DataObject();
    explicit DataObject(const DataType& theType);
    DataObject(const DataType& theType, const Data& theData);
    DataObject(const DataObject& theObjR);

    // Destructor
    virtual ~DataObject();

    // Operators
    DataObject& operator=(const DataObject& theObjR);
    bool operator==(const DataObject& theObjectR) const;
    bool operator!=(const DataObject& theObjectR) const;
    bool operator< (const DataObject& theObjectR) const;
    bool operator<=(const DataObject& theObjectR) const;
    bool operator> (const DataObject& theObjectR) const;
    bool operator>=(const DataObject& theObjectR) const;

    // Compare 2 DataObjects
    virtual int Compare(const DataObject& theObjectR, bool compareData=true) const;

    // Member access
    const DataType& GetDataType() const;
    DataType&       GetDataType();
    const Data&     GetData() const;
    Data&           GetData(bool copy=true);
    unsigned long   GetLifespan() const;

    // Has DataObject expired?
    bool IsExpired() const;

    // Member update
    void SetDataType(const DataType& theDataTypeR);
    void SetData(const Data& theDataR, bool copy=true);
    void SetLifespan(unsigned long theLifespan, time_t theStartTime=0, bool copy=true);

    // Compute raw size in bytes
    unsigned long ComputeSize() const;

protected:
    DataObjectRep* mRepP;

    // Dup representation and break ref link if needed
    virtual void CopyOnWrite();
};


// List, set, and multiset of DataObjects (keyed by DataType where applicable)
typedef std::list<DataObject> DataObjectList;
typedef std::set<DataObject> DataObjectTypeSet;
typedef std::multiset<DataObject> DataObjectTypeMultiSet;


// DataObject Representation
struct DataObjectRep
{
    long                 mRefCt;       // Reference count
    DataObject::DataType mDataType;    // Type portion (binary)
    DataObject::Data     mData;        // Data portion (binary)
    unsigned long        mLifespan;    // Lifespan in seconds
    time_t               mExpireTime;  // Time object expires

    DataObjectRep() :
        mRefCt(1), mDataType(), mData(), mLifespan(0), mExpireTime(0)
    {}
    explicit DataObjectRep(const DataObject::DataType& theType) :
        mRefCt(1), mDataType(theType), mData(), mLifespan(0), mExpireTime(0)
    {}
    DataObjectRep(const DataObject::DataType& theType, const DataObject::Data& theData) :
        mRefCt(1), mDataType(theType), mData(theData), mLifespan(0), mExpireTime(0)
    {}
    DataObjectRep(const DataObjectRep& theObjR) :
        mRefCt(1), mDataType(theObjR.mDataType), mData(theObjR.mData),
        mLifespan(theObjR.mLifespan), mExpireTime(theObjR.mExpireTime)
    {}

private:
    // Disable assignment
    DataObjectRep& operator=(const DataObjectRep& theObjR);
};


// Inlines

inline bool
DataObject::operator==(const DataObject& theObjectR) const
{ return (Compare(theObjectR, false) == 0); }

inline bool
DataObject::operator!=(const DataObject& theObjectR) const
{ return (Compare(theObjectR, false) != 0); }

inline bool
DataObject::operator<(const DataObject& theObjectR) const
{ return (Compare(theObjectR, false) < 0); }

inline bool
DataObject::operator<=(const DataObject& theObjectR) const
{ return (Compare(theObjectR, false) <= 0); }

inline bool
DataObject::operator>(const DataObject& theObjectR) const
{ return (Compare(theObjectR, false) > 0); }

inline bool
DataObject::operator>=(const DataObject& theObjectR) const
{ return (Compare(theObjectR, false) >= 0); }

inline const DataObject::DataType&
DataObject::GetDataType() const
{ return mRepP->mDataType; }

inline DataObject::DataType&
DataObject::GetDataType()
{ CopyOnWrite();  return mRepP->mDataType; }

inline const DataObject::Data&
DataObject::GetData() const
{ return mRepP->mData; }

inline DataObject::Data&
DataObject::GetData(bool copy)
{ if (copy) CopyOnWrite();  return mRepP->mData; }

inline unsigned long
DataObject::GetLifespan() const
{ return mRepP->mLifespan; }

inline bool
DataObject::IsExpired() const
{ return (time(NULL) >= mRepP->mExpireTime); }

inline void
DataObject::SetDataType(const DataObject::DataType& theDataTypeR)
{ CopyOnWrite();  mRepP->mDataType = theDataTypeR; }

inline void
DataObject::SetData(const DataObject::Data& theDataR, bool copy)
{ if (copy) CopyOnWrite();  mRepP->mData = theDataR; }

inline unsigned long
DataObject::ComputeSize() const
{ return (sizeof(unsigned char)  + mRepP->mDataType.size() +
          sizeof(unsigned short) + mRepP->mData.size()); }

}; // namespace WONCommon

#endif // DATAOBJECT_H