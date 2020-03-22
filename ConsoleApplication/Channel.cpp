#include "Channel.h"

/*
const int Channel::noneEventFlag = 0;
const int Channel::readEventFlag = EPOLLIN | EPOLLPRI;
const int Channel::writeEventFlag = EPOLLOUT;
const int Channel::errorEventFlag = EPOLLERR;

Channel::Channel(EventLoop* loop, int fd)
	:fd_(fd)
{

}

Channel::~Channel()
{

}

void Channel::handleEvent(Timestamp receiveTime)
{
	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
	{
		if (closeHandle)
			closeHandle();
	}

	if (revents_ & EPOLLERR)
	{
		if (errorHandle)
			errorHandle();
	}
	if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
	{
		if (readHandle)
			readHandle();
	}
	if (revents_ & EPOLLOUT)
	{
		if (writeHandle)
			writeHandle();
	}
}

int Channel::fd() const
{
	return fd_;
}

int Channel::events() const
{
	return events_;
}

void Channel::set_revents(int revt)
{
	revents_ = revt;
}

void Channel::add_revents(int revt)
{

}

void remove_events()
{

}

void Channel::updateEvent()
{
	epoll_->mod_event(fd_, events_);
}

bool Channel::enableReading()
{
	events_ |= readEventFlag;
	updateEvent();
}

bool Channel::disableReading()
{
	events_ &= ~readEventFlag;
	updateEvent();
}

bool Channel::enableWriting()
{
	events_ |= writeEventFlag;
	updateEvent();
}

bool Channel::disableWriting()
{
	events_ &= ~writeEventFlag;
	updateEvent();
}

bool Channel::enableErrorEvent()
{
	events_ |= errorEventFlag;
	updateEvent();
}

bool Channel::disableErrorEvent()
{
	events_ &= ~errorEventFlag;
	updateEvent();
}

bool Channel::disableAll()
{
	events_ = noneEventFlag;
	updateEvent();
}

bool Channel::isNoneEvent() const
{
	// TODO: isNoneEvent
}

bool Channel::isReading() const
{ 
	return events_ & writeEventFlag;
}

bool Channel::isWriting() const 
{ 
	return events_ & writeEventFlag;
}
*/