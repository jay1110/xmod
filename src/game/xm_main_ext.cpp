#include <bgame/impl.h>
#include <game/g_local.h>
#include <game/xm_main_ext.h>
#include <game/jxac/jxac_server.h>
#include <bgame/xm_auth_shared.h>
#include <bgame/jxac_common.h>
#include <game/xmod_globals.h>
#include <game/Client.h>
#include <game/UserManager.h>

///////////////////////////////////////////////////////////////////////////////

namespace xmod {

///////////////////////////////////////////////////////////////////////////////

// Helper function to validate hexadecimal strings
static bool isValidHexString(const char* str, size_t expectedLen) {
	if (str == NULL) return false;
	if (strlen(str) != expectedLen) return false;
	for (size_t i = 0; i < expectedLen; i++) {
		char c = str[i];
		if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

// Called from ClientCommand BEFORE the ent->client check
// This is critical for authentication - the authenticate command arrives
// before the client is fully connected (ent->client may be NULL)
qboolean OnClientCommand(int clientNum, const char* cmd) {

	// Handle authenticate command - this is the xmodguid authentication
	if (Q_stricmp(cmd, "authenticate") == 0) {
		// Validate client number - reject invalid indices
		if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
			return qtrue; // Command handled (error case - invalid client)
		}

		gentity_t* ent = &g_entities[clientNum];
		
		// Skip authentication for bots - they don't run cgame and shouldn't send this
		if (ent->r.svFlags & SVF_BOT) {
			G_LogPrintf("[Auth] Client %d: authenticate command from bot - ignoring\n", clientNum);
			return qtrue;
		}

		char guid[64];
		char hwid[64];
		trap_Argv(1, guid, sizeof(guid));
		trap_Argv(2, hwid, sizeof(hwid));

		// Get client name if available
		const char* clientName = (ent->client) ? ent->client->pers.netname : "connecting";

		G_LogPrintf("[Auth] Client %d (%s): authenticate received\n", clientNum, clientName);

		// Validate GUID and HWID format (SHA1 hex = 40 chars)
		if (isValidHexString(guid, xm_auth::GUID_LENGTH) && isValidHexString(hwid, xm_auth::HWID_LENGTH)) {
			// Store in Client object
			Client& clientObject = g_clientObjects[clientNum];
			clientObject.authGuid = guid;
			clientObject.authHwid = hwid;
			clientObject.authenticated = true;
			clientObject.authWarningShown = false;

			G_LogPrintf("[Auth] Client %d (%s): authenticated successfully (GUID: %.8s...)\n",
				clientNum, clientName, guid);

			// Use xmod session system for database integration
			if (xmod::g_database && xmod::g_sessions[clientNum]) {
				xmod::g_sessions[clientNum]->onGuidReceived(guid, hwid);
			}

			// Update the legacy connectedUsers with the real GUID
			// This is needed for !setlevel and other admin commands
			if (connectedUsers[clientNum] && connectedUsers[clientNum] != &User::BAD) {
				std::string err;
				User& newUser = userManager.fetchByKey(guid, err, true);
				if (&newUser != &User::BAD) {
					User* oldUser = connectedUsers[clientNum];
					// Transfer session data from PENDING user to real user
					if (oldUser->guid.find("PENDING") == 0 || oldUser->fakeguid) {
						newUser.name = oldUser->name;
						newUser.namex = oldUser->namex;
						newUser.ip = oldUser->ip;
						newUser.mac = oldUser->mac;
						newUser.timestamp = time(NULL);
					}
					// Mark as real GUID (not fake)
					newUser.fakeguid = false;
					connectedUsers[clientNum] = &newUser;
					G_LogPrintf("[Auth] Client %d: Updated connectedUsers with GUID %.8s... (fakeguid=false)\n", clientNum, guid);
				} else {
					// If we can't create a new user entry, at least mark current user as not fake
					// so admin commands will work
					connectedUsers[clientNum]->fakeguid = false;
					G_LogPrintf("[Auth] Client %d: Marked existing user as authenticated (fakeguid=false)\n", clientNum);
				}
			}
		} else {
			G_LogPrintf("[Auth] Client %d (%s): authentication FAILED - invalid format (GUID len=%d, HWID len=%d)\n",
				clientNum, clientName, (int)strlen(guid), (int)strlen(hwid));
		}
		return qtrue;
	}
	
	// Handle JXAC heartbeat command
	if (Q_stricmp(cmd, "jxac_heartbeat") == 0) {
		jxac::Server::handleHeartbeat(clientNum);
		return qtrue;
	}
	
	// Handle JXAC module scan data
	if (Q_stricmp(cmd, "jxac_module") == 0) {
		char moduleName[256];
		char checksum[64];
		trap_Argv(1, moduleName, sizeof(moduleName));
		trap_Argv(2, checksum, sizeof(checksum));
		jxac::Server::checkModuleSignature(clientNum, moduleName, checksum);
		return qtrue;
	}
	
	// Handle JXAC module complete
	if (Q_stricmp(cmd, "jxac_module_complete") == 0) {
		// Silently handled - no action needed
		return qtrue;
	}

	// Handle JXAC screenshot complete
	if (Q_stricmp(cmd, "jxac_ss_complete") == 0) {
		jxac::Server::handleScreenshotComplete(clientNum);
		return qtrue;
	}
	
	// Handle JXAC screenshot data chunk
	if (Q_stricmp(cmd, "jxac_ss_data") == 0) {
		char chunkNumStr[16];
		char sizeStr[16];
		char hexData[1024];
		
		trap_Argv(1, chunkNumStr, sizeof(chunkNumStr));
		trap_Argv(2, sizeStr, sizeof(sizeStr));
		trap_Argv(3, hexData, sizeof(hexData));
		
		int chunkSize = atoi(sizeStr);
		
		// Validate chunk size
		if (chunkSize <= 0 || chunkSize > JXAC_CMD_DATA_CHUNK_SIZE) {
			return qtrue;
		}
		
		// Validate hex data length
		int hexLen = strlen(hexData);
		int expectedHexLen = chunkSize * 2;
		if (hexLen != expectedHexLen || hexLen % 2 != 0) {
			return qtrue;
		}
		
		// Convert hex to binary
		unsigned char binaryData[JXAC_CMD_DATA_CHUNK_SIZE];
		for (int i = 0; i < chunkSize; i++) {
			char hexByte[3] = { hexData[i*2], hexData[i*2+1], '\0' };
			unsigned int byte;
			if (sscanf(hexByte, "%02x", &byte) != 1) {
				return qtrue;
			}
			binaryData[i] = (unsigned char)byte;
		}
		
		jxac::Server::handleScreenshotData(clientNum, binaryData, chunkSize);
		return qtrue;
	}
	
	// Handle JXAC CVAR response
	if (Q_stricmp(cmd, "jxac_cvar_resp") == 0) {
		char cvarName[64];
		char cvarValue[256];
		trap_Argv(1, cvarName, sizeof(cvarName));
		trap_Argv(2, cvarValue, sizeof(cvarValue));
		jxac::Server::handleCvarResponse(clientNum, cvarName, cvarValue);
		return qtrue;
	}
	
	// Handle JXAC violation reports
	if (Q_stricmp(cmd, "jxac_violation") == 0) {
		char violationType[64];
		char details[256];
		trap_Argv(1, violationType, sizeof(violationType));
		trap_Argv(2, details, sizeof(details));
		
		jxacViolationType_t type = JXAC_VIOLATION_TAMPER;
		if (Q_stricmp(violationType, "tamper") == 0) {
			type = JXAC_VIOLATION_TAMPER;
		}
		
		G_LogPrintf("[JXAC] Violation from client %d: %s - %s\n", clientNum, violationType, details);
		jxac::Server::reportViolation(clientNum, type, details);
		return qtrue;
	}

	// Handle old "auth" command (legacy MAC/version check) - silently ignore
	// This command is sent by CG_Authenticate() and is no longer needed
	// We handle it here to prevent "unknown cmd auth" warnings
	if (Q_stricmp(cmd, "auth") == 0) {
		// Silently handled - legacy command, no longer needed
		return qtrue;
	}

	return qfalse; // Command not handled, continue normal processing
}

///////////////////////////////////////////////////////////////////////////////

} // namespace xmod
