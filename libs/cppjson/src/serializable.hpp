//序列化,反序列化功能
#pragma once


#ifndef __SERIALIZABLE_H__
#define __SERIALIZABLE_H__
#endif

#include "reflectable.hpp"

namespace cppjson {

class Serializable  : public Reflectable {
public:

    template<typename T> //注册
    static void Regist(){
        configPair::string_to_value[GET_TYPE_NAME(T)] = [](void * object,const std::string & value)->void
        {
            config __conf = Serializable::parse(value);
            Serializable::config_to_object<T>( reinterpret_cast<T*>(object) , __conf);
        };
        Reflectable::Regist<T>();
    }
    
    /**
     * @brief 序列化对象
     */
    template<typename Object>
    inline static std::string dumps(const Object&object){
        if constexpr (has_config_member_function<Object>::value)
            return object.get_config().serialized_to_string();
        else
            return configPair::value_to_string(object);
    }

    /**
     * @brief 反序列化/加载
     */
    template<typename T>
    static T loads(const std::string& json){
        std::string class_name=GET_TYPE_NAME(T);
        if constexpr (has_config_member_function<T>::value) {
            try {
                T obj;
                configPair::string_to_value[class_name](&obj,json);
                return obj;
            }
            catch(std::exception & e){
                std::cerr << " Exception : " << e.what() << "\n";
            }
        }
        else //其他的没有实现get_config方法，但是是int,std::vector等可以序列化的class
        {
            T obj;
            configPair::string_to_value[class_name](&obj,json);
            return obj;
        }
    }

    /**
     * @brief 解析一个json 转成config对象,是一个状态机,是整个cppjson的核心
     */
    static config parse(std::string_view json){

        //状态 
        enum State {
            init,
            parse_value,
            parse_struct,
            parse_fundamental,
            parse_iterable,
            parse_string,
            end_parse
        } state = init;
        config __conf; //

        while ( !json.empty() && Any_Of(json.front(), '\t','\r','\n',' ') ) { json.remove_prefix(1); }
        while ( !json.empty() && Any_Of(json.back(),  '\t','\r','\n',' ') ) { json.remove_suffix(1); }

        if( json.front() != '{') throw JsonDecodeDelimiterException('{');
        if( json.back()  != '}') throw JsonDecodeDelimiterException('}');
        std::string key,value;

        int nested_struct_layer=0,nested_iterable_layer=0;

        for(std::size_t i = 0 ;i < json.size(); ++i){
            auto& it = json[i];
            if(state == init){
                if( it == ':') 
                    state = parse_value;
                else if( !isBlankChar(it) && None_of(it, ',','{','\"') )
                    key.push_back(it);
            }
            else if( state == parse_value){
                if( isBlankChar(it) ) continue;
                if( it == '{' ){
                    value.push_back(it);
                    nested_struct_layer++;
                    state = parse_struct;
                }
                else if( it == '[' ){
                    value.push_back(it);
                    nested_iterable_layer++;
                    state = parse_iterable;
                }
                else if( it == '\"' ){
                    value.push_back(it);
                    state = parse_string;
                }
                else if( !isBlankChar(it) ){
                    value.push_back(it);
                    state = parse_fundamental;
                }
            }
            else if( state == parse_string){
                value.push_back(it);
                if( it == '\"' && i > 0 && json[i-1] != '\\' ){  // 结尾
                    state = end_parse;
                    --i;
                }
            }
            else if( state == parse_fundamental ){
                if( Any_Of(it, ',','}', '\"') || isBlankChar(it) ) { //结束
                    if( it == '\"')
                        value.push_back(it);
                    state = end_parse;
                    --i;
                    continue;
                }
                value.push_back(it);
            }
            else if( state == parse_iterable ){
                if( Any_Of(it, '[',']') ){
                    nested_iterable_layer += (it == ']' ? -1 : 1);
                    value.push_back(it);
                    if( nested_iterable_layer == 0){
                        state = end_parse;
                        --i;
                    }
                    continue;
                }
                value.push_back(it);
            }
            else if( state == parse_struct){
                if(Any_Of(it, '{','}') ){
                    nested_struct_layer += ( it == '}' ? -1 : 1);
                    value.push_back(it);
                    if(nested_struct_layer == 0){
                        state = end_parse;
                        --i;
                    }
                    continue;
                }
                value.push_back(it);
            }
            else if ( state == end_parse){
                //std::cout << key << " " << value << std::endl;
                state = init;
                __conf[key] = value;
                key.clear();
                value.clear();
            }
        }

        return __conf;
    }

    /**
     * @brief 从config 转成这个对象
     */
    template<typename T>
    static void config_to_object(T * objP,config & __conf){
        std::string class_name = GET_TYPE_NAME(T);
        //std::cout << "== config_to_object " << class_name << std::endl;
        for( auto & it : __conf){
            if( it.first != "class_name" ){
                auto &[field_name,value] = it;
                std::string str_type =  Reflectable::get_field_type(class_name, field_name);
                void *field = Reflectable::get_field(objP, class_name, field_name);
                if( str_type.back() == '*' && value == "null")
                    *(void**)field = nullptr;
                else
                    try {
                        configPair::string_to_value[str_type](field,value);
                    }
                    catch(std::exception & e){
                        std::cerr << " Exception : " << e.what() << "\n";
                    }

            }
        }
    }

};
    
} // end namespace cppjson

