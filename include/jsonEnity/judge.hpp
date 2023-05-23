
/**
 * @发送过来的judge的json 信息
 */
#pragma once

#include "serializable.hpp"
#include "appException.hpp"
// #include "judge_server/src/Send.hpp"

//using namespace cppjson;

struct judgeEntiy {
    std::string language;   //语言
    std::string code;       //代码
    std::string pid;        //题目的编号
    int memoryLimit; //mb
    int timeLimit;   //ms

    cppjson::config get_config()const 
    {
        cppjson::config config=cppjson::Serializable::get_config(this);
        config.update({
            {"language", language},
            {"code", code},
            {"memoryLimit", memoryLimit},
            {"timeLimit", timeLimit},
            {"pid", pid}
        });
        return config;
    }
    // 转换成 judge server 的信息
    // MessageSendJudge convertToMessageSendJudge(std::string_view key){
    //     return  MessageSendJudge(key,code,language,pid,timeLimit,memoryLimit);
    // }
};
