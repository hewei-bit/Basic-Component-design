// FileName: test_log4cpp1.cpp
#include "log4cpp/Category.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/SimpleLayout.hh"

// 编译 g++ -o 2-test_log4cpp 2-test_log4cpp.cpp -llog4cpp
int main(int argc, char *argv[])
{
    // 1实例化一个layout 对象
    log4cpp::Layout *layout = new log4cpp::SimpleLayout();   // 有不同的layout
    // 2. 初始化一个appender 对象
    log4cpp::Appender *appender = new log4cpp::FileAppender("FileAppender",
     "./2-test_log4cpp.log");
    log4cpp::Appender *osappender = new log4cpp::OstreamAppender("OstreamAppender",
        &std::cout);
    // 3. 把layout对象附着在appender对象上
    appender->setLayout(layout);
    // appender->addLayout 没有addLayout，一个layout格式样式对应一个appender
    // 4. 实例化一个category对象
    log4cpp::Category &warn_log =
        log4cpp::Category::getInstance("darren");   // 是一个单例工厂
    // 5. 设置additivity为false，替换已有的appender
    warn_log.setAdditivity(false);
    // 5. 把appender对象附到category上
    warn_log.setAppender(appender);
    warn_log.addAppender(osappender);
    // 6. 设置category的优先级，低于此优先级的日志不被记录
    warn_log.setPriority(log4cpp::Priority::INFO);
    // 记录一些日志
    warn_log.info("Program info which cannot be wirten, darren = %d", 100);
    warn_log.warn("Program info which cannot be wirten, darren = %d", 100);
    warn_log.debug("This debug message will fail to write");
    warn_log.alert("Alert info");       // C 风格
    warn_log.log(log4cpp::Priority::CRIT, "Importance depends on context");
    // 其他记录日志方式
    warn_log.log(log4cpp::Priority::WARN, "This will be a logged warning, darren = %d", 100);
    warn_log.warnStream() << "This will be a logged warning, darren = " << 100; // C++ 风格

    log4cpp::Priority::PriorityLevel priority;
    bool this_is_critical = true;
    if (this_is_critical)
        priority = log4cpp::Priority::CRIT;
    else
        priority = log4cpp::Priority::DEBUG;
    warn_log.log(priority, "Importance depends on context");

    warn_log.critStream() << "This will show up << as "
                          << 1 << " critical message";
    // clean up and flush all appenders
    log4cpp::Category::shutdown();
    return 0;
}