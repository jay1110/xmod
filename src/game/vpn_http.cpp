#include "vpn_service.h"

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
#include <jsoncpp/json/json.h>
#include <chrono>
#include <limits>
#include <memory>
#include <mutex>
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>
#include <condition_variable>
#else
#include <curl/curl.h>
#include <dlfcn.h>
#endif
#endif

namespace vpnblocker {

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
Verdict ParseProviderResponse(Provider, const std::string&, std::string& diagnostic) {
	diagnostic = "VPN checks are unavailable on this platform";
	return Verdict::Unknown;
}
#else

namespace {
constexpr std::size_t MaxBody = 64 * 1024;

std::string EscapeQuery(const std::string& input) {
	static const char hex[] = "0123456789ABCDEF";
	std::string escaped;
	for (unsigned char c : input) {
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped.push_back(static_cast<char>(c));
		} else {
			escaped.push_back('%');
			escaped.push_back(hex[c >> 4]);
			escaped.push_back(hex[c & 15]);
		}
	}
	return escaped;
}

#if defined(_WIN32)

struct HttpHandle {
	HINTERNET value;
	explicit HttpHandle(HINTERNET handle) : value(handle) {}
	~HttpHandle() { if (value) WinHttpCloseHandle(value); }
	HttpHandle(const HttpHandle&) = delete;
	HttpHandle& operator=(const HttpHandle&) = delete;
};

struct Completion {
	std::mutex mutex;
	std::condition_variable changed;
	DWORD status = 0;
	DWORD bytes = 0;
	bool failed = false;
	bool closed = false;

	static void CALLBACK Callback(HINTERNET, DWORD_PTR context, DWORD status,
		void*, DWORD length) {
		if (!context) return;
		auto& self = *reinterpret_cast<Completion*>(context);
		std::lock_guard<std::mutex> lock(self.mutex);
		if (status == WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING) self.closed = true;
		else if (status == WINHTTP_CALLBACK_STATUS_REQUEST_ERROR) self.failed = true;
		else { self.status = status; self.bytes = length; }
		self.changed.notify_one();
	}

	void reset() {
		std::lock_guard<std::mutex> lock(mutex);
		status = 0;
		bytes = 0;
	}

