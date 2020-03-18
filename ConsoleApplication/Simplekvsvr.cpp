#include "Simplekvsvr.h"

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

/*
	Byte util
*/
bool GetBit(int i, char var) { return var & (1 << i); };
void SetBit(int i, char* var) { *var |= 1 << i; };
void ClearBit(int i, char var) { var &= ~(1 << i); };

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

	io_file->seekg(m_db[key], ios::beg);

	char state = 0;
	io_file->read((char*)&state, sizeof(state));

	int size = 0;
	io_file->read((char*)&size, sizeof(size));

	char value[size + 1];
	value[size] = '\0'; // 因为本地文件中未保存结尾 \0 终止符，因此需要手动添加终止符

	io_file->read(value, size);

	m_cache[key] = value; // add cache

	printf("[INFO] read from file: state %d, length %d, value %s \n", state, size, value);
}

bool CSimplekvsvr::setValue(char* key, char* value)
{
	char data[100];

	if (io_file->is_open())
	{
		cout << "[INFO] file open successfully." << endl;

		char state = dealWithDirtyData(key); // return old state or create new state

		io_file->seekg(0, ios::end);
		long long start = io_file->tellg();

		m_db[key] = start;

		int size = strlen(value); // sizeof(*value) 计算的是指针大小而非字符串大小

		/*
				+--------------------+--------------------+--------------------+
				| Type(State) 1Byte  | Length（int）4Byte | Value (Length Byte)|
				+--------------------+--------------------+--------------------+

				Type : 0000 0000
				the last bit of type mean this data is dirty or not
		 */

		printf("[INFO] begin write to file: state %d, length %d, value %s \n", state, size, value);

		// io_file->seekp(0, ios::end);
		io_file->write((char*)&state, sizeof(state));
		io_file->write((char*)&size, sizeof(size));
		io_file->write(value, size);

		// persist
		persist(key, start);

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
		char state = 0;
		long long offset = iter->second;
		io_file->seekg(offset, ios::beg);
		io_file->read((char*)&state, sizeof(state));

		SetBit(0, &state);

		io_file->seekp(offset, ios::beg);
		io_file->write((char*)&state, sizeof(state));

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
	CSimplekvsvr* cs = new CSimplekvsvr();
	cs->setValue("h", "hello");
	//cs->setValue("w", "world");
	//cs->setValue("y", "yang");
	//cs->setValue("f", "fan");

	//cs->getValue("w");
	//cs->getValue("f");
	//cs->getValue("h");

	cs->setValue("h", "huawei");
	cs->getValue("h");
	cs->reorganizeStorage();
	cs->getValue("h");

	//cout << endl;
	//cs->getPersistFileContent();
	//cs->loadData();

	getchar();
	return 0;
}