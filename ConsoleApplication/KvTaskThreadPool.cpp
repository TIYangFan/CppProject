#include "KvTaskThreadPool.h"
#include "WorkQueue.cpp"
#include "KvTaskQueue.cpp"

template<typename T>
KvTaskThreadPool<T>::KvTaskThreadPool()
{
	m_pending_queue = new KvTaskQueue<Tptr>();
	m_processed_queue = new KvTaskQueue<Tptr>();
}

template<typename T>
KvTaskThreadPool<T>::KvTaskThreadPool(int thread_num, function<void(void*)> worker_func, void* worker_args)
{
	m_shutdown = false;
	m_thread_num = thread_num;
	m_worker_threads = new shared_ptr<thread>[thread_num];
	m_worker_func = worker_func;
	m_worker_args = worker_args;

	m_pending_queue = new KvTaskQueue<Tptr>();
	m_processed_queue = new KvTaskQueue<Tptr>();
}

template<typename T>
KvTaskThreadPool<T>::~KvTaskThreadPool()
{
	destroy();
}

template<typename T>
bool KvTaskThreadPool<T>::init()
{
	for (int i = 0; i < m_thread_num; i++)
	{
		m_worker_threads[i].reset(new thread(m_worker_func, m_worker_args));
	}
}

template<typename T>
bool KvTaskThreadPool<T>::destroy()
{
	for (int i = 0; i < m_thread_num; i++)
	{
		m_worker_threads[i]->join();
	}
}

template<typename T>
bool KvTaskThreadPool<T>::enqueue_pending_queue(Tptr task)
{
	m_pending_queue->enqueue(task);
}

template<typename T>
bool KvTaskThreadPool<T>::dequeue_pending_queue(Tptr& task)
{
	m_pending_queue->dequeue(task);
}

template<typename T>
bool KvTaskThreadPool<T>::enqueue_processed_queue(Tptr task)
{
	m_processed_queue->enqueue(task);
}

template<typename T>
bool KvTaskThreadPool<T>::dequeue_processed_queue(Tptr& task)
{
	m_processed_queue->dequeue(task);
}