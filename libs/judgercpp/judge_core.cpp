#include <iostream>
#include <thread>
#include <chrono>
#include <utility>
#include <string>
#include <fstream>
#include <unistd.h> 
#include <variant>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <grp.h>
#include <signal.h>

#include "commandline.hpp"
#include "judger.h"
#include "src/utils.hpp"
#include "src/log.hpp"
#include "src/run.hpp"

// 全局变量
//// == for arguments
CommandLine cmd("judger for noier and acmer.");

bool     help                    = false;
bool     show_version            = false;

uint32_t version = 1;

result RESULT;


int main(int argc,char *argv[]){

    // ================= 命令行解析 功能
    // example :
    //  judger -h
    //  judger -t=1000 -e 1         设置时间为1000ms 执行程序 1
    //  judger --max_cpu_time=1000 -e 1
    cmd.addArgument({"-h", "--help"},    &help,    "输出帮助,然后退出。");
    cmd.addArgument({"-v", "--version"}, &show_version, "输出版本号,然后退出。");
    cmd.addArgument({"-t","--max_cpu_time"}, &CONFIG.max_cpu_time,  "程序的最大CPU运行时间 (ms),0 表示没有限制,默认为0。");
    cmd.addArgument({"-r","--max_real_time"}, &CONFIG.max_real_time,  "程序的最大实际时间(ms),0表示没有限制,默认为0。");
    cmd.addArgument({"-m","--max_memory"}, &CONFIG.max_memory,  "程序运行的内存限制(byte),默认512M。");
    cmd.addArgument({"-only","--memory_limit_check_only"}, &CONFIG.memory_limit_check_only,  "只检查内存使用，不加以限制，默认为False");
    cmd.addArgument({"-s","--max_stack"}, &CONFIG.max_stack,  "栈空间限制,默认没有限制");
    cmd.addArgument({"-p","--max_process_number"}, &CONFIG.max_process_number,  "最大进程数量，默认没有限制");
    cmd.addArgument({"-os","--max_output_size"}, &CONFIG.max_output_size,  "最大输出大小(byte)");

    cmd.addArgument({"--cwd"}, &CONFIG.cwd, "运行的评测程序的work_path");
    cmd.addArgument({"-e","--exe_path"}, &CONFIG.exe_path, "要判断的程序的路径");
    cmd.addArgument({"-i","--input_path"}, &CONFIG.input_path,  "输入文件路径");
    cmd.addArgument({"-o","--output_path"}, &CONFIG.output_path,  "输出文件路径");
    cmd.addArgument({"-ep","--error_path"}, &CONFIG.error_path,  "错误输出路径");

    cmd.addArgument({"--args"}, &CONFIG.args,  "程序运行的参数表"); // TODO
    cmd.addArgument({"--env"}, &CONFIG.env,  "环境表");

    cmd.addArgument({"--log_path"}, &CONFIG.log_path,  "日志路径,默认 judge_log.txt");
    cmd.addArgument({"-rule","--seccomp_rule_name"}, &CONFIG.seccomp_rule_name,  "Seccomp Rule Name");

//#ifndef LOCAL //local也接收这个参数 但不会产生任何效果
    cmd.addArgument({"-u","--uid"}, &CONFIG.uid,  "UID (default 65534)");
    cmd.addArgument({"-g","--gid"}, &CONFIG.gid,  "GID (default 65534)");
//#endif
    cmd.parse(argc, argv);

    if( help ){
        cmd.printHelp();
        return 0;
    }
    else if( show_version ){
        std::cout << "version: "  << version << std::endl;
        return 0;
    }

    LOG_INIT(CONFIG.log_path.c_str());

    #ifdef DEBUG
#define print_config(node) std::cout << std::setw(25) << #node ":" << '\t'<< CONFIG.node << '\n'
        print_config(max_cpu_time);
        print_config(max_real_time);
        print_config(max_process_number);
        print_config(memory_limit_check_only);
        print_config(max_output_size);
        print_config(max_memory);
        print_config(max_stack);
        print_config(exe_path);
        print_config(input_path);
        print_config(output_path);
        print_config(error_path);
        print_config(log_path);
        print_config(seccomp_rule_name);
        print_config(seccomp_rule_name);
        print_config(uid);
        print_config(gid);

        std::cout << "--args count " << CONFIG.args.size() << std::endl ;
        for (const auto& e : CONFIG.args) {
            std::cout << "     " ;
            std::cout << e << std::endl;
        }
        std::cout << "\n\n" ;

        std::cout << "--env count " << CONFIG.env.size() << std::endl ;
        for (const auto& e : CONFIG.env) {
            std::cout << "     " ;
            std::cout << e << std::endl;
        }
        std::cout << "\n\n" ;
    #endif
    // ================= 命令行解析 功能 结束

    run(CONFIG, RESULT);
    print_result();
    return 0;
}
