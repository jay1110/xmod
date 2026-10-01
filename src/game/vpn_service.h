// Background IP classification. This interface deliberately has no game/DB dependencies.
#ifndef XMOD_VPN_SERVICE_H
#define XMOD_VPN_SERVICE_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace vpnblocker {

enum class Verdict { Unknown, Allowed, Blocked };
enum class Provider { VpnApi, IpApi };

struct Config {
	std::string apiKey1;
	std::string apiKey2;
};

struct Result {
	std::uint64_t id;
	std::string ip;
	Verdict verdict;
	std::string diagnostic;
};

using Transport = std::function<Verdict(const Config&, const std::string&,
	const std::atomic<bool>&, std::string&)>;

bool supported() noexcept;
// Accept only public numeric addresses; strip an optional connection port.
bool NormalizeAddress(const std::string& raw, std::string& canonical);
Verdict ParseProviderResponse(Provider provider, const std::string& body,
	std::string& diagnostic);

class Service {
public:
	explicit Service(Transport transport = Transport());
	~Service();
	Service(const Service&) = delete;
	Service& operator=(const Service&) = delete;

	// False means disabled, invalid, or the bounded queue is full; never waits for HTTP.
	bool submit(std::uint64_t id, const std::string& ip, const Config& config,
		bool refresh = false);
	void cancel(std::uint64_t id);
	std::vector<Result> poll();
	// Cancel and join before the game DLL is unloaded. Safe to call repeatedly.
	void stop();

private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};

} // namespace vpnblocker
#endif
