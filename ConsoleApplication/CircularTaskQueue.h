#pragma once
#include <iostream>
#include <time.h>
#include <atomic>
#include <errno.h>
#include <string>
#include <sys/mman.h>

#include "SharedMemory.h"
#define MAX_BUF_SIZE 19
#define MAX_NAME_SIZE 32

using namespace std;


struct mt
{
	int num;
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;

	pthread_cond_t not_empty;
	pthread_cond_t not_full;
	pthread_condattr_t condattr;

	bool init()
	{
		pthread_mutexattr_init(&mutexattr);									// 初始化 mutex 属性
		pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);   // 修改属性为进程间共享
		pthread_mutex_init(&mutex, &mutexattr);								// 初始化一把 mutex 锁

		pthread_condattr_init(&condattr);
		pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED);
		pthread_cond_init(&not_empty, &condattr);
		pthread_cond_init(&not_full, &condattr);
	}

	bool lock()
	{
		pthread_mutex_lock(&mutex);
	}

	bool unlock()
	{
		pthread_mutex_unlock(&mutex);
	}

	bool destroy()
	{
		pthread_mutexattr_destroy(&mutexattr);  // 销毁 mutex 属性对象
		pthread_mutex_destroy(&mutex);          // 销毁 mutex 锁
	}
};

struct Task
{
	// Header
	time_t		timestamp;			// 8 byte
	unsigned int serial_number;		// 4 byte
	char type;						// 1 byte

	// Body
	char buf[MAX_BUF_SIZE];			// 19 byte
};

template<typename T>
class CircularTaskQueue
{
	struct QueueHeader
	{
		char name[MAX_NAME_SIZE];
		unsigned int len;
		size_t size;
		atomic<size_t> front;
		atomic<size_t> rear;
	};

public:
	CircularTaskQueue();
	~CircularTaskQueue();

	bool put(const T task);
	void enqueue(const T task);
	bool take(T& task);
	T dequeue();

	bool dequeue(T* task);

	bool waitAndPop(T& t);
	bool tryPop(T& t);

	int size();
	bool isEmpty();
	bool isFull();

	atomic<size_t> getTail();
	atomic<size_t> getHead();

	void termination();
	bool isTermination();

	T*				queue_;
	struct mt*		lock_;
	int				length_;
	int				count_;

private:
	int task_size_;
	int task_cnt_;


	int				put_index;
	int				take_index;

	QueueHeader*	header_;
	void*			shmaddr_;
	void*			shmHanlde_;
};

