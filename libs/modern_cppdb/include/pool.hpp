//@desc 连接池
#pragma once

#include <memory>
#include <mutex>
#include <list>
#include <chrono>

#include "connection_info.hpp"
#include "backend/mysql_backend.hpp"
#include "backend/result_backend.hpp"

namespace cppdb {


//==========================
//==========================
//==========================

class pool : public std::enable_shared_from_this<pool>{

    using T = cppdb::backend::connection;

private:

    pool() = delete;
    pool(pool const &) = delete;
    void operator=(pool const &) = delete;

public:

    /**
     * 从pool中获取的连接类型,
     * 当生命周期结束时可以自动的释放 mysql conn 回到 pool 里
     */
    struct connection_raii {
        connection_raii(std::shared_ptr<pool> pool_,std::shared_ptr<T> conn_) 
            : pool_{pool_} ,conn_{conn_}
        {}

        ~ connection_raii(){
            if(!pool_.expired()){
                if(conn_ != nullptr){
                    if( auto pool_ptr = pool_.lock(); pool_ptr) {
                        conn_->update_last_used();
                        pool_ptr->put(conn_);
                    }
                }
            }
        }

        inline auto get_last_used() const{ 
            if( conn_ == 0)
                throw cppdb::cppdb_error("conn_ is nullptr!");
            return  conn_ -> get_last_used(); 
        }

        auto conn() { return conn_; }
        auto escape(std::string_view str) { return conn_->escape(str);}

        std::shared_ptr<backend::result> exec(std::string_view cmd) {
            conn_ -> exec(cmd);
            return std::make_shared<backend::result>(conn_.get());
        }

        void onlyExec(std::string_view cmd) {
            conn_ -> exec(cmd); // ref https://dev.mysql.com/doc/refman/8.0/en/commands-out-of-sync.html
            // 不可以只执行qeury,但不store结果
            auto res_ = mysql_store_result(conn_.get()->get_raw_conn());
            if(res_)
                mysql_free_result(res_);
        }

        std::weak_ptr<pool> pool_;
        std::shared_ptr<T> conn_ = nullptr;

    };

    pool(connection_info const &ci)
        :limit_{0},life_time_{0},size_{0},ci_{ci}
    {
        limit_     = ci_.get("pool_size",16);
        life_time_ = std::chrono::seconds( ci_.get("pool_max_idle",600));
    }
    using pointer = std::shared_ptr<pool>;

    static std::shared_ptr<pool> create(connection_info const & ci){
        return std::make_shared<pool>(ci);
    }

    static std::shared_ptr<pool> create(const std::string& connection_string) {
        connection_info ci(connection_string);
        return std::make_shared<pool>(ci); }

    ~pool() {}

    //@desc 打开一个连接, 要么从池里拿出一个 或 创建一个新
    std::shared_ptr<connection_raii> open()
    {
        cppdb_log("pool -> open() ");
        if(limit_ == 0){
            auto conn_ = std::make_shared<T>(ci_);
            return std::make_shared<connection_raii>(
                    this->shared_from_this(),
                    conn_);
        }
        auto p = get();
        if( !p ) {
            cppdb_log("pool : not get exits connection,creat new one");
            auto new_ = std::make_shared<T>(ci_);
            return std::make_shared<connection_raii>(
                    this->shared_from_this(),
                    new_
                    );
        }
        return p;
    }

    //@desc 收回长时间不用的连接
    void gc(){
        put(std::shared_ptr<T>(nullptr));
    }
    //清空所有的连接
    void clear()
    {
        std::lock_guard<std::mutex> lck(mtx_);
        pool_.erase(pool_.begin(), pool_.end());
        size_ = 0;
    }

    //放回
    void put(std::shared_ptr<T> c_in) 
    {
        cppdb_log("pool -> put()");
        cppdb_log("life_time_ :",life_time_.count());
        if( limit_ == 0 ) return;

        {
            std::lock_guard<std::mutex> lck(mtx_);
            typename pool_type::iterator p = pool_.begin(),tmp;
            auto now = std::chrono::system_clock::now();
            // ___ old ........... new_____
            // ___ small ...........big____ 
            while ( p != pool_.end() ) {
                if( (*p)->get_last_used() + life_time_ < now) {
                    tmp = p;
                    ++p;
                    pool_.erase(tmp);
                    //cppdb_log("del one expired connection");
                    size_--;
                }
                else break; //后面的都比较新
            }
            //cppdb_log("del expired connection end");
            if( c_in ) {
                pool_.push_back( c_in);
                size_++;
            }

            while ( size_ > limit_ ) { //删除多余connection
                pool_.pop_front();
                size_--;
            }
        }

        cppdb_log("pool -> put() end;");
    }

    //得到可用连接的数量, 不是线程安全的
    auto size() const { return size_; }

private:
    std::shared_ptr<connection_raii> get() {
        if( limit_ == 0 ) 
            return std::shared_ptr<connection_raii>(nullptr);
        auto now = std::chrono::system_clock::now();
        {
            std::lock_guard<std::mutex> lck(mtx_);
            typename pool_type::iterator p = pool_.begin(),tmp;
            //pool_.remove_if([this,now](entry & e){ return e.last_used + limit_ < now; } );
            // _old_____new_
            while ( p != pool_.end() ) {
                if( (*p)->get_last_used() + life_time_ < now) {
                    tmp = p;
                    ++p;
                    pool_.erase(tmp);
                    size_--;
                }
                else break;
            }
            if( !pool_.empty() ) {
                std::shared_ptr<T> c = pool_.front();
                pool_.pop_front();
                size_--;
                return std::make_shared<connection_raii>( this->shared_from_this(), c);
            }
        }
        return std::shared_ptr<connection_raii>(nullptr); //没有找到
    }

    std::size_t limit_; // 上限
    std::size_t size_;  // 当前connection的数量
    std::chrono::seconds life_time_; //生命周期
    connection_info ci_;
    std::mutex mtx_;

    using pool_type = std::list<std::shared_ptr<T> >;
    pool_type pool_;
};

//==========================
//==========================
//==========================


} // end namespace cppdb

