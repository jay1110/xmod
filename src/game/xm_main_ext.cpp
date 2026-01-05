#include <bgame/impl.h>
#include <game/g_local.h>
#include <game/xm_main_ext.h>
#include <game/jxac/jxac_server.h>
#include <bgame/xm_auth_shared.h>
#include <bgame/jxac_common.h>
#include <game/xmod_globals.h>
#include <game/Client.h>

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

// Called from ClientCommand BEFORE the CS_ACTIVE check
// Returns qtrue if command was handled, qfalse to continue normal processing
qboolean OnClientCommand(gentity_t *ent) {
	char cmd[MAX_TOKEN_CHARS];
	trap_Argv(0, cmd, sizeof(cmd));
	
	int clientNum = ent - g_entities;
	
	// Handle authenticate command (can come before CS_ACTIVE)
	if (Q_stricmp(cmd, xm_auth::CMD_AUTHENTICATE) == 0) {
		char guid[64];
		char hwid[64];
		trap_Argv(1, guid, sizeof(guid));
		trap_Argv(2, hwid, sizeof(hwid));
		
		G_LogPrintf("Received authenticate command from client %d (%s): GUID=%s, HWID=%s\n", 
			clientNum, ent->client->pers.netname, guid, hwid);
		
		// Validate GUID and HWID format (SHA1 hex = 40 chars with valid hex characters)
		if (isValidHexString(guid, xm_auth::GUID_LENGTH) && isValidHexString(hwid, xm_auth::HWID_LENGTH)) {
			// Store in legacy Client object for compatibility
			Client& clientObject = g_clientObjects[clientNum];
			clientObject.authGuid = guid;
			clientObject.authHwid = hwid;
			clientObject.authenticated = true;
			
			G_LogPrintf("Client %d (%s) authenticated successfully\n", 
				clientNum, ent->client->pers.netname);
			
			// Use xmod session system for authentication
			if (xmod::g_database && xmod::g_sessions[clientNum]) {
				xmod::g_sessions[clientNum]->onGuidReceived(guid, hwid);
			}
		} else {
			G_LogPrintf("Client %d (%s) authentication FAILED: Invalid GUID or HWID format\n", 
				clientNum, ent->client->pers.netname);
		}
		return qtrue;
	}
	
	// Handle JXAC heartbeat command (can come before CS_ACTIVE)
	if (Q_stricmp(cmd, "jxac_heartbeat") == 0) {
		G_LogPrintf("[JXAC DEBUG] Received jxac_heartbeat from client %d\n", clientNum);
		jxac::Server::handleHeartbeat(clientNum);
		return qtrue;
	}
	
	// Handle JXAC module scan data (can come before CS_ACTIVE)
	if (Q_stricmp(cmd, "jxac_module") == 0) {
		char moduleName[256];
		char checksum[64];
		trap_Argv(1, moduleName, sizeof(moduleName));
		trap_Argv(2, checksum, sizeof(checksum));
		
		G_LogPrintf("[JXAC] Received module info from client %d: %s\n", clientNum, moduleName);
		jxac::Server::checkModuleSignature(clientNum, moduleName, checksum);
		return qtrue;
	}
	
	// Handle JXAC screenshot complete (can come before CS_ACTIVE)
	if (Q_stricmp(cmd, "jxac_ss_complete") == 0) {
		G_LogPrintf("[JXAC] Received screenshot complete from client %d\n", clientNum);
		jxac::Server::handleScreenshotComplete(clientNum);
		return qtrue;
	}
	
	// Handle JXAC screenshot data chunk (can come before CS_ACTIVE)
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
			Com_Printf("JXAC: Invalid chunk size %d from client %d\n", chunkSize, clientNum);
			return qtrue;
		}
		
		// Validate hex data
		int hexLen = strlen(hexData);
		int expectedHexLen = chunkSize * 2;
		
		// Check if hex length matches reported size
		if (hexLen != expectedHexLen) {
			Com_Printf("JXAC: Hex length mismatch from client %d: got %d, expected %d (chunk size %d)\n",
			           clientNum, hexLen, expectedHexLen, chunkSize);
			return qtrue;
		}
		
		// Check for odd hex length
		if (hexLen % 2 != 0) {
			Com_Printf("JXAC: Odd hex length %d from client %d\n", hexLen, clientNum);
			return qtrue;
		}
		
		// Convert hex to binary
		unsigned char binaryData[JXAC_CMD_DATA_CHUNK_SIZE];
		for (int i = 0; i < chunkSize; i++) {
			char hexByte[3] = { hexData[i*2], hexData[i*2+1], '\0' };
			unsigned int byte;
			if (sscanf(hexByte, "%02x", &byte) != 1) {
				Com_Printf("JXAC: Invalid hex data at offset %d from client %d\n", i*2, clientNum);
				return qtrue;
			}
			binaryData[i] = (unsigned char)byte;
		}
		
		// Pass to JXAC server handler
		G_LogPrintf("[JXAC] Received screenshot data from client %d\n", clientNum);
		jxac::Server::handleScreenshotData(clientNum, binaryData, chunkSize);
		return qtrue;
	}
	
	// Handle JXAC CVAR response (can come before CS_ACTIVE)
	if (Q_stricmp(cmd, "jxac_cvar_resp") == 0) {
		char cvarName[64];
		char cvarValue[256];
		trap_Argv(1, cvarName, sizeof(cvarName));
		trap_Argv(2, cvarValue, sizeof(cvarValue));
		
		G_LogPrintf("[JXAC] Received CVAR response from client %d: %s=%s\n", clientNum, cvarName, cvarValue);
		jxac::Server::handleCvarResponse(clientNum, cvarName, cvarValue);
		return qtrue;
	}
	
	// Handle JXAC client-side violation reports (can come before CS_ACTIVE)
	if (Q_stricmp(cmd, "jxac_violation") == 0) {
		char violationType[64];
		char details[256];
		trap_Argv(1, violationType, sizeof(violationType));
		trap_Argv(2, details, sizeof(details));
		
		// Map violation type string to enum
		jxacViolationType_t type = JXAC_VIOLATION_TAMPER;
		if (Q_stricmp(violationType, "tamper") == 0) {
			type = JXAC_VIOLATION_TAMPER;
		}
		
		G_LogPrintf("[JXAC] Received violation report from client %d: %s - %s\n", clientNum, violationType, details);
		jxac::Server::reportViolation(clientNum, type, details);
		return qtrue;
	}
	
	return qfalse; // Command not handled, continue normal processing
}

///////////////////////////////////////////////////////////////////////////////

} // namespace xmod
