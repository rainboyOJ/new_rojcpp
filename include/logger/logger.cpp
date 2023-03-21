#include <iostream>
#include "logger.h"

namespace ThreadInfo {
    __thread char t_errnobuff[512];
    __thread char t_time[64];
    __thread std::size_t t_lastSecond;
    
} // end namespace ThreadInfo

namespace LOGGER {

const char * getErrnoMsg(int savedErrno) {
    return strerror_r(savedErrno, 
            ThreadInfo::t_errnobuff, 
            sizeof(ThreadInfo::t_errnobuff)
            );
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
