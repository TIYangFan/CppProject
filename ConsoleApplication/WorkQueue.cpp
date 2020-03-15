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

int main(int argc, char * argv[])
{
	cout << "test for CWorkQueue class" << endl;

	CWorkQueue<int> q;
	q.push(1);

	cout << *q.waitAndPop() << endl;

	cout << "End" << endl;

	getchar();
	return 0;
}