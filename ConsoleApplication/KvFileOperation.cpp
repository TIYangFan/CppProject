#include "KvFileOperation.h"

/*
	Byte util
*/
bool GetBit(int i, char var) { return var & (1 << i); };
void SetBit(int i, char* var) { *var |= 1 << i; };
void ClearBit(int i, char var) { var &= ~(1 << i); };

SVal::SVal()
{
	c_state = 0;
	i_val_len = 0;
	s_val = NULL;
}

SVal::SVal(const char* val)
{
	c_state = 0;
	i_val_len = strlen(val);
	s_val = val;
}

SVal::SVal(char state, const char* val)
{
	c_state = state;
	i_val_len = strlen(val);
	s_val = val;
}

long long SVal::write(fstream* io_file)
{
	printf("[INFO] write to file: state %d, length %d, value %s \n", c_state, i_val_len, s_val);

	io_file->seekg(0, ios::end);
	long long offset = io_file->tellg();

	io_file->seekp(0, ios::end);
	io_file->write((char*)&c_state, sizeof(c_state));
	io_file->write((char*)&i_val_len, sizeof(i_val_len));
	io_file->write(s_val, i_val_len);

	return offset;
}

bool SVal::read(fstream* io_file, int offset)
{
	io_file->seekg(offset, ios::beg);
	io_file->read((char*)&c_state, sizeof(c_state));
	io_file->read((char*)&i_val_len, sizeof(i_val_len));

	char* val = new char[i_val_len + 1];
	val[i_val_len] = '\0';
	io_file->read(val, i_val_len);
	s_val = val;

	printf("[INFO] read from file: state %d, length %d, value %s \n", c_state, i_val_len, s_val);
}

char SVal::changeState(fstream* io_file, long long val_offset, int index)
{
	char old_state = 0;
	io_file->seekg(val_offset, ios::beg);
	io_file->read((char*)&old_state, sizeof(old_state));

	char new_state = old_state;
	SetBit(index, &new_state);
	io_file->seekp(val_offset, ios::beg);
	io_file->write((char*)&new_state, sizeof(new_state));

	return old_state;
}


SKey::SKey()
{

}

SKey::SKey(const char* key)
{
	val_offset = 0;
	i_key_len = strlen(key);
	s_key = key;
}

SKey::SKey(long long offset, const char* key)
{
	val_offset = offset;
	i_key_len = strlen(key);
	s_key = key;
}

bool SKey::write(fstream* io_persist)
{
	printf("[INFO] write to persist: state %d, length %d, value %s \n", val_offset, i_key_len, s_key);

	io_persist->seekg(0, ios::end);	// append write
	long long offset = io_persist->tellg();

	io_persist->write((char*)&val_offset, sizeof(val_offset));
	io_persist->write((char*)&i_key_len, sizeof(i_key_len));
	io_persist->write(s_key, i_key_len);
}

bool SKey::read(fstream* io_persist)
{
	io_persist->read((char*)&val_offset, sizeof(val_offset));
	io_persist->read((char*)&i_key_len, sizeof(i_key_len));

	char* key = new char[i_key_len + 1];
	key[i_key_len] = '\0';
	io_persist->read(key, i_key_len);
	s_key = key;
}

bool SKey::readAll(fstream* io_persist, map<const char*, long long, ptrCmp>* db)
{
	io_persist->seekg(0, ios::beg);
	while (io_persist->peek() != EOF)
	{
		long long offset = 0;
		io_persist->read((char*)&offset, sizeof(offset));
		int key_len = 0;
		io_persist->read((char*)&key_len, sizeof(key_len));

		char* key = new char[key_len + 1];
		key[key_len] = '\0';
		io_persist->read(key, key_len);

		printf("[INFO] read from persist: state %d, length %d, value %s \n", offset, key_len, key);

		(*db)[key] = offset;

		cout << "test: " << (*db)[key] << endl;
	}
}