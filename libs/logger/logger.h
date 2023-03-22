#pragma once
#include <string_view>

#include "timeStamp.hpp"
#include "logStream.hpp"
#include <errno.h>

namespace LOGGER {

class sourceFile {}; //not use

//How to get filename from __FILE__ and concat with __LINE__ at compile time
//https://stackoverflow.com/a/72663972/19867157


const char * getErrnoMsg(int savedErrno);

class logger {
    public:
        enum class LogLevel : int {
            TRACE,
            _DEBUG,
            INFO,
            WARN,
            ERROR, FATAL
        };

        logger(const char * finename,int line,LogLevel level);
        logger(const char * finename,int line,LogLevel level,const char *funcName);
        ~logger();

        //设置 线程 id
        static void set_thread_id(int id);

        
        logStream & stream() { return m_logStream; }

        static LogLevel logLevel();
        static void setLogLevel(LogLevel level);

        //输出函数 与 刷新缓冲区函数
        using OutputFuncType = void(*)(const char * msg,int len);
        using FlushFuncType = void(*)();

        static void setOutput(OutputFuncType);
        static void setFlush(FlushFuncType);

    private:

        logger(LogLevel level,int savedErrno,const char *filename,int line);

        //如果做到,只更新microseconds部分呢
        void formatTime();
        void finish();

        TimeStamp m_timeStamp;
        LogLevel m_log_level;

        int m_line;
        const char * m_filename = nullptr;

        logStream m_logStream;

};


extern logger::LogLevel g_log_level;

inline logger::LogLevel logLevel() {
    return  g_log_level;
}


constexpr std::string_view getLevelName(logger::LogLevel level) {
    using namespace std::literals::string_view_literals;
    switch ( level ) {
        case logger::LogLevel::INFO :
            return "INFO  "sv;
        case logger::LogLevel::_DEBUG:
            return "DEBUG "sv;
        case logger::LogLevel::TRACE:
            return "TRACE "sv;
        case logger::LogLevel::WARN:
            return "WARN  "sv;
        case logger::LogLevel::ERROR:
            return "ERROR "sv;
        case logger::LogLevel::FATAL:
            return "FATAL "sv;
    }
    return "INFO"sv;
}

#define LOG_DEBUG if ( logLevel() <= logger::LogLevel::DEBUG ) \
        logger(__FILE_NAME__,__LINE__,logger::LogLevel::DEBUG,__func__).stream()


#define LOG_INFO if ( logLevel() <= logger::LogLevel::INFO) \
        logger(__FILE_NAME__,__LINE__,logger::LogLevel::INFO).stream()

#define LOG_WARN logger(__FILE_NAME__,__LINE__,logger::LogLevel::WARN).stream()

#define LOG_ERROR logger(__FILE_NAME__,__LINE__,logger::LogLevel::ERROR).stream()

#define LOG_FATAL logger(__FILE_NAME__,__LINE__,logger::LogLevel::FATAL).stream()


} // end namespace LOGGER
