#include "SharedMemory.h"

template<typename T>
SharedMemory<T>::SharedMemory(char* path, int id)
{
	id_ = id;
	path_ = path;
	if ((key_ = ftok(path_, id_)) == -1)
	{
		printf("[ERROR] ftok error: %s(errno: %d)\n", strerror(errno), errno);
		return;
	}
}

template<typename T>
SharedMemory<T>::~SharedMemory()
{
	destroy_shm();
}

template<typename T>
int SharedMemory<T>::create_shm(int size)
{
	if ((shmid_ = shmget(key_, size, IPC_CREAT | IPC_EXCL | 0666)) == -1)
	{
		printf("[ERROR] create shm error: %s(errno: %d)\n", strerror(errno), errno);
		return -1;
	}
	return shmid_;
}

template<typename T>
int SharedMemory<T>::get_shm(int size)
{
	if ((shmid_ = shmget(key_, size, IPC_CREAT)) == -1)
	{
		printf("[ERROR] get shm error: %s(errno: %d)\n", strerror(errno), errno);
		return -1;
	}
	return shmid_;
}

template<typename T>
T* SharedMemory<T>::get_shm_ptr()
{
	return (T*)shmat(shmid_, NULL, 0);
}

template<typename T>
bool SharedMemory<T>::save_shm(T* memptr)
{
	if (shmdt(memptr) == -1)
	{
		printf("[ERROR] save shm error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}
	return true;
}

template<typename T>
bool SharedMemory<T>::destroy_shm()
{
	if ((shmctl(shmid_, IPC_RMID, NULL)) == -1)
	{
		printf("[ERROR] destroy shm error: %s(errno: %d)\n", strerror(errno), errno);
		return false;
	}
	return true;
}

int main(int argc, char * argv[])
{
	pid_t childPid = fork();

	if (childPid != 0)
	{
		printf("now we are in parent progress,pid=%d\n", (int)getpid());
		printf("now we are in parent progress,childPid = %d\n", childPid);

		SharedMemory<test>* sm = new SharedMemory<test>("/tmp", 66);
		test t;
		t.age = 10;

		printf("---------------save--------------\n");
		sm->create_shm(sizeof(t));
		test* ptr = sm->get_shm_ptr();
		ptr->age = 10;
		sm->save_shm(ptr);
		//delete sm;
	}
	else
	{
		printf("now we are in child progress,pid = %d\n", (int)getpid());
		printf("now we are in child progress,parentPid = %d\n", (int)getppid());

		SharedMemory<test>* sm1 = new SharedMemory<test>("/tmp", 66);

		sleep(1);
		printf("---------------get--------------\n");
		test t;
		sm1->get_shm(sizeof(t));
		test* ptr1 = sm1->get_shm_ptr();
		printf("age:%d", ptr1->age);
		sm1->save_shm(ptr1);
		delete sm1;
	}

	getchar();

	return 0;
}