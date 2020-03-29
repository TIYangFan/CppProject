#pragma once
#include <thread>
#include <sys/time.h>

#include "KvTaskQueue.h"

using namespace std;

template<typename T>
class KvTaskThreadPool
{
	typedef T* Tptr;

public:
	KvTaskThreadPool();
	KvTaskThreadPool(int thread_num, function<void(void*)> worker_func, void* worker_args);
	~KvTaskThreadPool();

	bool init();
	bool destroy();

	bool enqueue_pending_queue(Tptr task);
	bool dequeue_pending_queue(Tptr& task);

	bool enqueue_processed_queue(Tptr task);
	bool dequeue_processed_queue(Tptr& task);

//private:

	bool					m_shutdown;
	int						m_thread_num;
	void*					m_worker_args;
	function<void(void*)>	m_worker_func;
	shared_ptr<thread>*		m_worker_threads;

	KvTaskQueue<Tptr>*		m_pending_queue;			// �������������
	KvTaskQueue<Tptr>*		m_processed_queue;			// �Ѵ����������
};

