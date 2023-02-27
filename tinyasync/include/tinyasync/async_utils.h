#ifndef __ASYNC_UTILS__
#define __ASYNC_UTILS__

#include <iostream>
#include <string>
#include <iterator>


const char * __LAST_SLASH(const char * s){
    int last = 0;
    for(int i = 0 ; 1 ;i++){
        if( s[i] == '/') last = i;
        if( s[i] == '\0') break;
    }
    return s+last+1;
}


#define log(args...) { std::cout << __LAST_SLASH(__FILE__) << ", LINE:" << __LINE__ << " ";std::string _s = #args; std::replace(_s.begin(), _s.end(), ',', ' '); std::stringstream _ss(_s); std::istream_iterator<std::string> _it(_ss); err(_it, args); }

void err(std::istream_iterator<std::string> it) {}

template<typename T, typename... Args>
void err(std::istream_iterator<std::string> it, T a, Args... args) {
    std::cerr << *it << " = " << a << std::endl;
	err(++it, args...);
}


#endif
