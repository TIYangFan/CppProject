#pragma once
#include <sys/epoll.h>

#include "Epoll.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Timestamp.h"

/*
class Channel
{
public:
	static const int noneEventFlag;
	static const int readEventFlag;
	static const int writeEventFlag;
	static const int errorEventFlag;

	Channel(EventLoop* loop, int fd);
	~Channel();

	void handleEvent(Timestamp receiveTime);

	int fd() const;
	int events() const;
	void set_revents(int revt);
	void add_revents(int revt);
	void remove_events();

	bool enableReading();
	bool disableReading();
	bool enableWriting();
	bool disableWriting();
	bool enableErrorEvent();
	bool disableErrorEvent();
	bool disableAll();
	void updateEvent();

	bool isReading() const;
	bool isWriting() const;
	bool isNoneEvent() const;

private:
	CEpoll*			epoll_;
	EventLoop*		eventLoop_;
	
	const int		fd_;		// socketfd
	int				events_;	// 当前需要检测的事件
	int				revents_;	// 处理后的事件
};
*/
