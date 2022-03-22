#include "CachePool.h"

#include <stdlib.h>
#include <string.h>
#include "Thread.h"

#define log_error printf
#define log_info printf

#define MIN_CACHE_CONN_CNT 2
#define MAX_CACHE_CONN_FAIL_NUM 10

CacheConn::CacheConn(const char *server_ip, int server_port, int db_index, const char *password,
					 const char *pool_name)
{
	m_server_ip = server_ip;
	m_server_port = server_port;

	m_db_index = db_index;
	m_password = password;
	m_pool_name = pool_name;
	m_pContext = NULL;
	m_last_connect_time = 0;
}

CacheConn::CacheConn(CachePool *pCachePool)
{
	m_pCachePool = pCachePool;
	if (pCachePool)
	{
		m_server_ip = pCachePool->GetServerIP();
		m_server_port = pCachePool->GetServerPort();
		m_db_index = pCachePool->GetDBIndex();
		m_password = pCachePool->GetPassword();
		m_pool_name = pCachePool->GetPoolName();
	}
	else
	{
		log_error("pCachePool is NULL\n");
	}

	m_pContext = NULL;
	m_last_connect_time = 0;
}

CacheConn::~CacheConn()
{
	if (m_pContext)
	{
		redisFree(m_pContext);
		m_pContext = NULL;
	}
}

/*
 * redis初始化连接和重连操作，类似mysql_ping()
 */
int CacheConn::Init()
{
	if (m_pContext)	// 非空，连接是正常的
	{
		return 0;
	}

	// 1s 尝试重连一次
	uint64_t cur_time = (uint64_t)time(NULL);
	if (cur_time < m_last_connect_time + 1) 		// 重连尝试 间隔1秒 
	{
		printf("cur_time:%lu, m_last_connect_time:%lu\n", cur_time, m_last_connect_time);
		return 1;
	}
	// printf("m_last_connect_time = cur_time\n");
	m_last_connect_time = cur_time;

	// 1000ms超时
	struct timeval timeout = {0, 1000000};
	// 建立连接后使用 redisContext 来保存连接状态。
	// redisContext 在每次操作后会修改其中的 err 和  errstr 字段来表示发生的错误码（大于0）和对应的描述。
	m_pContext = redisConnectWithTimeout(m_server_ip.c_str(), m_server_port, timeout);

	if (!m_pContext || m_pContext->err)
	{
		if (m_pContext)
		{
			log_error("redisConnect failed: %s\n", m_pContext->errstr);
			redisFree(m_pContext);
			m_pContext = NULL;
		}
		else
		{
			log_error("redisConnect failed\n");
		}

		return 1;
	}

	redisReply *reply;
	// 验证
	if (!m_password.empty())
	{
		reply = (redisReply *)redisCommand(m_pContext, "AUTH %s", m_password.c_str());

		if (!reply || reply->type == REDIS_REPLY_ERROR)
		{
			log_error("Authentication failure:%p\n", reply);
			if (reply)
				freeReplyObject(reply);
			return -1;
		}
		else
		{
			// log_info("Authentication success\n");
		}

		freeReplyObject(reply);
	}

	reply = (redisReply *)redisCommand(m_pContext, "SELECT %d", 0);

	if (reply && (reply->type == REDIS_REPLY_STATUS) && (strncmp(reply->str, "OK", 2) == 0))
	{
		freeReplyObject(reply);
		return 0;
	}
	else
	{
		if (reply)
			log_error("select cache db failed:%s\n", reply->str);
		return 2;
	}
}

void CacheConn::DeInit()
{
	if (m_pContext)
	{
		redisFree(m_pContext);
		m_pContext = NULL;
	}
}

const char *CacheConn::GetPoolName()
{
	return m_pool_name.c_str();
}

string CacheConn::get(string key)
{
	string value;

	if (Init())
	{
		return value;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "GET %s", key.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return value;
	}

	if (reply->type == REDIS_REPLY_STRING)
	{
		value.append(reply->str, reply->len);
	}

	freeReplyObject(reply);
	return value;
}

