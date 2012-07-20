#ifndef _ServerStatus_H_
#define _ServerStatus_H_

//
// ServerStatus.h
// Defines the status codes returned in the status field of reply messages
// from the all of the Titan servers.


#ifdef __cplusplus
namespace WONMsg
{
#endif

	enum ServerStatus
	{

		//////////////////////////////////////////////////////////////////////////////////
		//
		// Success status codes
		//
		//////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////////////
		// Common status codes
		//////////////////////////////////////////////////////////////////////////////////

		StatusSuccessCommonStart			= 0,
		StatusCommon_Success				= StatusSuccessCommonStart,		// 0 Request succeeded with not result code
		StatusCommon_ReplyTruncated			= StatusSuccessCommonStart+1,	// 1 Reply was truncated
		StatusCommon_StartSuccess			= StatusSuccessCommonStart+2,	// 2 StartProcess success
		StatusSuccessCommonEnd				= 999,

		//////////////////////////////////////////////////////////////////////////////////
		// Server specific status codes
		//////////////////////////////////////////////////////////////////////////////////

		StatusSpecificSuccessStart			= 1000,

		// This is reserved for game clients who want to use our status codes. 
		// There codes will be allowed to overlap.
		// unused 1000 - 1099

		// Chat Server
		StatusSuccessChatStart				= 1100,
		StatusSuccessChatEnd				= 1199,
		
		// Directory Server	
		StatusSuccessDirStart				= 1200,
		StatusSuccessDirEnd					= 1299,

		// Parameter Server
		StatusParamSuccessStart				= 1300,
		StatusParamSuccessEnd				= 1399,

		// Factory Server
		StatusFactSuccessStart				= 1400,
		StatusFactSuccessEnd				= 1499,

		// Authentication Server
		StatusAuthSuccessStart				= 1500,
		StatusAuthSuccessEnd				= 1599,

		// Observation Server
		StatusObsSuccessStart				= 1600,
		StatusObsSuccessEnd					= 1699,

		// SIGS Auth Server
		StatusSIGSAuthSuccessStart			= 1700,
		StatusSIGSAuthSuccessEnd			= 1799,

		// Ping Server
		StatusPingSuccessStart				= 1800,
		StatusPingSuccessEnd				= 1899,

		// Event Server
		StatusEventSuccessStart				= 1900,
		StatusEventSuccessEnd				= 1999,

		// Routing Server
		StatusRoutingSuccessStart			= 2000,
		StatusRoutingSuccessEnd				= 2099,

		StatusFirewallSuccessStart			= 2100,
		StatusFirewallSuccessConnect		= StatusFirewallSuccessStart,
		StatusFirewallSuccessEnd			= 2199,

		// Next Server 2100 - 2199

		// SHORTMAX rounded down to nearest hundred
		StatusSpecificSuccessEnd			= 32700,


		//////////////////////////////////////////////////////////////////////////////////
		//
		// Failure status codes
		//
		//////////////////////////////////////////////////////////////////////////////////

		
		//////////////////////////////////////////////////////////////////////////////////
		// Common status codes
		//////////////////////////////////////////////////////////////////////////////////

		StatusCommonFailureStart			= -1, 
		StatusCommon_Failure				= StatusCommonFailureStart,		// -1 General Failure no status
		StatusCommon_NotAvailable			= StatusCommonFailureStart-1,   // -2 Server not available
		StatusCommon_NotAllowed				= StatusCommonFailureStart-2,   // -3 Request not allowed
		StatusCommon_InvalidParameters		= StatusCommonFailureStart-3,   // -4 Request had invalid parameters
		StatusCommon_AlreadyExists			= StatusCommonFailureStart-4,   // -5 Item being added already exists
		StatusCommon_MessageExceedsMax		= StatusCommonFailureStart-5,   // -6 Maximum message is exceeded
		StatusCommon_InvalidMessage			= StatusCommonFailureStart-6,	// -7 Message is invalid
		StatusCommon_NotAuthorized			= StatusCommonFailureStart-7,	// -8 Server/Client is not Authorized to perform this action
		StatusCommon_ResourcesLow			= StatusCommonFailureStart-8,	// -9 Unable to create resources are low
		StatusCommon_NoPortAvailable		= StatusCommonFailureStart-9,	// -10 No port in range was available
		StatusCommon_TimedOut				= StatusCommonFailureStart-10,	// -11 A request has timed out
		StatusCommon_NotSupported			= StatusCommonFailureStart-11,	// -12 A request is not supported
		StatusCommon_DecryptionFailed       = StatusCommonFailureStart-12,  // -13 Decryption of a message failed
		StatusCommon_InvalidSessionId       = StatusCommonFailureStart-13,  // -14 Bad session id in encrypted message header
		StatusCommon_InvalidSequenceNumber  = StatusCommonFailureStart-14,  // -15 Bad sequence number in encrypted message
		StatusCommonFailureEnd				= -999,

