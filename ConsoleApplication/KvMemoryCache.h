#pragma once
#include <map>
#include <string.h>
#include <iostream>
#include <stdlib.h>

#include "KvDataSource.h"

using namespace std;

struct ptrCmp
{
	bool operator()(const char * s1, const char * s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};

class KvMemoryCache
{
public:
	KvMemoryCache();
	~KvMemoryCache();

	const char* get_val(const char* key);
	bool set_val(const char* key, const char* val);
	bool insert_val(const char* key, const char* val);
	bool update_val(const char* key, const char* val);
	bool del_val(const char* key);
	bool del_all_val();

private:
	map<const char*, const char*, ptrCmp>* m_cache; // TODO: LRU and Thread Safe and Read and write separation and Segment lock

	// TODO: something statistics

};

