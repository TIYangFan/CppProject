#pragma once
#include <iostream>
#include <string>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <memory>
#include <atomic>

using namespace std;

template<typename T>
class CWorkQueue
{
public:
	CWorkQueue();
	~CWorkQueue();

	void push(T t);

	bool waitAndPop(T& t);
	bool tryPop(T& t);

	shared_ptr<T> waitAndPop();
	shared_ptr<T> tryPop();

	int size();
	bool isEmpty();

	void termination();
	bool isTermination();


private:
	queue<shared_ptr<T>> m_queue;
	mutex m_mutex;
	condition_variable m_condition;
	atomic<bool> m_termination;
};

