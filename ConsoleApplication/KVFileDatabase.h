#pragma once
#include <map>
#include <stdlib.h>
#include <string.h>

#include "KvTaskQueue.h"
#include "KvFileOperation.h"

using namespace std;

// Note: Reading files from multiple threads does not improve performance. The bottleneck in reading files is disk IO.
// Note: So read and write using single thread.

/*
	Write Process:
	1. write cache(m_cache)
	2. enqueue write_persist queue then return
	3. task in queue achieve limit then batch write to disk (first data file then persist file)
	4. write db(m_db)
*/

struct ptrCmp
{
	bool operator()(const char * s1, const char * s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};

struct Task
{

};

template<typename T>
class KvFileDatabase
{
	typedef T* Tptr;

public:
	KvFileDatabase();
	~KvFileDatabase();

	const char* get_val(const char* key);
	bool set_val(const char* key, const char* val);
	bool insert_val(const char* key, const char* val);
	bool update_val(const char* key, const char* val);
	bool del_val(const char* key);
	bool del_all_val();

private:
	bool verify_data_dirty(char data_state);

private:
	fstream* io_file;
	fstream* io_persist;

	map<const char*, long long, ptrCmp>* m_db;		// TODO: LRU and Thread Safe and Read and write separation and Segment lock
	KvTaskQueue<Tptr>* m_write_file_queue;			// the queue which contain items to be write to disk file

	// TODO: something statistics
};

