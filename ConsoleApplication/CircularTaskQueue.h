#pragma once
#include <iostream>
#include <time.h>

#include "SharedMemory.h"
#define MAX_BUF_SIZE 19
#define MAX_NAME_SIZE 32

using namespace std;



struct Task
{
	// Header
	time_t timestamp;				// 8 byte
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
		size_t front_;
		size_t rear_;
	};

public:
	CircularTaskQueue();
	~CircularTaskQueue();

	void enqueue(const T task);
	bool dequeue(T* task);

	bool waitAndPop(T& t);
	bool tryPop(T& t);

	shared_ptr<T> waitAndPop();
	shared_ptr<T> tryPop();

	int size();
	bool isEmpty();

	void termination();
	bool isTermination();

private:
	int task_size_;
	int task_cnt_;

	T*				pQueue_;
	QueueHeader*	header_;
	void*			shmaddr_;
	void*			shmHanlde_;

};

