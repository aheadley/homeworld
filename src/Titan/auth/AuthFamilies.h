#ifndef _AuthFamilies_H_
#define _AuthFamilies_H_

//
// Authentication Families
// This enum defines constants for each Authentication family.  An Authentication
// family includes an Auth Certifcate type and a Auth Key Block type.

// NEVER change an existing enum value and always add new values to
// the end!!!!


namespace WONAuth
{
	enum AuthFamilies
	{
		AuthFamily0 = 0,
		AuthFamily1 = 1,
		AuthFamily2 = 2,

		// last one, don't use
		AuthFamilyMax
	};
};

#endif