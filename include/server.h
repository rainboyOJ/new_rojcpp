//创建服务器

#pragma once
#include <iostream>
#include <thread>
#include "tinyasync/tinyasync.h"
#include "debug.h"
#include "http_session.h"
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
        void join(){
            m_thread.join();
        }

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
    /**
     * 1. detect point that client send half close
     * 2. send shutdown(RD,client_fd) to client
     * 3. close(client_fd) , when http Session object approach Endpoint
     */

    //1. 创建一个http_session
    rojcpp::http_session Session( std::pmr::new_delete_resource() );

    //3.处理数据与路由
    //4.异步读取
    
    // char buff[1024];
    //1 读取数据,保证数据读取完毕
    // 2.异步读取
    std::size_t nread=  0;
    for(;;) {
        try {
            auto [buff,buff_size] = Session.req_buff();
            // nread = co_await conn.async_read_timeout(buff,buff_size);
            nread = co_await conn.async_read(buff,buff_size);
        }
        catch(std::exception & e){
            std::cout << "error" << "\n";
            std::cout << e.what() << "\n";
        }

        if(nread == 0 ) break; // represent that client close socket
        Session.update_req_buff_used_size(nread);


        // check
        if( Session.handle_read() != -2) // -2 represent header data not complete
            break;

        debug("read count is",nread);
    }

    // 进行相应的处理
    Session.process();

    //发送
    // repeat to send all read
    auto [send_buff,remain] = Session.res_buff();
    for(;;) {
        printf("wait send...");
        auto sent = co_await conn.async_send(send_buff, remain);
        if(!sent)
            break;
        send_buff += sent;
        remain -= sent;
        if(!remain)
            break;
    }

}

