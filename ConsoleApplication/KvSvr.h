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
	TODO����ҵ�����������һ������
	ҵ������������������д���������ٽ����ͨ������㷢��
	ע�������ֻ�����ҵ���Ķ�д���Žӿڣ�������Ҫ����ҵ���Ĳ����߼�
	���ʹ��������������ҵ����߼����н�������ж�д��������һ������ϵͳ����
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

	//list<T> m_task_queue;			// �������

	KvMemoryCache*			m_memory_cache;
	KvFileDatabase<T>*		m_file_db;

	list<T> m_write_disk_queue;		// ��д��������ݶ���
	list<T> m_persist_data_queue;	// ���־û�����

	shared_ptr<thread>		m_writerthread;							// io file writer
	shared_ptr<thread>		m_persistthread;						// io persist writer
	shared_ptr<thread>		m_readerthreads[READER_THREAD_NUM];		// io file reader

	KvTaskThreadPool<T>*	m_task_threadpool;
};

