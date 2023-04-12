//多次执行一条命令,
//目的: 是否可以正确的从连接池中获取与释放
#include <iostream>
#include "utils.hpp"
#include "connection_info.hpp"
#include "sql/query.hpp"

int main(){

    std::string connection_info_ ="mysql:host=127.0.0.1;port=3306;user=root;password=root;database=girls;";
    cppdb::pool_manager::init(connection_info_);

    for(int i = 1 ;i <= 10;i++) {
        std::cout << ">>>>>>>>>>>>>>>>>> run :" << i << std::endl;
        //执行命令,但不接受反回信息
        using boysRow = 
            cppdb::row_type<
                cppdb::column<"boyName", std::string>,
                cppdb::column<"userCP", int>
                >;
        cppdb::query<"select boyName,userCP from boys;", void> q_row;

        //boysRow boyrow_1 = 
            q_row << cppdb::exec;
        //输出
        //std::cout << cppdb::get<"boyName",boysRow>(boyrow_1) << std::endl;
        //std::cout << cppdb::get<"userCP",boysRow>(boyrow_1) << std::endl;
    }


    return 0;
}
