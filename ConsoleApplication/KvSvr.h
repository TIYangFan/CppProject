#pragma once
#include <list>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <iostream>
#include <sstream>

#include "WorkQueue.h"
#include "KvMemoryCache.h"
#include "KvFileDatabase.h"
#include "KvTaskQueue.h"
#include "KvTaskThreadPool.h"

#define READER_THREAD_NUM 5

/*
	TODO：将业务层与网络层进一步解耦
	业务层从网络层接收请求进行处理，处理后再将结果通过网络层发送
	注：网络层只需调用业务层的读写开放接口，而不需要关心业务层的操作逻辑
	充分使用阻塞队列来对业务层逻辑进行解耦，并进行读写分离来进一步提升系统性能
*/

struct Task
{
	unsigned long time_stamp;
	int clientfd;
	char type;
	char* key;
	char* val;

	unsigned long long thread_id;

	// continue..
};

template<typename T>
class KvSvr
{
public:
	KvSvr();
	~KvSvr();

	bool read();
	bool write();

//private:

	bool deal_with_task();
	bool finish_task();				// should 


//private:

	//list<T> m_task_queue;			// 任务队列

	KvMemoryCache*			m_memory_cache;
	KvFileDatabase<T>*		m_file_db;

	list<T> m_write_disk_queue;		// 待写入磁盘数据队列
	list<T> m_persist_data_queue;	// 待持久化队列

	shared_ptr<thread>		m_writerthread;							// io file writer
	shared_ptr<thread>		m_persistthread;						// io persist writer
	shared_ptr<thread>		m_readerthreads[READER_THREAD_NUM];		// io file reader

	KvTaskThreadPool<T>*	m_task_threadpool;
};

