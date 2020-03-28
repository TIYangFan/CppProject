#include "CReactor.h"

CReactor::CReactor()
{

}

CReactor::~CReactor()
{

}

bool CReactor::init(const char* ip, short nport)
{
	if (!create_server_listener(ip, nport))
	{
		printf("[DEBUG] epoll ctl del error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	printf("[DEBUG] init main thread: %d\n", getTid());

	m_acceptthread.reset(new thread(accept_thread_proc, this));

	for (auto& t : m_workerthreads)
	{
		t.reset(new thread(worker_thread_proc, this));
	}

	m_kvsvr = new CSimplekvsvr();	// kv server

	return true;
}

bool CReactor::uninit()
{
	m_bStop = true;
	m_acceptcond.notify_one();
	m_acceptcond.notify_all();

	m_acceptthread->join();
	for (auto& t : m_workerthreads)
	{
		t->join();
	}

	if (epoll_ctl(m_epollfd, EPOLL_CTL_DEL, m_listenfd, NULL) == -1)
	{
		printf("[ERROR] epoll ctl del error: %s(errno: %d)\n", strerror(errno), errno);
	}

	shutdown(m_listenfd, SHUT_RDWR);
	close(m_listenfd);
	close(m_epollfd);

	return true;
}

bool CReactor::close_client(int clientfd)
{
	if (epoll_ctl(m_epollfd, EPOLL_CTL_DEL, clientfd, NULL) == -1)
	{
		printf("[ERROR] close client error: %s(errno: %d)\n", strerror(errno), errno);
	}

	close(clientfd);
	return true;
}

void* CReactor::main_loop(void* p)
{
	printf("[DEBUG] main loop thread: %d\n", getTid());

	CReactor* pReactor = static_cast<CReactor*>(p);

	while (!pReactor->m_bStop)
	{
		struct epoll_event ev[1024];
		int n = epoll_wait(pReactor->m_epollfd, ev, 1024, 10);
		if (n == 0)
			continue;
		else if (n < 0)
		{
			printf("[ERROR] epoll wait error: %s(errno: %d)\n", strerror(errno), errno);
			continue;
		}

		int m = min(n, 1024);
		for (int i = 0; i < m; i++)
		{
			if (ev[i].data.fd == pReactor->m_listenfd)
				pReactor->m_acceptcond.notify_one();
			else
			{
				{
					unique_lock<mutex> guard(pReactor->m_workermutex);
					pReactor->m_listClients.push_back(ev[i].data.fd);
				}
				pReactor->m_workercond.notify_one();
			}
		}
	}

	printf("[DEBUG] main loop exit\n");

	return NULL;
}

bool CReactor::create_server_listener(const char* ip, short port)
{
	if ((m_listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
	{
		printf("[ERROR] create socket error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	int on = 1;
	if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)	// 设置重用地址,防止 Address already in use
	{
		printf("[ERROR] set reuse addr error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) == -1)	// 设置重用端口,防止 Port already in use
	{
		printf("[ERROR] set reuse port error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	if (bind(m_listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)	// 将本地地址绑定到所创建的套接字上
	{
		printf("[ERROR] bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	if (listen(m_listenfd, 50) == -1)	// 开始监听是否有客户端连接
	{
		printf("[ERROR] listen socket error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	if ((m_epollfd = epoll_create(1)) == -1)
	{
		printf("[ERROR] create epoll error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}
	;
	struct epoll_event e;
	memset(&e, 0, sizeof(e));
	e.events = EPOLLIN | EPOLLRDHUP;
	e.data.fd = m_listenfd;

	if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_listenfd, &e) == -1)
	{
		printf("[ERROR] epoll ctl error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	printf("create socket success\n");
	return true;
}

void CReactor::accept_thread_proc(CReactor* pReactor)
{
	printf("[DEBUG] accept thread: %d\n", this_thread::get_id());

	while (true)
	{
		int newfd;
		struct sockaddr_in clientaddr;
		socklen_t addrlen;
		{
			unique_lock<mutex> guard(pReactor->m_acceptmutex);
			pReactor->m_acceptcond.wait(guard);
			if (pReactor->m_bStop)
				break;

			newfd = accept(pReactor->m_listenfd, (struct sockaddr*)&clientaddr, &addrlen);
		}
		if (newfd == -1)
		{
			printf("[ERROR] accept error: %s(errno: %d)\n", strerror(errno), errno);
			continue;
		}

		printf("[DEBUG] new client connected: %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		int oldflag = fcntl(newfd, F_GETFL, 0);
		int newflag = oldflag | O_NONBLOCK;
		if (fcntl(newfd, F_SETFL, newflag) == -1)
		{
			printf("[ERROR] set fcntl error: %s(errno: %d)\n", strerror(errno), errno);
			continue;
		}

		struct epoll_event e;
		memset(&e, 0, sizeof(e));
		e.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
		e.data.fd = newfd;
		if (epoll_ctl(pReactor->m_epollfd, EPOLL_CTL_ADD, newfd, &e) == -1)
		{
			printf("[ERROR] epoll ctl error: %s(errno: %d)\n", strerror(errno), errno);
			continue;
		}
	}
}

void CReactor::worker_thread_proc(CReactor* pReactor)
{
	printf("[DEBUG] worker start thread: %d\n", getTid());

	while (true)
	{
		int clientfd;
		{
			unique_lock<mutex> guard(pReactor->m_workermutex);
			while (pReactor->m_listClients.empty())
			{
				if (pReactor->m_bStop)
				{
					printf("[DEBUG] worker exit thread: %d\n", getTid());
					return;
				}
				pReactor->m_workercond.wait(guard);
			}

			clientfd = pReactor->m_listClients.front();
			pReactor->m_listClients.pop_front();
		}

		string strclientmsg;
		char buff[256];
		bool bError = false;
		while (true)
		{
			memset(buff, 0, sizeof(buff));
			int nRecv = recv(clientfd, buff, 256, 0);
			if (nRecv == -1)
			{
				if (errno == EWOULDBLOCK)
					break;
				else
				{
					printf("[ERROR] recv error: %s(errno: %d)\n", strerror(errno), errno);
					pReactor->close_client(clientfd);
					bError = true;
					break;
				}
			}
			else if (nRecv == 0)
			{
				printf("[DEBUG] peer closed, client disconnected and fd: %d\n", clientfd);
				pReactor->close_client(clientfd);
				bError = true;
				break;
			}
			strclientmsg += buff;
		}

		if (bError)
			continue;

		printf("[INFO] get client msg: %s\n", strclientmsg.c_str());


		// Test kv server —— Start

		vector<string> msginfo;	// default capital 3
		string reply = "";
		splitWithStrtok(strclientmsg.erase(strclientmsg.length()-1).c_str(), " ", msginfo);	// delete /n
		if (msginfo[0] == "set")
		{
			printf("[DEBUG] client set key:%s val:%s\n", msginfo[1].c_str(), msginfo[2].c_str());
			pReactor->m_kvsvr->setValue(msginfo[1].c_str(), msginfo[2].c_str());
			reply = "set success";
		}
		else if (msginfo[0] == "get")
		{
			printf("[DEBUG] client get key:%s\n", msginfo[1].c_str());
			reply = pReactor->m_kvsvr->getValue(msginfo[1].c_str());
			if (reply.size() == 0)
			{
				reply = "Not Exist key.";
			}

			printf("[DEBUG] client get val:%s success\n", reply.c_str());
		}
		else if (msginfo[0] == "delete")
		{
			printf("[DEBUG] client get key:%s\n", msginfo[1].c_str());
			reply = pReactor->m_kvsvr->deleteValue(msginfo[1].c_str());
			printf("[DEBUG] client get val:%s success\n", reply.c_str());
		}
		else if (msginfo[0] == "stats")
		{
			// holding
			printf("[DEBUG] client get stats:%s\n", msginfo[1].c_str());
			Stats* stats = pReactor->m_kvsvr->getStats();
			reply = spliceStatsString(stats);
			printf("[DEBUG] client get stats:%s success\n", reply.c_str());
		}
		else if (msginfo[0] == "quit")
		{
			printf("[DEBUG] client quit\n");
			reply = "OK";
			send(clientfd, reply.c_str(), reply.length(), 0);
			pReactor->close_client(clientfd);
			printf("[DEBUG] quit success\n");
			continue;
		}

		// Test kv server —— End


		while (true)
		{
			int nSend = send(clientfd, reply.c_str(), reply.length(), 0);
			if (nSend == -1)
			{
				if (errno == EWOULDBLOCK)
				{
					this_thread::sleep_for(chrono::milliseconds(10));
					continue;
				}
				else
				{
					printf("[ERROR] send error: %s(errno: %d)\n", strerror(errno), errno);
					pReactor->close_client(clientfd);
					break;
				}
			}

			printf("[DEBUG] send msg:%s to client: %d\n", reply.c_str(),clientfd);

			reply.erase(0, nSend);
			if (reply.empty())
				break;
		}
	}
}

unsigned long long CReactor::getTid() {
	std::ostringstream oss;
	oss << std::this_thread::get_id();
	std::string stid = oss.str();
	unsigned long long tid = std::stoull(stid);
	return tid;
}

void CReactor::splitWithStrtok(const char* str, const char* delim, vector<string>& ret)
{
	char* strcopy = new char[strlen(str) + 1];
	strcpy(strcopy, str);
	char *word = strtok(strcopy, delim);
	ret.push_back(word);
	while (word = strtok(nullptr, delim))
		ret.push_back(word);
	delete[] strcopy;
}

string CReactor::spliceStatsString(Stats* stats)
{
	string ret = "";
	ret.append("count: ").append(to_string(stats->key_count)).append(", ")
		.append("mem: ").append(to_string(stats->mem_size)).append(", ")
		.append("file: ").append(to_string(stats->file_size)).append(", ")
		.append("hits: ").append(to_string(stats->hit_count)).append(", ")
		.append("misses: ").append(to_string(stats->miss_count)).append(", ")
		.append("cache-hits: ").append(to_string(stats->hit_count)).append(", ")
		.append("cache-misses: ").append(to_string(stats->miss_count));
	return ret;
}

/*
int main(int argc, char * argv[])
{
	CReactor reator;
	reator.init("0.0.0.0", 6666);
	reator.main_loop(&reator);
	getchar();
	
	return 0;
}
*/