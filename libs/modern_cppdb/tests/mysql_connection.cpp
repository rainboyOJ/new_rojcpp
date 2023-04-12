/**
 * 连接
 */
#include <iostream>
//#include "pool_manager.hpp"
#include "backend/mysql_backend.hpp"
#include "backend/result_backend.hpp"

using namespace cppdb;

int main(){

    std::string connection_info_ ="mysql:host=127.0.0.1;port=3306;user=root;password=root;database=rojcpp;";
    backend::connection myconn(connection_info_);

    auto ver = myconn.server_version();
    std::cout << "server_version : " << ver << std::endl;

    //得到结果

    myconn.exec("select 1+1;");
    backend::result myresult(myconn);
    std::cout << "cols : " << myresult.cols() << std::endl;
    if( myresult.has_next() == backend::result::next_row_exists ){
        if( !myresult.next() )  {
            return 0;
        }
    }
    else  {
        std::cout << "next row not exists" << std::endl;
        return 0 ;
    }
    bool succ;
    auto num = myresult.fetch<int>(0,succ);
    if( succ)
        std::cout << "get value success : "  << num << std::endl;
    else
        std::cout << "get value failed!" << std::endl;

    auto name = myresult.column_to_name(0);
    std::cout << "col 0 name : "  << name << std::endl;




    return 0;
}