		//////////////////////////////////////////////////////////////////////////////////
		// Server specific status codes
		//////////////////////////////////////////////////////////////////////////////////

		StatusSpecificFailureStart			= -1000,

		// This is reserved for game clients who want to use our status codes. 
		// There codes will be allowed to overlap.
		// unused -1000 - -1099

		// Chat Server
		StatusFailureChatStart				= -1100,
		StatusChat_DuplicateNameNotAllowed	= StatusFailureChatStart,	// -1100
		StatusChat_LurkerNotAllowed			= StatusFailureChatStart-1,	// -1101
		StatusChat_LurkerOnlyAllowed		= StatusFailureChatStart-2,	// -1102
		StatusChat_UserNotAuthenticated		= StatusFailureChatStart-3,	// -1103
		StatusChat_PresenceNotAllowed		= StatusFailureChatStart-4,	// -1104
		StatusChat_UserNotAllowed			= StatusFailureChatStart-5,	// -1105
		StatusFailureChatEnd				= -1199,
		
		// Directory Server
		StatusFailureDirStart				= -1200,
		StatusDir_DirNotFound				= StatusFailureDirStart,		// -1200 Requested directory not found
		StatusDir_ServiceNotFound			= StatusFailureDirStart-1,		// -1201 Requested service not found
		StatusDir_DirExists					= StatusFailureDirStart-2,		// -1202 Directory already exists
		StatusDir_ServiceExists				= StatusFailureDirStart-3,		// -1203 Service alread exists
		StatusDir_DirIsFull					= StatusFailureDirStart-4,		// -1204 Directory is full
		StatusDir_AlreadyConnected			= StatusFailureDirStart-5,		// -1205 Peer already connected
		StatusDir_EntityTooLarge			= StatusFailureDirStart-6,		// -1206 Entity size (bytes) too large
		StatusDir_MaxDataObjects			= StatusFailureDirStart-7,		// -1207 Max data object for entity exceeded
		StatusDir_BadDataOffset				= StatusFailureDirStart-8,		// -1208 Offset exceeds length of data in data object
		StatusDir_InvalidPath				= StatusFailureDirStart-9,		// -1209 Invalid path
		StatusDir_InvalidGetFlags			= StatusFailureDirStart-10,		// -1210 Invalid GetFlags
		StatusDir_InvalidKey				= StatusFailureDirStart-11,		// -1211 Invalid directory or service key
		StatusDir_InvalidMode				= StatusFailureDirStart-12,		// -1212 Invalid GetNumEntries mode
		StatusDir_InvalidLifespan			= StatusFailureDirStart-13,		// -1213 Invalid lifespan
		StatusDir_InvalidDataObject			= StatusFailureDirStart-14,		// -1214 Invalid data object
		StatusDir_NoDataObjects				= StatusFailureDirStart-15,		// -1215 No data objects when at least one required
		StatusDir_DataObjectExists			= StatusFailureDirStart-16,		// -1216 Data object already exists.
		StatusDir_DataObjectNotFound		= StatusFailureDirStart-17,		// -1217 Data object does not exist.
		StatusFailureDirEnd					= -1299,


