// FileName: test_log4cpp2.cpp
// Test log4cpp by config file.
#include "log4cpp/Category.hh"
#include "log4cpp/PropertyConfigurator.hh"
#include "log4cpp/NDC.hh"
// 编译：g++ -o 3-test_log4cpp  3-test_log4cpp.cpp -llog4cpp -lpthread

void test(log4cpp::Category& category)
{
    log4cpp::NDC::push(__FUNCTION__);     // 记录NDC信息
    category.info("零声学院");
    log4cpp::NDC::pop();
}
int main(int argc, char* argv[])
{
    // 1 读取解析配置文件
    // 读取出错, 完全可以忽略，可以定义一个缺省策略或者使用系统缺省策略
    // BasicLayout输出所有优先级日志到ConsoleAppender
    try { 
        log4cpp::PropertyConfigurator::configure("./3-test_log4cpp.conf");
    } catch(log4cpp::ConfigureFailure& f) {
        std::cout << "Configure Problem " << f.what() << std::endl;
        return -1;
    }
     
    // 2 实例化category对象
    // 这些对象即使配置文件没有定义也可以使用，不过其属性继承其父category
    // 通常使用引用可能不太方便，可以使用指针，以后做指针使用
    // log4cpp::Category* root = &log4cpp::Category::getRoot();
    log4cpp::Category& root = log4cpp::Category::getRoot();
    log4cpp::NDC::push(__FUNCTION__);     // 记录NDC信息
    // log4cpp::NDC::push("ndc2");
    log4cpp::Category& sub1 = 
        log4cpp::Category::getInstance(std::string("sub1"));
         log4cpp::Category& sub1 = 
        log4cpp::Category::getInstance(std::string("sub1"));
    log4cpp::Category& sub1_sub2 = 
        log4cpp::Category::getInstance(std::string("sub1.sub2"));
    // 3 正常使用这些category对象进行日志处理。
    // sub1 has appender A1 and rootappender.
    sub1.info("sub1 This is some info");

    test(root);
    root.info("root This is some info");
    // sub1.alert("sub1 A warning");
     
    // // sub1_sub2 only have A2 appender.
    sub1_sub2.debug("sub1_sub2 This debug message will fail to write");
    sub1_sub2.alert("sub1_sub2 All hands abandon ship");
    sub1_sub2.critStream() << "sub1_sub2 This will show up << as " << 1 << " critical message";
    sub1_sub2 << log4cpp::Priority::ERROR 
              << "sub1_sub2 And this will be an error";
    sub1_sub2.log(log4cpp::Priority::WARN, "sub1_sub2 This will be a logged warning");
     // clean up and flush all appenders
    log4cpp::Category::shutdown();
    return 0;
}