#pragma once
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <map>

using namespace std;

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
	bool changeState(fstream* io_file, long long val_offset, int index);
};

struct SKey
{
	long long val_offset;
	int key_len;
	char* key;

	bool write(struct SKey key);
	bool read();
	bool tell();
};

struct KVio
{
	bool write(fstream* io_file, char* data, int len);
	bool read();
};

class CSimplekvsvr
{
public:
	CSimplekvsvr();
	~CSimplekvsvr();

	char* getValue(char* key);
	bool setValue(char* key, char* value);
	bool deleteValue(char* key);
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

