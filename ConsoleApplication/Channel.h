#pragma once

#include "EventLoop.h"
#include "InetAddress.h"
#include "Timestamp.h"

class Channel
{
public:
	Channel(EventLoop* loop, int fd);
	~Channel();

	void handleEvent(Timestamp receiveTime);

	int fd() const;
	int events() const;
	void set_revents(int revt);
	void add_revents(int revt);
	void remove_events();
	bool isNoneEvent() const;

	bool enableReading();
	bool disableReading();
	bool enableWriting();
	bool disableWriting();
	bool disableAll();

	bool isReading() const { return events_ & kReadEvent; }
	bool isWriting() const { return events_ & kWriteEvent; }

private:
	// should use bit to save event
	enum EventE
	{
		kReadEvent,
		kWriteEvent
	};

	const int	fd_;		// socketfd
	int			events_;	//当前需要检测的事件
	int			revents_;	//处理后的事件
};

