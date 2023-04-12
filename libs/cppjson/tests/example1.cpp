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
    //char *abc;
    std::tuple<int,float,std::string> tp = std::make_tuple(1,1.1,"str");
    //std::tuple<int,float,std::string> tp = std::make_tuple(1,1.1,"str");
    //std::pair<int, std::string> Pair = std::make_pair(100, "hello");
    std::vector<int> vec{1,2,3,4};
    config get_config() const
    {
        config _conf=Reflectable::get_config(this);
        _conf.update({
            {"x",x},
            {"y",y},
            {"z",z},
            //{"abc",abc},
            //{"tp",tp},
            //{"pair",Pair},
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
    Node object;
    //object.abc = "abc";

    /*序列化与反序列化*/
    
    std::string json=Serializable::dumps(object);                 //序列化
    cout<<json<<endl;

    std::cout << "============ Serializable::parse" << std::endl;
    auto decode_conf = Serializable::parse(json);
    std::cout << "config iterable" << std::endl;
    for (const auto& e : decode_conf) {
        std::cout << e.first << " " << e.second << std::endl;
    }
    std::cout << "======== loads ======= " << std::endl;
    auto load_object =  Serializable::loads<Node>(json);
    std::cout << "end load" << std::endl;
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
