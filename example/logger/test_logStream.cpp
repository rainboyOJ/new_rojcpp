#include <iostream>
#include "logger/logStream.hpp"

using namespace LOGGER;

int main(){

    logStream myLogStream; // create logStream object

    long long b = 123;
    std::string_view sv = "string_view";
    //test input number(int,long long)
    //string_view
    //const char
    //float double
    //negative number
    myLogStream << 1 << " \nlong long:" << b << "\nstring_view:" << sv ;
    myLogStream << " \nconst char string\n";
    float f = 1.23;
    double d = 21.23;
    myLogStream << f<< " " << d << ' ';
    myLogStream << -123;


    std::cout << (char *)myLogStream.get_buff().data() << "\n";
    
    return 0;
}
