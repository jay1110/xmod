//
// Created by nick on 21.02.2026.
//

#ifdef XMOD_LINUX64
#include <thread>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#endif

#include <game/g_local.h>
#include <game/vpn_globals.h>

namespace vpnblocker {
	vmCvar_t g_vpnBlockerEnabled;
	vmCvar_t g_vpnBlockerDBPath;
	vmCvar_t g_vpnBlockerApiKey1;
	vmCvar_t g_vpnBlockerApiKey2;
	vmCvar_t g_vpnBlockerMaxLevel;
	vmCvar_t g_vpnBlockerBanMessageVPN;
	vmCvar_t g_vpnBlockerBanMessageBlacklist;

#ifdef XMOD_LINUX64

	bool isVpnApi1(const int& clientNum, const std::string& ip);
	bool isVpnApi2(const int& clientNum, const std::string& ip);
	bool httpRequest(const int& clientNum, const char* url, std::string& resultBuffer);
	static size_t curlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

	bool clientConnect(const int& clientNum, const char *userinfo, std::string& outmsg) {
		const std::string ip = Info_ValueForKey(userinfo, "ip");
		//G_Printf("[VPN Blocker][clientNum=%d] userinfo = %s\n", clientNum, userinfo); //TODO
		//const int level = atoi(Info_ValueForKey(userinfo, "level"));
		//const std::string ip = "185.205.79.70"; // random test vpn ip

		/*if (level > g_vpnBlockerMaxLevel.integer) {
			G_Printf("[VPN Blocker][clientNum=%d] Level privileged (%d)\n", clientNum, level);
			return false;
		}*/

		//TODO: check nguid whitelist, ip whitelist, ip blacklist

		//TODO: check session (we have to store some map/queue of last checks...)

		if (isVpnApi1(clientNum, ip) or isVpnApi2(clientNum, ip)) {
			outmsg = g_vpnBlockerBanMessageVPN.string;
			return true;
		}

		return false;
	}

	bool isVpnApi1(const int& clientNum, const std::string& ip) {
		const std::string apiKey = g_vpnBlockerApiKey1.string;
		if (apiKey.empty()) return false;

		const std::string url = "https://vpnapi.io/api/" + ip + "?key=" + apiKey;
		std::string resultBuffer;
		if (!httpRequest(clientNum, url.c_str(), resultBuffer)) {
			return false;
		}

		Json::Value jsonResult;
		Json::Reader jsonReader;
		if (!jsonReader.parse(resultBuffer, jsonResult)) {
			G_Printf(
				"[VPN Blocker][clientNum=%d] Error: couldn't parse json: %s\n",
				clientNum,
				jsonReader.getFormattedErrorMessages().c_str()
			);
			return false;
		}

		const bool isVpn = jsonResult["security"]["vpn"].asBool();
		const bool isProxy = jsonResult["security"]["proxy"].asBool();
		const bool isTor = jsonResult["security"]["tor"].asBool();
		const bool isRelay = jsonResult["security"]["relay"].asBool();

		return isVpn || isProxy || isTor || isRelay;
	}

	bool isVpnApi2(const int& clientNum, const std::string& ip) {
		const std::string apiKey = g_vpnBlockerApiKey2.string;
		if (apiKey.empty()) return false;

		const std::string url = "https://api.ipapi.is/?q=" + ip + "&key=" + apiKey;
		std::string resultBuffer;
		if (!httpRequest(clientNum, url.c_str(), resultBuffer)) {
			return false;
		}

		Json::Value jsonResult;
		Json::Reader jsonReader;
		if (!jsonReader.parse(resultBuffer, jsonResult)) {
			G_Printf(
				"[VPN Blocker][clientNum=%d] Error: couldn't parse json: %s\n",
				clientNum,
				jsonReader.getFormattedErrorMessages().c_str()
			);
			return false;
		}

		const bool isBogon = jsonResult["is_bogon"].asBool();
		const bool isCrawler = jsonResult["is_crawler"].asBool();
		const bool isDatacenter = jsonResult["is_datacenter"].asBool();
		const bool isTor = jsonResult["is_tor"].asBool();
		const bool isProxy = jsonResult["is_proxy"].asBool();
		const bool isVpn = jsonResult["is_vpn"].asBool();
		const bool isAbuser = jsonResult["is_abuser"].asBool();

		return isBogon || isCrawler || isDatacenter || isTor || isProxy || isVpn || isAbuser;
	}

	bool httpRequest(const int& clientNum, const char* url, std::string& resultBuffer) {
		std::this_thread::sleep_for(std::chrono::seconds(VPN_API_REQUEST_RETRY_INTERVAL_SEC)); //TODO: remove
		for (int attempt = 0; attempt <= VPN_API_REQUEST_RETRIES_COUNT; attempt++) {
			CURL *curl = curl_easy_init();
			if (!curl) {
				G_Printf("[VPN Blocker][clientNum=%d] Error: couldn't initialize curl\n", clientNum);
				return false;
			}

			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, VPN_API_REQUEST_CONNECT_TIMEOUT_SEC);
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, VPN_API_REQUEST_READ_TIMEOUT_SEC);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resultBuffer);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
			const CURLcode resultCode = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			if (resultCode == CURLE_OK) return true;

			G_Printf("[VPN Blocker][clientNum=%d] Error: curl result code is not ok: %d\n", clientNum, resultCode);
			std::this_thread::sleep_for(std::chrono::seconds(VPN_API_REQUEST_RETRY_INTERVAL_SEC));
		}
		return false;
	}

	static size_t curlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
		static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
		return size * nmemb;
	}

#else

	bool clientConnect(const int& clientNum, const char *userinfo, std::string& outmsg) {
		return false;
	}

#endif
}
