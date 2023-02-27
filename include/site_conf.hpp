
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
    static constexpr int  session_expire = 15*24*60*70; // 15 days
    static constexpr auto static_dir = "www"sv;
    static constexpr auto upload_dir = "upload"sv;
    static constexpr int  alarm_time = 30; //seconds

    //服务器起动的配置
    static constexpr int port            = 8099;
    static constexpr int trigMode        = 0; // 暂时不能运行在ET模式,工作在LT模式
    static constexpr int timeoutMS       = 3*60*1000; //3分钟
    static constexpr bool OptLinger      = true; //优雅的关闭socket
    static constexpr int sqlPort         = 3306;
    static constexpr const char* sqlUser = "root";
    static constexpr const char* sqlPwd = "root";
    static constexpr const char* dbName  = "netcore";
    static constexpr int connPoolNum     = 4;
    static constexpr int threadNum       = 4;

    //其它配置
    static constexpr std::size_t maxRequestSize = 3_MB;
    static constexpr std::size_t staticResCacheMaxAge = 0;

    //下载类型
    static constexpr transfer_type transfer_type_ = transfer_type::ACCEPT_RANGES;

    //websocket
    static constexpr std::size_t ws_thpool_size = 4; // ws_manager 里的线程池大小,用来主动发送数据
};



