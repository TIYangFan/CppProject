#include "KvSvr.h"
#include "KvTaskThreadPool.cpp"

template<typename T>
KvSvr<T>::KvSvr()
{

}

template<typename T>
KvSvr<T>::~KvSvr()
{

}

unsigned long long get_tid() {
	std::ostringstream oss;
	oss << std::this_thread::get_id();
	std::string stid = oss.str();
	unsigned long long tid = std::stoull(stid);
	return tid;
}

void accept_task(KvSvr<Task>* kvsvr)
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

		kvsvr->m_task_threadpool->enqueue_pending_queue(task);
	}

	printf("[DEBUG] accept thread has enqueued all tasks\n");
}

void deal_with_task(KvSvr<Task>* kvsvr)
{
	printf("[DEBUG] deal thread id: %lld start\n", get_tid());

	Task* task;
	kvsvr->m_task_threadpool->dequeue_pending_queue(task);
	printf("[DEBUG] deal thread get task %d\n", task->clientfd);

	printf("[DEBUG] task dealing start ----------------------\n");
	this_thread::sleep_for(chrono::milliseconds(2000));	// sleep 2 second
	printf("[DEBUG] task dealing end ----------------------\n");

	kvsvr->m_task_threadpool->enqueue_processed_queue(task);
	printf("[DEBUG] accept thread has enqueued task\n");
}

void send_task(KvSvr<Task>* kvsvr)
{
	printf("[DEBUG] send thread id: %lld start\n", get_tid());
	struct timeval tv;
	unsigned long total_time = 0;

	int i = 0;

	while (i < 200)
	{
		Task* task;
		kvsvr->m_task_threadpool->dequeue_processed_queue(task);

		gettimeofday(&tv, NULL);
		total_time += (tv.tv_sec * 1000000 + tv.tv_usec) - task->time_stamp;

		printf("[SEND] send thread get task %d deal by %lld\n", task->clientfd, task->thread_id);
		delete task;

		i++;
	}

	printf("total time %ld and average %ld\n", total_time, total_time / 100);
}

void work_func(void* args)
{
	printf("[DEBUG] work thread id: %lld start\n", get_tid());
	KvSvr<Task>* kvsvr = (KvSvr<Task>*)args;
	KvTaskThreadPool<Task>* thread_pool = kvsvr->m_task_threadpool;
	while (!thread_pool->m_shutdown)
	{
		Task* task;
		thread_pool->dequeue_pending_queue(task);
		//printf("[WORK] worker thread %lld get task %d\n", get_tid(), task->clientfd);

		//this_thread::sleep_for(chrono::milliseconds(100));	// deal with task
		task->thread_id = get_tid();

		/*
			Write Process:
			1. write cache(m_cache)
			2. enqueue write_persist queue then return
			3. task in queue achieve limit then batch write to disk (first data file then persist file)
			4. write db(m_db)
		*/
		if (true)
		{

		}


		//printf("[WORK] worker thread %lld finish task %d\n", get_tid(), task->clientfd);
		thread_pool->enqueue_processed_queue(task);
	}
}

int main(int argc, char * argv[])
{
	KvSvr<Task>* kvsvr = new KvSvr<Task>();
	KvTaskThreadPool<Task>* tp = new KvTaskThreadPool<Task>(5, work_func, kvsvr);

	kvsvr->m_task_threadpool = tp;
	tp->init();

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