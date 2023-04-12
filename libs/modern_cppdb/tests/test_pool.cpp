#include <iostream>
#include "pool.hpp"

using namespace cppdb;

int main(){

    std::string connection_info_ ="mysql:host=127.0.0.1;port=3306;user=root;password=root;database=rojcpp;";
    //auto t = pool<connection>::create(connection_info);
    auto ci = connection_info(connection_info_);
    //pool<connection> t(ci);
    auto mypool = pool::create(ci);

    auto print_address = [](auto address){
        std::cout << GET_TYPE_NAME(address) << std::endl;
        //auto _add = std::reinterpret_cast<std::uintptr_t>(address);
        auto _add = (uint64_t)(address);
        std::cout << std::hex << _add << std::endl;
        std::cout << std::dec;
    };

    print_address(mypool.get());

    std::cout << "pool szie :" << mypool ->size() << std::endl;

    {
        auto conn = mypool->open();
        //print_address(conn->pool_.lock().get());
        auto ver = conn->conn()->server_version();
        std::cout << "server_version : " << ver << std::endl;
    }

    std::cout << "pool szie :" << mypool ->size() << std::endl;
    std::cout << "does conn put self to pool?" << std::endl;

    return 0;
}
