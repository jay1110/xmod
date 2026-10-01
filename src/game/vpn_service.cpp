#include "vpn_service.h"

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#endif
#endif

namespace vpnblocker {

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)

bool supported() noexcept { return false; }
bool NormalizeAddress(const std::string&, std::string& canonical) {
	canonical.clear();
	return false;
}
struct Service::Impl {};
Service::Service(Transport) : impl_(new Impl) {}
Service::~Service() = default;
bool Service::submit(std::uint64_t, const std::string&, const Config&, bool) { return false; }
void Service::cancel(std::uint64_t) {}
std::vector<Result> Service::poll() { return {}; }
void Service::stop() {}

#else

Verdict QueryProviders(const Config&, const std::string&, const std::atomic<bool>&,
	std::string&);
void InitializeTransport();

namespace {
using Clock = std::chrono::steady_clock;
constexpr std::size_t MaxPending = 64;
constexpr std::size_t MaxCache = 256;

bool ValidPort(const std::string& port) {
	if (port.empty() || port.size() > 5) return false;
	unsigned value = 0;
	for (char c : port) {
		if (c < '0' || c > '9') return false;
		value = value * 10 + static_cast<unsigned>(c - '0');
	}
	return value > 0 && value <= 65535;
}

bool PublicV4(const unsigned char* p) {
	return !(p[0] == 0 || p[0] == 10 || p[0] == 127 || p[0] >= 224 ||
		(p[0] == 100 && (p[1] & 0xc0) == 64) ||
		(p[0] == 169 && p[1] == 254) ||
		(p[0] == 172 && (p[1] & 0xf0) == 16) ||
		(p[0] == 192 && p[1] == 168) ||
		(p[0] == 192 && p[1] == 0 && (p[2] == 0 || p[2] == 2)) ||
		(p[0] == 198 && (p[1] == 18 || p[1] == 19)) ||
		(p[0] == 198 && p[1] == 51 && p[2] == 100) ||
		(p[0] == 203 && p[1] == 0 && p[2] == 113));
}

std::string CacheKey(const std::string& ip, const Config& config) {
	// Length prefixes avoid ambiguities if an administrator pastes punctuation in a key.
	return ip + "|" + std::to_string(config.apiKey1.size()) + ":" + config.apiKey1 +
		std::to_string(config.apiKey2.size()) + ":" + config.apiKey2;
}
} // namespace

bool supported() noexcept { return true; }

bool NormalizeAddress(const std::string& raw, std::string& canonical) {
	canonical.clear();
	if (raw.empty() || raw.size() > 80) return false;
	std::string address = raw;
	if (address[0] == '[') {
		const auto close = address.find(']');
		if (close == std::string::npos) return false;
		if (close + 1 != address.size() &&
			(address[close + 1] != ':' || !ValidPort(address.substr(close + 2)))) return false;
		address = address.substr(1, close - 1);
	} else {
		const auto colon = address.find(':');
		if (colon != std::string::npos && colon == address.rfind(':')) {
			if (!ValidPort(address.substr(colon + 1))) return false;
			address.resize(colon);
		}
	}
	unsigned char packed[16] = {};
	char output[INET6_ADDRSTRLEN] = {};
	if (inet_pton(AF_INET, address.c_str(), packed) == 1) {
		if (!PublicV4(packed)) return false;
		if (!inet_ntop(AF_INET, packed, output, sizeof(output))) return false;
	} else if (inet_pton(AF_INET6, address.c_str(), packed) == 1) {
		const bool leadingZero = std::all_of(packed, packed + 10,
			[](unsigned char c) { return c == 0; });
		if (leadingZero && packed[10] == 0xff && packed[11] == 0xff) {
			if (!PublicV4(packed + 12)) return false;
			if (!inet_ntop(AF_INET, packed + 12, output, sizeof(output))) return false;
		} else {
			// Only globally routed unicast space; excludes loopback, link-local and ULA.
			if ((packed[0] & 0xe0) != 0x20 ||
				(packed[0] == 0x20 && packed[1] == 0x01 && packed[2] == 0x0d && packed[3] == 0xb8)) return false;
			if (!inet_ntop(AF_INET6, packed, output, sizeof(output))) return false;
		}
	} else return false;
	canonical = output;
	return true;
}

struct Service::Impl {
	struct Request {
		std::uint64_t id;
		std::string ip;
		Config config;
		bool refresh;
	};
	struct CacheEntry {
		std::string key;
		Verdict verdict;
		Clock::time_point expires;
	};

	Transport transport;
	std::mutex mutex;
	std::condition_variable changed;
	std::deque<Request> requests;
	std::vector<Result> results;
	std::deque<CacheEntry> cache;
	bool stopping = false;
	bool active = false;
	std::uint64_t activeId = 0;
	std::atomic<bool> cancelled{false};
	std::thread worker;

	explicit Impl(Transport fn) {
		if (fn) transport = std::move(fn);
		else {
			// Older system libcurl builds must initialize before our worker starts.
			InitializeTransport();
			transport = QueryProviders;
		}
		worker = std::thread(&Impl::run, this);
	}

