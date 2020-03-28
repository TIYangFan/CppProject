#include "KvTaskThreadPool.h"
#include "WorkQueue.cpp"
#include "KvTaskQueue.cpp"

unsigned long long get_tid() {
	std::ostringstream oss;
	oss << std::this_thread::get_id();
	std::string stid = oss.str();
	unsigned long long tid = std::stoull(stid);
	return tid;
}

template<typename T>
KvTaskThreadPool<T>::KvTaskThreadPool()
{
	m_pending_queue = new KvTaskQueue<Tptr>();
	m_processed_queue = new KvTaskQueue<Tptr>();
}

template<typename T>
KvTaskThreadPool<T>::KvTaskThreadPool(int thread_num, function<void(void*)> worker_func)
{
	m_shutdown = false;
	m_thread_num = thread_num;
	m_worker_threads = new shared_ptr<thread>[thread_num];
	m_worker_func = worker_func;

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
		m_worker_threads[i].reset(new thread(m_worker_func, this));
	}
}

template<typename T>
bool KvTaskThreadPool<T>::destroy()
{
	for (int i = 0; i < m_thread_num; i++)
	{
		printf("[DEBUG] send thread id: %lld destroy\n", get_tid());
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

void accept_task(KvTaskThreadPool<Task>* kvsvr)
{
	//printf("[DEBUG] accept thread id: %lld get task %d\n", get_tid(), task->clientfd);

	struct timeval tv;

	for (int i = 0; i < 100; i++)
	{
		Task* task = new Task();
		task->clientfd = i;
		this_thread::sleep_for(chrono::milliseconds(10));

		// time stamp test delay
		gettimeofday(&tv, NULL);
		task->time_stamp = tv.tv_sec * 1000000 + tv.tv_usec;

		kvsvr->enqueue_pending_queue(task);
	}

	printf("[DEBUG] accept thread has enqueued all tasks\n");
}

void deal_with_task(KvTaskThreadPool<Task>* kvsvr)
{
	printf("[DEBUG] deal thread id: %lld start\n", get_tid());

	Task* task;
	kvsvr->dequeue_pending_queue(task);
	printf("[DEBUG] deal thread get task %d\n", task->clientfd);

	printf("[DEBUG] task dealing start ----------------------\n");
	this_thread::sleep_for(chrono::milliseconds(2000));	// sleep 2 second
	printf("[DEBUG] task dealing end ----------------------\n");

	kvsvr->enqueue_processed_queue(task);
	printf("[DEBUG] accept thread has enqueued task\n");
}

void send_task(KvTaskThreadPool<Task>* kvsvr)
{
	printf("[DEBUG] send thread id: %lld start\n", get_tid());
	struct timeval tv;
	unsigned long total_time = 0;

	int i = 0;

	while (i < 200)
	{
		Task* task;
		kvsvr->dequeue_processed_queue(task);

		gettimeofday(&tv, NULL);
		total_time += (tv.tv_sec * 1000000 + tv.tv_usec) - task->time_stamp;

		printf("[SEND] send thread get task %d deal by %lld\n", task->clientfd, task->thread_id);
		delete task;

		i++;
	}

	printf("total time %ld and average %ld\n", total_time, total_time/100);
}

void work_func(void* args)
{
	printf("[DEBUG] work thread id: %lld start\n", get_tid());
	KvTaskThreadPool<Task>* thread_pool = (KvTaskThreadPool<Task>*)args;
	while (!thread_pool->m_shutdown)
	{
		Task* task;
		thread_pool->dequeue_pending_queue(task);
		//printf("[WORK] worker thread %lld get task %d\n", get_tid(), task->clientfd);

		//this_thread::sleep_for(chrono::milliseconds(100));	// deal with task
		task->thread_id = get_tid();

		//printf("[WORK] worker thread %lld finish task %d\n", get_tid(), task->clientfd);
		thread_pool->enqueue_processed_queue(task);
	}
}

int main(int argc, char * argv[])
{
	KvTaskThreadPool<Task>* kvsvr = new KvTaskThreadPool<Task>(5, work_func);
	kvsvr->init();

	thread t1(accept_task, kvsvr);
	thread t2(accept_task, kvsvr);
	thread t3(accept_task, kvsvr);

	thread t4(send_task, kvsvr);

	t1.join();
	t2.join();
	t3.join();
	t4.join();

	getchar();
	return 0;
}