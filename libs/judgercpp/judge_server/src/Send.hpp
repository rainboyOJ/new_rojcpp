/**
 * @desc client 发送的 评测数据
 */
#pragma once

#include <string_view>
#include <sstream>
#include <cstring>
#include <iostream>

#include "MessageBuffer.hpp"

class MessageSendJudge {
public:
    MessageSendJudge() = default;
    explicit MessageSendJudge(
            std::string_view key,
            std::string_view code,
            std::string_view language,
            std::string_view pid,
            int timeLimit,
            int memoryLimit
            );

    void loads(std::string_view str); //反序列化,从字符串转换过中
    void loads(MessageBuffer & buf); //反序列化,从字符串转换过中
    MessageBuffer dumps(); //序列化,转成字符串
    friend std::ostream & operator<<(std::ostream & out,const MessageSendJudge& msgBuf);
    std::string key;        // 返回结果时携带的key值
    std::string code;       // 代码
    std::string language;   // 语言
    std::string pid;        // 评测的题目的id,也有可能是题目的路径
    int timeLimit;          // ms
    int memoryLimit;        // mb
};

MessageSendJudge::MessageSendJudge(
            std::string_view key,
            std::string_view code,
            std::string_view language,
            std::string_view pid,
            int timeLimit,
            int memoryLimit
        ) :
        key{key}, code{code}, language{language}, pid{pid},
        timeLimit{timeLimit}, memoryLimit{memoryLimit}
{}


std::ostream & operator<<(std::ostream & out,const MessageSendJudge& msgBuf){
    out << "key: " << msgBuf.key << "\n";
    out << "code: " << msgBuf.code << "\n";
    out << "language: " << msgBuf.language << "\n";
    out << "pid: " << msgBuf.pid << "\n";
    out << "timeLimit: " << msgBuf.timeLimit << "\n";
    out << "memoryLimit: " << msgBuf.memoryLimit << "\n";
    return out;
}


MessageBuffer MessageSendJudge::dumps(){
    MessageBuffer ret;

    ret.appendMessge(key);
    ret.appendMessge(code);
    ret.appendMessge(language);
    ret.appendMessge(pid);
    ret.push_back(timeLimit);
    ret.push_back(memoryLimit);

    return ret;
}

void MessageSendJudge::loads(MessageBuffer & buf){
    key         = buf.dumpsMessage<std::string>();
    code        = buf.dumpsMessage<std::string>();
    language    = buf.dumpsMessage<std::string>();
    pid         = buf.dumpsMessage<std::string>();
    timeLimit   = buf.dumpsMessage<int>();
    memoryLimit = buf.dumpsMessage<int>();
}

void MessageSendJudge::loads(std::string_view str) //反序列化,从字符串转换过中
{
    int cur = 0;
    key         = MessageBuffer::dumpsMessageFromStr<std::string>(str.data(),cur);
    code        = MessageBuffer::dumpsMessageFromStr<std::string>(str.data(),cur);
    language    = MessageBuffer::dumpsMessageFromStr<std::string>(str.data(),cur);
    pid         = MessageBuffer::dumpsMessageFromStr<std::string>(str.data(),cur);
    timeLimit   = MessageBuffer::dumpsMessageFromStr<int>(str.data(),cur);
    memoryLimit = MessageBuffer::dumpsMessageFromStr<int>(str.data(),cur);
}
