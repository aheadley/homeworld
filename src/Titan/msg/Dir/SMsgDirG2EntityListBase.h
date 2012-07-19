#ifndef _SMsgDirG2DirEntityBase_H
#define _SMsgDirG2DirEntityBase_H

// SMsgDirG2EntityListBase.h

// Base class derived from SMsgDirG2EntityBase that adds a list of DirEntity
// objects and a sequence/last reply byte.  Does not override Pack/Unpack.

// Adds a PackEntities() and UnpackEntities() methods to pack/unpack the list of
// entities.  These methods should be called be derived class Pack/Unpack.

// Adds a PackSequence() and UnpackSequence() methods tp pack/unpack the sequence
// and last reply flag.  These methods should be called be derived class Pack/Unpack.

// Adds ComputePackSize().  This method returns the size in bytes required to
// pack the current list of DirEntity objects and the sequence byte.

// Note: This class does not override the pure virtual Duplicate() and GetFlags()
// methods from SMsgDirG2EntityBase.  Derived classes must still override
// these methods.


#include <limits.h>
#include "common/DataObject.h"
#include "SMsgDirG2EntityBase.h"
#include "DirEntity.h"


namespace WONMsg {

class SMsgDirG2EntityListBase : public SMsgDirG2EntityBase
{
public:
	// Default ctor
	SMsgDirG2EntityListBase(void);

	// SmallMessage ctor
	explicit SMsgDirG2EntityListBase(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2EntityListBase(const SMsgDirG2EntityListBase& theMsgR);

	// Destructor
	virtual ~SMsgDirG2EntityListBase(void);

	// Assignment
	SMsgDirG2EntityListBase& operator=(const SMsgDirG2EntityListBase& theMsgR);

	// Entity access (const and non-const versions)
	const DirEntityList& Entities() const;
	DirEntityList&       Entities();

	// Sequence access
	unsigned char GetSequence() const;
	void SetSequence(unsigned char theSeq);
	void IncSequence();

	// LastReply access
	bool IsLastReply() const;
	void SetLastReply(bool theFlag);

	// Get size in bytes for packing this message
	virtual unsigned long ComputePackSize() const;

protected:
	DirEntityList mEntities;   // Entities returned (may be empty)
	unsigned char mSequence;   // Reply sequence counter (0 based, 7 bits)
	bool          mLastReply;  // Last reply flag (packed into 8th bit of sequence)

	// Pack entities into raw buffer (call in Pack())
	void PackEntities();

	// Unpack entities from raw buffer (call in Unpack())
	void UnpackEntities();

	// Pack sequence into raw buffer (call in Pack())
	void PackSequence();

	// Unpack sequence from raw buffer (call in Unpack())
	void UnpackSequence();


private:
};


// Inlines
inline const DirEntityList&
SMsgDirG2EntityListBase::Entities() const
{ return mEntities; }

inline DirEntityList&
SMsgDirG2EntityListBase::Entities()
{ return mEntities; }

inline unsigned char
SMsgDirG2EntityListBase::GetSequence() const
{ return mSequence; }

inline void
SMsgDirG2EntityListBase::SetSequence(unsigned char theSeq)
{ mSequence = (theSeq > SCHAR_MAX ? 0 : theSeq); }

inline void
SMsgDirG2EntityListBase::IncSequence()
{ if (++mSequence > SCHAR_MAX) mSequence = 0; } 

inline bool
SMsgDirG2EntityListBase::IsLastReply() const
{ return mLastReply; }

inline void
SMsgDirG2EntityListBase::SetLastReply(bool theFlag)
{ mLastReply = theFlag; }

};  // Namespace WONMsg

#endif