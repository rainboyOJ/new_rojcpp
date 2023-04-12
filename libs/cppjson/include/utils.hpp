#pragma once

#include <string>
#include <typeinfo>
#include <sstream>
#include <cstring>
#include <vector>
#include <cxxabi.h>
#include <unordered_map>
#include <functional>

namespace cppjson {

#define GET_TYPE_NAME(type) abi::__cxa_demangle(typeid(type).name(),0,0,0)

using STR_TO_VALUE_FUNC_MAP_TYPE = std::unordered_map <std::string, std::function<void(void*,const std::string&)> > ;
    
struct EmptyClass{}; //在Reflectanle::get_method与Reflectable::classmethod_wrapper中有使用，用于类型转换的"中介"

template<typename T,typename... U>
inline bool Any_Of(T&& t,U... args){
    return  ( (t==args)||...);
}

template<typename T,typename... U>
inline bool None_of(T&& t,U... args){ return !Any_Of(std::forward<T>(t),args...);
}

inline bool isBlankChar(char c){
    return Any_Of(c,'\t','\n','\r',' ');
}

inline std::vector<std::string>unpacking_list(const std::string&serialized)              //列表解包,"[1,2,3,4]"->["1","2","3","4"]
{                                                                                        //思路参考Serializable::decode
    enum State{init,parse_fundamental,parse_string,parse_struct,parse_iterable,end_parse}state=init;
    std::vector<std::string>vec;
    std::string temp;
    int length=serialized.size();
    int nested_struct=0;             //嵌套的情况：{{},{}}
    int nested_iterable=0;           //嵌套的情况：[[],[],{}]
    for(int i=0;i<length;++i)
    {
        auto&it=serialized[i];
        if(i==0)
            continue;
        if(state==init)
        {
            if(it=='{')
            {
                state=parse_struct;
                nested_struct++;
                temp.push_back(it);
            }
            else if(it=='[')
            {
                state=parse_iterable;
                nested_iterable++;
                temp.push_back(it);
            }    
            else if(it!=','&&it!=' ')
            {
                state=parse_fundamental;
                temp.push_back(it);
            }
            else if(it=='\"')
            {
                state=parse_string;
                temp.push_back(it);
            }
        }
        else if(state==parse_string)
        {
            temp.push_back(it);
            if(it=='\"'&&serialized[i-1]!='\\') //转义字符不是结束
            {
                state=end_parse;
                --i;
            }
        }
        else if(state==parse_struct)
        {
            if(it=='}'||it=='{')
                nested_struct+=(it=='}'?-1:1);
            if(nested_struct==0) //解析完毕
            {
                state=end_parse;
                --i;
                temp.push_back(it);
                continue;
            }
            temp.push_back(it);
        }
        else if(state==parse_iterable)
        {
            if(it==']'||it=='[')
                nested_iterable+=(it==']'?-1:1);
            if(nested_iterable==0)
            {
                state=end_parse;
                --i;
                temp.push_back(it);
                continue;
            }
            temp.push_back(it);
        }
        else if(state==parse_fundamental)
        {
            if(it==','||it==']')
            {
                state=end_parse;
                --i;
                continue;
            }
            temp.push_back(it);
        }
        else if(state==end_parse)
        {
//            std::cout<<"<"<<temp<<">\n";
            vec.push_back(temp);
            temp.clear();
            state=init;
        }
    }
    return vec;
}


template<typename T>
struct has_config_member_function {
    template<typename U>
    static auto check(int) -> decltype(std::declval<U>().get_config(),std::true_type());

    //sink hole function
    template<typename U>
    static std::false_type check(...);

