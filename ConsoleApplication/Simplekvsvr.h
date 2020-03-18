#pragma once
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <map>

using namespace std;

struct SVal
{	
	char type;
	int val_len;
	char* val;
};

struct SKey
{
	int key_len;
	long long val_offset;
	char* key;
};

class CSimplekvsvr
{
public:
	CSimplekvsvr();
	~CSimplekvsvr();

	char* getValue(char* key);
	bool setValue(char* key, char* value);
	bool deleteValue();
	bool persist(char* key, long long val_offset);
	bool loadData();
	char dealWithDirtyData(char* key);
	bool reorganizeStorage();

	void getPersistFileContent();

private:
	map<char*, long long> m_db;
	map<char*, char*> m_cache; // TODO: LRU

	fstream* io_file;
	fstream* io_persist;
};

