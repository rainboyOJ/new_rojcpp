#define USE_POLYMORPIC_RESOURCE
#include <iostream>
#include "all_headers.h"

int main(){
    // std::pmr::polymorphic_allocator<char> default_pmr_allocator(std::pmr::get_default_resource());
     // std::pmr::polymorphic_allocator<char> default_pmr_allocator;
    rojcpp::__string mystring("hello pmr");
    std::cout << mystring << " \n";
    return 0;
}