    static constexpr bool value = decltype(check<T>(0))::value;
};

//使用偏特化
template <typename> struct is_tuple: std::false_type {};
template <typename ...T> struct is_tuple<std::tuple<T...>>: std::true_type {};

template <typename> struct is_pair: std::false_type {};
template <typename T,typename U> struct is_pair<std::pair<T,U>> : std::true_type {};

template <typename> struct is_vector: std::false_type {};
template <typename T> struct is_vector<std::vector<T>>: std::true_type {};


/// ====================== 函数

//对于每一个元素进行遍历
template<typename Object,int index>
inline auto for_each_element(Object&object,auto&&callback) 
{
    callback(std::get<index>(object),index);
    if constexpr(index+1 < std::tuple_size<Object>::value)
        for_each_element<Object,index+1>(object,callback);
}

template<typename T,typename F,std::size_t... idx>
void __for_each(T&& t,F&& f,std::index_sequence<idx...> ){
    (f(std::get<idx>(t),idx == sizeof...(idx)-1 ),...); //fold expression

}

template<typename... Ts,typename F>
void for_each_tuple(std::tuple<Ts...> const & t,F&& f){
    __for_each(t,std::forward<F>(f),std::make_index_sequence<sizeof... (Ts)>{});
}
/// ====================== 函数 end


// value_to_string 利用了偏特化的技巧
// ============== 定义

template<typename T,typename = void>
struct To_String {
    static std::string to(const T & object,STR_TO_VALUE_FUNC_MAP_TYPE & to_func) {
        return "unkown";
    }
};

template<typename T>
struct To_String <T,
    std::enable_if_t< std::is_fundamental_v<T> && (! std::is_same_v<T, char *>) >
    >
{
    static std::string to(const T & object,STR_TO_VALUE_FUNC_MAP_TYPE & to_func) {
#ifdef __SERIALIZABLE_H__
        to_func[GET_TYPE_NAME(T)] = [](void * field,const std::string& str) ->void
        {
            std::istringstream iss(str);
            T value;
            iss >> value;
            *reinterpret_cast<T*>(field) = value;
        };
#endif
        return std::to_string(object);
    }
};

template<typename T>
struct To_String <T,
    std::enable_if_t< std::is_same_v<T, std::string>>
> {
    static std::string to(const T & object,STR_TO_VALUE_FUNC_MAP_TYPE & to_func) {
#ifdef __SERIALIZABLE_H__
        to_func[GET_TYPE_NAME(T)] = [](void * field,const std::string& str) ->void
        {
            std::string_view value(str);
            if( value.front() == '\"' ) value.remove_prefix(1);
            if( value.back() == '\"' )  value.remove_suffix(1);
            *reinterpret_cast<T*>(field) = std::string(value);
        };
#endif
        return std::string("\"") + object + std::string("\"");
    }
};

template<typename T>
struct To_String <T,
    std::enable_if_t< std::is_same_v<T, char *>>
> {
    static std::string to(const T & object,STR_TO_VALUE_FUNC_MAP_TYPE & to_func) {
        return std::string("\"") + std::string(object) + std::string("\"");
    }
};

//可序列化 get_config
template<typename T>
struct To_String <T,
    std::enable_if_t< has_config_member_function<T>::value>
> {
    static std::string to(const T & object,STR_TO_VALUE_FUNC_MAP_TYPE & to_func) {
        return object.get_config().serialized_to_string();
    }
};

template<typename T>
struct To_String <T,
    std::enable_if_t< is_tuple<T>::value >
> {
    static std::string to(const T & object,STR_TO_VALUE_FUNC_MAP_TYPE & to_func) {
        std::ostringstream oss;
        oss << "[";
        for_each_tuple(object, [&oss](auto x,bool last){
                    oss << To_String<std::remove_cv_t<decltype(x)>>::to(x);
                    if( !last ) oss << ",";
                });
        oss << "]";

        return oss.str();
    }
};

template<typename T>
struct To_String <T,
    std::enable_if_t< is_pair<T>::value >
> {
    static std::string to(const T & object,STR_TO_VALUE_FUNC_MAP_TYPE & to_func) {
        std::ostringstream oss;
        oss << "[";
        oss << To_String<std::tuple_element_t<0,T>>::to(object.first);
        oss << ",";
        oss << To_String<std::tuple_element_t<1,T>>::to(object.second);
        oss << "]";
        return oss.str();
    }
};

template<typename T>
struct To_String <T,
    std::enable_if_t< is_vector<T>::value >
> {
    template<typename U>
    static std::string to(const std::vector<U> & object,STR_TO_VALUE_FUNC_MAP_TYPE & to_func) {
        std::ostringstream oss;
        oss << "[";
        std::size_t idx = 0;
        for (const auto& e : object) {
            oss << To_String<U>::to(e,to_func);
            if( ++idx != object.size() )
                oss << ",";
        }
        oss << "]";
#ifdef __SERIALIZABLE_H__
        to_func[GET_TYPE_NAME(T)] = [&to_func](void*field,const std::string&str)->void 
        {
            auto values=unpacking_list(str);                                     //字符串形式的值
            T object(values.size());
            std::size_t idx{0};
            for (auto& e : object) {
                to_func[GET_TYPE_NAME(U)](&e,values[idx++]);
            }
            *reinterpret_cast<T*>(field) = std::move(object);
        };
#endif
        return oss.str();
    }
};

template<typename T>
struct To_String <T,
    std::enable_if_t< std::is_array<T>::value >
> {
    template<typename U,std::size_t N>
    static std::string to(const U(&arr)[N] ,STR_TO_VALUE_FUNC_MAP_TYPE & to_func) ;
};

template<typename T>
template<typename U,std::size_t N>
std::string
To_String <T,
    std::enable_if_t< std::is_array<T>::value >
>::to(const U(&arr)[N], STR_TO_VALUE_FUNC_MAP_TYPE &to_func){
    std::ostringstream oss;
    oss << "[";
    for(auto i = 0 ;i< N ; ++i){
        oss << To_String<U>::to(arr[i],to_func);
        if( i != N-1)
            oss << ",";
    }
    oss << "]";
#ifdef __SERIALIZABLE_H__
        to_func[GET_TYPE_NAME(T)] = [&to_func](void*field,const std::string&str)->void 
        {
            auto values=unpacking_list(str);                                     //字符串形式的值
            auto arr = reinterpret_cast<U*>(field);
            for (std::size_t i = 0; i < N ; ++i) {
                to_func[GET_TYPE_NAME(U)](&arr[i],values[i]);
            }
            //*reinterpret_cast<T*>(field) = std::move(object);
        };
#endif
    return oss.str();
}


} // end namespace cppjson

