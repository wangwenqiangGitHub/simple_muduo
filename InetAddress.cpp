//=====================================================================
//
// InetAddress.cpp - 
//
// Created by wwq on 2022/08/22
// Last Modified: 2022/08/22 17:33:44
//
//=====================================================================
#include "InetAddress.h"
#include "Logger.h"
#include "SocketsOps.h"
#include <assert.h>

#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

using namespace simple_muduo;

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddress is same size as sockaddr_in6");

InetAddress::InetAddress(uint16_t portArg, bool loopbackOnly, bool ipv6)
{
  if (ipv6)
  {
	::memset(&addr6_, 0, sizeof addr6_);
    addr6_.sin6_family = AF_INET6;
    in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr = ip;
    addr6_.sin6_port = sockets::hostToNetwork16(portArg);
  }
  else
  {
	::memset(&addr_, 0, sizeof addr_);
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
    addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
    addr_.sin_port = sockets::hostToNetwork16(portArg);
  }
}

InetAddress::InetAddress(std::string ip, uint16_t portArg, bool ipv6)
{
  if (ipv6 || strchr(ip.c_str(), ':'))
  {
	::memset(&addr6_, 0, sizeof addr6_);
    sockets::fromIpPort(ip.c_str(), portArg, &addr6_);
  }
  else
  {
	::memset(&addr_, 0, sizeof addr_);
    sockets::fromIpPort(ip.c_str(), portArg, &addr_);
  }
}

std::string InetAddress::toIpPort() const
{
  char buf[64] = "";
  sockets::toIpPort(buf, sizeof buf, getSockAddr());
  return buf;
}

std::string InetAddress::toIp() const
{
  char buf[64] = "";
  sockets::toIp(buf, sizeof buf, getSockAddr());
  return buf;
}

uint32_t InetAddress::ipv4NetEndian() const
{
  assert(family() == AF_INET);
  return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::port() const
{
  return sockets::networkToHost16(portNetEndian());
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(std::string hostname, InetAddress* out)
{
  assert(out != NULL);
  struct hostent hent;
  struct hostent* he = NULL;
  int herrno = 0;
  ::memset(&hent, 0, sizeof(hent));

  int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
  if (ret == 0 && he != NULL)
  {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  }
  else
  {
    if (ret)
    {
	  LOG_INFO("InetAddress::resolve\n");
    }
    return false;
  }
}

void InetAddress::setScopeId(uint32_t scope_id)
{
  if (family() == AF_INET6)
  {
    addr6_.sin6_scope_id = scope_id;
  }
}
