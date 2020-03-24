#include "LockFreeCircularTaskQueue.h"
#include "SharedMemory.cpp"

template<typename T>
LockFreeCircularTaskQueue<T>::LockFreeCircularTaskQueue()
{
	lock_ = Mutex::create();
	lock_->init();

	header_ = new Header();
	task_sm_ = new SharedMemory<Task>("/tmp", 66);
	header_sm_ = new SharedMemory<Header>("/tmp", 67);

	task_sm_->get_shm(sizeof(T) * 10);
	queue_ = task_sm_->get_shm_ptr();
	memset(queue_, 0x00, sizeof(T) * 10);

	header_sm_->get_shm(sizeof(Header));
	header_ = header_sm_->get_shm_ptr();
	memset(header_, 0x00, sizeof(Header));
	header_->length = 10;
}

template<typename T>
LockFreeCircularTaskQueue<T>::~LockFreeCircularTaskQueue()
{
	task_sm_->destroy_shm();
	header_sm_->destroy_shm();
}

template<typename T>
bool LockFreeCircularTaskQueue<T>::put(const T task)
{

}

template<typename T>
void LockFreeCircularTaskQueue<T>::enqueue(const T task)
{
	int tail = 0;
	int new_tail = 0;

	do {
		tail = header_->tail;
		new_tail = tail + 1;
		if (new_tail == header_->length)
			new_tail = 0;

	} while (!header_->tail.compare_exchange_weak(tail, new_tail));

	queue_[tail] = task;
	header_->count++;
}

template<typename T>
bool LockFreeCircularTaskQueue<T>::take(T& task)
{

}

template<typename T>
T LockFreeCircularTaskQueue<T>::dequeue()
{
	T task;
	int head = 0;
	int new_head = 0;

	do {
		head = header_->head;
		new_head = head + 1;
		if (new_head == header_->length)
			new_head = 0;

	} while (!header_->head.compare_exchange_weak(head, new_head));
	task = queue_[head];
	header_->count--;

	/*
	if (header_->count > 0)
	{
		task = queue_[header_->head];
		header_->head++;
		header_->count--;
	}
	*/
	
	return task;
}

class Node
{
public:
	atomic<int> val;
};


int main(int argc, char * argv[])
{

	LockFreeCircularTaskQueue<Task>* lf = new LockFreeCircularTaskQueue<Task>();
	pid_t childPid = fork();

	if (childPid != 0)
	{
		printf("now we are in parent progress,pid=%d\n", (int)getpid());
		printf("now we are in parent progress,childPid = %d\n", childPid);

		Task* task = new Task();
		task->serial_number = 13;
		lf->enqueue(*task);
		printf("Parent put task\n");

		sleep(2);
		printf("Parent begin get task\n");
		Task task1 = lf->dequeue();
		printf("Parent get task %d\n", task1.serial_number);

		getchar();
	}
	else
	{
		printf("now we are in child progress,pid = %d\n", (int)getpid());
		printf("now we are in child progress,parentPid = %d\n", (int)getppid());

		sleep(1);
		printf("Child begin get task\n");
		Task task = lf->dequeue();
		printf("Child get task %d\n", task.serial_number);

		Task* task1 = new Task();
		task1->serial_number = 22;
		lf->enqueue(*task1);
		printf("\nChild put task\n");

		getchar();
	}

	getchar();
	return 0;
}