#ifndef _AuthCertificateBase_H
#define _AuthCertificateBase_H

// AuthCertificateBase

// Abstract base class for WON authentication certificates.  A place holder currently
// as it ads no functionality to AuthFamilyBuffer.  It is implemented to allow any
// common certificate functionality to be added later.


#include "AuthFamilyBuffer.h"


// In the WONAuth namespace
namespace WONAuth {


class AuthCertificateBase : public AuthFamilyBuffer
{
public:
	// Default constructor
	AuthCertificateBase();

	// Copy Constructor
	AuthCertificateBase(const AuthCertificateBase& theCertR);

	// Destructor
	virtual ~AuthCertificateBase();

	// Operators
	AuthCertificateBase& operator=(const AuthCertificateBase& theCertR);

protected:

private:
};


// Inlines
inline
AuthCertificateBase::AuthCertificateBase() :
	AuthFamilyBuffer()
{}

inline
AuthCertificateBase::AuthCertificateBase(const AuthCertificateBase& theCertR) :
	AuthFamilyBuffer(theCertR)
{}

inline
AuthCertificateBase::~AuthCertificateBase()
{}

inline AuthCertificateBase&
AuthCertificateBase::operator=(const AuthCertificateBase& theCertR)
{ AuthFamilyBuffer::operator=(theCertR);  return *this; }


};  // Namespace WONAuth

#endif