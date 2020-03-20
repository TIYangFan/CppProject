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
	StateE					state_;					// ������Ϣ
	shared_ptr<Channel>		channel_;				// ���� Channel ����
	const InetAddress		localAddr_;				// ���˵ĵ�ַ��Ϣ
	const InetAddress		peerAddr_;				// �Զ˵ĵ�ַ��Ϣ

	ConnectionCallback		connectionCallback_;
	MessageCallback			messageCallback_;
	CloseCallback			closeCallback_;
	ErrorCallback			errorCallback_;

	Buffer					inputBuffer_;			// ���ջ�����
	Buffer					outputBuffer_;			// ���ͻ�����

	CFlowStatistics			flowStatistics;			// ����ͳ����
};