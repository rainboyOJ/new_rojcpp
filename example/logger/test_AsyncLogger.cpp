#include "logger/AsyncLogger.hpp"

using namespace LOGGER;

// 一个全局的 AsyncLogger 指针
AsyncLogger * g_asyncLogger = nullptr;

inline AsyncLogger * get_global_asynclogger() {
    return g_asyncLogger;
}

void async_log (const char *msg,int len){
    auto * logging = get_global_asynclogger();
    if( logging) {
        logging->append(msg, len);
    }
}

void test_log() {
    LOG_DEBUG << "debug";
    LOG_INFO << "info";
    LOG_WARN << "warn";
    LOG_ERROR << "error";
    const int a = 10;
    for(int i=1;i<=10;++i){
        LOG_INFO << "Hello , " << i << " abc...xyz";
    }
}

int main(){
    
    // test_log();
    // 1. 创建AsyncLogger
    AsyncLogger myLog("test_log",1*1024*1024);
    g_asyncLogger = &myLog;
    logger::setOutput(async_log);
    myLog.start();
    test_log();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    myLog.stop();
    return 0;
}
