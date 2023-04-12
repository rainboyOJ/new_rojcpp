#include <iostream>
#include "utils.hpp"
#include "connection_info.hpp"
#include "sql/query.hpp"

int main(){

    std::string connection_info_ ="mysql:host=127.0.0.1;port=3306;user=root;password=root;database=girls;";
    cppdb::pool_manager::init(connection_info_);

    /*
    // 不需要返回值的时候
    cppdb::query<
        "insert into boys(boyName,userCP) VALUES('?',?);"
        , void> q;

    q << "audi2" << 300 << cppdb::exec;

*/
    //行 接收一行的信息
    using boysRow = 
        cppdb::row_type<
            cppdb::column<"boyName", std::string>,
            cppdb::column<"userCP", int>
            >;
    cppdb::query<"select boyName,userCP from boys;", boysRow> q_row;

    boysRow boyrow_1 = q_row << cppdb::exec;
    //输出
    std::cout << cppdb::get<"boyName",boysRow>(boyrow_1) << std::endl;
    std::cout << cppdb::get<"userCP",boysRow>(boyrow_1) << std::endl;

    //std::cout << "res1 :" << res1 << std::endl;
    //query
    //del
    //update

    return 0;
}
