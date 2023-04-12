#pragma once

#include <numeric>
#include <array>
#include <string>
#include <chrono>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <vector>

#include "pool_manager.hpp"
#include "cexpr/string.hpp"
#include "sql/schema.hpp"

#include "errors.hpp"
#include "utils.hpp"



namespace cppdb
{

struct __exec {};
const constexpr auto exec = __exec();

//std::intern

//using fenc_grave        = std::integral_constant<char, '`'>;
//using fenc_single_quote = std::integral_constant<char, '\''>;
//using fenc_quote        = std::integral_constant<char, '"'>;


enum fenc {
    fenc_grave        =  '`',
    fenc_single_quote =  '\'',
    fenc_quote        =  '"'
};

template<std::size_t... As,std::size_t... Bs>
constexpr auto operator+(std::index_sequence<As...>,std::index_sequence<Bs...>) 
    -> std::index_sequence<As...,Bs...>
{ return {}; }

template<cexpr::string str>
struct GET_STR_QUESTION_MARK_SEQ {
    static constexpr auto S {str};
    static constexpr std::size_t size__ = S.size();

    static constexpr auto get()
    {
        return  filter(std::make_index_sequence<size__>{});
    }

    template<std::size_t... Is>
    static constexpr auto filter(std::index_sequence<Is...>){
        return (filter_single<Is>()  + ...);
    }

    template <std::size_t Val>
    static constexpr auto filter_single() {
        if constexpr ( S.get(Val) == '?')
            return std::index_sequence<Val> {};
        else
            return std::index_sequence<> {};
    }
    using index_seq = decltype(get());

};

template<typename T>
static constexpr std::size_t get_mark_size(T & s){
    int cnt{0};
    auto b = s.cbegin();
    auto e = s.cend();
    for( ; b!= e; b++)
        if(*b == '?') cnt++;
    return cnt;
}


/// each 工具函数
//
template<typename Row,typename Result,std::size_t... idx>
constexpr void my_set_each_row_column(Row& r,Result res,
        std::index_sequence<idx...>) 
{
    bool succ;
    using new_Row = std::remove_cvref_t<Row>;
    (( cppdb::set<idx>(r, res-> template fetch< typename std::tuple_element<idx, new_Row>::type >(idx,succ) )),...);
}

template<typename Func,std::size_t... Is,std::size_t... Bs>
constexpr auto __each_seq(
        Func&& f,
        std::index_sequence<Is...>&& a,
        std::index_sequence<Bs...>&& b
        ) {
    static_assert(a.size() == b.size(),"two seq size not equ");
    (std::forward<Func>(f)(Is,Bs),...);
}


// sql 支持的类型
template <typename T,typename TYPE = std::remove_cvref_t<T> >
concept SQL_SUPPORT_TYPE = 
    std::is_fundamental_v<TYPE> || 
    std::is_same_v<TYPE, char *> ||
    std::is_same_v<TYPE, TIME_Pt> ||
    std::is_same_v<TYPE, std::string_view> ||
    std::convertible_to<TYPE, std::string>;

/**
 * schema 的类型有三种
 * int,std::string,单个类型
 * row类型
 * schema
 */
template <cexpr::string Str, typename Schema>
class query {
public:
    using binders_seq = typename GET_STR_QUESTION_MARK_SEQ<Str>::index_seq;

    static constexpr auto query_str_ {Str};
    static constexpr std::size_t mark_size_ {get_mark_size(Str)};

    query()
    {
        //TODO 去获得 连接
        conn_ = pool_manager::get();
    }

    auto params_size() const { return mark_size_; }

    template<typename Input>
    query& operator<<(Input && in) {
        bind(std::forward<Input>(in));
        if( use_fenc ) {
            at(cols-1) = fenc_char + at(cols) + fenc_char;
            use_fenc = 0;
        }

        return *this;
    }

    void reset(){
        cols = 0;
        for (auto& e : params_) e.clear();
    }

    query& operator<<( fenc __f) {
        use_fenc = 1;
        fenc_char = static_cast<char>(__f);
    }

    auto operator<<(__exec const &) {
        //TODO 执行 得到 backend::result 继续填充schema
        return this->exec();
    }

    std::string & at(int col) {
        if(col < mark_size_)
            return params_[col];
        else
            throw cppdb_error( "overload cols size :" + std::to_string(mark_size_));
    }



