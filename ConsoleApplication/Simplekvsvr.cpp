#include "Simplekvsvr.h"

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

SVal::SVal(char* val)
{
	c_state = 0;
	i_val_len = strlen(val);
	s_val = val;
}

SVal::SVal(char state, char* val)
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

	s_val = new char[i_val_len+1];
	s_val[i_val_len] = '\0';
	io_file->read(s_val, i_val_len);

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

SKey::SKey(char* key)
{
	val_offset = 0;
	i_key_len = strlen(key);
	s_key = key;
}

SKey::SKey(long long offset, char* key)
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

	s_key = new char[i_key_len + 1];
	s_key[i_key_len] = '\0';
	io_persist->read(s_key, i_key_len);
}

bool SKey::readAll(fstream* io_persist, map<char*, long long, ptrCmp>* db)
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


CSimplekvsvr::CSimplekvsvr()
{
	io_file = new fstream();
	io_file->open("file.dat", ios::in | ios::out | ios::binary | ios::trunc);

	io_persist = new fstream();
	io_persist->open("persist.dat", ios::in | ios::out | ios::binary | ios::trunc); // 使用 ios::app 会无法创建文件
	if (!io_persist->is_open())
	{
		cout << "not open" << endl;
	}
}

CSimplekvsvr::~CSimplekvsvr()
{
	io_file->close();
	io_persist->close();
}

char* CSimplekvsvr::getValue(char* key)
{
	// first find at cache
	/*
	map<char*, char*>::iterator iter = m_cache.find(key);
	if (iter != m_cache.end())
	{
		cout << "[INFO] get from cache" << endl;
		return iter->second;
	}
	*/

	if (!io_file->is_open())
	{
		cout << "[INFO] Error open file" << endl;
		exit(1);
	}

	map<char*, long long>::iterator db_iter = m_db.find(key);
	if (db_iter != m_db.end())
	{
		struct SVal val;
		val.read(io_file, m_db[key]);

		// verify data state if dirty again
		if (!GetBit(0, val.c_state))
		{
			m_cache[key] = val.s_val; // add cache
			return val.s_val;
		}
	}

	return NULL;
}

bool CSimplekvsvr::setValue(char* key, char* value)
{

	if (io_file->is_open())
	{
		cout << "[INFO] file open successfully." << endl;

		struct SVal val(value);
		map<char*, long long>::iterator iter = m_db.find(key);
		char old_state = 0;
		if (iter != m_db.end())
		{
			old_state = val.changeState(io_file, iter->second, 0);	// delete old value in file
		}
		val.c_state = old_state;	// set value state
		m_db[key] = val.write(io_file);

		// persist Key
		struct SKey s_key(m_db[key], key);
		s_key.write(io_persist);

		// update cache
		map<char*, char*>::iterator cache_iter = m_cache.find(key);
		if (cache_iter != m_cache.end())
		{
			cout << "[INFO] update cache" << endl;
			cache_iter->second = value;
		}
	}

	return true;
}

bool CSimplekvsvr::deleteValue(char* key)
{
	map<char*, long long>::iterator iter = m_db.find(key);
	if (iter != m_db.end())
	{
		// change data dirty state
		struct SVal val;
		val.changeState(io_file, iter->second, 0);

		// delete data in m_db
		m_cache.erase(key);

		// delete data in m_cache
		m_db.erase(iter);
	}
	return true;
}

bool CSimplekvsvr::loadData()
{
	struct SKey s_key;
	s_key.readAll(io_persist, &m_db);
	return true;
}

bool CSimplekvsvr::reorganizeStorage()
{
	fstream* io_copy_file = new fstream();
	io_copy_file->open("file1.dat", ios::in | ios::out | ios::binary | ios::trunc);

	fstream* io_copy_persist = new fstream();
	io_copy_persist->open("persist1.dat", ios::in | ios::out | ios::binary | ios::trunc);

	// init file ptr
	io_persist->seekg(0, ios::beg);
	io_file->seekg(0, ios::beg);

	while (io_persist->peek() != EOF && io_file->peek() != EOF) // use "peek() == EOF" to determine if the file is at the end.(don't use eof())
	{
		struct SKey key;
		key.read(io_persist);

		struct SVal val;
		val.read(io_file, key.val_offset);

		if (!GetBit(0, val.c_state)) // not dirty data
		{	
			// write to new files
			struct SKey c_key(key.val_offset, key.s_key);
			c_key.write(io_copy_persist);

			struct SVal c_val(val.c_state, val.s_val);
			c_val.write(io_copy_file);

			printf("[DEBUG] copy to file: state %d, length %d, value %s \n", c_val.c_state, c_val.i_val_len, c_val.s_val);
		}
	}

	// change file ptr
	io_file->close();
	io_persist->close();

	io_file = io_copy_file;
	io_persist = io_copy_persist;

	loadData();
}

bool CSimplekvsvr::getStatistics()
{

}

// for test
bool CSimplekvsvr::getAllValueItemsInFile()
{
	io_file->seekg(0, ios::beg);
	while (io_file->peek() != EOF)
	{
		char c_state = 0;
		io_file->read((char*)&c_state, sizeof(c_state));
		int i_val_len = 0;
		io_file->read((char*)&i_val_len, sizeof(i_val_len));

		char* s_val = new char[i_val_len + 1];
		s_val[i_val_len] = '\0';
		io_file->read(s_val, i_val_len);

		printf("[DEBUG] read from file: state %d, length %d, value %s \n", c_state, i_val_len, s_val);
	}
}

bool CSimplekvsvr::getAllKeyItemsInPersist()
{
	io_persist->seekg(0, ios::beg);
	while (io_persist->peek() != EOF)
	{
		long long val_offset = 0;
		io_persist->read((char*)&val_offset, sizeof(val_offset));
		int i_key_len = 0;
		io_persist->read((char*)&i_key_len, sizeof(i_key_len));

		char* s_key = new char[i_key_len + 1];
		s_key[i_key_len] = '\0';
		io_persist->read(s_key, i_key_len);

		printf("[DEBUG] read from file: val_offset %d, length %d, key %s \n", val_offset, i_key_len, s_key);
	}
}

/*
	write 先写入 buffer 当切换至 read 时才会将数据从 Buffer 写入文件
*/
int main(int argc, char * argv[])
{
	CSimplekvsvr* cs = new CSimplekvsvr();
	cs->setValue("h", "hello");
	//cs->setValue("w", "world");
	//cs->setValue("y", "yang");
	//cs->setValue("f", "fan");

	//cs->getValue("w");
	//cs->getValue("f");
	//cs->getValue("h");

	cs->getValue("h");
	cs->setValue("h", "huawei");
	cs->getValue("h");
	cs->setValue("h", "haha");
	cs->getValue("h");

	cout << "---------------Before----------------" << endl;
 	cs->getAllValueItemsInFile();
	cs->getAllKeyItemsInPersist();
	cs->reorganizeStorage();

	cout << "---------------After----------------" << endl;
	cs->getAllValueItemsInFile();
	cs->getAllKeyItemsInPersist();

	//cs->loadData();

	getchar();
	return 0;
}