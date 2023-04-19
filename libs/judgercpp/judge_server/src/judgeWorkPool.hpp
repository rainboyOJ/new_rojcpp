/**
 * 工作线程池,一个特化的线程池,只能执行judgeWork
 * 基于 judge_Queue
 *
 * 当 加入数据时,会换醒工作线程
 *
 * 工作线程,会一会工作,直接队列为空时,挂起工作,等待下一次唤醒
 */

#pragma once

#include <mutex>
#include <condition_variable>
#include <vector>
#include <deque>        //双端队列

#include "socketBase.hpp"
#include "socketManager.hpp"
#include "judgeQueue.hpp"
#include "Result.hpp"
#include "utils.hpp"
#include "Problem.hpp"
#include "check.hpp"


class judgeWorkPool {
public:
    //默认4个线程
    explicit judgeWorkPool(unsigned int size = 4);
    judgeWorkPool(const judgeWorkPool &) = delete;
    judgeWorkPool& operator=(const judgeWorkPool &) = delete;

    ~judgeWorkPool() {
        _task_cv.notify_all();
        for(auto &th : _workPool){
            if( th.joinable()) th.join();
        }
    }
    void notify_all(); //唤醒所有

    //加入数据进队列
    bool enqueue(judge_Queue_node &jn){
        //bool ret = judge_Queue::get().enque(jn);
        //if( ret ) _task_cv.notify_one(); //唤醒一个
        //return ret;
    }


    // 评测
    result __judger(judge_args& args){
        std::stringstream ss;
        //log("参数",(judge_bin + static_cast<std::string>(args)).c_str() );
        //std::cout << std::endl ;
        exec( ( std::string(__CONFIG::judger_bin) + static_cast<std::string>(args)).c_str() ,ss);
        result RESULT;
        ss >> RESULT.cpu_time;
        ss >> RESULT.real_time;
        ss >> RESULT.memory;
        ss >> RESULT.signal;
        ss >> RESULT.exit_code;
        ss >> RESULT.error;
        ss >> RESULT.result;
        return RESULT;
    }

    std::tuple<bool,std::string> Compile(judge_args& args);     //编译
    //std::tuple<bool,result>      
    // out_file 输出的文件路径
    result Judge(judge_args& args,std::string_view out_file);       //评测

    /**
     * 写message
     */
    void write_message(int fd,MessageResultJudge& msg);

    bool enque(MessageSendJudge&& msj,int fd){
        std::unique_lock<std::mutex> lck(mtx);
        judge_Queue_node jn;
        jn.stage       = JUDGE_STAGE::PREPARE;
        jn.fd          = fd;
        jn.key         = std::move(msj.key);
        jn.code        = std::move(msj.code);
        jn.language    = std::move(msj.language);
        jn.pid         = std::move(msj.pid);
        jn.timeLimit   = msj.timeLimit;
        jn.memoryLimit = msj.memoryLimit;
        q.push_back(std::move(jn));
        _task_cv.notify_one(); //唤醒一个
        return true;
    }

private:
    void judgeWork();
    void work_stage1(judge_Queue_node & jn); //第一个阶段工作
    void work_stage2(judge_Queue_node & jn); //第二个阶段工作
    std::mutex mtx; //锁
    std::condition_variable _task_cv; //条件
    std::vector<std::thread> _workPool; //工作线程池
    std::atomic<int> ThrNum; //空闲线程数量
    std::deque<judge_Queue_node> q; //队列
};
//默认4个线程
judgeWorkPool::judgeWorkPool(unsigned int size){
    for(int i = 0 ;i< size ;++i){
        _workPool.emplace_back(&judgeWorkPool::judgeWork,this);
        ThrNum.fetch_add(1); //空闲线程+1
    }
}

/**
 * @desc 执行评测的流程
 *
 * 整个评测流程分和两个部分:
 *
 * 一: 准备阶段
 *      是否是支持的评语
 *      查找题目的位置,判断题目是否存在,并返回题目的相关信息
 *      创建评测的文件夹
 *      写入代码
 *      编译
 *      写入评测队列,进入评测阶段
 * 二: 评测阶段
 */

//阶段一,二都应该从一个队列里取放数据,所以要设计一个队列
void judgeWorkPool::judgeWork(){

    while ( 1 ) {
        judge_Queue_node jn;
        {
            std::unique_lock<std::mutex> lck(mtx);
            while( q.empty() )//队列为空,就挂起
            {
#ifdef JUDGE_SERVER_DEBUG
                std::cout << "go wait" << std::endl;
#endif
                _task_cv.wait(lck); 
            }
            jn = q.front();
            q.pop_front();
        }
#ifdef JUDGE_SERVER_DEBUG
        std::cout << "de judge queue succ :" << std::endl;
        std::cout << jn.fd << std::endl;
        std::cout << jn.key << std::endl;
        std::cout << jn.code << std::endl;
        std::cout << jn.language << std::endl;
        std::cout << jn.pid << std::endl;
#endif
        if( jn.stage == JUDGE_STAGE::PREPARE){
#ifdef JUDGE_SERVER_DEBUG
            //std::cout << "stage 1" << std::endl;
#endif
            work_stage1(jn);
        }
        // 因为设计的问题不出现state::JUDGING这个状态
        else if (jn.stage == JUDGE_STAGE::JUDGING ) {
            std::cout << "stage 2" << std::endl; work_stage2(jn);
        }
        //TODO 输出数据的内容
    }
}

