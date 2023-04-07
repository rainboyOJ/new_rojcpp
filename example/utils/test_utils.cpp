#include <iostream>
#include <array>
#include "tools/utils.hpp"
using namespace rojcpp;

template<int ...Is>
constexpr auto get_method_arr() {
    std::array<int,26> abc{0};
    std::string_view s;
    abc[0] = 1;
    // ((s = type_to_name(std::integral_constant<http_method, Is>{}), arr[s[0]-65] = s[0]), ...);

    return abc;
}

int main(){
    
    // constexpr auto methods = get_method_arr<1,2,3>();

    constexpr auto methods2 = rojcpp::get_method_arr<POST>();
    for (const auto& e : methods2) {
        if( e == 0 )
            std::cout << 0 << "\n";
        else
            std::cout << e << "\n";
    }
    return 0;
}
