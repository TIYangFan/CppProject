#pragma once
#include <list>
#include <thread>
#include <mutex>
#include <iostream>
#include <unistd.h>
#include <condition_variable>

using namespace std;

template<typename T>
class KvTaskQueue
{
public:
	KvTaskQueue();
	~KvTaskQueue();

	bool enqueue(T task);
	bool dequeue(T& task);

private:
	list<T>*				m_queue;
	mutex					m_mutex;
	condition_variable		m_cond_not_full;
	condition_variable		m_cond_not_empty;
};

