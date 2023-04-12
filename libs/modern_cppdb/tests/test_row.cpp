#include <string>
#include <iostream>

#include "sql/row.hpp"

int main(){
    //1. 创建
    cppdb::row_type<
        cppdb::column<"name", std::string>,
        cppdb::column<"age", int>
        > myrow;

    //2 设置值
    cppdb::set<0>(myrow, "hello");
    cppdb::set<1>(myrow, 18);

    //3 获得值
    std::cout << cppdb::get<0>(myrow) << std::endl;
    std::cout << cppdb::get<1>(myrow) << std::endl;

    //4 
    std::cout << "is_row type : " << cppdb::is_row_type<decltype(myrow) >::value << std::endl;




    return 0;
}
