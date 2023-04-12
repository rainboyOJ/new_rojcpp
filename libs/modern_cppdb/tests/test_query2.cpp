#include <iostream>
#include "sql/query.hpp"

int main(){

    std::string connection_info_ ="mysql:host=127.0.0.1;port=3306;user=root;password=root;database=girls;";
    cppdb::pool_manager::init(connection_info_);


    cppdb::query<
        "select 1+?;"
        , int> q_int;
    
    auto ret = q_int<< 1 << cppdb::exec;
    std::cout << "ret : " << ret << std::endl;

    cppdb::query<
        "select count(*) from ?;"
        , int> q_int2;
    ret = q_int2 << "beauty" << cppdb::exec;
    std::cout << "table beauty count =   " << ret << std::endl;



    return 0;
}
