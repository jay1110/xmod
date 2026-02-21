//
// Created by nick on 21.02.2026.
//

#ifndef GAME_VPN_GLOBALS_H
#define GAME_VPN_GLOBALS_H

#include <string>
#include <bgame/q_shared.h>

namespace vpnblocker {
	extern vmCvar_t	g_vpnBlockerEnabled;

	extern vmCvar_t	g_vpnBlockerDBPath;

	extern vmCvar_t g_vpnBlockerApiKey1;
	extern vmCvar_t g_vpnBlockerApiKey2;

	extern vmCvar_t	g_vpnBlockerMaxLevel;
	//extern vmCvar_t	g_vpnBlockerBanTimeoutSec;
	extern vmCvar_t	g_vpnBlockerBanMessageVPN;
	extern vmCvar_t	g_vpnBlockerBanMessageBlacklist;

	bool clientConnect(const int& clientNum, const char *userinfo, std::string& outmsg);

	constexpr int VPN_API_REQUEST_CONNECT_TIMEOUT_SEC = 10;
	constexpr int VPN_API_REQUEST_READ_TIMEOUT_SEC = 10;
	constexpr int VPN_API_REQUEST_RETRIES_COUNT = 2;
	constexpr int VPN_API_REQUEST_RETRY_INTERVAL_SEC = 3;
}

#endif //GAME_VPN_GLOBALS_H
