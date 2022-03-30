/*
FileAppender和RollingFileAppender是log4cpp中最常用的两个Appender，其功能是将日志写入文件中。
它们之间唯一的区别就是前者会一直在文件中记录日志(直到操作系统承受不了为止)，而后者会在文件长度到
达指定值时循环记录日志，文件长度不会超过指定值(默认的指定值是10M byte)。

FileAppender的创建函数如下：
FileAppender(const std::string& name, const std::string& fileName,
                bool append = true, mode_t mode = 00644);


一般仅使用前两个参数，即“名称”和“日志文件名”。第三个参数指示是否在日志文件后继续记入日志，
还是清空原日志文件再记录。第四个参数说明文件的打开方式。

RollingFileAppender的创建函数如下：
RollingFileAppender(const std::string& name, 
                        const std::string& fileName,
                        size_t maxFileSize = 10*1024*1024, 
                        unsigned int maxBackupIndex = 1,
                        bool append = true,
                        mode_t mode = 00644);


它与FileAppender的创建函数很类似，但是多了两个参数：maxFileSize指出了回滚文件的最大值;
maxBackupIndex指出了回滚文件所用的备份文件的最大个数。所谓备份文件，是用来保存回滚文件中
因为空间不足未能记录的日志，备份文件的大小仅比回滚文件的最大值大1kb。所以如果maxBackupIndex取值为3，
则回滚文件(假设其名称是rollwxb.log，大小为100kb)会有三个备份文件，
其名称分别是rollwxb.log.1，rollwxb.log.2和rollwxb.log.3，大小为101kb。
另外要注意：如果maxBackupIndex取值为0或者小于0，则回滚文件功能会失效，其表现如同FileAppender一样，
不会有大小的限制。
*/
#include <iostream>
#include <sys/time.h>
#include <stdint.h>
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/RollingFileAppender.hh>

#define LOG_NUM 10000         // 总共的写入日志行数

using namespace std;

static uint64_t get_tick_count()
{
    struct timeval tval;
    uint64_t ret_tick;
    
    gettimeofday(&tval, NULL);
    
    ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
    return ret_tick;
}

// g++ -g -o 4-RollingFileAppender  4-RollingFileAppender.cpp -llog4cpp -lpthread
int main(int argc, char* argv[])
{
    log4cpp::TimeStamp time;
    log4cpp::PatternLayout* pLayout1 = new log4cpp::PatternLayout();
    log4cpp::BasicLayout* pBasicLayout = new log4cpp::BasicLayout();
    //  2020-06-21 17:36:01,946: ERROR  RootName : 0:Root Error Message!
    //  %d: 2020-06-21 17:36:01,946: 
    //  %p ERROR  
    //  %c RootName : 0
    //  %m:Root Error Message!
    // pLayout1->setConversionPattern("%d: %p %c %x: %m%n");
    // 2020-06-21 17:40:34,772: ERROR RootName: 10Root Error Message!
    // pLayout1->setConversionPattern("%d: %p %c: %m%n");
    // 2020-06-21 17:41:41,369: ERROR : 0Root Error Message!
    pLayout1->setConversionPattern("%d: %p %c %x: %m%n");

    log4cpp::PatternLayout* pLayout2 = new log4cpp::PatternLayout();
    pLayout2->setConversionPattern("%d: %p %c %x: %m%n");
    
    log4cpp::Appender* fileAppender = new log4cpp::FileAppender("fileAppender","4-FileAppender.log",
                        false);
    fileAppender->setLayout(pLayout1);

    log4cpp::RollingFileAppender* rollfileAppender = new log4cpp::RollingFileAppender(
        "rollfileAppender","4-RollingFileAppender.log",100*1024, 5,  false);
    rollfileAppender->setLayout(pLayout2);
    
    log4cpp::Category& root = log4cpp::Category::getRoot().getInstance("RootName");
    root.addAppender(fileAppender);
    root.setPriority(log4cpp::Priority::DEBUG);

    std::cout << "FileAppender-----------------> "<< std::endl;
    uint64_t begin_time = get_tick_count();
    std::cout << "begin_time: " << begin_time << std::endl;
    for (int i = 0; i < LOG_NUM; i++)
    {
        string strError;
        ostringstream oss;
        oss<<"NO." << i<<" Root Error Message!";    // 47个字节
        strError = oss.str();
        root.error(strError);
    }
    uint64_t end_time = get_tick_count();
    std::cout << "end_time: " << end_time << std::endl;
    int64_t ops = LOG_NUM;
    ops = ops * 1000 /(end_time - begin_time);
    std::cout << "need the time: " << end_time << " " << begin_time << ", " << end_time - begin_time << "毫秒" << ", ops = " << ops << "ops/s\n\n" ;


    std::cout << "RollingFileAppender-----------------> "<< std::endl;
    root.removeAppender(fileAppender);
    root.addAppender(rollfileAppender);
    begin_time = get_tick_count();
    std::cout << "begin_time: " << begin_time << std::endl;
    for (int i = 0; i < LOG_NUM; i++)
    {
        string strError;
        ostringstream oss;
        oss<<"NO." << i<<" Root Error Message!";    // 47个字节
        strError = oss.str();
        root.error(strError);
    }
    end_time = get_tick_count();
    std::cout << "end_time: " << end_time << std::endl;
    ops = LOG_NUM;
    ops = ops * 1000 /(end_time - begin_time);
    std::cout << "need the time: " << end_time << " " << begin_time << ", " << end_time - begin_time << "毫秒" << ", ops = " << ops << "ops/s\n" ;

    log4cpp::Category::shutdown();
    return 0;
}