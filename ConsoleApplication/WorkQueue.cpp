#include "WorkQueue.h"

template<typename T>
CWorkQueue<T>::CWorkQueue()
{
	m_termination = false;
}

template<typename T>
CWorkQueue<T>::~CWorkQueue()
{

}

template<typename T>
void CWorkQueue<T>::push(T t)
{
	if (m_termination)
	{
		return;
	}
	shared_ptr<T> data(make_shared<T>(move(t)));
	lock_guard<mutex> lk(m_mutex);
	m_queue.push(data);
	m_condition.notify_one();
}

template<typename T>
bool CWorkQueue<T>::waitAndPop(T& t)
{
	unique_lock<mutex> lk(m_mutex);
	m_condition.wait(lk, [this] {
		return !m_queue.empty() || m_termination; 
	});

	if (!m_queue.empty())
	{
		t = move(*m_queue.front());
		m_queue.pop();
		return true;
	}

	return false;
}

template<typename T>
bool CWorkQueue<T>::tryPop(T& t)
{
	unique_lock<mutex> lk(m_mutex);
	if (m_queue.empty()) {
		return false;
	}
	t = move(*m_queue.front());
	m_queue.pop();
	return true;
}

template<typename T>
shared_ptr<T> CWorkQueue<T>::waitAndPop()
{
	unique_lock<mutex> lk(m_mutex);
	m_condition.wait(lk, [this] {
		return !m_queue.empty() || m_termination;
	});

	if (!m_queue.empty())
	{
		shared_ptr<T> ret = m_queue.front();
		m_queue.pop();
		return ret;
	}

	return nullptr;
}

template<typename T>
shared_ptr<T> CWorkQueue<T>::tryPop()
{
	unique_lock<mutex> lk(m_mutex);
	if (m_queue.empty()) {
		return nullptr;
	}
	shared_ptr<T> ret = m_queue.front();
	m_queue.pop();
	return ret;
}

template<typename T>
int CWorkQueue<T>::size()
{
	lock_guard<mutex> lk(m_mutex);
	return m_queue.size();
}

template<typename T>
bool CWorkQueue<T>::isEmpty()
{
	lock_guard<mutex> lk(m_mutex);
	return m_queue.empty();
}

template<typename T>
void CWorkQueue<T>::termination()
{
	lock_guard<mutex> lk(m_mutex);
	m_termination = true;
	m_condition.notify_all();
}

template<typename T>
bool CWorkQueue<T>::isTermination()
{
	return m_termination;
}


/*
#define NUM_THREADS 5

// 线程的运行函数
void* say_hello(void* args)
{
	CWorkQueue<int>* q = (CWorkQueue<int>*)args;
	while (true) {
		cout << pthread_self() << " : " << *q->waitAndPop() << endl;
	}
	return 0;
}
*/

/*
int main(int argc, char * argv[])
{
	cout << "test for CWorkQueue class" << endl;

	CWorkQueue<int> q;

	// 定义线程的 id 变量，多个变量使用数组
	pthread_t tids[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; ++i)
	{
		//参数依次是：创建的线程id，线程参数，调用的函数，传入的函数参数
		int ret = pthread_create(&tids[i], NULL, say_hello, &q);
		if (ret != 0)
		{
			cout << "pthread_create error: error_code=" << ret << endl;
		}
	}
	//等各个线程退出后，进程才结束，否则进程强制结束了，线程可能还没反应过来；
	//pthread_exit(NULL);

	for (int i = 0; i < 1000; i++)
	{
		q.push(i);
	}

	cout << "End" << endl;

	getchar();
	return 0;
}
*/