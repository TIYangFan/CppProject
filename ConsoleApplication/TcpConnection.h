#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <memory>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Timestamp.h"

using namespace std;

typedef void* ConnectionCallback;
typedef void* MessageCallback;
typedef void* CloseCallback;
typedef void* ErrorCallback;

class Buffer {};
class CFlowStatistics {};

class TcpConnection
{
public:
	TcpConnection(EventLoop* loop,
		const string& name,
		int sockfd,
		const InetAddress& localAddr,
		const InetAddress& peerAddr);
	~TcpConnection();

private:
	enum StateE
	{
		kDisconnected,
		kConnecting,
		kConnected,
		kDisconnecting
	};

	void handleRead(Timestamp receiveTime);
	void handleWrite();
	void handleClose();
	void handleError();
	
	void sendInLoop(const string& message);
	void sendInLoop(const void* message, size_t len);
	void shutdownInLoop();
	void forceCloseInLoop();
	void setState(StateE s);

private:
	StateE					state_;					// 连接信息
	shared_ptr<Channel>		channel_;				// 引用 Channel 对象
	const InetAddress		localAddr_;				// 本端的地址信息
	const InetAddress		peerAddr_;				// 对端的地址信息

	ConnectionCallback		connectionCallback_;
	MessageCallback			messageCallback_;
	CloseCallback			closeCallback_;
	ErrorCallback			errorCallback_;

	Buffer					inputBuffer_;			// 接收缓冲区
	Buffer					outputBuffer_;			// 发送缓冲区

	CFlowStatistics			flowStatistics;			// 流量统计类
};