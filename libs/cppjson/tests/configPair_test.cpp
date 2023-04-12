#include <iostream>
#include "configPair.hpp"
using namespace std;

#define  log(one) cout << #one << " : " << one << endl;

struct node {
    int a,b;
};
int main(){
    node _node;
    _node.a = 1;
    cppjson::configPair cp("a",_node.a);
    using namespace cppjson;
    
    log(cp.address);
    log(cp.value);
    log(cp.key);
    log(cp.type);
    return 0;
}
