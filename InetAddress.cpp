#include <cstring> // memset
#include <string>

#include <sys/socket.h>
#include "InetAddress.h"

using namespace simple_muduo;


InetAddress::InetAddress(uint64_t port,bool lookbackOnly, bool ipv6)
{}
InetAddress::InetAddress(std::string ip, uint64_t port)
{
    ::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = ::htons(port); // 本地字节序转为网络字节序
    addr_.sin_addr.s_addr = ::inet_addr(ip.c_str());
}

std::string InetAddress::toIp() const
{
	// addr_
	char buf[64] = {0};
	//将点分十进制的ip地址转化为用于网络传输的数值格式
	::inet_ntop(AF_INET,& addr_.sin_addr, buf, sizeof(buf));
	return buf;

}

std::string InetAddress:: toIpPort() const
{
    // ip:port
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = ::strlen(buf);
    uint16_t port = ::ntohs(addr_.sin_port);
    sprintf(buf+end, ":%u", port);
    return buf;
}

uint16_t InetAddress::toPort() const
{
	return ::ntohs(addr_.sin_port);
}
