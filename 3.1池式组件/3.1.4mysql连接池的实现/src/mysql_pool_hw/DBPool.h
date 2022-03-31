#ifndef DBPOOL_H_
#define DBPOOL_H_

#include <iostream>
#include <list>
#include <mutex>
#include <condition_variable>
#include <map>
#include <stdint.h>

#include <mysql.h>

#define MAX_ESCAPE_STRING_LEN 10240

using namespace std;

// 返回结果 select的时候用
class CResultSet
{
public:
    CResultSet(MYSQL_RES *res);
    virtual ~CResultSet();

    bool Next();
    int GetInt(const char *key);
    char *GetString(const char *key);

private:
    int _GetIndex(const char *key);

    MYSQL_RES *m_res;
    MYSQL_RES m_row;
    map<string, int> m_key_map;
};

// 插入数据用
class CPrepareStatement
{
};

class CDBPool;

class CDBConn
{
};

// 只是负责管理连接CDBConn，真正干活的是CDBConn
class CDBPool
{
};

#endif /* DBPOOL_H_ */