		// Parameter Server
		StatusParamFailureStart				= -1300,
		StatusParam_DBFailure				= StatusParamFailureStart,		// -1300	
		StatusParam_NoAddChange				= StatusParamFailureStart-1,	// -1301
		StatusParam_LogicalNotExist			= StatusParamFailureStart-2,	// -1302
		StatusParam_LogicalAlreadyIs		= StatusParamFailureStart-3,	// -1303
		StatusParam_LogicalTooLong			= StatusParamFailureStart-4,	// -1304
		StatusParam_LogicalRequired			= StatusParamFailureStart-5,	// -1305
		StatusParam_ParameterNotExist		= StatusParamFailureStart-6,	// -1306
		StatusParam_ParameterAlreadyIs		= StatusParamFailureStart-7,	// -1307
		StatusParam_MismatchNumParm			= StatusParamFailureStart-8,	// -1308
		StatusParam_RequestedZero			= StatusParamFailureStart-9,	// -1309
		StatusParam_InvalidLifespan			= StatusParamFailureStart-10,	// -1310
		StatusParam_ServiceNotFound			= StatusParamFailureStart-11,	// -1311
		StatusParam_ServiceRequired			= StatusParamFailureStart-12,	// -1312
		StatusParam_BaseChangeNotAllowed	= StatusParamFailureStart-13,	// -1313
		StatusParam_ServiceAlreadyExists	= StatusParamFailureStart-14,	// -1314
		StatusParamFailureEnd				= -1399,

		// Factory Server
		StatusFactFailureStart				= -1400,
		StatusFact_FailedToRegister			= StatusFactFailureStart,		// -1400 List too large to send back in TMessage
		StatusFactFailureEnd				= -1499,

		// Authentication Server
		StatusAuthFailureStart				= -1500,
        StatusAuth_ExpiredKey               = StatusAuthFailureStart,
        StatusAuth_VerifyFailed             = StatusAuthFailureStart-1,
		StatusAuth_LockedOut                = StatusAuthFailureStart-2,
		StatusAuth_KeyInUse                 = StatusAuthFailureStart-3,
		StatusAuth_CRCFailed                = StatusAuthFailureStart-4,
        StatusAuth_UserExists               = StatusAuthFailureStart-5,
        StatusAuth_UserNotFound             = StatusAuthFailureStart-6,
        StatusAuth_BadPassword              = StatusAuthFailureStart-7,
        StatusAuth_DirtyWord                = StatusAuthFailureStart-8,
        StatusAuth_InvalidName              = StatusAuthFailureStart-9,
		StatusAuthFailureEnd				= -1599,

		// Observation Server
		StatusObsFailureStart				= -1600,
		StatusObs_UnknownServer				= StatusObsFailureStart,
		StatusObs_UnknownPublisher			= StatusObsFailureStart-1,
		StatusObs_UnknownSubscriber			= StatusObsFailureStart-2,
		StatusObs_UnknownPublication		= StatusObsFailureStart-3,
		StatusObs_UnknownSubscription		= StatusObsFailureStart-4,
		StatusObs_RedirectToPeer			= StatusObsFailureStart-5,
		StatusObs_AccessByNameNotSupported	= StatusObsFailureStart-6,
		StatusObsFailureEnd					= -1699,

		// SIGS Auth Server
		StatusSIGSAuthFailureStart          = -1700,           // Minimum value
		StatusSIGSAuthNoContact             = StatusSIGSAuthFailureStart,   // General success (authenticate, created, etc)
		StatusSIGSAuthInvalidInput          = StatusSIGSAuthFailureStart-1, // General Failure
		StatusSIGSAuthBannedUser            = StatusSIGSAuthFailureStart-2, // System cannot be contacted
		StatusSIGSAuthDirtyWord             = StatusSIGSAuthFailureStart-3, // Dirty word
		StatusSIGSAuthDuplicateUsername     = StatusSIGSAuthFailureStart-4, // Duplicate username
		StatusSIGSAuthFailureEnd          	= -1799,

		// Ping Server
		StatusPingFailureStart				= -1800,
		StatusPing_NoResponse				= StatusPingFailureStart,
		StatusPing_TooManyPings				= StatusPingFailureStart-1,
		StatusPing_TooMuchTime				= StatusPingFailureStart-2,
		StatusPing_TargetNonexistant        = StatusPingFailureStart-3,
		StatusPingFailureEnd				= -1899,

