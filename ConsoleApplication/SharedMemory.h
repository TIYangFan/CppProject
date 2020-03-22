#pragma once
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

using namespace std;

typedef struct {
	char name[20]{ 'y','a','n','g' };
	int age;
}test;

template<typename T>
class SharedMemory
{
public:
	SharedMemory(char* path, int id);
	~SharedMemory();

	int create_shm(int size);
	int get_shm(int size);
	T* get_shm_ptr();
	bool save_shm(T* memptr);

	bool destroy_shm();

private:
	int id_;
	int shmid_;
	key_t key_;
	char* path_;
};

