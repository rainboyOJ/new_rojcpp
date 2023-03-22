// #include <iostream>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "logger.h"
#include "logFile.hpp"

namespace LOGGER {
    

//连接的前后端的纽带
class AsyncLogger {
    public:

        AsyncLogger(const char * filename,off_t rollSize,int flushInterval = 3)
            :m_flushInterval(flushInterval),
            m_running(false),
            m_fileName(filename),
            m_rollSize(rollSize)
            {
                m_currentBuffer.reset(new Buffer);
                m_nextBuffer.reset(new Buffer);

                m_currentBuffer->bzero();
                m_nextBuffer->bzero();
                m_buffers.reserve(16);
            }
        ~AsyncLogger () {
            if( m_running) {
                stop();
            }
        }

        //前端 接口
        void append(const char * msg,int len) {
            std::lock_guard<std::mutex> lock(m_mtx);

            //情况1: 容量足够
            if( m_currentBuffer -> avail() > len) {
                m_currentBuffer->append(msg, len);
            }
            else { //情况2: 容量不足够
                m_buffers.push_back(std::move(m_currentBuffer));
                if( m_nextBuffer ) { //move nextBuffer
                    m_currentBuffer = std::move(m_nextBuffer);
                }
                else {
                    m_currentBuffer.reset(new Buffer);
                }

                m_currentBuffer->append(msg, len);
                //唤醒写入磁盘得后端线程
                m_cond.notify_one();
            }
        }

        //启动输出的的线程
        void start() { 
            m_running.store(true);
            m_thd = std::thread([this] { this->threadFunc(); });
        }

        //停止输出的线程
        void stop() {
            m_running.store(false);
            m_cond.notify_one();
            m_thd.join();
        }

    private:

        //后端 ,一个不停运行的线程,负责写入数据到文件
        void threadFunc() {

            // std::cout << "in_thread func" << "\n";
            logFile file(m_fileName.c_str(),m_rollSize);

            //创建两个Buffer
            BufferPtr newBuff1(new Buffer);
            BufferPtr newBuff2(new Buffer);
            newBuff1->bzero();
            newBuff2->bzero();

            //用于和前端的BufferVecotr 交换
            BufferVecotr buffersToWrite;
            buffersToWrite.reserve(16);

            while( m_running.load() ) {

                {
                    // 互斥锁保护，这样别的线程在这段时间就无法向前端Buffer数组写入数据
                    std::unique_lock<std::mutex> lock(m_mtx);
                    if( m_buffers.empty() ) {
                        // 等待三秒也会接触阻塞
                        // std::cout << " in wait_for" << "\n";
                        m_cond.wait_for(lock, std::chrono::seconds(3));
                    }

                    m_buffers.push_back(std::move(m_currentBuffer));

                    m_currentBuffer = std::move(newBuff1);
                    buffersToWrite.swap(m_buffers);
                    if( !m_nextBuffer) {
                        m_nextBuffer = std::move(newBuff2);
                    }
                }

                for( const auto & buffer : buffersToWrite ) {
                    file.append(buffer->data(), buffer->length());
                }

                // 只保留两个缓冲区
                if( buffersToWrite.size() > 2 ) {
                    buffersToWrite.resize(2);
                }

                if( !newBuff1 && !buffersToWrite.empty() ) {
                    newBuff1 = std::move(buffersToWrite.back());
                    buffersToWrite.pop_back();
                    newBuff1->reset();
                }
                if( !newBuff2 && !buffersToWrite.empty()) {
                    newBuff2 = std::move(buffersToWrite.back());
                    buffersToWrite.pop_back();
                    newBuff2->reset();
                }

                buffersToWrite.clear();
                // std::cout << "write to file" << "\n";
                file.flush();
            }

        }
        
        std::mutex m_mtx;
        std::atomic<bool> m_running; //运行的标志

        const int m_flushInterval; // flush的时间间隔
        const std::string m_fileName; //文件名
        const off_t m_rollSize; //每个文件记录大小的上限
        std::thread m_thd; //后端输出线程
        std::condition_variable m_cond;//条件变量

        using Buffer = logBuffer<kLargeBuffer>; // 4mb
        using BufferVecotr = std::vector<std::unique_ptr<Buffer>>; //
        using BufferPtr = BufferVecotr::value_type;

        BufferPtr m_currentBuffer; // 前端的Buffer1
        BufferPtr m_nextBuffer;// 前端的Buffer2
        BufferVecotr m_buffers; //存已经用过的Buffer
        
};

/*
//单例模式
class AsyncLoggerSingleton {

    public:
        static AsyncLogger & get() {
            static AsyncLoggerSingleton log;
            return log.m_log;
        }

        static void append(const char * msg,int len) {
            AsyncLoggerSingleton::get().append(msg, len);
        }
    private:
        AsyncLogger m_log;

};
*/


} // end namespace LOGGER
