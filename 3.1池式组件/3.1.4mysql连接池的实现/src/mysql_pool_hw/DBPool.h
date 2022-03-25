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
};

// 插入数据用
class CPrepareStatement
{
};

class CDBPool;

class CDBConn
{
};

class CDBPool
{
    // 只是负责管理连接CDBConn，真正干活的是CDBConn
};

#endif /* DBPOOL_H_ */
