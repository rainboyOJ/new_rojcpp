//全局配置类
//当你需要修改的时候就修改个文件
#pragma once

#include <type_traits>
#include <string_view>
using namespace std::literals;

//下载类型
enum class transfer_type {
    CHUNKED,
    ACCEPT_RANGES
};


constexpr std::size_t operator"" _MB ( unsigned long long const x ){
    return 1024L*1024L*x;
}

struct __config__{

    static constexpr std::string_view connection_info_ ="mysql:host=127.0.0.1;port=3306;user=root;password=root;database=rojcpp;";

    static constexpr int  work_thread = 1; 

    static constexpr int  session_expire = 15*24*60*70; // 15 days
    static constexpr auto static_dir = "www"sv;
    static constexpr auto upload_dir = "upload"sv;
    static constexpr int  alarm_time = 30; //seconds

    //服务器起动的配置
    static constexpr int port            = 8099;
    static constexpr int trigMode        = 0; // 暂时不能运行在ET模式,工作在LT模式
    static constexpr int timeoutMS       = 3*60*1000; //3分钟
    static constexpr bool OptLinger      = true; //优雅的关闭socket

    static constexpr const char* sqlServer = "127.0.0.1";
    static constexpr int sqlPort         = 3306;
    static constexpr const char* sqlUser = "root";
    static constexpr const char* sqlPwd = "root";
    static constexpr const char* dbName  = "rojcpp";
    static constexpr int connPoolNum     = 4;
    static constexpr int threadNum       = 4;
    static constexpr bool openLog        = true;
    static constexpr int logLevel        = 0;
    static constexpr int logQueSize      = 1;

    //其它配置
    static constexpr std::size_t maxRequestSize = 20_MB;
    static constexpr std::size_t staticResCacheMaxAge = 0;

    //下载类型
    static constexpr transfer_type transfer_type_ = transfer_type::ACCEPT_RANGES;

    //websocket
    static constexpr std::size_t ws_thpool_size = 4; // ws_manager 里的线程池大小,用来主动发送数据

    //Redis的配置
    //std::string_view ip,int port,std::string_view pass,int poolSize
    static constexpr const char* Redis_ip       = "127.0.0.1";
    static constexpr const int Redis_port       = 6379;
    //static constexpr const char* Redis_password = nullptr;
    static constexpr const int Redis_poolsize   = 4;


    //judgeServer 连接相关
    static constexpr const char * JUDGE_SERVER_IP  = "127.0.0.1";
    static constexpr int JUDGE_SERVER_PORT         = 9000;
    static constexpr int JUDGE_SERVER_CONNECT_SIZE = 4;

    //加密的key,长度必须是16,128bit
    static constexpr unsigned char ENCRYT_KEY[17] = "0123456789ROJCPP";

};

//judger_server的配置
struct __CONFIG {
    //基础题目路径
    static constexpr std::string_view BASE_PROBLEM_PATH = "/home/rainboy/mycode/RainboyOJ/problems/problems";
    static constexpr std::string_view BASE_WORK_PATH= "/tmp";

    //judger 的位置
    static constexpr std::string_view judger_bin = "/usr/bin/judger_core";
    static constexpr std::size_t memory_base = 16*1024*1024; // 16mb
    
};



