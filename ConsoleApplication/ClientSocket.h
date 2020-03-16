#pragma once
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class CClientSocket
{
public:
	CClientSocket();
	~CClientSocket();

	int connectSocket(char* ip_addr);
	int sendMessage();

private:
	int m_sockfd;
	int m_cur_size;
	char* m_recv_line;
	char* m_send_line;	
	struct sockaddr_in m_servaddr;
};

