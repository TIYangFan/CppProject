#pragma once
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <map>

using namespace std;

struct ptrCmp
{
	bool operator()(const char * s1, const char * s2) const
	{
		return strcmp(s1, s2) < 0;
	}
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
	char* s_val;	// content of value

	SVal();
	SVal(char* val);
	SVal(char state, char* val);

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
	char* s_key;

	SKey();
	SKey(char* key);
	SKey(long long offset, char* key);

	bool write(fstream* io_persist);
	bool read(fstream* io_persist);
	bool readAll(fstream* io_persist, map<char*, long long, ptrCmp>* db);
};

class CSimplekvsvr
{
public:
	CSimplekvsvr();
	~CSimplekvsvr();

	char* getValue(char* key);
	bool setValue(char* key, char* value);
	bool deleteValue(char* key);
//	bool persist(char* key, long long val_offset);
	bool loadData();
	bool reorganizeStorage();

	bool getStatistics();
	bool getAllValueItemsInFile();
	bool getAllKeyItemsInPersist();

private:
	map<char*, long long, ptrCmp> m_db;
	map<char*, char*, ptrCmp> m_cache; // TODO: LRU

	fstream* io_file;
	fstream* io_persist;

	/*
	 * TODO: use Proxy Mode to statistics (should first extracting the data access layer)
	 */
	int key_count;

	int mem_size;
	int file_size;

	int hit_count;
	int miss_count;

	int cache_hit_count;
	int cache_miss_count;
};

