//创建服务器

#pragma once
#include <iostream>
#include <string_view>
#include <thread>
#include "tinyasync/tinyasync.h"
#include "logger/AsyncLogger.hpp"
#include "http_session.h"

#include "http_route.h"

// 评测转发中心
#include "judge/judgeServerMediator.hpp"
#include "judgercpp/judger.h"
#include "judgercpp/src/utils.hpp"
#include "curd/judgeTable.h"

using namespace LOGGER;
using namespace tinyasync;

namespace rojcpp {
    

class Session;

class server;

//函数
judgeServerMediator<rojcpp::server> & JSM() {
   static judgeServerMediator<rojcpp::server> myJSM(__config__::server_size);
   return myJSM;
}

class server {
    private:
        IoContext m_ctx; //事件中心
        Acceptor m_acceptor;  // acceptor
        PoolResource m_pool;  //内存池
        std::thread m_thread;
        int m_id;


        rojcpp::http_handle_func http_handle_ = nullptr;
        rojcpp::http_handle_check_func http_handle_check_ = nullptr;
        

    public:

        static rojcpp::http_router m_http_route; //静态的route容器

        //服务器的初始化
        void init() {

            //把server注册到 评测转发中心
            JSM().Register(this->m_id,this);
            JSM().set_result_handle( [this](MessageResultJudge & msg_result) {

                    judgeResult_id jr_id = msg_result.get_code();

                    //提交题目的sid
                    std::string_view sid = msg_result.get_key();
                    std::size_t int_sid = 0;
                    std::from_chars(sid.data(),sid.data()+sid.size(),int_sid);
                    std::string_view msg = msg_result.get_msg();
                    // LOG_INFO << "judgeResult_id = " << msg_result.get_code_int() ;
                    switch(jr_id) {
                        case judgeResult_id::SUCCESS:
                        {
                            if ( msg.length() == 0 )
                                break;
                            if(  msg[0] == 'a') { // 通知所测试点的数量

                            }
                            else if( msg[0] == 'A') { // 最后一次发送全部的测试结果
                                int final_result = 0;
                                std::size_t sum_real_time = 0;
                                std::size_t sum_cpu_time = 0;
                                std::size_t sum_memory = 0;
                                int right_size = 0;
                                std::string mini_result{};
                                std::string full_result = msg_result
                                    .dump_to_json();

                                int total_size = msg_result.results_size();
                                mini_result.reserve(total_size);
                                for( auto & e : msg_result ) {
                                    sum_real_time += e.real_time;
                                    sum_cpu_time += e.cpu_time;
                                    sum_memory += e.memory;
                                    if( final_result == 0&& e.result != 0 ) {
                                        final_result = e.result;
                                    }
                                    if( e.result == RESULT_MEAN::ACCEPT)
                                        ++right_size;
                                    result_to_mini_result(e.result,mini_result);
                                }

                                int score = total_size == 0 ? 0 : 100*right_size/total_size;

                                //把结果存入数据库
                                CURD::judgeTable::update_solution(
                                        sid,
                                        final_result,
                                        sum_cpu_time,
                                        sum_memory,
                                        score,
                                        mini_result,
                                        full_result
                                        );
                                LOG_DEBUG << "recv all point : " << full_result;
                                //存入JSM
                                JSM().create(int_sid,-1,total_size,final_result,std::move(full_result));

                            }
                            else if( msg[0] >='0' && msg[0] <='9') { //发送单个点
                                int now_point = 0;
                                int total_size = 0;
                                int state = 1;
                                for(char c : msg) {
                                    if( c >='0' && c <= '9') {
                                        if( state == 1)
                                            now_point = now_point * 10 + c-'0';
                                        else if( state == 2)
                                            total_size= total_size* 10 + c-'0';
                                    }
                                    else if( state ==  1)
                                        state = 2;
                                    else break;
                                }
                                LOG_DEBUG << "recv single judge point: " << now_point
                                    << msg_result.dump_to_json();
                                //存入JSM
                                JSM().create(int_sid,now_point,total_size,
                                        msg_result.get_code()
                                        ,msg_result.dump_to_json());

                            }
                            break;
                        }
                        case judgeResult_id::INVALID_CONFIG     ://= -1,
                        case judgeResult_id::FORK_FAILED        :// = -2,
                        case judgeResult_id::PTHREAD_FAILED     :// = -3,
                        case judgeResult_id::WAIT_FAILED        :// = -4,
                        case judgeResult_id::ROOT_REQUIRED      :// = -5,
                        case judgeResult_id::LOAD_SECCOMP_FAILED:// = -6,
                        case judgeResult_id::SETRLIMIT_FAILED   :// = -7,
                        case judgeResult_id::DUP2_FAILED        :// = -8,
                        case judgeResult_id::SETUID_FAILED      :// = -9,
                        case judgeResult_id::EXECVE_FAILED      :// = -10,
                        case judgeResult_id::SPJ_ERROR          :// = -11,
                        case judgeResult_id::COMPILE_FAIL       :// = -12 // TODO
                            //把结果存入数据库: 出错了
                            break;
                        default:
                            //其它信息
                            break;
                    }
                    
                    //输出结果
            });

            http_handle_check_  = [this](request& req,response & res){
                try  {
                    bool find_route = m_http_route.route(
                                req.get_method(),
                                req.get_path(),
                                req,res);
                    if( find_route == false ) {
                        res.set_status(status_type::not_found);
                        return false;
                    }
                    return true;
                }
                catch( const std::exception & ex ) {
                    res.set_status_and_content<status_type::internal_server_error, content_type::string>
                        (ex.what()+std::string(" exception in business function"));

                    return false;
                }
                catch (...) {
                    // res.set_status_and_content(status_type::internal_server_error, "unknown exception in business function");
                    res.set_status_and_content<status_type::internal_server_error, content_type::string>("unknown exception business function");
                    return false;
                }

            };

            http_handle_ = [this](request& req,response& res) {
                //设置res的头??
                // res.set_headers(req.get_headers()) // TODO
                this->http_handle_check_(req,res);
            };

        }

