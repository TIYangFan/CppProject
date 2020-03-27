#include "ClientSocket.h"

#define MAXLINE 4096

CClientSocket::CClientSocket()
{
	char m_recv_line[MAXLINE];
	char m_send_line[MAXLINE];
}

CClientSocket::~CClientSocket()
{
	
}

int CClientSocket::connectSocket(char* ip_addr)
{
	if ((m_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("[INFO] create socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	memset(&m_servaddr, 0, sizeof(m_servaddr));
	m_servaddr.sin_family = AF_INET;
	m_servaddr.sin_port = htons(6666);

	if (inet_pton(AF_INET, ip_addr, &m_servaddr.sin_addr) <= 0)
	{
		printf("[INFO] inet_pton error for %s \n", ip_addr);
		return 0;
	}

	if (connect(m_sockfd, (struct sockaddr*)&m_servaddr, sizeof(m_servaddr)) < 0)
	{
		printf("[INFO] connect socket error: %s(errno: %d)\n", strerror(errno), errno);
		return 0;
	}

	printf("[INFO] connect %s socket %d successfully\n", ip_addr, m_sockfd);

	return 1;
}

int CClientSocket::sendMessage()
{
	while (true)
	{
		printf("send msg to server: \n");

		char sendline[4096];
		fgets(sendline, MAXLINE, stdin);

		if (send(m_sockfd, sendline, strlen(sendline), 0) < 0)
		{
			printf("[INFO] send message error: %s(errno: %d)\n", strerror(errno), errno);
			return 0;
		}

		char buff[MAXLINE];
		int n = recv(m_sockfd, buff, MAXLINE, 0);
		buff[n] = '\0';
		printf("[MSG] recv msg from server: %s\n", buff);
	}

	close(m_sockfd);

	return 1;
}

/*
int main(int argc, char * argv[])
{
	CClientSocket cs;
	cs.connectSocket(argv[1]);
	cs.sendMessage();
	return 0;
}
*/