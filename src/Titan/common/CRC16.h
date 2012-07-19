#ifndef _H_CRC16
#define	_H_CRC16

// CRC16

// Class implements a 16 bit CRC on an arbitrary stream of bytes.

// *NOTE*
// Source taken from Dr. Brain CRC Source and then modified for integration into WON.
// Dr. Brain CRC16 Source:
//   ©1995 Bright Star Technology, Inc.  All Rights Reserved. 
//   U.S. and foreign patents pending.
//   Bright Star Technology, Inc. is A wholly-owned subsidiary of Sierra On-Line, Inc.


#include "common/won.h"

namespace WONCommon {

class CRC16
{
public:

	explicit CRC16(unsigned short theInitValue=0, unsigned short theXOROutValue=0);
	CRC16(const CRC16& theCRCR);
	~CRC16();

	// Assignment operator
	CRC16& operator=(const CRC16& theCRCR);

	// Reset for new CRC calc
	void Reset();

	// Get current CRC
	unsigned short GetCRC();

	// Validate an existing CRC with this one
	bool ValidateCRC(unsigned short theCRC);

	// Build a CRC from data.  Can call Put() reapeatably to build a CRC
	void Put(const unsigned char* theDataP, unsigned int theLen);
	void Put(unsigned char theData);
	void Put(unsigned long theData);
	void Put(unsigned short theData);
	void Put(const char* theStrP);
	void Put(const std::string& theStrR);
	void Put(const std::wstring& theStrR);
	void Put(const WONCommon::RawBuffer& theDataR);

private:
	unsigned short mRegister;
	unsigned short mInitValue;
	unsigned short mXOROutValue;

	void ProcessBytes(const unsigned char* theBytesP, unsigned long theNumBytes);
};


// Inlines

inline void
CRC16::Reset()
{ mRegister = mInitValue; }

inline unsigned short
CRC16::GetCRC()
{ return (mRegister ^ mXOROutValue); }

inline void
CRC16::Put(unsigned char theData)
{ Put(reinterpret_cast<const unsigned char*>(&theData), sizeof(theData)); }

inline void
CRC16::Put(unsigned long theData)
{ Put(reinterpret_cast<const unsigned char*>(&theData), sizeof(theData)); }

inline void
CRC16::Put(unsigned short theData)
{ Put(reinterpret_cast<const unsigned char*>(&theData), sizeof(theData)); }

inline void
CRC16::Put(const char* theStrP)
{ Put(reinterpret_cast<const unsigned char*>(theStrP), strlen(theStrP)); }

inline void
CRC16::Put(const std::string& theStrR)
{ Put(reinterpret_cast<const unsigned char*>(theStrR.data()), theStrR.size()); }

inline void
CRC16::Put(const std::wstring& theStrR)
{ Put(reinterpret_cast<const unsigned char*>(theStrR.data()), (theStrR.size() * sizeof(unsigned short))); }

inline void
CRC16::Put(const WONCommon::RawBuffer& theDataR)
{ Put(theDataR.data(), theDataR.size()); }

inline bool
CRC16::ValidateCRC(unsigned short theCRC)
{ return (GetCRC() == theCRC); }


};  // Namespace WONCommon

#endif // _H_CRC16
