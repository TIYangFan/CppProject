#pragma once
#include <vector>
#include <string>
#include <atomic>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "WorkQueue.cpp"

using namespace std;

class CWork
{
protected:
	string m_strWorkName;				// 任务名称
	void* m_ptrData;					// 要执行的任务的具体数据
public:
	CWork() {}
	CWork(string workName)
	{
		m_strWorkName = workName;
		m_ptrData = NULL;
	}
	virtual int run() = 0;
	void setData(void* data)
	{
		m_ptrData = data;
	}

public:
	virtual ~CWork() {}
};

template<typename T>
class CWorkThreadPool
{
public:
	CWorkThreadPool(int threadNum = 10);
	~CWorkThreadPool();

	static void * func_workFunction(void* args);
	int addTask(T *t);					// 把任务添加到任务队列中  
	int stopAll();						// 使线程池中的线程退出  
	int getTaskSize();					// 获取当前任务队列中的任务数 

private:
	CWorkQueue<T*> m_workQueue;			// 工作队列（队列内的元素为 " 任务指针 T* " 主要目的是为了实现多态重载）
	atomic<bool> m_shotdown;			// 线程池终止标志
	int m_threadNum;					// 线程池中线程数量
	vector<pthread_t> m_threadSet;		// 线程集合
};

