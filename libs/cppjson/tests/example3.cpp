/**
 * test loads
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

struct Node
{
    Node()
    {
        std::memset(z,0,sizeof(z));
        //t={1,2,3,4};
    }
    config get_config()const 
    {
        config config=Serializable::get_config(this);
        config.update({
            {"y",y},
            {"z",z},
            //{"t",t}
        });
        return config;
    }
    std::vector<Node*>y;
    int z[3];
    //std::array<int,4>t;
};
int main() 
{
    Serializable::Regist<Node>();                                                   //注册

    Node a=*(Node*)Reflectable::get_instance("Node");                               //创建实例
    auto share_A = std::make_shared<Node>(a);

    //Reflectable::set_field(a,"x",make_tuple(3.2f,make_pair(5,string{"test"})));     //通过名称修改属性
    //Reflectable::set_field(a,"y",std::vector<Node*>{new Node,nullptr}); 
    int*z=(int*)Reflectable::get_field<Node>(a,"z");                                      //获得属性，并进行修改
    z[0]=2021,z[1]=10,z[2]=18;
    //a.t[0]=2021,a.t[1]=10,a.t[2]=19,a.t[3]=10;
    std::string json=Serializable::dumps(a);                                        //序列化为json格式的字符串
    cout<<"json\n"<<json<<endl;         
    Node b=Serializable::loads<Node>(json);                                         //通过json格式的字符串进行反序列化
    //cout<<endl<<a.get_config().serialized_to_string(true);
    cout<<endl<<b.get_config().serialized_to_string(true);                          //打印结果
    return 0;
} 
