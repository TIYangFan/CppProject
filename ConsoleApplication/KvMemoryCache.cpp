#include "KvMemoryCache.h"

KvMemoryCache::KvMemoryCache()
{
	m_cache = new map<const char*, const char*, ptrCmp>();
}

KvMemoryCache::~KvMemoryCache()
{

}

const char* KvMemoryCache::get_val(const char* key)
{
	map<const char*, const char*, ptrCmp>::iterator cache_iter = m_cache->find(key);
	if (cache_iter != m_cache->end())
		return cache_iter->second;
	else
		return "";
}

bool KvMemoryCache::set_val(const char* key, const char* val)
{
	(*m_cache)[key] = val;
	return true;
}

bool KvMemoryCache::insert_val(const char* key, const char* val)
{
	map<const char*, const char*, ptrCmp>::iterator cache_iter = m_cache->find(key);
	if (cache_iter == m_cache->end())
		m_cache->insert(pair<const char*, const char*>(key, val));
	else
		return false;
	return true;
}

bool KvMemoryCache::update_val(const char* key, const char* val)
{
	map<const char*, const char*, ptrCmp>::iterator cache_iter = m_cache->find(key);
	if (cache_iter != m_cache->end())
		cache_iter->second = val;
	else
		return false;
	return true;
}

bool KvMemoryCache::del_val(const char* key)
{
	map<const char*, const char*, ptrCmp>::iterator cache_iter = m_cache->find(key);
	if (cache_iter != m_cache->end())
		m_cache->erase(cache_iter);
	else
		return false;
	return true;
}

bool KvMemoryCache::del_all_val()
{
	m_cache->clear();
	return true;
}