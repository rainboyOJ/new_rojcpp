#pragma once

#include <numeric>
#include <chrono>
#include <cstdint>
#include <sstream>
#include <mysql/mysql.h>

#include "mysql_backend.hpp"

namespace cppdb {
namespace backend {

/**
 * @desc 对结果进行封装
 */
class result {
public:
    typedef enum {
        last_row_reached, ///< No more rows exits, next() would return false
        next_row_exists,  ///< There are more rows, next() would return true
        next_row_unknown  ///< It is unknown, next() may return either true or false
    } next_row;

    result() = delete;
    result(MYSQL *conn);
    result(connection & mysql_conn_);
    result(connection * mysql_conn_);
    ~result();

    //下一行是否存在
    next_row has_next();
    //移到下行的数据
    bool next();

    //得到 col 列的数据
    char const * at(int col);
    //得到 col 列的数据,并得到这列的数据长度
    char const *at(int col,int &len);

    //从col列得到一个数据
    template<typename T>
    requires std::numeric_limits<T>::is_integer || std::is_pointer_v<T>
    auto fetch(int col,bool & succ) -> T {
        std::stringstream ss(at(col));
        T t{};
        if( ss >> t ){
            succ = true;
            return t;
        }
        succ = false;
        return T{};
    }

    template<typename T>
    requires std::is_same_v<T, std::string>
    auto fetch(int col,bool & succ) -> T {
        //字符串
        int len;
        char const *s = at(col,len);
        if(!s) {
            succ = false;
            return T{};
        }
        succ = true;
        std::string ret(s,len);
        return ret;
    }

    template<typename T>
    requires std::is_same_v<T, TIME_Pt>
    auto fetch(int col,bool & succ) -> T {
        //时间
        char const *s = at(col);
        if(!s) {
            succ=false;
            return T{};
        }
        std::tm tm_ = parse_time(s);
        auto tim_ = std::chrono::system_clock::from_time_t(std::mktime(&tm_));
        succ = true;
        return tim_;
    }

    // col列是否是null
    bool is_null(int col);

    // 结果集里的列数
    int cols() const;

    //通过名字得到对应的列数
    int name_to_column(std::string_view name);
    //通过列数 得到对应的名字
    std::string column_to_name(int col);

private:
    int cols_;
    int current_row_;
    MYSQL_RES *res_;
    MYSQL_ROW row_;
};


result::result(MYSQL *conn) : res_(0), cols_(0), current_row_(0), row_(0)
{
    res_ = mysql_store_result(conn);
    if(!res_) {
        cols_ = mysql_field_count(conn);
        if(cols_ == 0)
            throw cppdb_zero_filed(__FILE__,__LINE__);
    }
    else {
        cols_ = mysql_num_fields(res_);
    }
}

result::result(connection & mysql_conn_)
    :result(mysql_conn_.get_raw_conn())
{}

result::result(connection * mysql_conn_)
    :result(mysql_conn_->get_raw_conn())
{}

result::~result() {
    if(res_)
        mysql_free_result(res_);
}

result::next_row
    result::has_next()
{
    if( !res_ )
        return last_row_reached;
    if(current_row_ >= mysql_num_rows(res_))
        return last_row_reached;
    else
        return next_row_exists;
}

bool result::next()
{
    if(!res_) 
        return false;
    ++current_row_;
    row_ = mysql_fetch_row(res_);
    if(!row_)
        return false;
    return true;
}

char const * result::at(int col)
{
    if(!res_)
        throw empty_row_access();
    if(col < 0 || col >= cols_)
        throw invalid_column();
    return row_[col];
}

char const * result::at(int col,int &len)
{
    if(!res_)
        throw empty_row_access();
    if(col < 0 || col >= cols_)
        throw invalid_column();
    unsigned long *lengths = mysql_fetch_lengths(res_);
    if(lengths==NULL) 
        throw cppdb_error("Can't get length of column");
    len = lengths[col];
    return row_[col];
}


//template<typename T>
//auto result::fetch(int col,bool & succ) -> T
//{
    ////数字
    //if constexpr( std::is_integral_v<T> || std::is_floating_point_v<T>  ){
    //}

    ////字符串
    //if constexpr( std::is_same_v<T, std::string> ){
        //int len;
        //char const *s = at(col,len);
        //if(!s) {
            //succ = false;
            //return T{};
        //}
        //succ = true;
        //std::string ret(s,len);
        //return ret;
    //}

    ////时间
    //if constexpr (std::is_same_v<T, TIME_Pt >){
        //char const *s = at(col);
        //if(!s) {
            //succ=false;
            //return T{};
        //}
        //std::tm tm_ = parse_time(s);
        //auto tim_ = std::chrono::system_clock::from_time_t(std::mktime(&tm_));
        //succ = true;
        //return tim_;
    //}

    //throw cppdb_error(std::string("unsupport type : ") + GET_TYPE_NAME(T) + ",at col : " + std::to_string(col));
//}

int
result::cols() const
{
    return cols_;
}

std::string 
result::column_to_name(int col) 
{
    if(col < 0 || col >=cols_)
        throw invalid_column();
    if(!res_)
        throw empty_row_access();
    MYSQL_FIELD *flds=mysql_fetch_fields(res_);
    if(!flds) {
        throw cppdb_error("Internal error empty fileds");
    }
    return flds[col].name;
}

int 
result::name_to_column(std::string_view name) 
{
    if(!res_)
        throw empty_row_access();
    MYSQL_FIELD *flds=mysql_fetch_fields(res_);
    if(!flds) {
        throw cppdb_error("Internal error empty fileds");
    }
    for(int i=0;i<cols_;i++)
        if(name == flds[i].name)
            return i;
    return -1;
}

} // end namespace backend
} // end namespace cppdb
