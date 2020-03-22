#include "Epoll.h"

CEpoll::CEpoll()
{
	socketfd = 0;
	memset(&servaddr, 0, sizeof(servaddr));
}

CEpoll::~CEpoll()
{

}

bool CEpoll::start(int port, bool isblock)
{
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		snprintf(err_msg, sizeof(err_msg), "[ERROR] create socket error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	// ϵͳ�Զ���ȡ����IP��ַ
	servaddr.sin_port = htons(port);

	if (!isblock)
	{
		int flags = fcntl(socketfd, F_GETFL, 0);
		fcntl(socketfd, F_SETFL, flags | O_NONBLOCK);	// ������
	}

	int on = 1;
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)	// �������õ�ַ,��ֹ Address already in use
	{
		snprintf(err_msg, sizeof(err_msg), "[ERROR] set reuse addr error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	if (bind(socketfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)	// �����ص�ַ�󶨵����������׽�����
	{
		snprintf(err_msg, sizeof(err_msg), "[ERROR] bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	if (listen(socketfd, 5) == -1)	// ��ʼ�����Ƿ��пͻ�������
	{
		snprintf(err_msg, sizeof(err_msg), "[ERROR] listen socket error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	printf("create socket success\n");
	return true;
}

int CEpoll::get_socketfd()
{
	return socketfd;
}

void CEpoll::add_event(int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;

	/*
		// ����� ET ģʽ������ EPOLLET
		ev.events |= EPOLLET;

		// �����Ƿ�����
		int flags = fcntl(fd, F_GETFL);
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	*/
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void CEpoll::del_event(int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

void CEpoll::mod_event(int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

void CEpoll::handle_events(struct epoll_event *events, int num, char* buf, int& bufLen)
{
	for (int i = 0; i < num; i++)
	{
		int fd = events[i].data.fd;
		if ((fd == socketfd) && (events[i].events & EPOLLIN))
		{
			handle_accept();
		}
		else if (events[i].events & EPOLLIN)
		{
			do_read(fd, buf, bufLen);
		}
		else if (events[i].events & EPOLLOUT)
		{
			do_write(fd, buf, bufLen);
		}
		else
		{
			close(fd);
		}
	}
}

bool CEpoll::handle_accept()
{
	int clifd;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrLen = sizeof(cliaddr);
	if ((clifd = accept(socketfd, (struct sockaddr*)&cliaddr, &cliaddrLen)) == -1)	// == ���ȼ����� =
	{
		snprintf(err_msg, sizeof(err_msg), "[ERROR] handle accept error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	char msg[128] = { 0 };
	sprintf(msg, "accept new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);

	printf("[DEBUG] msg:%s\n", msg);

	add_event(clifd, EPOLLIN);	// ��ӿͻ������������¼�
}

bool CEpoll::do_epoll()
{
	struct epoll_event events[MAX_EVENTS];
	int ret;
	char buf[MAX_BUFF_SIZE] = { 0 };
	int bufLen = 0;

	if ((epollfd = epoll_create(FD_SIZE)) == -1)	// ����������
	{
		snprintf(err_msg, sizeof(err_msg), "[ERROR] create epoll error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}

	add_event(socketfd, EPOLLIN);	// ��Ӽ����������¼�
	while (true)
	{
		// ��ȡ�Ѿ�׼���õ��������¼�
		/*
		���Ҫ���� read ��ʱ
		1������ socket ������
		2������ epoll_wait ��ʱ 1 ��
		3��ÿ�ν��� epoll_wait ֮ǰ�����������û��б��߳���ʱ��û��������û�

		ps:ÿ���û��������ݣ�read ֮����¸��û� last_request ʱ�䣬Ϊ������Ĳ��� 3 ��
		*/
		ret = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		handle_events(events, ret, buf, bufLen);
	}
	close(epollfd);
}
	
bool CEpoll::do_read(int fd, char* buf, int& bufLen)
{
	bufLen = read(fd, buf, MAX_BUFF_SIZE);
	if (bufLen == -1)
	{
		snprintf(err_msg, sizeof(err_msg), "[ERROR] read error: %s(errno: %d)\n", strerror(errno), errno);
		close(fd);
		del_event(fd, EPOLLIN);
		return false;
	}
	else if (bufLen == 0)
	{
		close(fd);
		printf("[DEBUG] client close.\n");
		del_event(fd, EPOLLIN);
		return true;
	}
	else
	{
		char msg[MAX_BUFF_SIZE] = { 0 };
		sprintf(msg, "[DEBUG] read message is: %s\n", buf);

		printf("[DEBUG] msg:%s\n", msg);
		
		mod_event(fd, EPOLLOUT);	// �޸���������Ӧ���¼����ɶ���Ϊд
	}
	return true;
}
	
bool CEpoll::do_write(int fd, char* buf, int& bufLen)
{
	int nwrite = write(fd, buf, bufLen);
	if (nwrite == -1)
	{
		snprintf(err_msg, sizeof(err_msg), "[ERROR] write error: %s(errno: %d)\n", strerror(errno), errno);
		close(fd);
		del_event(fd, EPOLLOUT);
		return false;
	}
	else
	{
		char msg[MAX_BUFF_SIZE] = { 0 };
		sprintf(msg, "[DEBUG] write message is: %s\n", buf);

		printf("[DEBUG] msg:%s\n", msg);

		mod_event(fd, EPOLLIN);	// �޸���������Ӧ���¼�����д��Ϊ��
	}
	memset(buf, 0, MAX_BUFF_SIZE);
	return true;
}
	
char* CEpoll::get_errmsg()
{
	return err_msg;
}

/*
int main(int argc, char * argv[])
{
	CEpoll cepoll;
	if (!cepoll.start(6666, false))
	{
		printf("%s\n", cepoll.get_errmsg());
	}
	cepoll.do_epoll();
	return 0;
}
*/