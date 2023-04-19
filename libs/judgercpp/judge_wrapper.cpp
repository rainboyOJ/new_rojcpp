#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>

#include "judger.h"

void exec(const char* cmd,std::ostream& __out) {
    char buffer[128];
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error(std::string("popen() failed!") + __FILE__ + " line: " +  std::to_string(__LINE__).c_str());
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            __out << buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    //return result;
}

int main(int argc,char * argv[]){
    std::stringstream ss;

    std::string cmd = "/home/rainboy/mycode/RainboyOJ/judgecpp/judger_core";
    int i=1;
    while ( i < argc ) {
        cmd += " ";
        cmd += '"';
        cmd += argv[i++];
        cmd += '"';
    }
    exec(cmd.c_str(), ss);
#ifdef DEBUG
        std::cout << ss.str() << std::endl;
#else
        //cpu_time real_time memory signal exit_code error result 
        result RESULT;
        ss >> RESULT.cpu_time;
        ss >> RESULT.real_time;
        ss >> RESULT.memory;
        ss >> RESULT.signal;
        ss >> RESULT.exit_code;
        ss >> RESULT.error;
        ss >> RESULT.result;

#endif
    return 0;
}
