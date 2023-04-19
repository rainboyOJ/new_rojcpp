/**
 * @desc
 * 评测队列
 */
#pragma once
#include "judgeArgs.hpp"
#include "Send.hpp"
#include "concurrentqueue.h"

//评测队列里的一个点
struct judge_Queue_node {

    JUDGE_STAGE stage;      //评测的阶段
    int fd;                 //请求过的sock
    
    std::string key;        // 返回结果时携带的key值
    std::string code;       // 代码
    std::string language;   // 语言
    std::string pid;        // 评测的题目的id,也有可能是题目的路径
    std::string problem_path; //评测题目的目录

    int timeLimit;          // ms
    int memoryLimit;        // mb
    judge_args JA; //参数
};


////评测队列,单例模式
//class judge_Queue {
//public:
    //static judge_Queue& get(){
        //static judge_Queue jq;
        //return  jq;
    //}

    //bool enque(judge_Queue_node &jn){
        //return q.enqueue(jn);
    //}

    //bool enque(MessageSendJudge&& msj,int fd){
        //judge_Queue_node jn;
        //jn.stage = JUDGE_STAGE::PREPARE;
        //jn.fd = fd;
        //jn.key = std::move(msj.key);
        //jn.code= std::move(msj.code);
        //jn.language= std::move(msj.language);
        //jn.pid= std::move(msj.pid);
        //jn.timeLimit = msj.timeLimit;
        //jn.memoryLimit = msj.memoryLimit;
        //return q.enqueue(jn);
    //}

    //bool try_deque(judge_Queue_node &jn){
        //return q.try_enqueue(jn);
    //}
    //auto size() {
        //return  q.size_approx();
    //}

//private:
    //judge_Queue() = default;
    //judge_Queue(judge_Queue&)  = delete;
    //judge_Queue(judge_Queue&&) = delete;

    ////static
    //moodycamel::ConcurrentQueue<judge_Queue_node> q{0};     // 存接入的socket队列
//};

//moodycamel::ConcurrentQueue<judge_Queue_node> judge_Queue:: q{0};     // 存接入的socket队列
