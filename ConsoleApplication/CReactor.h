#pragma once
#include <list>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <unistd.h>

#include "Simplekvsvr.h"

#define WORKER_THREAD_NUM 5

using namespace std;

class CReactor
{
public:
	CReactor();
	~CReactor();

	bool init(const char* ip, short nport);
	bool uninit();

	bool close_client(int clientfd);

	static void* main_loop(void* p);

private:
	CReactor(const CReactor& rhs);
	CReactor& operator = (const CReactor& rhs);

	bool create_server_listener(const char* ip, short port);

	static void accept_thread_proc(CReactor* pReactor);
	static void worker_thread_proc(CReactor* pReactor);

	static unsigned long long getTid();
	static void splitWithStrtok(const char* str, const char* delim, vector<string>& ret);
	static string spliceStatsString(Stats* stats);

private:
	int						m_listenfd = 0;
	int						m_epollfd = 0;
	bool					m_bStop = false;

	shared_ptr<thread>		m_acceptthread;
	shared_ptr<thread>		m_workerthreads[WORKER_THREAD_NUM];

	mutex					m_acceptmutex;
	condition_variable		m_acceptcond;

	mutex					m_workermutex;
	condition_variable		m_workercond;

	list<int>				m_listClients;
	CSimplekvsvr*			m_kvsvr;
};

