#pragma once

#include "config.hpp"

/**
 * 核心作用是什么呢
 *
 * 提供一些static 方法
 *      get_config
 *      get_serializable_types
 *      get_field
 *      get_field_type
 *      get_field_offset
 *      get_field_names
 *      get_method_names
 *      get_method ??
 *      set_field
 *      delete_instance
 *      get_instance
 *  根据config提供的信息来操作对应的类
 */


namespace cppjson {

using ClassName  = std::string;
struct Reflectable
{
    public:
        virtual ~Reflectable(){};

        template<typename T> //注册
        static void Regist(){
            static_assert(has_config_member_function<T>::value,"There are some objects that use reflection but haven't implement public method Config get_config()const");
            T object;
            Reflectable::default_constructors[GET_TYPE_NAME(T)]=[](void)->void*          //默认构造函数 
            {
                return (void*)(new T());
            };
            Reflectable::default_deconstructors[GET_TYPE_NAME(T)]=[](void*object)->void  //析构函数 
            {
                delete ((T*)object);
            };
            auto config = object.get_config();  //注册时调用一下get_config
        }

        template<typename T>
        inline static config get_config(const T*object) {
            std::string class_name = GET_TYPE_NAME(T);
            //std::cout << "class_name " << class_name << std::endl;
            config conf(&field[class_name],object); // 产生了这个map
            //std::cout << "class_name " << class_name << std::endl;
            conf.update({{"class_name",class_name}}); //把类更新到map 里
            return conf;
        }

        inline static std::vector<std::string_view> get_serializable_types(); //得到一个存所有可以序列化的类型的名字的列表

        /**
         * @brief 根据类名,成员名,得到一对象指针指向的对象中的成员
         * @param object 对象指针
         * @param class_name 类名
         * @param field_name 成员名
         */
        template<typename FieldType=void*>
        inline static auto get_field(void*object,std::string class_name,std::string field_name){
            //std::cout << "get_field" << std::endl;
            try {
                //std::cout << class_name << std::endl;
                //std::cout << field_name<< std::endl;
                auto offset = field.at(class_name).at(field_name).second;
                std::size_t real_addr = (reinterpret_cast<std::size_t>(object) + offset );
                if( std::is_same_v<FieldType, void*>){
                    return reinterpret_cast<void*>(real_addr);
                }
                else 
                    return *(reinterpret_cast<FieldType*>(real_addr));

            }
            catch(std::exception & e){
                throw NoSuchFieldException(class_name,field_name);
            }
        }

        /**
         * @brief 根据类名,成员名,得到一对象指针指向的对象中的成员,只不过指定了类类型
         * @param object 对象指针
         * @param field_name 成员名
         */
        template<typename ClassType>
        inline static auto get_field(ClassType&object,std::string field_name){
            return get_field(&object, GET_TYPE_NAME(ClassType),field_name);
        }

        /**
         * @brief 得到类的成员 类型(字符串)
         */
        inline static std::string get_field_type(std::string class_name,std::string field_name){
            try {
                auto ret = field.at(class_name).at(field_name).first;
                return ret;
            }
            catch(std::exception & e){
                throw NoSuchFieldException(class_name, field_name);
            }
        }

        /**
         * @brief 得到类的成员 偏移
         */
        inline static std::size_t get_field_offset(std::string class_name,std::string field_name);

        /**
         * @brief 得到类的成员类型名列表
         */
        template<typename ClassType>
        inline static std::vector<std::string>get_field_names();

        //template<typename ClassType>
        //inline static std::vector<std::string>get_method_names();

        //template<typename ReturnType,typename ObjectType,typename...Args>
        //inline static auto get_method(ObjectType&object,const std::string&field_name,Args&&...args);

        //通过字符串访问成员函数,get_field<返回值类型>(对象,字段名,参数列表...);

        /**
         * @brief 设置属性值，因为已经有类型信息，所以不需要调用default_constructors[type]里面的函数来构造
         */
        template<typename FieldType,typename ClassType>
        inline static void set_field(ClassType&object,std::string field_name,const FieldType&data);

        inline static void set_field(void*object,std::string class_name,std::string field_name,const auto&value);

        /**
         * 删除一个对象实例
         */
        inline static void delete_instance(std::string class_name,void*object){
            default_deconstructors[class_name](object);
        }

        /**
         * @brief 创建一个类实例
         */
        inline static void*get_instance(std::string class_name){
            try {
                return Reflectable::default_constructors[class_name]();
            }
            catch(std::exception & e){
                throw NoSuchClassException(class_name);
            }
        }

    private:
        static std::unordered_map<ClassName,Field> field; //类名对应的 Filed
        static std::unordered_map<ClassName,std::function<void*(void)>> default_constructors; //构造函数
        static std::unordered_map<ClassName,std::function<void(void*)>> default_deconstructors;
};

std::unordered_map<ClassName,Field> Reflectable::field; //类名对应的 Filed
std::unordered_map<ClassName,std::function<void*(void)>> Reflectable::default_constructors; //构造函数
std::unordered_map<ClassName,std::function<void(void*)>> Reflectable::default_deconstructors;

//std::vector<std::string_view> Reflectable::get_serializable_types(){ //得到一个存所有可以序列化的类型的名字的列表
    ////std::vector<std::string_view> types;
    ////for(auto &it : configPair::from_config_string)
//}


}// end namespace cppjson
