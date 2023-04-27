#include "server.h"
#include "routes/user.hpp"
#include "sql/query.hpp"
#include "__config.h"
#include "serializable.hpp"
#include "jsonEnity/jsonEntiy.hpp"
#include "judge/judgeServerMediator.hpp"



//>>>> async logger 
LOGGER::AsyncLogger * g_global_logger = nullptr;

//proxy AsyncLogger append function
void append_log_msg(const char * msg,int len){
    if( g_global_logger) {
        g_global_logger->append(msg, len);
    }
}

void global_flush() {}

const int PORT = 8899;

int main() {
    


    //logger相关设置
#ifdef DEBUG    //debug mode
    LOGGER::logger::setLogLevel(LOGGER::logger::LogLevel::TRACE);
#else       //release mode
    //创建全局的AsyncLogger
    LOGGER::AsyncLogger global_async_log("rojcpp",32*1024*1024); //32mb
    g_global_logger = &global_async_log;
    LOGGER::logger::setLogLevel(LOGGER::logger::LogLevel::INFO);
    LOGGER::logger::setOutput(append_log_msg);
    LOGGER::logger::setFlush(global_flush);
#endif
    //1. 初始化cppjson
    cppjson::Serializable::Regist<userRegistJson>();
    cppjson::Serializable::Regist<userLoginJson>();

    //1. 初始化路由的连接

    const std::string connection_info_ = std::string(__config__::connection_info_);
    cppdb::pool_manager::init(connection_info_);

    LOG_INFO << "rojcpp Server start at port : " << PORT ;



//==== 注册路由
    {
        using namespace rojcpp;
        USR_API::regist_route<server>(); //注册用户相关的路由


        Acceptor myacc(Protocol::ip_v4(),Endpoint(Address::Any(),PORT));
        server myserver(1,myacc);
        myserver.run();
        myserver.join();
    }
    return 0;
}
