#include "KvFileDatabase.h"
#include "KvFileOperation.cpp"

template<typename T>
KvFileDatabase<T>::KvFileDatabase()
{
	io_file = new fstream();
	io_file->open("file.dat", ios::in | ios::out | ios::binary | ios::trunc);

	io_persist = new fstream();
	io_persist->open("persist.dat", ios::in | ios::out | ios::binary | ios::trunc); // 使用 ios::app 会无法创建文件

	if (!io_persist->is_open())
	{
		cout << "not open" << endl;
	}

	m_db = new map<const char*, long long, ptrCmp>();
}

template<typename T>
KvFileDatabase<T>::~KvFileDatabase()
{
	io_file->close();
	io_persist->close();
}

template<typename T>
const char* KvFileDatabase<T>::get_val(const char* key)
{
	map<const char*, long long, ptrCmp>::iterator db_iter = m_db->find(key);
	if (db_iter != m_db->end())
	{
		struct SVal val;
		val.read(io_file, db_iter->second);	// m_db[key]

		// verify data state if dirty again
		if (!verify_data_dirty(val.c_state))
			// TODO: add cache
			return val.s_val;
	}
	return "";	// there can't be NULL
}

template<typename T>
bool KvFileDatabase<T>::set_val(const char* key, const char* val)
{
	struct SVal sval(val);
	map<const char*, long long, ptrCmp>::iterator db_iter = m_db->find(key);
	char old_state = 0;
	if (db_iter != m_db->end())
		old_state = sval.changeState(io_file, db_iter->second, 0);	// delete old value in file

	sval.c_state = old_state;	// set value state
	(*m_db)[key] = sval.write(io_file);

	// TODO: persist ...

	// TODO: update cache ...

	return true;
}

template<typename T>
bool KvFileDatabase<T>::insert_val(const char* key, const char* val)
{

}

template<typename T>
bool KvFileDatabase<T>::update_val(const char* key, const char* val)
{

}

template<typename T>
bool KvFileDatabase<T>::del_val(const char* key)
{

}

template<typename T>
bool KvFileDatabase<T>::del_all_val()
{

}

template<typename T>
bool KvFileDatabase<T>::verify_data_dirty(char data_state)
{
	return GetBit(0, data_state);
}
