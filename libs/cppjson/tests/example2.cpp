/**
 * test loads
 */
#include<iostream>
#include<vector>
#include <tuple>

#include "serializable.hpp"

using namespace cppjson;

using namespace std;
struct Node
{
    int x=1;
    float y=5;
    std::string z="sjf";
    std::vector<int> vec{1,2,3,4};
    config get_config() const
    {
        config _conf=Reflectable::get_config(this);
        _conf.update({
            {"x",x},
            {"y",y},
            {"z",z},
            {"vec",vec}
        });
        return _conf;
    }
};

struct Node2 {
    Node node;
    config get_config() const{
        config _conf = Reflectable::get_config(this);
        _conf.update({{"node",node}});
        return _conf;
    }
};


int main()
{
    Serializable::Regist<Node>();
    std::string json = R"({ "vec":[4,3,2,1], "z":"hello world", "y":6.123, "x":100, "class_name":"Node" })";

    std::cout << "======== loads ======= " << std::endl;
    auto load_object =  Serializable::loads<Node>(json);
    std::cout << load_object.x << std::endl;
    std::cout << load_object.y << std::endl;
    std::cout << load_object.z << std::endl;
    std::cout << "vector : " << std::endl;
    for (const auto& e : load_object.vec) {
        std::cout << e << std::endl;
    }



    //Node2 o2;
    //auto json2 = Serializable::dumps(o2);
    //std::cout << json2 << std::endl;



    //Node b=Serializable::loads<Node>(json);                               //反序列化
    //[>正常访问<]
    //cout<<b.x<<endl;                                                      //正常访问成员变量
    //cout<<b.y<<endl;
    //cout<<b.z<<endl;
    //[>成员函数反射<]
    //cout<<Reflectable::get_method<int>(b,"add",5,6)<<endl;                //通过字符串名称访问成员函数
    //cout<<Reflectable::get_method<string>(b,"getName")<<endl;
}
