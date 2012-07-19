#ifndef _TMsgTypesAuth_H_
#define _TMsgTypesAuth_H_

//
// Titan Auth Server message types
// These values must be fixed in order to communiucate message
// structures to client.  For this reason, each enum value except
// for 'max' must have an assigned value.

// NEVER change an existing enum value and always add new values to
// the end!!!!


namespace WONMsg
{
	enum MsgTypeAuth
	{
		// Auth Server messages
		AuthMin             = 0,

        // AuthServer
        Auth1GetPubKeys       = 01, // new std
        Auth1GetPubKeysReply  = 02, // new std

        Auth1LoginRequest2    = 03, // new std
        Auth1LoginReply       = 04, // new std

        Auth1LoginRequest     = 05, // new std ( should migrate to Auth1LoginRequest2 )

        Auth1GetPubKeys23       = 10, // crypt++2.3 compatibility
        Auth1GetPubKeysReply23  = 11, // crypt++2.3 compatibility


        Auth1RefreshRequest   = 12, // old, in use
        Auth1RefreshReply     = 13, // old, in use
        Auth1LoginRequest23   = 14, // old, in use ( temporary crypt++2.3 compatibility )
        Auth1NewLoginRequest  = 15, // to Server (unused?)

        Auth1LoginReplyHL23     = 16, // HL crypt++2.3 compatibility

        Auth1ChangePasswordRequest = 17, // to Server (unused?)

        Auth1LoginRequestHL23   = 19, // HL crypt++2.3 compatibility
        Auth1LoginChallengeHL23 = 20, // HL crypt++2.3 compatibility
        Auth1LoginConfirmHL23   = 21, // HL crypt++2.3 compatibility
        Auth1RefreshHL23        = 22, // HL crypt++2.3 compatibility

        Auth1LoginReply23       = 23, // WONSwap crypt++2.3 compatibility
        Auth1LoginRequest223    = 24, // WONSwap crypt++2.3 compatibility

        Auth1CheckHLKey         = 25, // Cust support (rev the code)

        // Homeworld login messages
        Auth1LoginRequestHW   = 30,
        Auth1RefreshHW        = 31,
		Auth1LoginChallengeHW = 32,
		Auth1LoginConfirmHW   = 33,

		// Halflife login messages
        Auth1LoginRequestHL   = 40, // new HL
        Auth1LoginChallengeHL = 41, // new HL
        Auth1LoginConfirmHL   = 42, // new HL
        Auth1RefreshHL        = 43, // new HL
        Auth1LoginReplyHL     = 44, // new HL

        // Client/Client
        Auth1Request          = 50, // to Server
        Auth1Challenge1       = 51, // to Client
        Auth1Challenge2       = 52, // to Server
        Auth1Complete         = 53, // to Client

		// Last Message type.  Don't use
		AuthMax
	};

};

#endif
