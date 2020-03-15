#pragma once
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <string>

using namespace std;

struct MSG
{
	MSG():_msg_count(0), _msg_str_message("")
	{
	}

	int _msg_count;
	string _msg_str_message;
};

template<typename T>
class CThread
{
public:
	CThread();
	~CThread();

	void run();
	int setWorkMethod(void *(*methodFunction)(void*), void *args = NULL);
	int setWorkMethod(T & work, void *args = NULL);
	int setSleepTimeForSeconds(int sec);
	void sendMessage(void *(dealMessageFunction)(MSG *), void *type = NULL);
	void stop();
	void wait();

private:
	static void * func_workFunction(void * args);
	void *(*m_WorkFunction)(void *);
	void *m_args;
	pthread_t m_thread;
	int m_count;
	int m_state;
	bool m_destroy;
	int m_sleepSeconds;
	MSG m_msg;
	T *m_work;
};

