#pragma once
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <atomic>
#include <map>

using namespace std;

struct ptrCmp
{
	bool operator()(const char * s1, const char * s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};

struct Stats
{
	Stats()
	{
		key_count = 0;
		
		mem_size = 0;
		file_size = 0;
		
		hit_count = 0;
		miss_count = 0;

		cache_hit_count = 0;
		cache_miss_count = 0;
	}

	atomic<int> key_count;

	atomic<int> mem_size;
	atomic<int> file_size;

	atomic<int> hit_count;
	atomic<int> miss_count;

	atomic<int> cache_hit_count;
	atomic<int> cache_miss_count;
};

/*
		+--------------------+--------------------+--------------------+
		| Type(State) 1Byte  | Length£¨int£©4Byte | Value (Length Byte)|
		+--------------------+--------------------+--------------------+

		Type : 0000 0000
		the last bit of type mean this data is dirty or not
 */
struct SVal
{	
	char c_state;	// state of value
	int i_val_len;	// length of value
	const char* s_val;	// content of value

	SVal();
	SVal(const char* val);
	SVal(char state, const char* val);

	long long write(fstream* io_file);	// append write AND return value offset
	bool read(fstream* io_file, int offset);	// 
	char changeState(fstream* io_file, long long val_offset, int index);	// return old state
};


/*
		+--------------------+--------------------+--------------------+
		| Value Offset 8Byte | Length£¨int£©4Byte | Key (Length Byte)  |
		+--------------------+--------------------+--------------------+

		Type : 0000 0000
		the last bit of type mean this data is dirty or not
 */
struct SKey
{
	long long val_offset;
	int i_key_len;
	const char* s_key;

	SKey();
	SKey(const char* key);
	SKey(long long offset, const char* key);

	bool write(fstream* io_persist);
	bool read(fstream* io_persist);
	bool readAll(fstream* io_persist, map<const char*, long long, ptrCmp>* db);
};

class CSimplekvsvr
{
public:
	CSimplekvsvr();
	~CSimplekvsvr();

	const char* getValue(const char* key);
	bool setValue(const char* key, const char* value);
	bool deleteValue(const char* key);
	bool loadData();
	bool reorganizeStorage();

	Stats* getStats();
	bool getAllValueItemsInFile();
	bool getAllKeyItemsInPersist();

private:
	map<const char*, long long, ptrCmp>* m_db;
	map<const char*, const char*, ptrCmp>* m_cache; // TODO: LRU

	fstream* io_file;
	fstream* io_persist;

	/*
	 * TODO: use Proxy Mode to statistics (should first extracting the data access layer)
	 */
	Stats* m_stats;
};

