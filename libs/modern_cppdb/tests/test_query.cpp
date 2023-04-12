#include <iostream>
#include "sql/query.hpp"

int main(){

    std::string connection_info_ ="mysql:host=127.0.0.1;port=3306;user=root;password=root;database=grils;";
    cppdb::pool_manager::init(connection_info_);
    using myschema = cppdb::schema<"myschema", 
          cppdb::column<"int", int>
          >;
    cppdb::query<
        "select 1+?120;"
        , myschema> q;

    std::cout << q.mark_size_ << std::endl;

    //std::cout << q.at(0) << std::endl;
    //q << 123;
    //std::cout << q.at(0) << std::endl;

    q << 110;
    auto Result_scheme = q  << cppdb::exec;


    for (auto& e : Result_scheme) {
        std::cout << cppdb::get<0>(e) << std::endl;
    }

    return 0;
}
