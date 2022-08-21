#ifndef SIMPLE_MUDUO_H
#define SIMPLE_MUDUO_H

#include <arpa/inet.h>
#include <cstdint>
#include <netinet/in.h>
#include <string>
namespace simple_muduo {
class InetAddress
{
public:
	//TODO: ipv6
	explicit InetAddress(uint64_t port = 0,bool lookbackOnly = false, bool ipv6 = false);
	explicit InetAddress(std::string ip, uint64_t port);
    explicit InetAddress(const sockaddr_in &addr)
        : addr_(addr)
    { }
	explicit InetAddress(const struct sockaddr_in6& addr)
		: addr6_(addr)
	{ }

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const sockaddr_in *getSockAddr() const { return &addr_; }
    void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }
	void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

private:
	union
	{
		struct sockaddr_in addr_;
		struct sockaddr_in6 addr6_;
	};
};
}
#endif
