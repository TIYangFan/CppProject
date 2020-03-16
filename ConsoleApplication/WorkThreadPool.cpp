#include "WorkThreadPool.h"

/*
void CWork::setData(void * data)
{
	m_ptrData = data;
}
*/

class CMyWork : public CWork
{
public:
	CMyWork() {}

	inline int run()
	{
		cout << "Thread " << pthread_self() << (char*)this->m_ptrData << endl;
		sleep(1);
		return 0;
	}
};

template<typename T>
CWorkThreadPool<T>::CWorkThreadPool(int threadNum)
{
	m_threadNum = threadNum;
	cout << "[INFO] Create " << threadNum << " threads" << endl;
	
	CWorkQueue<T*> m_workQueue;
	vector<pthread_t> m_threadSet(threadNum);
	m_shotdown = false;

	for (int i = 0; i < threadNum; i++)
	{
		pthread_create(&m_threadSet[i], NULL, func_workFunction, this);
	}
}

template<typename T>
CWorkThreadPool<T>::~CWorkThreadPool()
{

}

template<typename T>
int CWorkThreadPool<T>::addTask(T* t)
{
	m_workQueue.push(t);
}

template<typename T>
int CWorkThreadPool<T>::stopAll()
{
	if (m_shotdown)
	{
		return -1;
	}
	m_shotdown = true;
}

template<typename T>
int CWorkThreadPool<T>::getTaskSize()
{
	return m_workQueue.size();
}

template<typename T>
void * CWorkThreadPool<T>::func_workFunction(void* args)
{
	cout << "[INFO] Thread " << pthread_self() << "begin to work" << endl;
	CWorkThreadPool* threadPool = (CWorkThreadPool*)args;
	while (!threadPool->m_shotdown)
	{
		cout << "[DEBUG] current work queue " << endl;

		T* work = *threadPool->m_workQueue.waitAndPop();

		//cout << "work: " << work << endl;
		work->run();
	}
}

/*
int main(int argc, char * argv[])
{
	CMyWork taskObj;
	char szTmp[] = "this is the first thread running";
	taskObj.setData((void*)szTmp);
	CWorkThreadPool<CMyWork> threadPool(10);

	for (int i = 0; i < 20; i++)
	{
		threadPool.addTask(&taskObj);
	}

	while (true)
	{
		cout << "there are still " << threadPool.getTaskSize() << " tasks need to handle" << endl;
		if (threadPool.getTaskSize() == 0)
		{
			if (threadPool.stopAll() == -1)
			{
				printf("Now I will exit from main/n");
				//exit(0);
				break;
			}
		}
		sleep(2);
	}

	getchar();
	return 0;
}
*/