	bool wait(DWORD expected, const std::atomic<bool>& cancelled,
		std::chrono::steady_clock::time_point deadline) {
		std::unique_lock<std::mutex> lock(mutex);
		while (!failed && status != expected && !cancelled.load() &&
			std::chrono::steady_clock::now() < deadline) {
			changed.wait_for(lock, std::chrono::milliseconds(25));
		}
		return !failed && status == expected && !cancelled.load() &&
			std::chrono::steady_clock::now() < deadline;
	}
};

// Close cancels pending asynchronous I/O. HANDLE_CLOSING is the final callback;
// wait for it before releasing the callback context or unloading the game DLL.
struct AsyncRequest {
	HINTERNET value;
	Completion& completion;
	bool registered = false;
	~AsyncRequest() {
		if (!value) return;
		WinHttpCloseHandle(value);
		if (registered) {
			std::unique_lock<std::mutex> lock(completion.mutex);
			completion.changed.wait(lock, [this] { return completion.closed; });
		}
	}
};

bool HttpGet(const char* host, const std::string& path, const std::atomic<bool>& cancelled,
	std::string& body, std::string& diagnostic) {
	diagnostic = "HTTPS request failed or timed out";
	if (cancelled.load()) return false;
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	HttpHandle session(WinHttpOpen(L"Xmod/2.0 VPN checker", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC));
	if (!session.value) return false;
	WinHttpSetTimeouts(session.value, 2000, 2000, 2000, 2000);
	const std::string hostText(host);
	const std::wstring wideHost(hostText.begin(), hostText.end());
	const std::wstring widePath(path.begin(), path.end());
	HttpHandle connection(WinHttpConnect(session.value, wideHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0));
	if (!connection.value) return false;
	Completion completion;
	// Keep the read buffer alive until AsyncRequest has drained its final callback.
	char buffer[8192];
	AsyncRequest request{WinHttpOpenRequest(connection.value, L"GET", widePath.c_str(), nullptr,
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE), completion};
	if (!request.value) return false;
	if (WinHttpSetStatusCallback(request.value, Completion::Callback,
		WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE | WINHTTP_CALLBACK_FLAG_HEADERS_AVAILABLE |
		WINHTTP_CALLBACK_FLAG_READ_COMPLETE | WINHTTP_CALLBACK_FLAG_REQUEST_ERROR |
		WINHTTP_CALLBACK_FLAG_HANDLES, 0) == WINHTTP_INVALID_STATUS_CALLBACK) return false;
	DWORD_PTR context = reinterpret_cast<DWORD_PTR>(&completion);
	if (!WinHttpSetOption(request.value, WINHTTP_OPTION_CONTEXT_VALUE, &context, sizeof(context))) return false;
	request.registered = true;
	DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
	if (!WinHttpSetOption(request.value, WINHTTP_OPTION_REDIRECT_POLICY,
		&redirectPolicy, sizeof(redirectPolicy))) return false;
	if (!WinHttpSendRequest(request.value, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		WINHTTP_NO_REQUEST_DATA, 0, 0, context) ||
		!completion.wait(WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE, cancelled, deadline)) return false;
	completion.reset();
	if (!WinHttpReceiveResponse(request.value, nullptr) ||
		!completion.wait(WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE, cancelled, deadline)) return false;
	DWORD code = 0;
	DWORD size = sizeof(code);
	if (!WinHttpQueryHeaders(request.value, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX, &code, &size, WINHTTP_NO_HEADER_INDEX) || code != 200) {
		diagnostic = "provider returned a non-success HTTP status";
		return false;
	}
	body.clear();
	for (;;) {
		completion.reset();
		if (!WinHttpReadData(request.value, buffer, sizeof(buffer), nullptr) ||
			!completion.wait(WINHTTP_CALLBACK_STATUS_READ_COMPLETE, cancelled, deadline)) return false;
		DWORD bytes;
		{
			std::lock_guard<std::mutex> lock(completion.mutex);
			bytes = completion.bytes;
		}
		if (!bytes) { diagnostic.clear(); return true; }
		if (bytes > MaxBody - body.size()) {
			diagnostic = "provider response exceeds the size limit";
			return false;
		}
		body.append(buffer, bytes);
	}
}

#else

bool PinCurlImage() {
	Dl_info curlImage{}, gameImage{};
	if (!dladdr(reinterpret_cast<const void*>(&curl_easy_init), &curlImage) ||
		!dladdr(reinterpret_cast<const void*>(&PinCurlImage), &gameImage) ||
		!curlImage.dli_fname || curlImage.dli_fbase == gameImage.dli_fbase) return false;
	// A threaded DNS resolver can outlive curl_easy_cleanup. Keep its shared
	// library loaded even if a map change unloads our last ordinary dependency.
	// Do not pin qagame itself when curl was linked statically by a custom build.
	int flags = RTLD_NOW | RTLD_LOCAL;
#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
	void* image = dlopen(curlImage.dli_fname, flags);
	if (!image) return false;
#ifdef RTLD_NODELETE
	// NODELETE persists after dlclose; avoid one extra reference per map load.
	dlclose(image);
#else
	// On loaders without NODELETE this reference intentionally lasts until exit.
#endif
	return true;
}

struct CurlRuntime {
	CURLcode status = PinCurlImage() ? curl_global_init(CURL_GLOBAL_DEFAULT) : CURLE_FAILED_INIT;
	~CurlRuntime() { if (status == CURLE_OK) curl_global_cleanup(); }
};

CurlRuntime& GetCurlRuntime() {
	static CurlRuntime runtime;
	return runtime;
}

struct CurlBody {
	std::string* body;
	bool oversized = false;
};

std::size_t CurlWrite(void* data, std::size_t size, std::size_t count, void* context) {
	auto& response = *static_cast<CurlBody*>(context);
	if (size && count > std::numeric_limits<std::size_t>::max() / size) return 0;
	const auto bytes = size * count;
	if (bytes > MaxBody - response.body->size()) { response.oversized = true; return 0; }
	try { response.body->append(static_cast<char*>(data), bytes); }
	catch (...) { return 0; }
	return bytes;
}

int CurlProgress(void* context, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
	return static_cast<const std::atomic<bool>*>(context)->load() ? 1 : 0;
}

bool HttpGet(const char* host, const std::string& path, const std::atomic<bool>& cancelled,
	std::string& body, std::string& diagnostic) {
	diagnostic = "HTTPS request failed or timed out";
	if (cancelled.load()) return false;
	auto& runtime = GetCurlRuntime();
	if (runtime.status != CURLE_OK) return false;
	const auto* version = curl_version_info(CURLVERSION_NOW);
	if (!version || !(version->features & CURL_VERSION_ASYNCHDNS)) {
		// A synchronous resolver plus NOSIGNAL cannot honor the request deadline.
		diagnostic = "libcurl requires an asynchronous DNS resolver";
		return false;
	}
	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> handle(curl_easy_init(), curl_easy_cleanup);
	if (!handle) return false;
	const std::string url = std::string("https://") + host + path;
	body.clear();
	CurlBody response{&body};
	curl_easy_setopt(handle.get(), CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle.get(), CURLOPT_USERAGENT, "Xmod/2.0 VPN checker");
	curl_easy_setopt(handle.get(), CURLOPT_CONNECTTIMEOUT_MS, 2000L);
	curl_easy_setopt(handle.get(), CURLOPT_TIMEOUT_MS, 5000L);
	curl_easy_setopt(handle.get(), CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(handle.get(), CURLOPT_FOLLOWLOCATION, 0L);
	curl_easy_setopt(handle.get(), CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(handle.get(), CURLOPT_SSL_VERIFYHOST, 2L);
	curl_easy_setopt(handle.get(), CURLOPT_WRITEFUNCTION, CurlWrite);
	curl_easy_setopt(handle.get(), CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(handle.get(), CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(handle.get(), CURLOPT_XFERINFOFUNCTION, CurlProgress);
	curl_easy_setopt(handle.get(), CURLOPT_XFERINFODATA, &cancelled);
	const auto result = curl_easy_perform(handle.get());
	long code = 0;
	curl_easy_getinfo(handle.get(), CURLINFO_RESPONSE_CODE, &code);
	if (result != CURLE_OK) {
		if (response.oversized) diagnostic = "provider response exceeds the size limit";
		return false;
	}
	if (code != 200) { diagnostic = "provider returned a non-success HTTP status"; return false; }
	diagnostic.clear();
	return true;
}
#endif

} // namespace

void InitializeTransport() {
#if !defined(_WIN32)
	(void)GetCurlRuntime();
#endif
}

Verdict ParseProviderResponse(Provider provider, const std::string& body,
	std::string& diagnostic) {
	diagnostic = "provider returned an invalid response";
	if (body.empty() || body.size() > MaxBody) return Verdict::Unknown;
	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;
	builder["allowComments"] = false;
	builder["strictRoot"] = true;
	builder["failIfExtra"] = true;
	builder["rejectDupKeys"] = true;
	builder["stackLimit"] = 32;
	std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	Json::Value root;
	std::string errors;
	try {
		if (!reader->parse(body.data(), body.data() + body.size(), &root, &errors) ||
			!root.isObject() || root.isMember("error") || root.isMember("errors") ||
			root.isMember("message")) return Verdict::Unknown;
	} catch (...) {
		return Verdict::Unknown;
	}
	static const char* vpnFields[] = {"vpn", "proxy", "tor", "relay"};
	static const char* ipFields[] = {"is_bogon", "is_crawler", "is_datacenter",
		"is_tor", "is_proxy", "is_vpn", "is_abuser"};
	const Json::Value& flags = provider == Provider::VpnApi ? root["security"] : root;
	if (!flags.isObject()) return Verdict::Unknown;
	const char* const* fields = provider == Provider::VpnApi ? vpnFields : ipFields;
	const std::size_t count = provider == Provider::VpnApi ? 4 : 7;
	bool blocked = false;
	for (std::size_t index = 0; index < count; ++index) {
		const auto& value = flags[fields[index]];
		if (!value.isBool()) return Verdict::Unknown;
		blocked = blocked || value.asBool();
	}
	diagnostic.clear();
	return blocked ? Verdict::Blocked : Verdict::Allowed;
}

Verdict QueryProviders(const Config& config, const std::string& ip,
	const std::atomic<bool>& cancelled, std::string& diagnostic) {
	bool configured = false;
	bool complete = true;
	diagnostic.clear();
	for (unsigned index = 0; index < 2; ++index) {
		const auto& key = index == 0 ? config.apiKey1 : config.apiKey2;
		if (key.empty()) continue;
		configured = true;
		if (cancelled.load()) { diagnostic = "request cancelled"; return Verdict::Unknown; }
		const char* host = index == 0 ? "vpnapi.io" : "api.ipapi.is";
		const auto path = index == 0 ? "/api/" + EscapeQuery(ip) + "?key=" + EscapeQuery(key) :
			"/?q=" + EscapeQuery(ip) + "&key=" + EscapeQuery(key);
		std::string body;
		std::string failure;
		const bool ok = HttpGet(host, path, cancelled, body, failure);
		const Verdict result = ok ? ParseProviderResponse(index == 0 ? Provider::VpnApi :
			Provider::IpApi, body, failure) : Verdict::Unknown;
		if (result == Verdict::Blocked) { diagnostic.clear(); return Verdict::Blocked; }
		if (result == Verdict::Unknown) {
			complete = false;
			diagnostic = (index == 0 ? "provider 1: " : "provider 2: ") + failure;
		}
	}
	if (!configured) diagnostic = "no API key configured";
	return configured && complete ? Verdict::Allowed : Verdict::Unknown;
}

#endif
} // namespace vpnblocker
