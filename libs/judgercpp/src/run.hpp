#pragma once

#include <chrono>

#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <grp.h>
#include <signal.h>

#include "judger.h"
#include "log.hpp"
#include "child_process.hpp"

void run(config & _config, result &_result) {

#ifndef LOCAL
    // 不是本地使用时,需要root权限
    uid_t uid = getuid();
    if (uid != 0) {
        ERROR_EXIT(ROOT_REQUIRED);
    }
#endif
        
    // TODO check arguments

    auto time_start = std::chrono::system_clock::now();

    pid_t child_pid = fork();
    if( child_pid < 0 ){
        ERROR_EXIT(FORK_FAILED);
    }
    else if (child_pid == 0){   // child_process
        child_process(_config);
    }
    else if ( child_pid > 0){   // 主进程
        int status;
        struct rusage resource_usage;
        //wait4 等到pid进程结束,并得到resource https://man7.org/linux/man-pages/man2/wait4.2.html
        // WSTOPPED 直到stop https://man7.org/linux/man-pages/man2/wait.2.html
        if( wait4(child_pid, &status, WSTOPPED, &resource_usage) == -1) {
            kill(child_pid,SIGKILL);
            ERROR_EXIT(WAIT_FAILED);
        }

        //std::chrono::time_point< std::chrono::system_clock > 
        auto time_end = std::chrono::system_clock::now();

        std::chrono::duration<double, std::milli>(time_start
                - time_end).count(); // 得到运行的时间
 
//WIFSIGNALED(wstatus) returns true if the child process was terminated by a signal.
              // WTERMSIG(wstatus)
              //    returns the number of the signal that caused the child
              //    process to terminate.  This macro should be employed only
              //    if WIFSIGNALED returned true.
        if (WIFSIGNALED(status) != 0) {
            _result.signal = WTERMSIG(status); //被哪个信号关闭的
        }

        if(_result.signal == SIGUSR1) {
            _result.result = SYSTEM_ERROR; // 为什么SIGUSR1 表示系统错误
        }
        else {
            _result.exit_code = WEXITSTATUS(status);
            _result.cpu_time = (int) (resource_usage.ru_utime.tv_sec * 1000 +
                                       resource_usage.ru_utime.tv_usec / 1000);
            _result.memory = resource_usage.ru_maxrss * 1024; // kb to bytes

            if (_result.exit_code != 0) {
                _result.result = RUNTIME_ERROR;
            }

            if (_result.signal == SIGSEGV) { // 段错误
                if (_config.max_memory != UNLIMITED && _result.memory > _config.max_memory) {
                    _result.result = MEMORY_LIMIT_EXCEEDED; //超内存
                }
                else {
                    _result.result = RUNTIME_ERROR; // 运行错误
                }
            }
            else {
                if (_result.signal != 0) {
                    _result.result = RUNTIME_ERROR;
                }
                if (_config.max_memory != UNLIMITED && _result.memory > _config.max_memory) {
                    _result.result = MEMORY_LIMIT_EXCEEDED;
                }
                if (_config.max_real_time != UNLIMITED && _result.real_time > _config.max_real_time) {
                    _result.result = REAL_TIME_LIMIT_EXCEEDED;
                }
                if (_config.max_cpu_time != UNLIMITED && _result.cpu_time > _config.max_cpu_time) {
                    _result.result = CPU_TIME_LIMIT_EXCEEDED;
                }
            }
        }
    }
}
