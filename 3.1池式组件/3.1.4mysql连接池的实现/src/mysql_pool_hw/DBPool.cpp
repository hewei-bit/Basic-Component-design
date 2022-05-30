#include "DBPool.h"
#include <string.h>

#define log_error printf
#define log_warn printf
#define log_info printf
#define MIN_DB_CONN_CNT 1
#define MAX_DB_CONN_FAIL_NUM 10

CResultSet::CResultSet(MYSQL_RES *res)
{
    m_res = res;
}

CResultSet::~CResultSet()
{
}

bool CResultSet::Next()
{
}

int CResultSet::_GetIndex(const char *key)
{
}

int CResultSet::GetInt(const char *key)
{
}

char *CResultSet::GetString(const char *key)
{
}

////////////////////////////////////////////////////
CPrepareStatement::CPrepareStatement()
{
}

CPrepareStatement::~CPrepareStatement()
{
}

bool CPrepareStatement::Init(MYSQL *mysql, string &sql)
{
}

void CPrepareStatement::SetParam(uint32_t index, int &value)
{
}

void CPrepareStatement::SetParam(uint32_t index, uint32_t &value)
{
}

void CPrepareStatement::SetParam(uint32_t index, string &value)
{
}

void CPrepareStatement::SetParam(uint32_t index, const string &value)
{
}

bool CPrepareStatement::ExecuteUpdate()
{
}

uint32_t CPrepareStatement::GetInsertId()
{
}

/////////////////////////////////////////////////
CDBConn::CDBConn(CDBPool *pPool)
{
}

CDBConn::~CDBConn()
{
}

int CDBConn::Init()
{
}

const char *CDBConn::GetPoolName()
{
}

bool CDBConn::ExecuteCreate(const char *sql_query)
{
}
bool CDBConn::ExecuteDrop(const char *sql_query)
{
}

CResultSet *CDBConn::ExecuteQuery(const char *sql_query)
{
}

/*
1.执行成功，则返回受影响的行的数目，如果最近一次查询失败的话，函数返回 -1

2.对于delete,将返回实际删除的行数.

3.对于update,如果更新的列值原值和新值一样,如update tables set col1=10 where id=1;
id=1该条记录原值就是10的话,则返回0。

mysql_affected_rows返回的是实际更新的行数,而不是匹配到的行数。
*/
bool CDBConn::ExecuteUpdate(const char *sql_query, bool care_affected_rows)
{
}

bool CDBConn::ExecuteUpdate2(const char *sql_query, bool care_affected_rows)
{
again: // 不能这么写
    if (mysql_real_query(m_mysql, sql_query, strlen(sql_query)))
    {
        log_error("mysql_real_query failed: %s, sql: %s\n", mysql_error(m_mysql), sql_query);
        // g_master_conn_fail_num ++;
        return false;
    }
    else
    {
        mysql_ping(m_mysql);
        goto again;
    }

    if (mysql_affected_rows(m_mysql) > 0)
    {
        return true;
    }
    else
    { // 影响的行数为0时
        if (care_affected_rows)
        { // 如果在意影响的行数时, 返回false, 否则返回true
            log_error("mysql_real_query failed: %s, sql: %s\n\n", mysql_error(m_mysql), sql_query);
            return false;
        }
        else
        {
            log_warn("affected_rows=0, sql: %s\n\n", sql_query);
            return true;
        }
    }
}

bool CDBConn::StartTransaction()
{
}

bool CDBConn::Rollback()
{
}

bool CDBConn::Commit()
{
}
uint32_t CDBConn::GetInsertId()
{
}

/////////////////////////////////////////////////////
CDBPool::CDBPool(const char *pool_name, const char *db_server_ip, uint16_t db_server_port,
                 const char *username, const char *password, const char *db_name, int max_conn_cnt)
{
    m_pool_name = pool_name;
    m_db_server_ip = db_server_ip;
    m_db_server_port = db_server_port;
    m_username = username;
    m_password = password;
    m_db_name = db_name;
    m_db_max_conn_cnt = max_conn_cnt;    //
    m_db_cur_conn_cnt = MIN_DB_CONN_CNT; // 最小连接数量
}

// 释放连接池
CDBPool::~CDBPool()
{
}

int CDBPool::Init()
{
}

/*
 *TODO: 增加保护机制，把分配的连接加入另一个队列，这样获取连接时，如果没有空闲连接，
 *TODO: 检查已经分配的连接多久没有返回，如果超过一定时间，则自动收回连接，放在用户忘了调用释放连接的接口
 * timeout_ms默认为 0死等
 * timeout_ms >0 则为等待的时间
 */
int wait_cout = 0;
CDBConn *CDBPool::GetDBConn(const int timeout_ms)
{
}

void CDBPool::RelDBConn(CDBConn *pConn)
{
}
// 遍历检测是否超时未归还
// pConn->isTimeout(); // 当前时间 - 被请求的时间
// 强制回收  从m_used_list 放回 m_free_list