//=====================================================================
//
// TcpClient.cpp - 
//
// Created by wwq on 2022/08/21
// Last Modified: 2022/08/21 20:07:20
//
//=====================================================================
#include "TcpClient.h"

using namespace simple_muduo;


TcpClient::TcpClient(EventLoop* loop,
            const InetAddress& serverAddr,
            const std::string& nameArg)
	: loop_(loop)
	, name_(nameArg)
	, retry_(false)
	, connect_(true)
{

}
TcpClient::~TcpClient()
{

}
  
void TcpClient::connect()
{

}
void TcpClient::disconnect()
{

}
void TcpClient::stop()
{

}

TcpConnectionPtr connection()
{

}