		// Event Server
		StatusEventFailureStart				= -1900,
		StatusEvent_UnknownError            = StatusEventFailureStart,
		StatusEvent_DatabaseFull            = StatusEventFailureStart-1, // the database to which the server is forwarding data is out of space
		StatusEvent_MissingConstant         = StatusEventFailureStart-2, // foreign key references non-existant parent key value
		StatusEvent_InvalidDateTime         = StatusEventFailureStart-3, // date time passed in RecordEvent message was invalid
		StatusEventFailureEnd				= -1999,

		// Routing Server
		StatusRoutingFailureStart		    	= -2000,
		StatusRouting_ClientAlreadyExists       = StatusRoutingFailureStart,
		StatusRouting_ClientAlreadyRegistered   = StatusRoutingFailureStart-1,
//		StatusRouting_ClientBanned              = StatusRoutingFailureStart-2,
		StatusRouting_ClientDoesNotExist        = StatusRoutingFailureStart-3,
		StatusRouting_GroupAlreadyExists        = StatusRoutingFailureStart-4,
		StatusRouting_GroupDoesNotExist         = StatusRoutingFailureStart-5,
		StatusRouting_HostRefused               = StatusRoutingFailureStart-6,
		StatusRouting_HostResponseTimeout       = StatusRoutingFailureStart-7,
		StatusRouting_InvalidPassword           = StatusRoutingFailureStart-8,
		StatusRouting_MustBeClient              = StatusRoutingFailureStart-9,
		StatusRouting_MustBeHost                = StatusRoutingFailureStart-10,
		StatusRouting_MustBeHostOrSuccessor     = StatusRoutingFailureStart-11,
		StatusRouting_NotValidOnHost            = StatusRoutingFailureStart-12,
		StatusRouting_ObjectAlreadyExists       = StatusRoutingFailureStart-13,
		StatusRouting_ObjectDoesNotExist        = StatusRoutingFailureStart-14,
		StatusRouting_ObjectTooLarge            = StatusRoutingFailureStart-15,
		StatusRouting_ReconnectImpossible       = StatusRoutingFailureStart-16,
		StatusRouting_RegistrationClosed        = StatusRoutingFailureStart-17,
		StatusRouting_RegistrationAlreadyClosed = StatusRoutingFailureStart-18,
		StatusRouting_RegistrationAlreadyOpen   = StatusRoutingFailureStart-19,
		StatusRouting_ServerFull                = StatusRoutingFailureStart-20,
		StatusRouting_SubscriptionAlreadyExists = StatusRoutingFailureStart-21,
		StatusRouting_SubscriptionDoesNotExist  = StatusRoutingFailureStart-22,
		StatusRouting_UserAlreadyExists         = StatusRoutingFailureStart-23,
		StatusRouting_UserBanned                = StatusRoutingFailureStart-24,
		StatusRouting_UserDoesNotExist          = StatusRoutingFailureStart-25,
		StatusRouting_UserAlreadyBanned         = StatusRoutingFailureStart-26,
		StatusRouting_UserNotBanned             = StatusRoutingFailureStart-27,
		StatusRouting_UserAlreadyInvited        = StatusRoutingFailureStart-28,
		StatusRouting_UserNotInvited            = StatusRoutingFailureStart-29,
		StatusRouting_OffsetTooLarge            = StatusRoutingFailureStart-30,
		StatusRouting_ClientRegistrationPending = StatusRoutingFailureStart-31,
		StatusRouting_HostAlreadyExists         = StatusRoutingFailureStart-32,
		StatusRouting_HostDoesNotExist          = StatusRoutingFailureStart-33,
		StatusRouting_NotEligible               = StatusRoutingFailureStart-34,
		StatusRoutingFailureEnd			    	= -2099,

		// Next -2100 - -2199

		// SHORTMIN rounded up to nearest hundred
		StatusSpecificFailureEnd			= -32700
	};

#ifdef __cplusplus
};
#endif

#endif