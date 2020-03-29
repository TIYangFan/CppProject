#pragma once
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <fstream>
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

class KvFileOperation
{
};

