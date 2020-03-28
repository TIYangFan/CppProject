#include "KvSvr.h"
#include "WorkQueue.cpp"
#include "KvTaskQueue.cpp"

template<typename T>
KvSvr<T>::KvSvr()
{
	m_pending_queue = new KvTaskQueue<T>();
	m_processed_queue = new KvTaskQueue<T>();
}

template<typename T>
KvSvr<T>::~KvSvr()
{

}

template<typename T>
bool KvSvr<T>::enqueue_pending_queue(T task)
{
	m_pending_queue->enqueue(task);
}

template<typename T>
bool KvSvr<T>::dequeue_pending_queue(T& task)
{
	m_pending_queue->dequeue(task);
}

template<typename T>
bool KvSvr<T>::enqueue_processed_queue(T task)
{
	m_processed_queue->enqueue(task);
}

template<typename T>
bool KvSvr<T>::dequeue_processed_queue(T& task)
{
	m_processed_queue->dequeue(task);
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
	Task* task = new Task();
	task->clientfd = 1;

	printf("[DEBUG] accept thread id: %lld get task %d\n", get_tid(), task->clientfd);

	this_thread::sleep_for(chrono::milliseconds(1000));	// sleep 1 second

	kvsvr->enqueue_pending_queue(*task);
	printf("[DEBUG] accept thread has enqueued task\n");
}

void deal_with_task(KvSvr<Task>* kvsvr)
{
	printf("[DEBUG] deal thread id: %lld start\n", get_tid());

	Task* task = new Task();
	kvsvr->dequeue_pending_queue(*task);
	printf("[DEBUG] deal thread get task %d\n", task->clientfd);

	printf("[DEBUG] task dealing start ----------------------\n");
	this_thread::sleep_for(chrono::milliseconds(2000));	// sleep 2 second
	printf("[DEBUG] task dealing end ----------------------\n");

	kvsvr->enqueue_processed_queue(*task);
	printf("[DEBUG] accept thread has enqueued task\n");
}

void send_task(KvSvr<Task>* kvsvr)
{
	printf("[DEBUG] send thread id: %lld start\n", get_tid());

	Task* task = new Task();
	kvsvr->dequeue_processed_queue(*task);
	printf("[DEBUG] send thread get task %d\n", task->clientfd);
}

int main(int argc, char * argv[])
{
	KvSvr<Task>* kvsvr = new KvSvr<Task>();
	thread t1(accept_task, kvsvr);
	thread t2(deal_with_task, kvsvr);
	thread t3(send_task, kvsvr);

	t1.join();
	t2.join();
	t3.join();

	getchar();
	return 0;
}