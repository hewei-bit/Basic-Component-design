#include "AsyncLogging.h"

#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include "Logging.h"
#define LOG_NUM 5000000 // 总共的写入日志行数

using namespace std;
off_t kRollSize = 1 * 1000 * 1000;    // 只设置1M

static AsyncLogging *g_asyncLog = NULL;

static void asyncOutput(const char *msg, int len)
{
  g_asyncLog->append(msg, len);
}

static uint64_t get_tick_count()
{
  struct timeval tval;
  uint64_t ret_tick;

  gettimeofday(&tval, NULL);

  ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
  return ret_tick;
}


void testLogPerformance(int argc, char *argv[])
{
  printf("pid = %d\n", getpid());

  char name[256] = {'\0'};
  strncpy(name, argv[0], sizeof name - 1);
  // 回滚大小kRollSize（1M）, 最大1秒刷一次盘（flush）
  AsyncLogging log(::basename(name), kRollSize, 1);
  Logger::setOutput(asyncOutput);   // 不是说只有一个实例

  g_asyncLog = &log;
  log.start();        // 启动日志写入线程
  uint64_t begin_time = get_tick_count();
  std::cout << "name:" <<::basename(name) << ", begin_time: " << begin_time << std::endl;
  for (int i = 0; i < LOG_NUM; i++)
  {
    LOG_INFO << "NO." << i << " Root Error Message!"; // 47个字节
  }

  // while (1)
  // {
  //   /* code */
  // }
  
  log.stop();
  uint64_t end_time = get_tick_count();
  std::cout << "end_time: " << end_time << std::endl;
  int64_t ops = LOG_NUM;
  ops = ops * 1000 / (end_time - begin_time);
  std::cout << "need the time1: " << end_time << " " << begin_time << ", " << end_time - begin_time << "毫秒"
            << ", ops = " << ops << "ops/s\n";
}

void testCoredump()
{
    AsyncLogging log("coredump", 200*1000*1000);
    log.start();   //开启日志线程
    g_asyncLog = &log;
    int msgcnt = 0;
    Logger::setOutput(asyncOutput);//设置日志输出函数
 

    for(int i=0;i<1000000;i++)   {//写入100万条日志消息
      LOG_INFO << "0voice 0123456789" << " abcdefghijklmnopqrstuvwxyz "
               << ++msgcnt;
      if(i == 500000) {
         int *ptr = NULL;
        *ptr = 0x1234;  // 人为制造异常
      }
    }
   
}


int main(int argc, char *argv[])
{
  // 这里每次二选1去进行测试
  testLogPerformance(argc, argv);   
  // testCoredump();
  
  return 0;
}
