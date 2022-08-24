//=====================================================================
//
// TcpClient.cpp - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 20:07:20
//
//=====================================================================
#include "TcpClient.h"
#include "Callbacks.h"
#include "Connector.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include "TcpConnection.h"
#include "Logger.h"

#include <cstdio>
#include <functional>
#include <memory>
#include <mutex>
#include <assert.h>

using namespace simple_muduo;

namespace detail
{
    void removeConnection(EventLoop *loop, const TcpConnectionPtr& conn)
    {
        loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }

    void removeConnector(ConnectorPtr connector)
    {
        // Safe_Delete(connector);
    }
}

TcpClient::TcpClient(EventLoop* loop,
		const InetAddress& serverAddr,
		const std::string& nameArg)
	: loop_(loop)
	, name_(nameArg)
	, connector_(new Connector(loop, serverAddr))
	, connectionCallback_()
	, messageCallback_()
	, retry_(false)
	  , connect_(true)
{
	connector_->setNewConnectionCallback(
			std::bind(&TcpClient::newConnection, this, 1));
	LOG_INFO("TcpClient: name:%s \n", name_.c_str());
}

TcpClient::~TcpClient()
{
	LOG_INFO("TcpClient::~TcpClient\n");
	TcpConnectionPtr conn;
	bool unique = false;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		unique = connection_.unique();
		conn = connection_;
	}
	if(conn)
	{
		assert(loop_ == conn->getLoop());
		CloseCallback cb = std::bind(&detail::removeConnection, loop_, std::placeholders::_1);
		loop_->runInLoop(
				std::bind(&TcpConnection::setCloseCallback, conn, cb));
		if (unique)
		{
			conn->forceClose();
		}
	}
}

void TcpClient::connect()
{
	connect_ = true;
	connector_->start();
}

void TcpClient::disconnect()
{
	connect_ = false;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		if(connection_)
		{
			connection_->shutdown();
		}
	}
}

void TcpClient::stop()
{
	connect_ = false;
	connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
	InetAddress peerAddr(sockets::getPeerAddr(sockfd));
	char buf[32];
	snprintf(buf, sizeof buf, ";%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
	++nextConnId_;
	std::string connName = name_ + buf;

	InetAddress localAddr(sockets::getLocalAddr(sockfd));
	
	TcpConnectionPtr conn(new TcpConnection(loop_,
						 connName,
						 sockfd,
						 localAddr,
						 peerAddr));

	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(
			std::bind(&TcpClient::removeConnection, this, std::placeholders::_1)); // FIXME: unsafe
	{
		std::unique_lock<std::mutex> lock(mutex_);
		connection_ = conn;
	}
	conn->connectEstablished();
}
void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
	assert(loop_ == conn->getLoop());
	{
		std::unique_lock<std::mutex> lock(mutex_);
		assert(connection_ == conn);
		connection_.reset();
	}

  loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  if (retry_ && connect_)
  {
	LOG_INFO("TcpClient::connect %s Reconnecting to %s\n", name_.c_str(),connector_->serverAddress().toIpPort().c_str() );
    connector_->restart();
  }
}

