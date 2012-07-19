#ifndef CRYPTFLAGS_H
#define CRYPTFLAGS_H

namespace WONAuth {

	// Authentication modes (for AuthMode field)
	enum AuthenticationMode {
		AUTH_PERSISTENT = 1,  // Persistent (TCP) mode
		AUTH_SESSION    = 2,  // Session based (UDP, Lightweight TCP) mode
		AUTH_MAX              // Max mode value (not used)
	};

	// Encryption Mode (for EncryptMode field)
	enum EncryptionMode {
		ENCRYPT_NONE     = 0,  // No encryption (Encrypt flags ignored)
		ENCRYPT_BLOWFISH = 1,  // Use Blowfish encryption
		ENCRYPT_MAX            // Max mode value (not used)
	};

	// Encryption flags ('or'ed into EncryptFlags field)
	enum EncryptionFlags {
		EFLAGS_NONE         = 0x0000,  // No flags
		EFLAGS_NOTSEQUENCED = 0x0001,  // Use sequence numbers
		EFLAGS_ALLOWCLEAR   = 0x0002,  // Allow clear messages
		EFLAGS_CLIENTKEY    = 0x8000,  // Use client specified session key
		EFLAGS_ALL          = 0xffff   // Use all flags
	};

}; // namespace WONAuth

#endif // CRYPTFLAGS_H
