#include <iostream>
#include "tools.hpp"
#include "sql/query.hpp"


int main(){
    
    std::string connection_info_ ="mysql:host=127.0.0.1;port=3306;user=root;password=root;database=girls;";
    cppdb::pool_manager::init(connection_info_);
    using myschema = cppdb::schema<"myschema", 
          cppdb::column<"id", int>,
          cppdb::column<"name", std::string>,
          cppdb::column<"sex", std::string>,
          cppdb::column<"borndata", cppdb::TIME_Pt>,
          cppdb::column<"phone", std::string>,
          cppdb::column<"photo", std::string>,
          cppdb::column<"boyfriend", int>
          >;

    cppdb::query<
        "select * from beauty b order by b.borndate desc limit 1;"
        , myschema> q;

    //using TT = std::tuple_element<0, myschema::row_type >::type;

    //using x = cppdb::column<"int", int>::type;
    //if( std::is_same_v<x, int>)
        //std::cout << "Yes" << std::endl;
    //else
        //std::cout << "no" << std::endl;
    //std::cout << GET_TYPE_NAME(TT) << std::endl;
    //std::cout << GET_TYPE_NAME(const int &) << std::endl;
    auto res = q << cppdb::exec;

    for (const auto& e : res) {
        //输出所有的值
        print_row(e);
    }

    return 0;
}
