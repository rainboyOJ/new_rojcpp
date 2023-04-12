#pragma once


#include "configPair.hpp"

namespace cppjson {

// { 成员名,{成员类型,成员大小} }
using Field=std::unordered_map<std::string,std::pair<std::string,std::size_t>>;

//存下一个类的所有成员信息
class config {
private: //成员
    mutable std::unordered_map<std::string,std::string> key_value;      //存键值对
    Field * field_info;     //指向 成员信息 的指针(批向外部),
    std::size_t class_size; //类大小
    std::size_t class_object_pointer; //类对象的起始地址
public: //函数
    
    config() : field_info{nullptr} {}

    template<typename T>
    config(Field * field_info,T * object) ;

    
    auto begin() { return key_value.begin();}
    auto end() { return key_value.end();}

    std::string serialized_to_string(bool first_nested_layer=false)const;      //序列化为字符串  

    //键值对
    std::string& operator[](const std::string&key) const { return key_value[key]; }
    std::string& operator[](std::string&key) { return key_value[key]; }

    void update(const std::initializer_list<configPair>&pairs);
    //添加变量对,这时候ConfigPair中会记录下类型信息，可以在运行时创建好反序列化时候需要对应的还原函数;
    //config.update({{"namea",this->name1},{"name2",this->name2}});
};

template<typename T>
config::config(Field * field_info,T * object) :
    field_info(field_info),
    class_object_pointer(reinterpret_cast<std::size_t>(object)),
    class_size(sizeof(T))
{}


void config::update(const std::initializer_list<configPair>&pairs){
    for( auto & it : pairs){
        key_value[it.key] = it.value;
        if( field_info != nullptr){
            field_info->emplace(it.key,
                    std::make_pair(it.type, it.address-this->class_object_pointer)
                    );
        }
    }
}


//序列化
std::string config::serialized_to_string(bool first_nested_layer)const //序列化的
{
    std::ostringstream oss;
    char end=first_nested_layer?'\n':' ';
    oss<<"{"<<end;
    for( auto it=key_value.begin(),next=++key_value.begin();
            it!=key_value.end();
            ++it,next!=key_value.end()?++next:next
        )
    {
        if(it!=key_value.end()&&next==key_value.end()) //最后一个元素，没有逗号
            oss<<"\""<<(*it).first<<"\":"<<(*it).second<<end;
        else
            oss<<"\""<<(*it).first<<"\":"<<(*it).second<<","<<end;
    }
    oss<<"}"<<end;
    return oss.str();
}
} // end namespace cppjson

