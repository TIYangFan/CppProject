#include "Epoll.h"

CEpoll::CEpoll()
{
	socketfd = 0;
	memset(&servaddr, 0, sizeof(servaddr));
}

CEpoll::~CEpoll()
{

}

bool CEpoll::start(int port, bool isblock = false)
{

}

int CEpoll::get_socketfd()
{

}

void CEpoll::add_event(int fd, int state)
{

}

void CEpoll::del_event(int fd, int state)
{

}

void CEpoll::mod_event(int fd, int state)
{

}

void CEpoll::handle_events(struct epoll_event *events, int num, char* buf, int& bufLen)
{

}

bool CEpoll::handle_accept()
{

}

bool CEpoll::do_epoll()
{

}
	
bool CEpoll::do_read(int fd, char* buf, int& bufLen)
{

}
	
bool CEpoll::do_write(int fd, char* buf, int& bufLen)
{

}
	
char* CEpoll::get_errmsg()
{

}