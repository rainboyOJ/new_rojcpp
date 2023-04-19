/**
 * @brief 对socket,select进行封装
 * 遵循一个笑意的原则,哪个socket发送过来的,就用哪个socket发用回去
 */

#pragma once
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <atomic>

#include "socketBase.hpp"
#include "Send.hpp"
#include "Result.hpp"
#include "utils.hpp"
#include "concurrentqueue.h"
#include "socketManager.hpp"
#include "judgeQueue.hpp"
#include "judgeWorkPool.hpp"

class Server :public socketBase {
public:
    /**
     * @brief 
     * 端口
     * 最多监听的socket数量
     * 评测的工作线程的数量
     *
     */
    explicit Server(int port,int socket_num,unsigned int judgeWorkSize);
    ~Server() {  Close();};
    void run();
    void Close(){

#ifdef JUDGE_SERVER_DEBUG
        //std::cout << "Close" << std::endl;
        miniLog::log("Close Server");
#endif
        if(server_sockfd !=-1) close(server_sockfd);
        if(client_sockfd !=-1) close(client_sockfd);
    }

    void addsig(int sig, void(handler)(int), bool restart);
    static void sig_handler_wrapper(int sig);

private:
    int server_sockfd{-1};
    int client_sockfd{-1};
    unsigned int server_len;
    unsigned int client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int result;
    fd_set readfds, testfds;
    int port;
    int socket_num;
    //static std::atomic<bool> runing; //是否在在执行
    //static int m_socket_pipe[2];            //管道socket,用来监听 signal
    //moodycamel::ConcurrentQueue<int> q;     // 存接入的socket队列
    //socketManager _SM;

    unsigned int _judgeWorkSize;

    judgeWorkPool workPool;
};

//int Server::m_socket_pipe[2];
//std::atomic<bool> Server::runing; //是否在在执行

void Server::addsig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void Server::sig_handler_wrapper(int sig){
    //为保证函数的可重入性，保留原来的errno
#ifdef JUDGE_SERVER_DEBUG
    std::cout << "sig_handler_wrapper " << sig  << std::endl;
#endif
    //runing.store(false);
    //int save_errno = errno;
    //int msg = sig;
    //std::cout << "send signal" << std::endl;
    //send(m_socket_pipe[1], (char *)&msg, 1, 0);
    //errno = save_errno;
}

Server::Server(int port,int socket_num,unsigned int judgeWorkSize)
    :port{port},socket_num{socket_num},_judgeWorkSize{judgeWorkSize},workPool{judgeWorkSize}
{
    server_sockfd                  = socket(AF_INET, SOCK_STREAM, 0);//建立服务器端socket server_address.sin_family      = AF_INET;
    if( server_sockfd == -1){
        std::cerr << "create socket failed" << std::endl;
        Close();
        throw "create socket failed";
    }
    memset(&server_address,0,sizeof(server_address));
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port        = htons(port);
    server_len                     = sizeof(server_address);

    //给socket绑定地址
    if( bind(server_sockfd, (struct sockaddr *)&server_address, server_len) != 0)
    {
        std::cerr << "bind failed" << std::endl;
        Close();
        throw "bind failed";
    }
    //监听队列最多容纳5个
    if( listen(server_sockfd, socket_num) != 0 ) {
        std::cerr << "listen failed" << std::endl;
        Close();
        throw "listen failed";
    }

    //int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_socket_pipe);
    ////set no block
    //int old_option = fcntl(m_socket_pipe[0], F_GETFL);
    //int new_option = old_option | O_NONBLOCK;
    //fcntl(m_socket_pipe[0], F_SETFL, new_option);

    //addsig(SIGPIPE, SIG_IGN,true);
    //addsig(SIGALRM, sig_handler_wrapper ,true);
    //addsig(SIGINT, sig_handler_wrapper ,true); //ctrl-c信号
    //alarm(5);

    FD_ZERO(&readfds);  //清空读的集合
    FD_SET(server_sockfd, &readfds);//将服务器端socket加入到集合中
    //FD_SET(m_socket_pipe[0], &readfds);//将服务器端socket加入到集合中

    //runing.store(true); //设置正在运行


}

