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

	int size = 0;
	io_file->read((char*)&size, sizeof(size));

	char value[size + 1];
	value[size] = '\0'; // 因为本地文件中未保存结尾 \0 终止符，因此需要手动添加终止符

	cout << "[INFO] get size: " << size << endl;

	io_file->read(value, size);

	m_cache[key] = value; // add cache

	cout << value << endl;
}

bool CSimplekvsvr::setValue(char* key, char* value)
{
	char data[100];

	if (io_file->is_open())
	{
		cout << "[INFO] file open successfully." << endl;

		io_file->seekg(0, ios::end);
		long start = io_file->tellg();

		cout << "[DEBUG] e: " << start << endl;

		m_db[key] = start;
		int size = strlen(value); // sizeof(*value) 计算的是指针大小而非字符串大小

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

	//char* key;
	//io_persist->read(key, size);

	cout << "size: " << size << " off:" << val_offset << " key: " << endl;
}

int main(int argc, char * argv[])
{
	CSimplekvsvr* cs = new CSimplekvsvr();
	cs->setValue("h", "hello");
	cs->setValue("w", "world");
	cs->setValue("y", "yang");
	cs->setValue("f", "fan");

	cs->getValue("h");
	cs->getValue("f");
	cs->getValue("h");

	cs->setValue("h", "huawei");

	cout << endl;
	cs->getPersistFileContent();

	getchar();
	return 0;
}