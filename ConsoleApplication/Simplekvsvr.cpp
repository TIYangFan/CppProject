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

char* CSimplekvsvr::getValue(char* key)
{
	// first find at cache
	map<char*, char*>::iterator iter = m_cache.find(key);
	if (iter != m_cache.end())
	{
		cout << "[INFO] get from cache" << endl;
		return iter->second;
	}

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

bool CSimplekvsvr::deleteValue()
{

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
	while (!io_persist->eof())
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

/*
	Byte util
*/
bool GetBit(int i, char var) { return var & (1 << i); };
void SetBit(int i, char* var) { *var |= 1 << i; };
void ClearBit(int i, char var) { var &= ~(1 << i); };

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

	//cout << endl;
	//cs->getPersistFileContent();
	//cs->loadData();

	getchar();
	return 0;
}