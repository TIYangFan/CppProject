#include "KvTaskQueue.h"

template<typename T>
KvTaskQueue<T>::KvTaskQueue()
{
	m_queue = new list<T>();
}

template<typename T>
KvTaskQueue<T>::~KvTaskQueue()
{
	delete m_queue;
}

template<typename T>
bool KvTaskQueue<T>::enqueue(T task)
{
	unique_lock<mutex> guard(m_mutex);
	m_queue->push_back(task);
	m_cond_not_empty.notify_one();
}

template<typename T>
bool KvTaskQueue<T>::dequeue(T& task)
{
	unique_lock<mutex> guard(m_mutex);
	while (m_queue->empty())
	{
		m_cond_not_empty.wait(guard);
	}
	task = m_queue->front();
	m_queue->pop_front();
}