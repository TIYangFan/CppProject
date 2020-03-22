#pragma once
#include <stdlib.h>
#include <map>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpConnection.h"

using namespace std;

class TcpServer
{
public:
	typedef function<void(EventLoop*)> ThreadInitCallback;
	typedef TcpConnection* TcpConnectionPtr;
	typedef map<string, TcpConnectionPtr> ConnectionMap;

	enum Option
	{
		kNoReusePort,
		kReusePort,
	};

	TcpServer(EventLoop* loop,
		const InetAddress& listenAddr,
		const string& nameArg,
		Option option = kReusePort);
	~TcpServer();

	void addConnection(int sockfd, const InetAddress& peerAddr);
	void removeConnection(const TcpConnection& conn);

private:
	int				nextConnId_;
	ConnectionMap	connections_;
};

