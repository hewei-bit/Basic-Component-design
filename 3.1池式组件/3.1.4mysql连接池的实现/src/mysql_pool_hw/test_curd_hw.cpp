/**
 * @File Name: test_curd.cpp
 * @Brief : In User Settings Edit
 * @Author : hewei (hewei_1996@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-03-25
 *
 */

#include "DBPool.h"
#include "IMUser.h"
using namespace std;
// 登录数据库: mysql -u root -p
// 创建数据库:  CREATE DATABASE 数据库名;
// 删除数据库: DROP DATABASE 数据库名;
// 显示所有的数据库: show DATABASE;
// 进入某个数据库: USE 数据库名;
// 查看所有表: SHOW tables
// 查看某个表结构:

#define DB_HOST_IP "127.0.0.1" // 数据库服务器ip
#define DB_HOST_PORT 3306
#define DB_DATABASE_NAME "mysql_pool_test" // 数据库对应的库名字, 这里需要自己提前用命令创建完毕
#define DB_USERNAME "root"                 // 数据库用户名
#define DB_PASSWORD "123456"               // 数据库密码
#define DB_POOL_NAME "mysql_pool"          // 连接池的名字，便于将多个连接池集中管理
#define DB_POOL_MAX_CON 4                  // 连接池支持的最大连接数量

static uint32_t IMUser_nId = 0;

// 把连接传递进去
bool insertUser(CDBConn *pDBConn)
{
}

bool queryUser(CDBConn *pDBConn)
{
}

bool updateUser(CDBConn *pDBConn)
{
}

// 测试本地的数据库连接
void testConnect()
{
}
// 测试本地数据库的创建（Create）、更新（Update）、读取（Retrieve）和删除（Delete）操作。
// 测试增删改查
void testCurd()
{
}

// 默认端口 3306
// 测试一次连接和端口的情况： tcpdump -i any port 3306
// 参考宏定义设置自己数据库的相关信息
/*
#define DB_HOST_IP          "127.0.0.1"             // 数据库服务器ip
#define DB_HOST_PORT        3306
#define DB_DATABASE_NAME    "mysql_pool_test"       // 数据库对应的库名字, 这里需要自己提前用命令创建完毕
#define DB_USERNAME         "root"                  // 数据库用户名
#define DB_PASSWORD         "123456"                // 数据库密码
#define DB_POOL_NAME        "mysql_pool"            // 连接池的名字，便于将多个连接池集中管理
#define DB_POOL_MAX_CON     4                       // 连接池支持的最大连接数量
*/
int main()
{
    printf("test TestConnect begin\n");
    testConnect();
    printf("test TestConnect finish\n\n");

    printf("test TestCurd begin\n");
    testCurd();
    printf("test TestCurd finish\n");
    return 0;
}