	void run() {
		bool transportSafe = true;
#if !defined(_WIN32)
		// CURLOPT_NOSIGNAL leaves SIGPIPE handling to us. Mask it only on this
		// dedicated worker; do not alter the engine's process-wide handlers.
		// Keep the mask until thread exit, discarding thread-pending SIGPIPE.
		sigset_t blocked;
		sigemptyset(&blocked);
		sigaddset(&blocked, SIGPIPE);
		transportSafe = pthread_sigmask(SIG_BLOCK, &blocked, nullptr) == 0;
#endif
		for (;;) {
			Request request;
			{
				std::unique_lock<std::mutex> lock(mutex);
				changed.wait(lock, [this] { return stopping || !requests.empty(); });
				if (stopping) return;
				request = std::move(requests.front());
				requests.pop_front();
				// A previous queued request may have classified the same address while
				// this request waited. Reuse that result instead of repeating the API call.
				const auto key = CacheKey(request.ip, request.config);
				const auto now = Clock::now();
				const auto cached = std::find_if(cache.begin(), cache.end(),
					[&](const CacheEntry& entry) { return entry.key == key && entry.expires > now; });
				if (!request.refresh && cached != cache.end()) {
					const auto entry = *cached;
					cache.erase(cached);
					cache.push_back(entry);
					results.push_back({request.id, request.ip, entry.verdict,
						entry.verdict == Verdict::Unknown ? "recent provider failure" : std::string()});
					continue;
				}
				active = true;
				activeId = request.id;
				cancelled.store(false);
			}

			std::string diagnostic;
			Verdict verdict = Verdict::Unknown;
			try {
				if (transportSafe)
					verdict = transport(request.config, request.ip, cancelled, diagnostic);
				else diagnostic = "worker signal mask could not be initialized";
			} catch (...) {
				// Exception text may include the request URL and its secret query parameters.
				diagnostic = "provider request failed";
			}

			std::lock_guard<std::mutex> lock(mutex);
			if (!stopping && !cancelled.load()) {
				const auto key = CacheKey(request.ip, request.config);
				cache.erase(std::remove_if(cache.begin(), cache.end(), [&key](const CacheEntry& e) {
					return e.key == key;
				}), cache.end());
				if (cache.size() >= MaxCache) cache.pop_front();
				cache.push_back({key, verdict, Clock::now() + std::chrono::seconds(
					verdict == Verdict::Unknown ? 15 : 300)});
				results.push_back({request.id, request.ip, verdict, std::move(diagnostic)});
			}
			active = false;
		}
	}
};

Service::Service(Transport transport) : impl_(new Impl(std::move(transport))) {}
Service::~Service() { stop(); }

bool Service::submit(std::uint64_t id, const std::string& ip, const Config& config, bool refresh) {
	std::string normalized;
	if (!NormalizeAddress(ip, normalized) ||
		(config.apiKey1.empty() && config.apiKey2.empty()) ||
		config.apiKey1.size() > 256 || config.apiKey2.size() > 256) return false;
	std::lock_guard<std::mutex> lock(impl_->mutex);
	if (impl_->stopping || impl_->requests.size() + impl_->results.size() +
		(impl_->active ? 1 : 0) >= MaxPending) return false;
	if ((impl_->active && impl_->activeId == id) ||
		std::any_of(impl_->requests.begin(), impl_->requests.end(), [id](const Impl::Request& r) { return r.id == id; }) ||
		std::any_of(impl_->results.begin(), impl_->results.end(), [id](const Result& r) { return r.id == id; })) return false;
	const auto key = CacheKey(normalized, config);
	const auto now = Clock::now();
	for (auto it = impl_->cache.begin(); it != impl_->cache.end();) {
		if (it->expires <= now) { it = impl_->cache.erase(it); continue; }
		if (!refresh && it->key == key) {
			const auto entry = *it;
			impl_->cache.erase(it);
			impl_->cache.push_back(entry);
			impl_->results.push_back({id, normalized, entry.verdict,
				entry.verdict == Verdict::Unknown ? "recent provider failure" : std::string()});
			return true;
		}
		++it;
	}
	impl_->requests.push_back({id, normalized, config, refresh});
	impl_->changed.notify_one();
	return true;
}

void Service::cancel(std::uint64_t id) {
	std::lock_guard<std::mutex> lock(impl_->mutex);
	if (impl_->active && impl_->activeId == id) impl_->cancelled.store(true);
	impl_->requests.erase(std::remove_if(impl_->requests.begin(), impl_->requests.end(),
		[id](const Impl::Request& r) { return r.id == id; }), impl_->requests.end());
	impl_->results.erase(std::remove_if(impl_->results.begin(), impl_->results.end(),
		[id](const Result& r) { return r.id == id; }), impl_->results.end());
}

std::vector<Result> Service::poll() {
	std::lock_guard<std::mutex> lock(impl_->mutex);
	std::vector<Result> results;
	results.swap(impl_->results);
	return results;
}

void Service::stop() {
	{
		std::lock_guard<std::mutex> lock(impl_->mutex);
		impl_->stopping = true;
		impl_->cancelled.store(true);
		impl_->requests.clear();
		impl_->results.clear();
	}
	impl_->changed.notify_one();
	if (impl_->worker.joinable()) impl_->worker.join();
}

#endif
} // namespace vpnblocker
