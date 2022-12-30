//创建服务器

#pragma once
#include <iostream>
#include <thread>
#include "tinyasync/tinyasync.h"
using namespace tinyasync;

class Session;

class server {
    private:
        IoContext m_ctx; //事件中心
        Acceptor m_acceptor;  // acceptor
        PoolResource m_pool;  //内存池
        std::thread m_thread;
        int m_id;
    public:

        server(int i,Acceptor & acc){
            m_id = i;
            m_acceptor = acc.reset_io_context(m_ctx);
        }

        Task<> deal(Connection s);

        //监听 连接
        Task<> listen(IoContext & ctx);

        // 启动
        void run(){
            m_thread = std::thread([this](){

                try {
                    //1 初始化 内存池
                    //nothing

                    //2 创建监听协程
                    co_spawn(listen(m_ctx));
                    //3 启动事件中心
                    m_ctx.run();
                }
                catch(...) {
                    printf("%s\n", to_string(std::current_exception()).c_str());
                }
            });

        }
    
};

Task<> server::listen(IoContext & ctx){
    for(;;){
        Connection conn = co_await m_acceptor.async_accept();
        co_spawn(deal(std::move(conn)));

    }
}

//对连接进行处理
Task<> server::deal(Connection conn){
    
    char buff[1024];
    //1 读取数据,保证数据读取完毕
    std::size_t nread=  0;
    for(;;) {
        try {
            nread = co_await conn.async_read_timeout(buff,1024);
        }
        catch(std::exception & e){
            std::cout << "error" << "\n";
            std::cout << e.what() << "\n";
        }
        if(nread == 0 ) break;
    }

    // 进行相应的处理

    //发送
    // repeat to send all read
    auto remain = nread;
    char *buf = buff;
    for(;;) {
        printf("wait send...");
        auto sent = co_await conn.async_send(buf, remain);
        if(!sent)
            break;
        buf += sent;
        remain -= sent;
        if(!remain)
            break;
    }

}
