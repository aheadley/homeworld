#ifndef _WONExceptCodes_H
#define _WONExceptCodes_H

// WONExceptCodes

// Defines Exception codes and standard text for WON exceptions.

// To add new code(s), follow these steps:
// (1) Add the code to the ExceptionCodes enum at the end of the list,
//     before ExceptionCodeMax.  All codes should begein with the letters 'Ex'.
//
// (2) Assign the code a unique number in the enum so it can be documented.
//
// (3) Add the standard text for the code to the LoadExceptionMap() method
//     defined below.  Use the form:
//       gExceptMap[code] = "Standard text.";


// Exception codes in WONEx namespace
namespace WONCommon
{

enum ExceptionCodes
{
	// System level codes, imply lack of resources or system screwup
	ExUndefined        = 0,   // No code
	ExMemory           = 1,   // Out of memory
	ExSoftwareFail     = 2,   // Software failure (programmer screwup)
	ExNoEventLog       = 3,   // Could not connect to event log
	ExBadOptionType    = 4,   // Requested option has wrong type
	ExEngineDied       = 5,   // Socket Engine terminated unexpectedly
	ExEngineFull       = 6,   // Socket Engine is at FULL capacity.  No more sockets can be added.
	ExFailWait		   = 7,   // A call to WaitForXXX has failed in error
	ExNoTryEnter       = 8,   // TryEnter called for a CritSec under Win95 build.
	ExCryptoLib        = 9,   // An exception was throw by the Crypto++ library.

	// Socket Level Exceptions
	SocketCodeMin      = 100,  // Socket min, don't use this one

	ExFailCreate       = 101,  // Fail to create socket
	ExFailListen       = 102,  // Fail listen on one or more ports
	ExFailBind         = 103,  // Socket bind has failed
	ExFailAccept       = 104,  // Socket accept has failed
	ExFailOverlappedIO = 105,  // Overlapped IO failed on socket, possible null buffer posted
	ExFailConnect      = 106,  // Socket connect has failed
	ExFailSend         = 107,  // Socket send has failed
	ExFailReceive      = 108,  // Socket receive has failed
	ExFailNetInit      = 109,  // Network init (WSAStartup) failed
	ExFailSelect       = 110,  // Socket select has failed
	ExFailEnum		   = 111,  // Could not enumerate network events
	ExFailIoctl		   = 112,  // ioctlsocket() failed
	ExFailSockOpt      = 113,  // Fail to fetch/set socket options

	// Server level exceptions start here
	ServerCodeMin      = 1000,  // Server min, don't use this one
	ExBadTitanMessage  = 1001,  // Received or processed bad/corrupt Titan Message
	                            // This implies that the sending process is broken.
	ExBadCommCommand   = 1002,  // Received or processed bad/corrupt CommCommand
	ExFailInit         = 1003,  // Server could not initialize
	ExCryptBadBlockLen = 1004,  // Bad block length for decryption
	ExCryptBlockToBig  = 1005,  // Block to big for encrypt/decrypt, or sign.
	ExCryptBadBlock    = 1006,  // Bad block encountered.
	ExEventBadDataType = 1007,  // Bad data type value passed in MMsgEventRecordEvent message
	ExDatabaseConnectionLost = 1008, // Connection to database lost
	
	ExceptionCodeMax  // Last code, Dont use this one
};

};  // namespace WONCommon


// This section is only to be included in WONException.cpp
// DO NOT DEFINE _WON_EXCEPTION_SOURCE in your module!
#ifdef _WON_EXCEPTION_SOURCE

static void
LoadExceptionMap(void)
{
	using WONCommon::ExceptionCodes;

	gExceptMap[ExUndefined]			=	"Undefined exception.";
	gExceptMap[ExMemory]			=	"Out of memory.";
	gExceptMap[ExSoftwareFail]		=	"Software failure.";
	gExceptMap[ExNoEventLog]		=	"Failed to connect to event log.";
	gExceptMap[ExBadOptionType]		=	"Requested option has wrong type.";
	gExceptMap[ExEngineDied]		=	"SocketEngine terminated unexpectedly.";
	gExceptMap[ExEngineFull]        =	"SocketEngine is at FULL capacity.  No more sockets can be added.";
	gExceptMap[ExFailWait]			=	"Wait for Object or Event has failed.";
	gExceptMap[ExNoTryEnter]		=	"TryEnter called for CriticalSection under Win95 build.";
	gExceptMap[ExCryptoLib]			=	"Crypto++ Library exception.";

	gExceptMap[ExFailCreate]		=	"Fail to create socket.";
	gExceptMap[ExFailListen]		=	"Failed to listen on one or more ports.";
	gExceptMap[ExFailOverlappedIO]	=	"Overlapped IO operation failed.";
	gExceptMap[ExFailAccept]		=	"Accept operation failed.";
	gExceptMap[ExFailBind]			=	"Bind operation failed.";
	gExceptMap[ExFailConnect]       =	"Connect operation failed.";
	gExceptMap[ExFailSend]	        =	"Send operation failed.";
	gExceptMap[ExFailReceive]       =	"Receive operation failed.";
	gExceptMap[ExFailNetInit]       =	"Network init (WSAStartup) failed.";
	gExceptMap[ExFailSelect]        =	"Select operation failed.";
	gExceptMap[ExFailEnum]			=	"Could not enumerate network events on socket";
	gExceptMap[ExFailIoctl]			=	"Ioctl operation failed.";
	gExceptMap[ExFailSockOpt]		=	"Fail to fetch/set socket option(s).";

	gExceptMap[ExBadTitanMessage]	=	"Bad/Corrupt Titan message encountered.";
	gExceptMap[ExBadCommCommand]	=	"Bad/Corrupt CommCommand object encountered.";
	gExceptMap[ExFailInit]			=	"Initialization of server has failed.";
	gExceptMap[ExCryptBadBlockLen]	=	"Decryption block length not divisible by block size.";
	gExceptMap[ExCryptBlockToBig]	=	"Block is to large for encryption/decryption/signature.";
	gExceptMap[ExCryptBadBlock]		=	"Bad block encountered.";
	gExceptMap[ExEventBadDataType]  =   "Bad data type in MMsgEventRecordEvent.";
	gExceptMap[ExDatabaseConnectionLost] =   "Connection to database lost.";
}

#endif  //_WON_EXCEPTION_SOURCE

#endif  // _WONExceptCodes_H