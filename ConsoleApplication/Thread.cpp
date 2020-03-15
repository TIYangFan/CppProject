#include "Thread.h"

template<typename T>
CThread<T>::CThread():m_WorkFunction(NULL),m_args(NULL),m_state(0),m_count(0), m_sleepSeconds(2), m_destroy(false)
{
	m_msg._msg_count = 1;
	m_msg._msg_str_message = "this is a message";
}

template<typename T>
CThread<T>::~CThread()
{
	m_WorkFunction = NULL;
	m_args = NULL;
	m_state = 0;
}

template<typename T>
void CThread<T>::run()
{
	m_state = pthread_create(&m_thread, NULL, func_workFunction, this);
}

template<typename T>
void CThread<T>::wait()
{
	if (0 == m_state)
	{
		pthread_join(m_thread, NULL);
	}
}

template<typename T>
void * CThread<T>::func_workFunction(void * args)
{
	CThread<T> * ptrCThread = (CThread<T> *)args;
	while (NULL == ptrCThread->m_WorkFunction && NULL == ptrCThread->m_work && !ptrCThread->m_destroy)
	{
		cout << "the CThread do nothing, sleep " << ptrCThread->m_sleepSeconds << " seconds" << endl;
		sleep(ptrCThread->m_sleepSeconds);
	}
	if (NULL != ptrCThread->m_WorkFunction && !ptrCThread->m_destroy)
	{
		ptrCThread->m_WorkFunction(ptrCThread->m_args);
	}
	else if (NULL != ptrCThread->m_work && !ptrCThread->m_destroy)
	{
		ptrCThread->m_work->init(ptrCThread->m_args);
	}
}

template<typename T>
int CThread<T>::setWorkMethod(void *(*methodFunction)(void *), void * args)
{
	if (args != NULL)
	{
		m_args = args;
	}
	m_WorkFunction = methodFunction;
	
	return 0;
}

template<typename T>
int CThread<T>::setWorkMethod(T & work, void * args)
{
	if (args != NULL)
	{
		m_args = args;
	}
	m_work = &work;

	return 0;
}

template<typename T>
void CThread<T>::sendMessage(void *(dealMessageFunction)(MSG *), void * type)
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
int CThread<T>::setSleepTimeForSeconds(int sec)
{
	if (sec > 0)
	{
		m_sleepSeconds = sec;
	}
}

template<typename T>
void CThread<T>::stop()
{
	m_destroy = true;
}

void * CThread_work(void* args)
{
	for (int i = 0; i < 20; i++)
	{
		cout << "the CThread output the number:" << i << " " << (char*)args << endl;
	}
}

void * dealWithMessage(MSG* msg)
{
	MSG tmpMsg;
	tmpMsg._msg_count = msg->_msg_count;
	tmpMsg._msg_str_message = msg->_msg_str_message;

	cout << "[INFO]***get the message from the CThread***:" << tmpMsg._msg_str_message << endl;
}

void * CThread_work1(void* args)
{
	for (int i = 0; i < 40; i++)
	{
		cout << "another CThread output the number:" << i << " " << (char*)args << endl;
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

/*
int main(int argc, char * argv[])
{
	cout << "test for CThread class" << endl;
	CThread<Work> t;
	//CThread t1;

	Work work;

	t.run();
	//t1.run();

	//t1.setWorkMethod(CThread_work1, (void *)"t1");
	//t1.sendMessage(dealWithMessage);
	sleep(6);

	t.setWorkMethod(work, (void *)"t");
	
	t.wait();
	//t1.wait();
	cout << "the process ending" << endl;

	getchar();
	return 0;
}
*/