void Server::run(){
    std::cout << "server run at port : " << port  << std::endl;
    struct timeval timeout={0,0}; //select等待1秒，1秒轮询，要非阻塞就置0
    while( 1 )
    {
        //std::cout << "run" << std::endl;
        char ch;
        int fd;
        int nread;
        testfds = readfds;//将需要监视的描述符集copy到select查询队列中，select会对其修改，所以一定要分开使用变量
        //printf("server waiting\n");
        //std::cout << "server waiting" << std::endl;


        /*无限期阻塞，并测试文件描述符变动 */
        result = select(FD_SETSIZE, &testfds, (fd_set *)0,(fd_set *)0, &timeout); //FD_SETSIZE：系统默认的最大文件描述符
        if(result < 0)
        {
            perror("server run");
            //exit(1);
            break;
        }
        if( result == 0 ){ //超时
            //std::cout << "time out" << std::endl;
            continue;
        }

        /*扫描所有的文件描述符*/
        for(fd = 0; fd < FD_SETSIZE; fd++)
        {
            /*找到相关文件描述符*/
            if(FD_ISSET(fd,&testfds))
            {
              /*判断是否为服务器套接字，是则表示为客户请求连接。*/
                if(fd == server_sockfd)
                {
                    client_len = sizeof(client_address);
                    client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
                    FD_SET(client_sockfd, &readfds);//将客户端socket加入到集合中
                    socketManager::Instance().insert(client_sockfd);
                    //printf("adding client on fd %d\n", client_sockfd);
#ifdef JUDGE_SERVER_DEBUG
                    //std::cout << "adding client on fd " << client_sockfd << std::endl;
                    miniLog::log("adding client on fd : ",client_sockfd);
#endif
                }
                //else if( fd == m_socket_pipe[0]) { //有信号过来
                    ////读取信号
                    //char signals[1024];
                    //int ret = recv(m_socket_pipe[0], signals, sizeof(signals), 0);
                    //std::cout << "at run signal "<< signals[0] << std::endl;
                    //if( signals[0] == SIGINT) //退出
                    //{
                        //std::cout << "SIGINT" << std::endl;
                        //runing.store(false);
                        //break;
                    //}
                //}
                /*客户端socket中有数据请求时*/
                else
                {
                    ioctl(fd, FIONREAD, &nread);//取得数据量交给nread

                    /*客户数据请求完毕，关闭套接字，从集合中清除相应描述符 */
                    if(nread == 0)
                    {
                        while( socketManager::Instance().remove(fd) == false) ; //可以单独开一个线程关闭这个fd
                        close(fd);
                        FD_CLR(fd, &readfds); //去掉关闭的fd
                        printf("removing client on fd %d\n", fd);
                    }
                    /*处理客户数据请求*/
                    else
                    {
                        int err;
                        int read_len;
                        std::string readStr;
                        //auto readStr = readAll(fd,&err);
                        TcpRead(fd, readStr, &read_len);
                        //read(fd, &ch, 1);
                        //sleep(5);
                        MessageSendJudge msgj;
                        msgj.loads(readStr);

#ifdef JUDGE_SERVER_DEBUG
                        //std::cout << "read content is : "  << std::endl;
                        //std::cout << msgj << std::endl;
                        //std::cout << "fd : " << fd << std::endl;
                        miniLog::log("read content is :\n",msgj,"\n from fd: ",fd);
#endif
                        
                        workPool.enque(std::move(msgj), fd);

                        //输出的数据
                        //MessageResultJudge msg_res(msgj.key,judgeResult_id::SUCCESS,"hello world");
                        //msg_res.push_back(1,2,3,4,5,6,7);
                        //msg_res.push_back(1,2,3,4,5,6,7);
                        //auto msg_res_dumps = msg_res.dumps();
                        //show_hex_code(msg_res_dumps);
                        //TcpWrite(fd, msg_res_dumps.data(), msg_res_dumps.size());
                    }
                }
            }
        }
        //if( runing.load() == false)
            //break;
    }
}
