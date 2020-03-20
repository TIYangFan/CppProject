#pragma once
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>

#define MAX_BUFF_SIZE	1024
#define MAX_EVENTS		500
#define FD_SIZE			1000

class CEpoll
{
public:
	CEpoll();
	~CEpoll();


public:
	bool start(int port, bool isblock = false);
	
	int get_socketfd();

	void add_event(int fd, int state);
	void del_event(int fd, int state);
	void mod_event(int fd, int state);

	void handle_events(struct epoll_event *events, int num, char* buf, int& bufLen);
	bool handle_accept();

	bool do_epoll();
	bool do_read(int fd, char* buf, int& bufLen);
	bool do_write(int fd, char* buf, int& bufLen);

	char* get_errmsg();

private:
	int socketfd;
	int epollfd;
	char err_msg[256];
	struct sockaddr_in servaddr;
};

