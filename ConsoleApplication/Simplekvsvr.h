#pragma once
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <map>

using namespace std;

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

	void getPersistFileContent();

private:
	map<char*, long long> m_db;
	map<char*, char*> m_cache; // TODO: LRU

	fstream* io_file;
	fstream* io_persist;
};

