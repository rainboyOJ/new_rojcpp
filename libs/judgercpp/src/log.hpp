#pragma once

#include <fstream>

// 折叠表达式
template<char Delimiter = ' ',typename... Args>
void debug_out(std::ostream &os, Args&&... args){
    ( (os << args << Delimiter),... ) <<std::endl;
}

/**
 * @desc 输出信息到文件用
 */
struct LOG {
    //LOG() = delete;
    //LOG(const char * file_name) 
        //: file_name{file_name},ofs{file_name}
    //{};
    ~LOG(){ ofs.close(); }
    void init(const char * file_name) {
        ofs = std::ofstream(file_name);
    }
    template<typename... Args>
    void write(Args&&... args){
        debug_out(ofs, std::forward<Args>(args)...);
    }
    std::ofstream ofs;
} __LOG__;

#define LOG_INIT(arg)        __LOG__.init(arg)
#define log_write( TAG, ...) __LOG__.write(TAG,"[at Function]:",__FUNCTION__,"[at LINE]:",__LINE__,';',__VA_ARGS__)
#define log_error(...)     log_write("[ERROR]",__VA_ARGS__)
#define log_waring(...)    log_write("[WARNING]",__VA_ARGS__)
#define log_fatal(...)     log_write("[FATAL]",__VA_ARGS__)
#define log_info(...)      log_write("[INFO]",__VA_ARGS__)


/**
 * @desc 退出时,输出的log
 */
#define ERROR_EXIT(error_code)\
    {\
        log_error("error_code:",error_code,"<->",#error_code);\
        _result.error = error_code; \
        return; \
    }

/**
 * @desc child_process 退出时,输出的log
 */
#define CHILD_ERROR_EXIT(error_code)\
    {\
        log_fatal("System_error:",#error_code);\
        __LOG__.~LOG();\
        raise(SIGUSR1);\
        exit(EXIT_FAILURE);\
    }

