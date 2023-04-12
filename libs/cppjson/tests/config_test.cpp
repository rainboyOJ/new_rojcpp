#include <iostream>
#include "config.hpp"
using namespace std;
using namespace cppjson;

#define  log(one) cout << #one << " : " << one << endl;

struct node {
    int a,b;
};

cppjson::Field class_field;

int main(){
    node _node;
    _node.a = 1;
    _node.b = 2;

    config myconfig(&class_field,&_node);
    myconfig.update(
            {
                {"a",_node.a},
                {"b",_node.b}
            }
            );

    for (const auto& e : myconfig) {
        std::cout << e.first << " " << e.second << std::endl;
    }

    std::cout << myconfig.serialized_to_string() << std::endl;
    
    return 0;
}