void judgeWorkPool::write_message(int fd,MessageResultJudge& msg){
    socketManagerRAII smra(fd);
    auto str = msg.dumps().to_string();

#ifdef JUDGE_SERVER_DEBUG
    //std::cout << "fd : "<< fd << std::endl;
    miniLog::log("fd : ",fd);
    miniLog::log("send msg: " , msg);
    
    //std::cout << msg << std::endl;
    //show_hex_code(msg.dumps());
    //std::cout << str << std::endl;
    //show_hex_code(str);
#endif

    socketBase::TcpWrite(fd,str.c_str(),str.length());
}
/*
 * 一: 准备阶段
 *      是否是支持的语言
 *      查找题目的位置,判断题目是否存在,并返回题目的相关信息
 *      创建评测的文件夹
 *      写入代码
 *      编译
        // 4.5 发送测试点的相关信息
 *      进入评测阶段
 */
void judgeWorkPool::work_stage1(judge_Queue_node &jn){
    try {
        //1 是否是支持的语言
        if( is_sport_language(jn.language) == false){
            MessageResultJudge res(jn.key,judgeResult_id::INVALID_CONFIG,std::string("unsupport language : ") + jn.language);
            write_message(jn.fd, res);
            return;
        }
        //2 查找题目的位置,判断题目是否存在,并返回题目的相关信息
        Problem p;
        if( jn.problem_path.length() == 0){
            p = Problem(__CONFIG::BASE_PROBLEM_PATH,jn.pid);
            //for (const auto& e : p.input_data) {
                //std::cout << e.first<< " " << e.second << std::endl;
            //}
            //for (const auto& e : p.output_data) {
                //std::cout << e.first<< " " << e.second << std::endl;
            //}
        }
        else
            p = Problem(jn.problem_path);
        //3 创建评测的文件夹 写入代码
        std::string uuid = UUID(); //生成uuid
#ifdef JUDGE_SERVER_DEBUG
        std::cout << "uuid " << uuid << std::endl;
#endif
        auto work_path = fs::path(__CONFIG::BASE_WORK_PATH) / uuid;

        //std::filesystem::create_directories(work_path);
        directoryRAII dirRaii(work_path);


        const std::string code_name  = "main.code"; // 代码名

        auto code_path = work_path / code_name ;
        writeFile(code_path.c_str(), jn.code);
        //std::cout << "uuid " << uuid << std::endl;
        // 4 编译
        auto compile_args = compile_CPP_args(work_path, code_name);
        result res = __judger(compile_args);
        if( res.result !=0 || res.error != 0  || res.exit_code != 0 ){
            std::string msg = readFile(compile_args.error_path.c_str());
            if( msg.length() == 0)
                msg =  readFile(compile_args.log_path.c_str());
            //return std::make_tuple(STATUS::COMPILE_ERROR,msg, std::vector<result> {});
            MessageResultJudge res(jn.key,judgeResult_id::COMPILE_FAIL,msg);
            write_message(jn.fd, res);
            return;
        }
        // 4.5 发送测试点的相关信息
        {
            // 所有测试点的数量
            const std::string msg = "allSize:";
            MessageResultJudge sendMsg(jn.key,judgeResult_id::SUCCESS,msg+ std::to_string(p.input_data.size()));
            write_message(jn.fd, sendMsg);
        }

        // 5 评测,依次评测
        auto Lang =  string_to_lang(jn.language);

        MessageResultJudge sendALLmsg(jn.key,judgeResult_id::SUCCESS,"all");
        for (int i = 0 ;i< p.input_data.size() ; ++i) {
            auto & in_file  = std::get<std::string>(p.input_data[i]);
            auto & out_file = std::get<std::string>(p.output_data[i]);

            std::string user_out_file = std::to_string(i) + ".out";
            auto judgeArgs = getJudgeArgs(Lang, work_path, code_name, in_file, user_out_file, jn.timeLimit , jn.memoryLimit);
            //auto res = __judger(judgeArgs);
            auto res = Judge(judgeArgs, user_out_file);

            std::ostringstream oss;
            oss << i << ","; //测试点的编号
            oss << getBaseName(in_file) << "," << getBaseName(out_file); //输入输出的名字
            MessageResultJudge sendmsg(jn.key,judgeResult_id::SUCCESS,oss.str());

            sendmsg.push_back(res);
            sendALLmsg.push_back(res);

#ifdef JUDGE_SERVER_DEBUG
            //print_result(res);
#endif
            //MessageResultJudge
            write_message(jn.fd, sendmsg);

        }
        write_message(jn.fd, sendALLmsg);

    }
    catch(std::exception & e){
        std::cerr << " Exception : " << e.what() << "\n";
    }
}


//TODO
void judgeWorkPool::work_stage2(judge_Queue_node &jn){
}

result    judgeWorkPool::Judge(judge_args& args,std::string_view out_file)       //评测
{
    auto res = __judger(args);
    if( res.error != 0  || res.result !=0){ //有错误,超时,超内存等等
        //return std::make_tuple(true,res) ; 
        return  res;
    }
    else  {
        //查检memory
        if( args.max_memory != unlimit && res.memory >= args.max_memory - __CONFIG::memory_base){ // 减去基础内存
            res.result = MEMORY_LIMIT_EXCEEDED;
        }
        else {
            //res.result = MEMORY_LIMIT_EXCEEDED;
            //答案检查
            //log_one(args.output_path);
            //log_one(out_file);
            if( !cyaron::Check::noipstyle_check(args.output_path.c_str(), out_file) ){
                res.result = WRONG_ANSWER;
            }
        }
        return res;
    }
}
