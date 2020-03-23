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

struct Header
{
	int length;
	int count;
	int put_index;
	int take_index;
};

struct Mutex
{
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;

	pthread_cond_t not_empty;
	pthread_cond_t not_full;
	pthread_condattr_t condattr;

	static struct Mutex* create()
	{
		struct Mutex* mm;
		mm = (Mutex*)mmap(NULL, sizeof(*mm), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
		memset(mm, 0x00, sizeof(*mm));
		return mm;
	}

	bool init()
	{
		pthread_mutexattr_init(&mutexattr);									// ��ʼ�� mutex ����
		pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);   // �޸�����Ϊ���̼乲��
		pthread_mutex_init(&mutex, &mutexattr);								// ��ʼ��һ�� mutex ��

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

	bool notifyNotEmpty()
	{
		pthread_cond_signal(&not_empty);
	}

	bool waitNotEmpty()
	{
		pthread_cond_wait(&not_empty, &mutex);
	}

	bool notifyNotFull()
	{
		pthread_cond_signal(&not_full);
	}

	bool waitNotFull()
	{
		pthread_cond_wait(&not_full, &mutex);
	}

	bool destroy()
	{
		pthread_mutexattr_destroy(&mutexattr);  // ���� mutex ���Զ���
		pthread_mutex_destroy(&mutex);          // ���� mutex ��

		pthread_condattr_destroy(&condattr);
		pthread_cond_destroy(&not_empty);
		pthread_cond_destroy(&not_full);
	}
};

struct Task
{
	// Task Header
	time_t		timestamp;			// 8 byte
	unsigned int serial_number;		// 4 byte
	char type;						// 1 byte

	// Task Body
	char buf[MAX_BUF_SIZE];			// 19 byte
};

template<typename T>
class CircularTaskQueue
{
public:
	CircularTaskQueue();
	~CircularTaskQueue();

	bool put(const T task);
	void enqueue(const T task);
	bool take(T& task);
	T dequeue();

	int				length_;
	int				count_;

private:
	int task_size_;
	int task_cnt_;

	int put_index;
	int take_index;

public:
	T*						queue_;
	struct Mutex*			lock_;
	struct Header*			header_;
	SharedMemory<Task>*		task_sm_;
	SharedMemory<Header>*	header_sm_;
};

