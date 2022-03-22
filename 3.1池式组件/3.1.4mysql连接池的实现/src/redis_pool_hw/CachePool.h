/*
 * @Author: your name
 * @Date: 2019-12-07 10:54:57
 * @LastEditTime : 2020-01-10 16:35:13
 * @LastEditors  : Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \src\cache_pool\CachePool.h
 */
#ifndef CACHEPOOL_H_
#define CACHEPOOL_H_

#include <iostream>
#include <vector>
#include <map>
#include <list>

#include "Thread.h"

#include "hiredis.h"


using std::string;
using std::list;
using std::map; 
using std::vector; 

class CachePool;

class CacheConn {
public:
	CacheConn(const char* server_ip, int server_port, int db_index, const char* password, 
		const char *pool_name ="");
	CacheConn(CachePool* pCachePool);	
	virtual ~CacheConn();
	
	int Init();
	void DeInit();
	const char* GetPoolName();
    // 通用操作
    // 判断一个key是否存在
    bool isExists(string &key);
    // 删除某个key
    long del(string &key);

    // ------------------- 字符串相关 -------------------
	string get(string key);
    string set(string key, string& value);
	string setex(string key, int timeout, string value);
	
	// string mset(string key, map);
    //批量获取
    bool mget(const vector<string>& keys, map<string, string>& ret_value);
	//原子加减1
    long incr(string key);
    long decr(string key);


	// ---------------- 哈希相关 ------------------------
	long hdel(string key, string field);
	string hget(string key, string field);
	bool hgetAll(string key, map<string, string>& ret_value);
	long hset(string key, string field, string value);

	long hincrBy(string key, string field, long value);
    long incrBy(string key, long value);
	string hmset(string key, map<string, string>& hash);
	bool hmget(string key, list<string>& fields, list<string>& ret_value);
    
    

	// ------------ 链表相关 ------------
	long lpush(string key, string value);
	long rpush(string key, string value);
	long llen(string key);
	bool lrange(string key, long start, long end, list<string>& ret_value);

	
    bool flushdb();

private:
	CachePool* 		m_pCachePool;
	redisContext* 	m_pContext;
	uint64_t		m_last_connect_time;
	uint16_t 		m_server_port;
	string 			m_server_ip;
    string          m_password;
	uint16_t        m_db_index;
	string 			m_pool_name;
};


class CachePool {
public:
	// db_index和mysql不同的地方 
	CachePool(const char* pool_name, const char* server_ip, int server_port, int db_index, 
		const char *password, int max_conn_cnt);
	virtual ~CachePool();

	int Init();
    // 获取空闲的连接资源
	CacheConn* GetCacheConn();
    // Pool回收连接资源
	void RelCacheConn(CacheConn* pCacheConn);

	const char* GetPoolName() { return m_pool_name.c_str(); }
	const char* GetServerIP() { return m_server_ip.c_str(); }
	const char* GetPassword() { return m_password.c_str(); }
	int GetServerPort() { return m_server_port; }
	int GetDBIndex() { return m_db_index; }
private:
	string 		m_pool_name;
	string		m_server_ip;
	string 		m_password;
	int			m_server_port;
	int			m_db_index;	// mysql 数据库名字， redis db index

	int			m_cur_conn_cnt;
	int 		m_max_conn_cnt;
	list<CacheConn*>	m_free_list;
	CThreadNotify		m_free_notify;
};



#endif /* CACHEPOOL_H_ */
