#ifndef _HeaderTypes_H_
#define _HeaderTypes_H_

//
// Titan Header Types (APPLY TO ALL MESSAGE HEADER STYLES/CLASSES)
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!

namespace WONMsg
{
	enum HeaderType
	{
		Undefined              = 0,

		CommonService          = 1,
		EncryptedService       = 2, // encrypted Service4Message4 (aka TMessage)
		HeaderService1Message1 = 3, // non-encrypted MiniMessages
		MiniEncryptedService   = 4, // encrypted Service1Message1 (aka MiniMessage)
		HeaderService2Message2 = 5, // non-encrypted SmallMessages
		SmallEncryptedService  = 6, // encrypted Service2Message2 (aka SmallMessage)
		HeaderService4Message4 = 7, // non-encrypted LargeMessages
		LargeEncryptedService  = 8, // encrypted Service4Message4 (aka LargeMessage)

		// Old-Style service type used as header types
		FactoryServer          = 10,
		AuthServer             = 20,
		DirServer              = 30,
		ParamServer            = 40,
		ChatServer             = 50,
		SIGSAuthServer         = 60,
		PingServer             = 70,
		ObsServer	           = 80,
		OverlordServer         = 100,

		// Reserved range for Auth Login protocols (200-229)
        AuthProtocolMin        = 200,
        Auth0Login             = 200,  // Auth0 - Brain-dead
        Auth1Login             = 201,  // Auth1 - Standard Login
        Auth1LoginHL           = 202,  // Auth1 - Half-life Login
        Auth1PeerToPeer        = 203,  // Auth1 - Persistant connections
        AuthProtocolMax        = 229,

		// reserved, don't use
		Reserved1              = 253,
		Reserved2              = 254,
		Extended               = 255,
		MaxHeaderType          = 255
	};

	typedef HeaderType ServiceType;
};

#endif


