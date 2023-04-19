/**
 * @brief 对client 进行封装
 */

#pragma once

#include <iostream>
#include <deque>
#include <cstring>
#include <string_view>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <functional>
#include <thread>

#include "socketBase.hpp"
#include "Send.hpp"
#include "Result.hpp"
#include "socketManager.hpp"

#include "utils.hpp"

//处理得到的结果集的函数
using result_handler = std::function<void(MessageResultJudge &)>;

class Client :public socketBase{
public:
    explicit Client(std::size_t connect_size,std::string_view judge_server_ip,int port = 9000);
    ~Client(){
        runing.store(false);
        for( int i = 0 ;i < connect_size;i++){
            {
                int sockfd = -1;
                //q.try_dequeue(sockfd);
                socketManager::Instance().removeAll();
                for (auto& fd : sockfd_vec) ::close(fd);
            }
        }
        Recv_th.join();
    }
    /**
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
            int memoryLimit
            ); //发送评测数据

    void clear(); // 清空

    void set_result_handle(result_handler&& handle){
        __handle = std::move(handle);
    }

private:
    bool Connect(int sockfd);

    struct sockaddr_in servaddr;
    bool   connected{false};
    int    port;

    std::vector<int> sockfd_vec;
    std::size_t connect_size; //连接数量
    result_handler __handle{nullptr};
    //moodycamel::ConcurrentQueue<int> q;
    //socketManager _SM;
    std::thread Recv_th; //接收信息的线程
    std::atomic_bool runing;
    std::string judge_server_ip;
    int epollfd;
};

Client::Client(std::size_t connect_size,std::string_view judge_server_ip,int port)
    :connect_size{connect_size}, port(port),judge_server_ip{judge_server_ip}
{

#ifdef JUDGE_SERVER_DEBUG
    std::cout << port << std::endl;
#endif
    
    if( (epollfd = epoll_create1(0)) == -1){
        throw  std::runtime_error("JUDGE_SERVER create epoll fail!");
    }

    for(int i =0;i < connect_size ;i++){
        int sockfd=socket(AF_INET,SOCK_STREAM,0);
#ifdef JUDGE_SERVER_DEBUG
        std::cout << sockfd << std::endl;
#endif
        if (sockfd < 0) 
        { 
#ifdef JUDGE_SERVER_DEBUG
            std::cout <<"socket() failed.\n";
#endif
            return;
        }

        memset(&servaddr,0,sizeof(servaddr));

        servaddr.sin_family = AF_INET;
        servaddr.sin_port   = htons(port);
        servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");

        if( Connect(sockfd) ) { //连接成功
            //加入到epoll里
            epoll_event _event;
            memset(&_event,0,sizeof(_event));
            _event.data.fd = sockfd;
            _event.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLET;
            epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &_event);
        }
        //q.try_enqueue(sockfd); //加入到队列里
        socketManager::Instance().insert(sockfd); // 加入到socket管理
        sockfd_vec.push_back(sockfd);
    }

    runing.store(true);


    Recv_th = std::thread([this](){
                //if(this->__handle == nullptr) return;
                struct timeval timeout={10,100}; //select等待1秒，1秒轮询，要非阻塞就置0
    while(this->runing.load()){

            //int result = select(FD_SETSIZE, &tmpset, (fd_set *)0,(fd_set *)0, &timeout); //FD_SETSIZE：系统默认的最大文件描述符
            //int result = select(FD_SETSIZE, &tmpset, (fd_set *)0,(fd_set *)0, NULL); //FD_SETSIZE：系统默认的最大文件描述符
            epoll_event __events[255];
            int timeout = 10;
            int nfds= epoll_wait(epollfd, __events, 255, 10);
            if( nfds<=0) {
                //std::cout << "no read" << std::endl;
                continue;
            }
            //std::cout << "yes read" << std::endl;
            /*扫描所有的文件描述符*/
            for(int i = 0 ;i < nfds ;++i)
            {

                if(__events[i].events & EPOLLIN) {
                    int fd = __events[i].data.fd;
                    std::string readStr;
                    int read_len;
                    TcpRead(fd, readStr, &read_len);
                    MessageResultJudge msg_res;
                    msg_res.loads(readStr);
                    //show_hex_code(readStr);
                    //#ifdef JUDGE_SERVER_DEBUG
                    //std::cout << msg_res << std::endl;
                    if( __handle != nullptr)
                        __handle(msg_res); //处理数据
                }
                else {
                    //TODO
                }

            }

    } // end while
            });
}

bool Client::Connect(int sockfd) {
    int result= connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr));
#ifdef JUDGE_SERVER_DEBUG
    std::cout << result << std::endl;
#endif
    if ( result == -1)
    {
        char msg[2048] = {0};
        sprintf(msg,"connect(%s:%d) failed.\n","127.0.0.1",port); ::close(sockfd);  
#ifdef JUDGE_SERVER_DEBUG
        std::cout << msg << std::endl;
#endif
        return false; 
    } 
#ifdef JUDGE_SERVER_DEBUG
    std::cout << "connect ok" << std::endl;
#endif
    connected = true;
    return true;
}

void Client::send(
            std::string_view key,
            std::string_view code,
            std::string_view language,
            std::string_view pid,
            int timeLimit,
            int memoryLimit
        ){
    MessageSendJudge msg(key,code,language,pid,timeLimit,memoryLimit);
    auto msg_dumps = msg.dumps();
    //TODO
    //TcpWrite(sockfd,msg_dumps.data(),msg_dumps.size());
    {
        int fd  = -1;
        if (connect_size <= 0 ){
#ifdef JUDGE_SERVER_DEBUG
            std::cout  << "LINE: "<< __LINE__ << "connect_size <= 0 "  << std::endl;
#endif
            return;
        }
        std::cout << "socket Manager start.. " << std::endl;
        assert(socketManager::Instance().size() != 0);
        socketManagerRAII smRa;
        fd = smRa.get();
        //std::cout << "end pick " << sock << std::endl;
        TcpWrite(fd, msg_dumps.data(),msg_dumps.size() );
        std::cout << "socket Manager end fd : " << fd  << std::endl;
    }
}
