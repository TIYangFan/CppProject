#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>

class CServerSocket
{
public:
	CServerSocket();
	~CServerSocket();

	int openSocket();
	int dealWithMessage();
	void disconnect();

private:
	int m_listenfd;
	int m_connfd;
	struct sockaddr_in m_servaddr;
	char* m_buff;
	int m_cur_size;
};

