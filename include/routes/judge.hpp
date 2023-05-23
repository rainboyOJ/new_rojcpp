/**
 * @desc 发送评测的相关评测
 */
#pragma once
#include "server.h"

#include "jsonEnity/judge.hpp"
// #include "judgeConnect.hpp"
// #include "rojcppUtils.hpp"
// #include "fastCache.hpp"
// #include "user_ap.hpp"
#include "curd/judgeTable.h"

// using  netcore::Cache;
// using namespace netcore;

namespace rojcpp
{
    


struct judgeRoutes {
    /**
     * @desc 处理评测信息
     */
    static void handleJudgeMsg(rojcpp::request & req,rojcpp::response & res);
    // @desc 连接上webSocket后返回的信息
    // static void handleResultWS(request & req,response & res);
    // @desc 使用Get返回的信息 GET /judge_result?id=
    // static void handleResult(request & req,response & res);

    //@desc lang str to int
    // exit
    static int lang_to_int(std::string_view lang_name) {
        if( lang_name == "cpp"sv || lang_name == "c++"sv)
            return 1;
        return 0;
    }

    template<typename Server>
    static void regist_route(){
        using namespace std::literals::string_view_literals;
        Server::m_http_route.template register_handler<POST>
            ("/handleJudge",handleJudgeMsg /*UserAP_is_logined{}*/);

        /*
            http_server.regist_ws_conn_check("/judgews",[](netcore::request & req,netcore::response & res) -> bool
                    {
                    //检查id
                    std::cout << "handleResultWS=====================" << std::endl;
                    try {
                    //得到请求的哪个题目的 result,id
                    auto id = req.get_query_value<uint64_t>("id");
                    //LOG_DEBUG("at ws_conn_check for /judgews, id is %lld",id);

                    //TODO 应该在redisCache 里注册 是哪个id
                    //TODO 应该检查是否有哪个client fd注册过 这个id
                    //TODO 只允许一个id 对应一个fd
                    }
                    catch(std::exception & e){
                    return  false;
                    }
                    return true;
                    });
                    */

            // http_server.set_http_handler<GET,POST>("/judgews",handleResultWS);
            // http_server.set_http_handler<GET>("/judge_result",handleResult,UserAP_is_logined{});
        }

};
/**
 * @desc 用来处理用户发来的评测信息
 * 1. 验证数据
 *      代码长度
 *      pid 是否存在
 *      lang 语言
 * 2. 创建一个评测的sql
 * 3. 发送评测给judgeServer
 * 4. judge client的 handle 等待处理
 */
void judgeRoutes::handleJudgeMsg(rojcpp::request &req, rojcpp::response &res){

    LOG_DEBUG << req.body();
    //TODO try catch
    // 1. catch cppjson
    // 2. catch modern_cpp
    //
    // 1. 解析数据
    auto judgeJson = cppjson::Serializable::loads<judgeEntiy>( std::string(req.body()) );

    LOG_DEBUG << "language: " << judgeJson.language;
    // 1. 创建一个solutions
    // TODO check language is suported or not
    unsigned long long solution_id
        = CURD::judgeTable::add_solutions(
                // req.get_user_id(), //TODO Fix
                62,
                judgeJson.pid,
                // judgeJson.language
                lang_to_int(judgeJson.language)
                );
    LOG_DEBUG << "solution_id " << solution_id;
    // 2. 创建一个solution_codes
    CURD::judgeTable::add_solution_codes(solution_id, judgeJson.code);
    // 3. 创建一个solution_full_result
    CURD::judgeTable::add_solution_full_result(solution_id);

    //TODO check language is suport
    // 1 cpp
    // 2 python3

    // 2 发送评测给judgeServer
    auto server_ptr =  req.get_session_ptr() -> get_server_ptr() ;
    std::string solution_id_str = std::to_string(solution_id);
    server_ptr->send(
            solution_id_str,
            judgeJson.code,
            judgeJson.language,
            judgeJson.pid,
            judgeJson.timeLimit,
            judgeJson.memoryLimit
            );


    res.set_status_and_content<status_type::ok, content_type::json>("{msg:\"ok\",code:1}");
    /*


    // 4. 创建一个Cache
    std::string solution_id_str = std::to_string(solution_id);
    rojcppForServer::judgeCacheKeyFactory JKFC(solution_id_str);

    //4.1 xxx_is_judging
    netcore::Cache::get().set(JKFC.get_is_judging_key(),"",60*30); //保存 30分钟

    // 3. 发送评测给judgeServer
    judgeConnectSingleton::Get().send(
            solution_id_str, //key
            rojcppForServer::unescape_newline(judgeJson.code),
            "cpp", //language TODO language transfer
            judgeJson.pid,
            1000,// time TODO
            128 // memoryLimit
            );


    MsgEntity msgRet(solution_id_str);
    res.set_status_and_content(status_type::ok,
            msgRet.dumps()
            ,req_content_type::json);
*/
}



} // namespace rojcpp