string CacheConn::set(string key, string &value)
{
	string ret_value;

	if (Init())
	{
		return ret_value;
	}
	// 返回的结果存放在redisReply
	redisReply *reply = (redisReply *)redisCommand(m_pContext, "SET %s %s", key.c_str(), value.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	ret_value.append(reply->str, reply->len);
	freeReplyObject(reply); // 释放资源
	return ret_value;
}

string CacheConn::setex(string key, int timeout, string value)
{
	string ret_value;

	if (Init())
	{
		return ret_value;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "SETEX %s %d %s", key.c_str(), timeout, value.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	ret_value.append(reply->str, reply->len);
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::mget(const vector<string> &keys, map<string, string> &ret_value)
{
	if (Init())
	{
		return false;
	}
	if (keys.empty())
	{
		return false;
	}

	string strKey;
	bool bFirst = true;
	for (vector<string>::const_iterator it = keys.begin(); it != keys.end(); ++it)
	{
		if (bFirst)
		{
			bFirst = false;
			strKey = *it;
		}
		else
		{
			strKey += " " + *it;
		}
	}

	if (strKey.empty())
	{
		return false;
	}
	strKey = "MGET " + strKey;
	redisReply *reply = (redisReply *)redisCommand(m_pContext, strKey.c_str());
	if (!reply)
	{
		log_info("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}
	if (reply->type == REDIS_REPLY_ARRAY)
	{
		for (size_t i = 0; i < reply->elements; ++i)
		{
			redisReply *child_reply = reply->element[i];
			if (child_reply->type == REDIS_REPLY_STRING)
			{
				ret_value[keys[i]] = child_reply->str;
			}
		}
	}
	freeReplyObject(reply);
	return true;
}

bool CacheConn::isExists(string &key)
{
	if (Init())
	{
		return false;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "EXISTS %s", key.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	if (0 == ret_value)
	{
		return false;
	}
	else
	{
		return true;
	}
}

long CacheConn::del(string &key)
{
	if (Init())
	{
		return 0;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "DEL %s", key.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return 0;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::hdel(string key, string field)
{
	if (Init())
	{
		return 0;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "HDEL %s %s", key.c_str(), field.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return 0;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

string CacheConn::hget(string key, string field)
{
	string ret_value;
	if (Init())
	{
		return ret_value;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "HGET %s %s", key.c_str(), field.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	if (reply->type == REDIS_REPLY_STRING)
	{
		ret_value.append(reply->str, reply->len);
	}

	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::hgetAll(string key, map<string, string> &ret_value)
{
	if (Init())
	{
		return false;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "HGETALL %s", key.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if ((reply->type == REDIS_REPLY_ARRAY) && (reply->elements % 2 == 0))
	{
		for (size_t i = 0; i < reply->elements; i += 2)
		{
			redisReply *field_reply = reply->element[i];
			redisReply *value_reply = reply->element[i + 1];

			string field(field_reply->str, field_reply->len);
			string value(value_reply->str, value_reply->len);
			ret_value.insert(make_pair(field, value));
		}
	}

	freeReplyObject(reply);
	return true;
}

long CacheConn::hset(string key, string field, string value)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::hincrBy(string key, string field, long value)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "HINCRBY %s %s %ld", key.c_str(), field.c_str(), value);
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::incrBy(string key, long value)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "INCRBY %s %ld", key.c_str(), value);
	if (!reply)
	{
		log_error("redis Command failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

string CacheConn::hmset(string key, map<string, string> &hash)
{
	string ret_value;

	if (Init())
	{
		return ret_value;
	}

	int argc = hash.size() * 2 + 2;
	const char **argv = new const char *[argc];
	if (!argv)
	{
		return ret_value;
	}

	argv[0] = "HMSET";
	argv[1] = key.c_str();
	int i = 2;
	for (map<string, string>::iterator it = hash.begin(); it != hash.end(); it++)
	{
		argv[i++] = it->first.c_str();
		argv[i++] = it->second.c_str();
	}

	redisReply *reply = (redisReply *)redisCommandArgv(m_pContext, argc, argv, NULL);
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		delete[] argv;

		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	ret_value.append(reply->str, reply->len);

	delete[] argv;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::hmget(string key, list<string> &fields, list<string> &ret_value)
{
	if (Init())
	{
		return false;
	}

	int argc = fields.size() + 2;
	const char **argv = new const char *[argc];
	if (!argv)
	{
		return false;
	}

	argv[0] = "HMGET";
	argv[1] = key.c_str();
	int i = 2;
	for (list<string>::iterator it = fields.begin(); it != fields.end(); it++)
	{
		argv[i++] = it->c_str();
	}

	redisReply *reply = (redisReply *)redisCommandArgv(m_pContext, argc, (const char **)argv, NULL);
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		delete[] argv;

		redisFree(m_pContext);
		m_pContext = NULL;

		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY)
	{
		for (size_t i = 0; i < reply->elements; i++)
		{
			redisReply *value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	delete[] argv;
	freeReplyObject(reply);
	return true;
}

long CacheConn::incr(string key)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "INCR %s", key.c_str());
	if (!reply)
	{
		log_error("redis Command failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::decr(string key)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "DECR %s", key.c_str());
	if (!reply)
	{
		log_error("redis Command failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::lpush(string key, string value)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "LPUSH %s %s", key.c_str(), value.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::rpush(string key, string value)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "RPUSH %s %s", key.c_str(), value.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::llen(string key)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "LLEN %s", key.c_str());
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::lrange(string key, long start, long end, list<string> &ret_value)
{
	if (Init())
	{
		return false;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "LRANGE %s %d %d", key.c_str(), start, end);
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY)
	{
		for (size_t i = 0; i < reply->elements; i++)
		{
			redisReply *value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	freeReplyObject(reply);
	return true;
}

bool CacheConn::flushdb()
{
	bool ret = false;
	if (Init())
	{
		return false;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "FLUSHDB");
	if (!reply)
	{
		log_error("redisCommand failed:%s\n", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_STRING && strncmp(reply->str, "OK", 2) == 0)
	{
		ret = true;
	}

	freeReplyObject(reply);

	return ret;
}
///////////////
CachePool::CachePool(const char *pool_name, const char *server_ip, int server_port, int db_index,
					 const char *password, int max_conn_cnt)
{
	m_pool_name = pool_name;
	m_server_ip = server_ip;
	m_server_port = server_port;
	m_db_index = db_index;
	m_password = password;
	m_max_conn_cnt = max_conn_cnt;
	m_cur_conn_cnt = MIN_CACHE_CONN_CNT;
}

CachePool::~CachePool()
{
	m_free_notify.Lock();
	for (list<CacheConn *>::iterator it = m_free_list.begin(); it != m_free_list.end(); it++)
	{
		CacheConn *pConn = *it;
		delete pConn;
	}

	m_free_list.clear();
	m_cur_conn_cnt = 0;
	m_free_notify.Unlock();
}

int CachePool::Init()
{
	for (int i = 0; i < m_cur_conn_cnt; i++)
	{
		CacheConn *pConn = new CacheConn(m_server_ip.c_str(), m_server_port,
										 m_db_index, m_password.c_str(), m_pool_name.c_str());
		if (pConn->Init())
		{
			delete pConn;
			return 1;
		}

		m_free_list.push_back(pConn);
	}

	log_info("cache pool: %s, list size: %lu\n", m_pool_name.c_str(), m_free_list.size());
	return 0;
}

CacheConn *CachePool::GetCacheConn()
{
	m_free_notify.Lock();

	while (m_free_list.empty())
	{
		if (m_cur_conn_cnt >= m_max_conn_cnt)
		{
			m_free_notify.Wait();
		}
		else
		{
			CacheConn *p_cache_conn = new CacheConn(m_server_ip.c_str(), m_server_port,
													m_db_index, m_password.c_str(), m_pool_name.c_str());
			int ret = p_cache_conn->Init();
			if (ret)
			{
				log_error("Init CacheConn failed\n");
				delete p_cache_conn;
				m_free_notify.Unlock();
				return NULL;
			}
			else
			{
				m_free_list.push_back(p_cache_conn);
				m_cur_conn_cnt++;
				log_info("new cache connection: %s, conn_cnt: %d\n", m_pool_name.c_str(), m_cur_conn_cnt);
			}
		}
	}

	CacheConn *pConn = m_free_list.front();
	m_free_list.pop_front();

	m_free_notify.Unlock();

	return pConn;
}

void CachePool::RelCacheConn(CacheConn *p_cache_conn)
{
	m_free_notify.Lock();

	list<CacheConn *>::iterator it = m_free_list.begin();
	for (; it != m_free_list.end(); it++)
	{
		if (*it == p_cache_conn)
		{
			break;
		}
	}

	if (it == m_free_list.end())
	{
		m_free_list.push_back(p_cache_conn);
	}

	m_free_notify.Signal();
	m_free_notify.Unlock();
}
