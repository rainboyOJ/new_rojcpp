/**
 * 嵌套使用
 */
#include<iostream>
#include <memory>
#include<vector>
#include <tuple>
#include<iostream>
#include<fstream>
#include<cstring> 

#include "serializable.hpp"
using namespace std;
using namespace cppjson;

int main(){
    //dump vector
    std::vector<int> v1= {1,2,3,4,5,6};
    auto json1 = Serializable::dumps(v1);
    std::cout << json1 << std::endl;
    using T1  =decltype(v1);
    std::cout << GET_TYPE_NAME(T1) << std::endl;
    T1 v1_loads = Serializable::loads<T1>(json1);
    for (const auto& e : v1_loads) {
        std::cout << e << std::endl;
    }
    return 0;
}
