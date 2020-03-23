#include "CircularTaskQueue.h"

/*
struct mta
{
	int num;
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;
	pthread_cond_t cond;
	pthread_condattr_t condattr;
};

struct header
{
	int length;
	int count;
	int put_index;
	int take_index;
};

int main(int argc, char * argv[])
{

	struct mta* mm;
	mm = (mta*)mmap(NULL, sizeof(*mm), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
	memset(mm, 0x00, sizeof(*mm));

	pthread_mutexattr_init(&mm->mutexattr);												// 初始化 mutex 属性
	pthread_mutexattr_setpshared(&mm->mutexattr, PTHREAD_PROCESS_SHARED);               // 修改属性为进程间共享
	pthread_mutex_init(&mm->mutex, &mm->mutexattr);

	pthread_condattr_init(&mm->condattr);
	pthread_condattr_setpshared(&mm->condattr, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(&mm->cond, &mm->condattr);

	pid_t childPid = fork();

	if (childPid != 0)
	{
		printf("now we are in parent progress,pid=%d\n", (int)getpid());
		printf("now we are in parent progress,childPid = %d\n", childPid);

		SharedMemory<test>* sm = new SharedMemory<test>("/tmp", 66);
		SharedMemory<header>* sm2 = new SharedMemory<header>("/tmp", 67);

		printf("---------------save--------------\n");
		test t;
		sm->get_shm(sizeof(test)*10);
		header h;
		sm2->get_shm(sizeof(header));

		sleep(2);

		pthread_mutex_lock(&mm->mutex);
		printf("Parent Lock\n");

		test* ptr = sm->get_shm_ptr();

		ptr[1].age = 10;
		ptr[2].age = 12;

		header* ptr2 = sm2->get_shm_ptr();
		ptr2->count = 2;

		//ptr->age = 10;
		sm->save_shm(ptr);
		sm2->save_shm(ptr2);
		//delete sm;

		sleep(2);

		printf("Parent Cond signal A\n");
		pthread_cond_signal(&mm->cond);
		printf("Parent Cond signal B\n");

		pthread_mutex_unlock(&mm->mutex);
		printf("Parent unLock\n");
	}
	else
	{
		printf("now we are in child progress,pid = %d\n", (int)getpid());
		printf("now we are in child progress,parentPid = %d\n", (int)getppid());

		SharedMemory<test>* sm1 = new SharedMemory<test>("/tmp", 66);
		SharedMemory<header>* sm3 = new SharedMemory<header>("/tmp", 67);

		pthread_mutex_lock(&mm->mutex);
		printf("Child Lock\n");

		printf("Child Cond wait A\n");
		pthread_cond_wait(&mm->cond, &mm->mutex);
		printf("Child Cond wait B\n");


		//sleep(1);
		printf("---------------get--------------\n");
		test t;
		sm1->get_shm(sizeof(test) * 10);
		header h;
		sm3->get_shm(sizeof(header));

		test* ptr1 = sm1->get_shm_ptr();

		header* ptr2 = sm3->get_shm_ptr();

		printf("age:%d %d\n", ptr1[1].age, ptr1[2].age);

		printf("count: %d\n", ptr2->count);

		sm1->save_shm(ptr1);
		delete sm1;
		delete sm3;

		pthread_mutex_unlock(&mm->mutex);
		printf("Child unLock\n");
	}

	pthread_mutexattr_destroy(&mm->mutexattr);  // 销毁 mutex 属性对象
	pthread_mutex_destroy(&mm->mutex);          // 销毁 mutex 锁

	pthread_condattr_destroy(&mm->condattr);
	pthread_cond_destroy(&mm->cond);

	getchar();

	return 0;
}
*/