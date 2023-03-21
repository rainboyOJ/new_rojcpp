#include <thread>
#include <vector>
#include <iostream>
#include "logger/logger.h"

using namespace LOGGER;

void test(int id) {
    logger::set_thread_id(id);
    LOG_INFO << "hello world " ;
    LOG_INFO << "not show info";
    LOG_INFO << "show info";
}


int main(){

    // logStream myLogStream;
    // myLogStream << "hello world";
    // std::cout << myLogStream.get_buff().length() << "\n";

    logger::set_thread_id(0);
    LOG_INFO << "hello world " ;
    logger::setLogLevel(logger::LogLevel::ERROR);
    LOG_INFO << "not show info";
    logger::setLogLevel(logger::LogLevel::INFO);
    LOG_INFO << "show info";
    logger::setLogLevel(logger::LogLevel::INFO);

    std::vector<std::thread> vec;
    for(int i=1;i<=2;++i){
        vec.emplace_back(test,i);
        // std::thread a(test);
        // a.join();
    }
    for (auto& e : vec) {
        e.join();
    }


    return 0;
}
