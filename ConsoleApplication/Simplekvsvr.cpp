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

bool SVal::changeState(fstream* io_file, long long val_offset, int index)
{
	char state = 0;
	io_file->seekg(val_offset, ios::beg);
	io_file->read((char*)&state, sizeof(state));

	SetBit(index, &state);
	io_file->seekp(val_offset, ios::beg);
	io_file->write((char*)&state, sizeof(state));

	return true;
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

		// store value to file
		char state = dealWithDirtyData(key); // return old state or create new state
		struct SVal val(state, value);
		m_db[key] = val.write(io_file);

		// persist
		persist(key, m_db[key]);

		// update cache
		map<char*, char*>::iterator iter = m_cache.find(key);
		if (iter != m_cache.end())
		{
			cout << "[INFO] update cache" << endl;
			iter->second = value;
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

bool CSimplekvsvr::persist(char* key, long long val_offset)
{
	if (io_persist->is_open())
	{
		cout << "[DEBUG] key: " << key << " off: " << val_offset << endl;

		// 1. key length
		int size = strlen(key);
		io_persist->write((char*)&size, sizeof(size));

		// 2. long long value offset
		io_persist->write((char*)&val_offset, sizeof(val_offset));

		// 3. key
		io_persist->write(key, size);
	}
	return true;
}

void CSimplekvsvr::getPersistFileContent()
{
	io_persist->seekg(0, ios::beg);

	int size = 0;
	io_persist->read((char*)&size, sizeof(size));

	long long val_offset = 0;
	io_persist->read((char*)&val_offset, sizeof(val_offset));

	char key[size + 1];
	key[size] = '\0';
	io_persist->read(key, size);

	cout << "size: " << size << " off:" << val_offset << " key: " << key << endl;
}

bool CSimplekvsvr::loadData()
{
	io_persist->seekg(0, ios::beg);
	while (io_persist->peek() != EOF)
	{
		int size = 0;
		io_persist->read((char*)&size, sizeof(size));

		long long val_offset = 0;
		io_persist->read((char*)&val_offset, sizeof(val_offset));

		char key[size + 1];
		key[size] = '\0';
		io_persist->read(key, size);

		cout << "size: " << size << " offset:" << val_offset << " key: " << key << endl;

		m_db[key] = val_offset;
	}

	return true;
}

char CSimplekvsvr::dealWithDirtyData(char* key)
{
	char old_state = 0;
	map<char*, long long>::iterator iter = m_db.find(key);

	// exist dirty data
	if (iter != m_db.end())
	{
		cout << "[INFO] find dirty data: " << iter->second << endl;
		
		int offset = iter->second;

		io_file->seekg(offset, ios::beg);
		io_file->read((char*)&old_state, sizeof(old_state));

		cout << "old_state: " << old_state << endl;

		char new_state = old_state;

		SetBit(0, &new_state); // change dirty bit

		cout << "new_state: " << old_state << endl;

		io_file->seekp(offset, ios::beg);
		io_file->write((char*)&new_state, sizeof(new_state));
	}

	return old_state; // return old state
}

bool CSimplekvsvr::reorganizeStorage()
{
	fstream* io_copy_file = new fstream();
	io_copy_file->open("file1.dat", ios::in | ios::out | ios::binary | ios::trunc);

	fstream* io_copy_persist = new fstream();
	io_copy_persist->open("persist1.dat", ios::in | ios::out | ios::binary | ios::trunc);

	io_persist->seekg(0, ios::beg);
	io_file->seekg(0, ios::beg);

	int size_of_key = 0;
	int size_of_val = 0;
	char state = 0;
	long long val_offset = 0;

	while (io_persist->peek() != EOF && io_file->peek() != EOF) // use "peek() == EOF" to determine if the file is at the end.(don't use eof())
	{
		io_persist->read((char*)&size_of_key, sizeof(size_of_key));
		io_persist->read((char*)&val_offset, sizeof(val_offset));

		io_file->seekg(val_offset, ios::beg);
		io_file->read((char*)&state, sizeof(state));

		if (!GetBit(0, state)) // not dirty data
		{
			// TODO: copy items from file.dat and persist.dat to new files;
			io_file->read((char*)&size_of_val, sizeof(size_of_val));
			char val[size_of_val];
			io_file->read((char*)&val, size_of_val);

			char key[size_of_key];
			io_persist->read((char*)&key, size_of_key);
			
			// write to new files
			val_offset = io_copy_file->tellg();
			io_copy_file->write((char*)&state, sizeof(state));
			io_copy_file->write((char*)&size_of_val, sizeof(size_of_val));
			io_copy_file->write((char*)&val, size_of_val);

			io_copy_persist->write((char*)&size_of_key, sizeof(size_of_key));
			io_copy_persist->write((char*)&val_offset, sizeof(val_offset));
			io_copy_persist->write((char*)&key, size_of_key);

			printf("[INFO] copy to file: state %d, length %d, value %s \n", state, size_of_val, val);
		}
		else
		{
			io_persist->seekg(size_of_key, ios::cur);
		}
	}

	// test
	/*
	io_copy_persist->seekg(0, ios::beg);
	int zi = 0;
	io_copy_persist->read((char*)&zi, sizeof(zi));
	long long oi = 0;
	io_copy_persist->read((char*)&oi, sizeof(oi));
	char di[zi + 1];
	di[zi] = '\0';
	io_copy_persist->read((char*)&di, sizeof(di));
	cout << "[DEBUG] get key: " << di << endl;

	io_file->seekg(oi, ios::beg);
	char s = 0;
	io_file->read((char*)&s, sizeof(s));
	int z = 0;
	io_file->read((char*)&z, sizeof(z));
	char d[z + 1];
	d[z] = '\0';
	io_file->read((char*)&d, sizeof(d));
	cout << "[DEBUG] get val: " << d << endl;
	*/

	// change file ptr
	io_file->close();
	io_persist->close();

	io_file = io_copy_file;
	io_persist = io_copy_persist;

	loadData();
}

/*
	write 先写入 buffer 当切换至 read 时才会将数据从 Buffer 写入文件
*/
int main(int argc, char * argv[])
{

	/*
	struct SVal val("hello");

	fstream* io_file = new fstream();
	io_file->open("file.dat", ios::in | ios::out | ios::binary | ios::trunc);
	val.write(io_file);

	struct SVal val1;
	io_file->seekg(0, ios::beg);
	val1.read(io_file);
	cout << val1.c_state << " " << val1.i_val_len << " " << val1.s_val << endl;
	*/

	 
	CSimplekvsvr* cs = new CSimplekvsvr();
	cs->setValue("h", "hello");
	//cs->setValue("w", "world");
	//cs->setValue("y", "yang");
	//cs->setValue("f", "fan");

	//cs->getValue("w");
	//cs->getValue("f");
	//cs->getValue("h");

	//cs->setValue("h", "huawei");
	cs->getValue("h");
	cs->deleteValue("h");
	cs->getValue("h");

	//cs->reorganizeStorage();
	//cs->getValue("h");

	//cout << endl;
	//cs->getPersistFileContent();
	//cs->loadData();

	getchar();
	return 0;
}