#pragma once

#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <grp.h>
#include <signal.h>

#include "judger.h"

#ifdef DEBUG
//#define __print_result(node) debug_out<'\0'>(std::cout,#node ":","\t\t",RESULT.node)
#define __print_result(node) std::cout << std::setw(12) << #node ": " << RESULT.node <<'\n';

#else
#define __print_result(node) debug_out(std::cout,RESULT.node)
#endif

#define print_result()\
    __print_result(cpu_time);\
    __print_result(real_time);\
    __print_result(memory);\
    __print_result(signal);\
    __print_result(exit_code);\
    __print_result(error);\
    __print_result(result);

// ==================== utils end
// ==================== 子进程处理
//
template <class C, typename T>
T getPointerType(T C::*v);


/**
 * @desc 设定资源限制
 */
template<typename CONFIG,typename Member,
    typename LIMIT_Type,
    typename pointTotype = decltype(getPointerType(std::declval<Member>()))
>
inline bool __do_serlimit(
        CONFIG& c,              // config 的引用
        Member m,               // 成员指针
        pointTotype v,          // 成员指针指向的类型
        LIMIT_Type limit        // 限制哪个资源
    )
{
    if(c.*m != UNLIMITED){
        struct rlimit LIMIT;
        LIMIT.rlim_cur = LIMIT.rlim_max = static_cast<rlim_t>(v);
        return setrlimit(limit, &LIMIT) == 0; //设定资源限制
    }
    return true;
}

// 通过给定的成员 Member 来限制资源
#define do_serlimit(Member,LIMIT_Type)\
    if( not __do_serlimit(_config,&config::Member,_config.Member,LIMIT_Type) )\
            CHILD_ERROR_EXIT(SETRLIMIT_FAILED);
// 设定资源限制,
#define do_serlimit_new_value(Member,LIMIT_Type,value)\
    if( not __do_serlimit(_config,&config::Member,value,LIMIT_Type) )\
            CHILD_ERROR_EXIT(SETRLIMIT_FAILED);

