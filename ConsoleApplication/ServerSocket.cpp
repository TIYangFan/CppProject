#include "ServerSocket.h"
#include "WorkThreadPool.cpp"

#define MAXLINE 4096

CServerSocket::CServerSocket()
{
	char m_buff[MAXLINE];
	struct sockaddr_in m_servaddr;
	CWorkThreadPool<CNetWork> m_threadPool(10);
}

CServerSocket::~CServerSocket()
{
	disconnect();
}

int CServerSocket::openSocket()
{
	if ((m_listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("[INFO] create socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	printf("[INFO] create socket successfully %d\n", m_listenfd);

	memset(&m_servaddr, 0, sizeof(m_servaddr));
	m_servaddr.sin_family = AF_INET;
	m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_servaddr.sin_port = htons(6666);

	if (bind(m_listenfd, (struct sockaddr*)&m_servaddr, sizeof(m_servaddr)) == -1)
	{
		printf("[INFO] bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	printf("[INFO] bind socket successfully\n");

	if (listen(m_listenfd, 10) == -1)
	{
		printf("[INFO] listen socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	printf("[INFO] listen socket successfully\n");

	return 1;
}

int CServerSocket::dealWithMessage()
{
	printf("[INFO] begin deal message %d\n", m_listenfd);
	while (true)
	{
		printf("[INFO] begin accept message %d\n", m_listenfd);

		int connfd;
		if ((connfd = accept(m_listenfd, (struct sockaddr*)NULL, NULL)) == -1)
		{
			printf("[INFO] accpet socket error: %s(errno: %d)\n", strerror(errno), errno);
			continue;
		}

		/*
		char buff[MAXLINE];
		int n = recv(connfd, buff, MAXLINE, 0);
		buff[n] = '\0';
		printf("[MSG] recv msg from client: %s\n", buff);

		send(connfd, buff, strlen(buff), 0);
		close(connfd);
		*/

		printf("[DEBUG] get accept connfd %d\n", connfd);
		CNetWork work;
		work.setData((void*)connfd);
		m_threadPool.addTask(&work);
		
	}
	
	return 0;
}

void CServerSocket::disconnect()
{
	close(m_listenfd);
}

inline int CNetWork::run()
{
	long connfd = (long)m_ptrData;
	printf("[INFO] thread %d get conn %d\n", pthread_self(), connfd);

	char buff[MAXLINE];
	int n = recv(connfd, buff, MAXLINE, 0);
	buff[n] = '\0';
	printf("[MSG] thread %d recv msg from client: %s\n", pthread_self(), buff);

	send(connfd, buff, strlen(buff), 0);
	close(connfd);

	return 0;
}


int main(int argc, char * argv[])
{
	CServerSocket ss;
	ss.openSocket();
	ss.dealWithMessage();
	return 0;
}