    template<typename Input>
    requires SQL_SUPPORT_TYPE<Input>
    void bind(Input&& in) {
        //std::cout << in << std::endl;
        //std::cout << "col :" << cols << std::endl;
        //using TYPE = typename std::remove_const_t< std::remove_cv_t<Input> >;
        using TYPE = std::remove_cvref_t<Input>;

        if constexpr ( std::is_fundamental_v<TYPE> && (! std::is_same_v<TYPE, char *>)  ) // # 基础的类型
        {
            at(cols) = std::to_string(in);
            ++cols;
            return;
        }
        
        if constexpr ( std::is_same_v<std::string_view, TYPE>) {
            at(cols) = std::string(in);
            ++cols;
            return;
        }
        if constexpr ( std::is_same_v<std::string, TYPE>) {
            at(cols) = in;
            ++cols;
            return;
        }
        
        if constexpr ( std::is_same_v<TYPE,TIME_Pt >)   // # 时间
        {
            std::string &s = at(cols);
            s.clear();
            //s.reserve(30);
            //s+='\'';
            s = cppdb::format_time(in);
            //s+='\'';
            ++cols;
            return;
        }

        if constexpr (std::is_same_v<TYPE, char *>
            || (std::is_array_v<TYPE> && std::is_same_v< std::remove_extent_t<TYPE> , char >)
                ) {
        //if constexpr ( std::convertible_to<TYPE, std::string>) {
            //std::vector<char> buf(2*(e-b)+1);
            //size_t len = mysql_real_escape_string(conn_,&buf.front(),b,e-b);
            std::string &s=at(cols);
            //s.reserve(e-b+2);
            //s+='\'';
            s = std::string(in); // C++ 20 
            //s+='\'';
            cols++;
            return;
        }
        //if constexpr (std::is_same_v<TYPE, std::string> || std::is_same_v<TYPE, std::string_view>) {
            //return;
        //}


#ifdef MODERN_CPPDB_DEBUG
        std::cout << GET_TYPE_NAME(TYPE) << std::endl;
        std::cout << "same v" << std::is_same_v<TYPE,char [8]> << std::endl;
        std::cout << "is_array_v " << std::is_array_v<TYPE> << std::endl;
        std::cout << "is_array_v " << std::is_array_v<Input> << std::endl;
        std::cout << std::convertible_to<TYPE,std::string> << std::endl;
#endif
        throw cppdb_error(__FILE__,__LINE__,std::string("Do not support type: ") + GET_TYPE_NAME(Input));
    }

    inline std::string get_query() {
        if( cols != mark_size_)
            throw cppdb_error("must insert full value_ use << before exec ");

        // 遍历 binders_seq
        int pre_index = 0;
        std::string last_query_{};
        if constexpr(mark_size_ != 0) {
        __each_seq([this,&pre_index,&last_query_](std::size_t now_index,std::size_t col){
                    //std::cout << pre_index << " " << now_index << std::endl;
                    last_query_.append(query_str_.cbegin() + pre_index,now_index-pre_index);
                    last_query_.append(params_[col]);
                    pre_index = now_index+1;
                }, 
                binders_seq{},
                std::make_index_sequence<binders_seq::size()> {}
                );
        }
        //最后一个mark 
        if( pre_index < query_str_.size())
            last_query_.append(query_str_.cbegin() + pre_index , query_str_.size() - pre_index);
#ifdef MODERN_CPPDB_DEBUG
        cppdb_log("==>finish last query : ",last_query_);
#endif
        return last_query_;
    }

    //不需要结果,只需要执行就可以了
    Schema exec() requires 
        std::is_same_v<Schema,void>
    {
        auto last_query_ = get_query();
        conn_->onlyExec(last_query_);
    }

    //需要返回的类型是一个数字或字符串,或时间 这种单个的例子
    Schema exec() requires 
        std::numeric_limits<Schema>::is_integer || 
        std::is_pointer_v<Schema> ||
        std::is_same_v<Schema, std::string> ||
        std::is_same_v<Schema, TIME_Pt>
    {
        auto Result_set = get_raw_result();
        if( Result_set->has_next()  != backend::result::next_row_exists)
            //throw cppdb::cppdb_error(__FILE__,__LINE__,"not get value");
            throw cppdb::cppdb_zero_row(__FILE__,__LINE__);
        Result_set->next();
        bool succ;
        return Result_set -> template fetch<Schema>(0,succ);
    }

    Schema exec() 
        requires cppdb::is_row_type<Schema>::value
    {
        auto Result_set = get_raw_result();
        if( Result_set->has_next()  != backend::result::next_row_exists)
            throw cppdb::cppdb_error(__FILE__,__LINE__,"not get value");
        Schema myrow;
        Result_set->next();
        my_set_each_row_column(myrow, Result_set.get(), 
                std::make_index_sequence<Schema::depth> {}
                );
        return myrow;
    }

    Schema exec()
        requires cppdb::is_schema_type<Schema>::value
    {
        auto Result_set = get_raw_result();
        Schema real_result;

        //执行并填充结果
        while ( Result_set->has_next()  == backend::result::next_row_exists) 
        {
            Result_set->next();
            typename Schema::row_type row; // 定义一个新的row
            my_set_each_row_column(row, Result_set.get(), std::make_index_sequence<Schema::row_type::depth>{} );
            real_result.emplace(std::move(row));
        } 

        return real_result;
    }

private:

    inline auto get_raw_result() -> std::shared_ptr<backend::result>
    {
        auto last_query_ = get_query();
        return conn_->exec(last_query_);
    }
    //template<std::size_t idx,typename ValueType,typename Result>
    //auto fetch(Result res_conn) -> ValueType{
        //return res_conn->fetch<ValueType>(idx);
    //}


    //mutable std::vector<std::string> params_;
    //std::array<std::string, mark_size_> params_;
    std::string params_[mark_size_];
    int cols{0}; //当前存的数据的行数
    bool use_fenc{false};
    char fenc_char;

    std::shared_ptr<pool::connection_raii> conn_{nullptr};

};



} // namespace cppdb
