#include <iostream>
#include "logger.h"
#include <algorithm>

#include <cstring>
#include <thread>
#include <sstream>


namespace ThreadInfo {
    thread_local char t_errnobuff[512];
    thread_local char t_time[66];
    thread_local std::size_t t_lastSecond;
    //thread_local std::hash<std::thread::id> th_id ;// = std::this_thread::get_id();
    thread_local char t_thread_id[64] = {0};
} // end namespace ThreadInfo

namespace LOGGER {

const char * getErrnoMsg(int savedErrno) {
    return strerror_r(savedErrno, 
            ThreadInfo::t_errnobuff, 
            sizeof(ThreadInfo::t_errnobuff)
            );
}

const char * get_thread_id() {
    if( ThreadInfo::t_thread_id[0] == 0 ) {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        auto str = ss.str();
        memcpy(ThreadInfo::t_thread_id+1,str.c_str(),str.length());
        ThreadInfo::t_thread_id[0] = 1;
    }
    return ThreadInfo::t_thread_id+1;
}

void logger::set_thread_id(int id) {
    ThreadInfo::t_thread_id[0] = 1;
    auto * m_cur  = ThreadInfo::t_thread_id + 1;
    auto * start  = ThreadInfo::t_thread_id + 1;
    int num_len = 0;
    int __id = id;
    do { 
        char c = __id % 10 + '0';
        __id /= 10;
        *m_cur++ = c;
    } while ( __id !=0 ) ;
    *m_cur = '\0';
    std::reverse(start,m_cur);
}
    
logger::LogLevel g_log_level = logger::LogLevel::INFO;

static void defaultOutput(const char *msg,int len) {
    fwrite(msg, len, sizeof(char), stdout);
}

static void defaultFlush() {
    fflush(stdout);
}

logger::OutputFuncType g_output = defaultOutput;
logger::FlushFuncType g_flush = defaultFlush;

logger::logger(const char *filename,int line,LogLevel level)
    :logger(level,0,filename,line)
    { }

logger::logger(const char *filename,int line,LogLevel level,const char *funcName)
    :logger(level,0,filename,line)
    { 
        m_logStream<< funcName << ' ';
    }

logger::logger(LogLevel level,int savedErrno,const char *filename,int line)
    :m_log_level(level),
     m_timeStamp(TimeStamp::now()),
     m_logStream(),
     m_line(line),
     m_filename(filename)
    {
        formatTime();
        m_logStream << get_thread_id() << ' ';
        m_logStream << getLevelName(level);
        
        if( savedErrno != 0) {
            m_logStream << getErrnoMsg(savedErrno) 
                << " (errno=" 
                << savedErrno <<") ";
        }
    }

void logger::formatTime() {
    std::string timestr = m_timeStamp.toFormattedString(true);
    m_logStream << timestr << ' ';
}

void logger::finish() {
    m_logStream << " - "
            << m_filename
            << ':'
            << m_line
            << '\n';
}

logger::~logger() {
    // std::cout << "buff length: "<< m_logStream.get_buff().length() << "\n";
    finish();

    auto & buff = m_logStream.get_buff();

    g_output(buff.data(),buff.length());

    // FATAL情况终止程序
    if( m_log_level == LogLevel::FATAL) {
        g_flush();
        abort();
    }
}

void logger::setLogLevel(LogLevel level) {
    g_log_level = level;
}

void logger::setOutput(OutputFuncType out) {
    g_output = out;
}

void logger::setFlush(FlushFuncType flu) {
    g_flush = flu;
}


} // end namespace LOGGER
