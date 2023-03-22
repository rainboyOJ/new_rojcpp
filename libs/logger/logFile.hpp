#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <mutex>
#include <cstdio>
#include "logger.h"

namespace LOGGER {
    

//作用: 文件操作的API, 
//   1. 打开文件
//   2. 写入文件
//   3. 关闭文件
class fileUtil {
    
    public:
        explicit fileUtil(const char * fileName)
            :m_fp(::fopen(fileName, "ae")),
            m_writtenBytes(0)
        {
            //将 m_fp 的缓冲区设置为本地的 m_buff
            ::setbuffer(m_fp, m_buff, sizeof(m_buff));
        }

        ~fileUtil() { ::fclose(m_fp);}

        void append(const char * data,std::size_t len) {
            std::size_t written = 0;
            while( written != len) {
                auto remain = len - written;
                std::size_t n = write(data + written,remain);
                if( n != remain) {
                    int err = ferror(m_fp);
                    if( err ) {
                        fprintf(stderr, "fileUtil::append() failed %s\n",getErrnoMsg(err));
                    }
                }
                written += n;
            }
            m_writtenBytes += written;
        }

        void flush() { ::fflush(m_fp); }

        off_t writtenBytes() const { return m_writtenBytes; }

    private:
        std::size_t write(const char * data,std::size_t len) {
                /**
                 * size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
                 * -- buffer:指向数据块的指针
                 * -- size:每个数据的大小，单位为Byte(例如：sizeof(int)就是4)
                 * -- count:数据个数
                 * -- stream:文件指针
                 */
            return ::fwrite_unlocked(data, 1, len, m_fp);
        }

        FILE * m_fp;          // 文件句柄
        char m_buff[64*1024]; // 64kb
        off_t m_writtenBytes; // 表示文件的偏移量
};


class logFile {
    using TimeType = std::chrono::microseconds;

    public:

        logFile(const char * filename,
                off_t rollSize,
                int flushInterval = 3, // flush per 3 second 
                int checkEveryN = 1024
                ) 
            :m_rollSize(rollSize),
            m_flushInterval(flushInterval),
            m_checkEveryN(checkEveryN),
            m_count(0),
            m_startOfPerid(0),
            m_lastRoll(0),
            m_lastFlush(0)
        { 
            memcpy(m_filename,filename,strlen(filename));
            rollFile();
        }

        ~logFile() = default;

        void append(const char * data,int len) {
            std::lock_guard<std::mutex> lck(m_mtx);
            appendInLock(data, len);
        }

        void flush() {
            m_file->flush();
        }

        //滚动日志,创建log文件
        bool rollFile()
        {
            std::time_t now = 0;
            std::string filename = getLogFileName(m_filename, &now);

            // 计算现在是第几天 now/kRollPerSeconds求出现在是第几天，再乘以秒数相当于是当前天数0点对应的秒数
            std::time_t start = now / kRollPerSeconds * kRollPerSeconds;

            if( now > m_lastRoll) {
                m_lastRoll = now;
                m_lastFlush = now;
                m_startOfPerid = start;
                m_file.reset(new fileUtil(filename.c_str()));
                return true;
            }
            return false;
        }

    private:
        // 根据文件名 和 当前的时间 得到新的文件名
        // 同时设置 now 的时间
        static std::string getLogFileName(const char * filename,std::time_t * now) {
            std::string filename_str;
            filename_str.reserve(strlen(filename) + 64);
            filename_str = filename;

            char timeBuf[32];
            struct tm _tm;
            *now = ::time(NULL);
            localtime_r(now, &_tm);

            strftime(timeBuf, sizeof(timeBuf), ".%Y%m%d-%H%M%S", &_tm);
            filename_str += timeBuf;
            filename_str += ".log";

            return filename_str;
        }
        void appendInLock(const char * data, int len) {
            m_file -> append(data, len);
            //写入的尺寸超过了设定的尺寸
            if( m_file -> writtenBytes() > m_rollSize) {
                rollFile();
            }
            else {
                ++m_count; // 写入的次数
                if(m_count >= m_checkEveryN ) {
                    m_count = 0;
                    std::time_t now = ::time(NULL);
                    std::time_t thisPeriod = now / kRollPerSeconds * kRollPerSeconds;
                    if( thisPeriod != m_startOfPerid) {
                        rollFile();
                    }
                    else if ( now - m_lastFlush > m_flushInterval ) {
                        m_lastFlush = now;
                        m_file -> flush();
                    }
                }
            }
        }

        char m_filename[128];       // 文件名
        const off_t m_rollSize;     // 写入多少尺寸后需要,rollFile
        const int m_flushInterval;  // 多少时间间隔,需要flush
        const int m_checkEveryN;    // 写入多少次后,需要检查一下

        int m_count; // 记录写入次数

        // std::unique_ptr<std::mutex> m_mutex_ptr;
        std::mutex m_mtx;
        std::time_t m_startOfPerid;
        std::time_t m_lastRoll;
        std::time_t m_lastFlush;

        std::unique_ptr<fileUtil> m_file;
        // fileUtil m_file;

        //24h 
        constexpr static int kRollPerSeconds = 60*60*24;
};



} // end namespace LOGGER
