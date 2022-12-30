#pragma once
#include <iostream>

#define  debug(...) __print(__FILE__,__LINE__,__VA_ARGS__)

template<typename ...Args>
void __print(Args&& ... args){
    ( ( std::cout << args << " "),...);
}
