
// 一个处理 http 请求的 类
#pragma once
#include "tinyasync/tinyasync.h"
using namespace tinyasync;

class session {
    private:
        PoolResource * m_pool; //内存池
        IoContext * m_ctx; // 事件中心
        Connection m_conn; //连接
        char * buff[1024];
    public:
        
        //读取内容,一直读取完毕
        void read() {
            std::size_t nread;
            try {
                nread = co_await m_conn.async_read_timeout(buff);
            }
            catch(...) {
                // print error
                // break;
            }
        }
        //发送内容
        void send();
};