        void join(){
            m_thread.join();
        }

        server(int i,Acceptor & acc){
            init();
            m_id = i;
            m_acceptor = acc.reset_io_context(m_ctx);
        }

        /**
         * @desc: 发送评测数据
         *
         * pid 评测的题目的id,也有可能是题目的路径
         * timeLimit
         * memoryLimit
         * */
        void send(
                std::string_view key,
                std::string_view code,
                std::string_view language,
                std::string_view pid,
                int timeLimit,
                int memoryLimit) 
        { JSM().send(key,code,language,pid,timeLimit,memoryLimit);}

        Task<> deal(Connection s);

        //监听 连接
        Task<> listen(IoContext & ctx);

        // 启动
        void run(){
            m_thread = std::thread([this](){

                //logger的相关设置
                LOGGER::logger::set_thread_id(this->m_id);

                LOG_DEBUG << "start run..\n";


                try {
                    //1 初始化 内存池
                    //nothing

                    //2 创建监听协程
                    co_spawn(listen(m_ctx));
                    //3 启动事件中心
                    m_ctx.run();
                }
                catch(...) {
                    // printf("%s\n", to_string(std::current_exception()).c_str());
                    LOG_DEBUG << "has exception in run()";
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
    rojcpp::http_session Session( std::pmr::new_delete_resource() ,
            &http_handle_, //执行 路由
            &http_handle_check_, //执行并检查 路由
            this
            );

    //3.处理数据与路由
    //4.异步读取
    
    // char buff[1024];
    //1 读取数据,保证数据读取完毕
    // 2.异步读取 ,TODO 更加细化的处理
    std::size_t nread=  0;
    for(;;) {
        try {
            auto [buff,buff_size] = Session.req_buff();

            LOG_DEBUG << "get buff from Session";
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
        LOG_DEBUG << "start Session parse_raw_request_data";
        int parse_ret =  Session.parse_raw_request_data();
        if( parse_ret == -2) // -2 represent header data not complete
            continue;
        else if( parse_ret == -1) {
            auto [buff,buff_size] = Session.req_buff();
            LOG_ERROR << "Parse Error: " ;
            // std::cout   << std::string_view((char *)buff,buff_size) <<"\n";
            throw std::runtime_error("parse_Error");
            // TODO parse_Error
        }
        else // > 0
            break;

    }
    LOG_INFO << "read end and read count : " << nread;

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

http_router server::m_http_route;

} // end namespace rojcpp
