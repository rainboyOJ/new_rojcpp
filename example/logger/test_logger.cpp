#include <iostream>
#include "logger/logger.h"

using namespace LOGGER;
int main(){

    // logStream myLogStream;
    // myLogStream << "hello world";
    // std::cout << myLogStream.get_buff().length() << "\n";
    LOG_INFO << "hello world " ;
    logger::setLogLevel(logger::LogLevel::ERROR);
    LOG_INFO << "not show info";
    logger::setLogLevel(logger::LogLevel::INFO);
    LOG_INFO << "show info";
    return 0;
}
