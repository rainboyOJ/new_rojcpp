#include <iostream>
#include <string>
#include "cexpr/string.hpp"

int main(){
    //测试1 cexpr_string 进行+操作

    cexpr::string t1("hello1"); //
    cexpr::string t2("hello2"); //
                                //
    //两都的类型是一样的 cexpr::string<char,6>
    std::cout << 
        std::is_same_v<decltype(t1), decltype(t2)>
        << std::endl;

    //输出
    std::cout << 
        t1.string_
        << std::endl;

    std::string str("raw ");
    //auto s1 = str + t1;
    str += t1;
    std::cout << 
        str
        << std::endl;
    return 0;
}
