#pragma once

#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <grp.h>
#include <signal.h>

#include "judger.h"
#include "utils.hpp"
#include "log.hpp"

void child_process(config & _config){
    FILE *input_file = NULL, *output_file = NULL, *error_file = NULL;

    // 改变当前的work目录
    if( _config.cwd.length() != 0){
        chdir(_config.cwd.c_str());
    }

    // 1. 栈空间
    do_serlimit(max_stack,RLIMIT_STACK)

    // 2. 内存
    if( not _config.memory_limit_check_only ){
        do_serlimit_new_value(max_memory,RLIMIT_AS,_config.max_memory*2)
    }

    // 3. cpu time
    do_serlimit_new_value(max_cpu_time, RLIMIT_CPU,(_config.max_cpu_time+1000)/1000 )

    // 4. 最大fork进程数量
    do_serlimit(max_process_number, RLIMIT_NPROC)

    // 5. 最大输出大小
    do_serlimit(max_output_size, RLIMIT_FSIZE)

    // 输入重定向
    if(_config.input_path.length() != 0){
        input_file = fopen(_config.input_path.c_str(), "r"); 
        if (input_file == NULL) {
            CHILD_ERROR_EXIT(DUP2_FAILED);
        }

        if( dup2(fileno(input_file),fileno(stdin)) == -1){
            CHILD_ERROR_EXIT(DUP2_FAILED);
        }
    }

    // 输出重定向
    if(_config.output_path.length() != 0){
        output_file = fopen(_config.output_path.c_str(), "w");
        if (output_file == NULL) {
            CHILD_ERROR_EXIT(DUP2_FAILED);
        }

        if( dup2(fileno(output_file),fileno(stdout)) == -1){
            CHILD_ERROR_EXIT(DUP2_FAILED);
        }
    }


    // 错误输出重定向
    if (_config.error_path.length() != 0 ) {
        // if outfile and error_file is the same path, we use the same file pointer
        if (_config.output_path.length() != 0 &&_config.output_path == _config.error_path) {
            error_file = output_file;
        }
        else {
            error_file = fopen(_config.error_path.c_str(), "w");
            if (error_file == NULL) {
                CHILD_ERROR_EXIT(DUP2_FAILED);
            }
        }
        // redirect stderr -> file
        if (dup2(fileno(error_file), fileno(stderr)) == -1) {
            CHILD_ERROR_EXIT(DUP2_FAILED);
        }
    }

#ifndef LOCAL
    //set gid
    gid_t group_list[] = {_config.gid};
    if (_config.gid != -1 && (setgid(_config.gid) == -1 || setgroups(sizeof(group_list) / sizeof(gid_t), group_list) == -1)) {
        CHILD_ERROR_EXIT(SETUID_FAILED);
    }
#endif

#ifndef LOCAL
    //set uid
    if (_config.uid != -1 && setuid(_config.uid) == -1) {
        CHILD_ERROR_EXIT(SETUID_FAILED);
    }
#endif

    char * args[256]{NULL},* env[256]{NULL};
    args[0] = _config.exe_path.data();

    // 设定执行的参数
    for(int i = 0 ;i < 256 && i < _config.args.size(); ++i)
        args[i+1] = _config.args[i].data();
    // 设定env值
    for(int i = 0 ;i < 256 && i < _config.env.size(); ++i)
        env[i] = _config.env[i].data();

    //#ifdef DEBUG
    //std::cout << "==args=="  << std::endl;
        //for(int i=0;i<256;++i){
            //if( args[i] != NULL)
                //std::cout << args[i] << std::endl;
        //}
    //#endif

    // 变身
    execve(_config.exe_path.c_str(), args,env);
    // 变身后不应该执行这里
    CHILD_ERROR_EXIT(EXECVE_FAILED);

}
