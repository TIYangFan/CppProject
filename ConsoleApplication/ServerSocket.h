#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "WorkThreadPool.h"

class CNetWork
{
public:
	CNetWork() {}
	CNetWork(string workName)
	{
		m_strWorkName = workName;
		m_ptrData = NULL;
	}
	~CNetWork() {}

	inline int run();
	void setData(void* data)
	{
		m_ptrData = data;
	}

private:
	string m_strWorkName;
	void* m_ptrData;
};

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

	CWorkThreadPool<CNetWork> m_threadPool;
};

