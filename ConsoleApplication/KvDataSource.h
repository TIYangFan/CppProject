#pragma once
class KvDataSource
{
protected:
	virtual const char* get_val(const char* key);
	virtual bool set_val(const char* key, const char* val);
	virtual bool update_val(const char* key, const char* val);
	virtual bool del_val(const char* key);
	virtual bool del_all_val();

protected:
	// Statistics
};

