#pragma once

#include "judge_server/src/Client.hpp"

//连接服务器,发送与接收相关的信息
class JudgeConnector {

    public:
        
        //@desc 发送评测数据
        void send_jduge_msg();
    private:
        //@desc 设定返回的结果的处理
        void set_result_handle();
        std::vector<int>
        Client __client; //定义一个客户端
};
