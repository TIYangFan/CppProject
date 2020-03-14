#include "Thread.h"

template<typename T>
Thread<T>::Thread():m_WorkFunction(NULL),m_args(NULL),m_state(0),m_count(0), m_sleepSeconds(2), m_destroy(false)
{
	m_msg._msg_count = 1;
	m_msg._msg_str_message = "this is a message";
}

template<typename T>
Thread<T>::~Thread()
{
	m_WorkFunction = NULL;
	m_args = NULL;
	m_state = 0;
}

template<typename T>
void Thread<T>::run()
{
	m_state = pthread_create(&m_thread, NULL, func_workFunction, this);
}

template<typename T>
void Thread<T>::wait()
{
	if (0 == m_state)
	{
		pthread_join(m_thread, NULL);
	}
}

template<typename T>
void * Thread<T>::func_workFunction(void * args)
{
	Thread<T> * ptrThread = (Thread<T> *)args;
	while (NULL == ptrThread->m_WorkFunction && NULL == ptrThread->m_work && !ptrThread->m_destroy)
	{
		cout << "the thread do nothing, sleep " << ptrThread->m_sleepSeconds << " seconds" << endl;
		sleep(ptrThread->m_sleepSeconds);
	}
	if (NULL != ptrThread->m_WorkFunction && !ptrThread->m_destroy)
	{
		ptrThread->m_WorkFunction(ptrThread->m_args);
	}
	else if (NULL != ptrThread->m_work && !ptrThread->m_destroy)
	{
		ptrThread->m_work->init(ptrThread->m_args);
	}
}

template<typename T>
int Thread<T>::setWorkMethod(void *(*methodFunction)(void *), void * args)
{
	if (args != NULL)
	{
		m_args = args;
	}
	m_WorkFunction = methodFunction;
	
	return 0;
}

template<typename T>
int Thread<T>::setWorkMethod(T & work, void * args)
{
	if (args != NULL)
	{
		m_args = args;
	}
	m_work = &work;

	return 0;
}

template<typename T>
void Thread<T>::sendMessage(void *(dealMessageFunction)(MSG *), void * type)
{
	if (!m_msg._msg_str_message.empty())
	{
		if (NULL == type)
		{
			dealMessageFunction(&m_msg);
		}
		else
		{

		}
	}
}

template<typename T>
int Thread<T>::setSleepTimeForSeconds(int sec)
{
	if (sec > 0)
	{
		m_sleepSeconds = sec;
	}
}

template<typename T>
void Thread<T>::stop()
{
	m_destroy = true;
}

void * thread_work(void* args)
{
	for (int i = 0; i < 20; i++)
	{
		cout << "the thread output the number:" << i << " " << (char*)args << endl;
	}
}

void * dealWithMessage(MSG* msg)
{
	MSG tmpMsg;
	tmpMsg._msg_count = msg->_msg_count;
	tmpMsg._msg_str_message = msg->_msg_str_message;

	cout << "[INFO]***get the message from the thread***:" << tmpMsg._msg_str_message << endl;
}

void * thread_work1(void* args)
{
	for (int i = 0; i < 40; i++)
	{
		cout << "another thread output the number:" << i << " " << (char*)args << endl;
	}
}

class Work
{
public:
	Work();
	~Work();
	void init(void *);
};
Work::Work() {}
Work::~Work(){}
void Work::init(void * args)
{
	cout << "this is the class function result: " << (char*)args << endl;
}

int main(int argc, char * argv[])
{
	cout << "test for thread class" << endl;
	Thread<Work> t;
	//Thread t1;

	Work work;

	t.run();
	//t1.run();

	//t1.setWorkMethod(thread_work1, (void *)"t1");
	//t1.sendMessage(dealWithMessage);
	sleep(6);

	t.setWorkMethod(work, (void *)"t");
	
	t.wait();
	//t1.wait();
	cout << "the process ending" << endl;

	getchar();
	return 0;
}