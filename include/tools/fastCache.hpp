/**
 * 参考 https://github.com/active911/fastcache 的Cache容器,
 * 
 * target:
 *  1. 取代使用Redis,Redis还是比较适合较大型的项目
 *  2. 使用c++17
 *  
 * Features
 *
 *  1. 支持expire
 *  2. 使用分片,加快速度
 */

#pragma once
#include <thread>
#include <memory>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <chrono>
#include <cxxabi.h>

namespace rojcpp {


#define GET_TYPE_NAME(type) abi::__cxa_demangle(typeid(type).name(),0,0,0)

//工具类 测试T1类型是是否可以 append(T2) 类型
template<typename T1,typename T2>
struct has_append_member_function {
    template<typename U1,typename U2>
    static auto check(int) -> decltype(std::declval<U1>().append( std::declval<U2>() ),std::true_type());

    //sink hole function
    template<typename U1,typename U2>
    static std::false_type check(...);

    static constexpr bool value = decltype(check<T1,T2>(0))::value;
};

   
using std::literals::chrono_literals::operator ""s;

template<typename Key_t,typename Val_t ,std::size_t Shard_size = 255u>
class Fastcache {

public:

        using Duration_type = std::chrono::seconds;
        using Clock_type = std::chrono::steady_clock;
        using time_point_type = std::chrono::time_point<Clock_type,Duration_type>;

private:

    static inline time_point_type getNowSeconds() {
        return std::chrono::time_point_cast<Duration_type>(Clock_type::now());
    }

    struct CacheItem {
        Val_t       data;          //数据
        time_point_type expiration{}; //结束的时间 seconds

        CacheItem() = default;

        template<typename U>
        CacheItem(U && _d, Duration_type expired_duration_time){
            data = std::forward<U>(_d);
            expiration =std::chrono::time_point_cast<Duration_type>(Clock_type::now()) + expired_duration_time;
        }

        //追加
        //pre_split,是否在前面 添加一个 字符,固定为,
        void append(const Val_t & _data,Duration_type new_exp,bool use_pre_split_char = true) {
            static_assert( has_append_member_function<Val_t,const Val_t &>::value ,"CacheItem append fail");

            if( use_pre_split_char )
                data.append(",");
            data.append(_data);
            expiration = std::chrono::time_point_cast<Duration_type>(Clock_type::now()) + new_exp;
        }

        //转成字符串
        std::string dumps(){
            std::ostringstream oss;
            oss << expiration.time_since_epoch() << " ";
            if constexpr ( std::is_same_v<Val_t, std::string>)
                oss << data;
            else
                throw std::string("unsuport dumps type ") + GET_TYPE_NAME(Val_t);
            return oss.str();
        }

        //TODO 从字符串读取
        void loads(const std::string & _str);

        //是否过期
        bool expired(Clock_type::time_point now_time = Clock_type::now() ){
            return std::chrono::time_point_cast<Duration_type>(now_time) > expiration;
        }
    };


    //分片,一片数据
    struct Shard {
        std::mutex mtx;
        std::unordered_map<Key_t, CacheItem> _container;

        //@desc 删除过期的key
        void del_expired_keys(time_point_type now_time = getNowSeconds() ) {
            std::lock_guard<std::mutex> _lock(mtx);
            for( auto it = _container.begin() ; it != _container.end();) {
                if( it->second.expired(now_time) ){
                    //std::cout << "del expired key : " << it->second.data << std::endl;
                    it = _container.erase(it);
                }
                else
                    ++it;
            }
        }
    };

public:
    //ctor
    Fastcache() : m_shards{Shard_size}
    {
        //del_expired_thread
        expired_check_thread = std::thread(&Fastcache::expired_check_func,this);
    }

    ~Fastcache(){
        run_flag.store(false);
        if( expired_check_thread.joinable())
            expired_check_thread.join();
    }

    //@desc 得到所有key的数量
    std::size_t total_size();

    //@desc 设定一个值
    void set(const Key_t& key,Val_t&& val,Duration_type expiration = 10s);

    //@desc key是否存在
    bool exists(const Key_t& key);

    //@desc 删除一个key
    std::size_t del(const Key_t& key);

    //@desc 设定新的过期时间
    void new_expire(const Key_t& key,Duration_type dura = 10s);

    //@desc 得到一个key的值
    std::optional<Val_t > get(const Key_t& key); 

    //@desc 向一个key里追加值
    void append(const Key_t & key,const Val_t & _data,Duration_type expiration = 10s);

protected:
    //desc 检查每一个shard里过期的key
    void expired_check_func();

    //@desc 计算key对应的分片
    inline std::size_t calc_index(const Key_t& key);
private:
    std::hash<Key_t> m_hash_tool;
    std::vector<Shard> m_shards;
    std::thread expired_check_thread;
    std::atomic_bool run_flag{true}; // 运行标记
};

template<typename Key_t,typename Val_t ,std::size_t Shard_size>
std::size_t 
    Fastcache<Key_t,Val_t,Shard_size>::
    calc_index(const Key_t& key)
{
    return m_hash_tool(key) % Shard_size;
}

template<typename Key_t,typename Val_t ,std::size_t Shard_size>
std::size_t 
    Fastcache<Key_t,Val_t,Shard_size>::
    total_size()
{
    std::size_t tot{0};
    for (auto& e : m_shards) {
        std::lock_guard<std::mutex> lck(e.mtx);
        tot += e._container.size();
    }
    return tot;
}


template<typename Key_t,typename Val_t ,std::size_t Shard_size>
void Fastcache<Key_t,Val_t,Shard_size>::
    set(const Key_t & key,Val_t&& val,Duration_type expiration)
{
    std::size_t index = calc_index(key);
    auto & _shard = m_shards[index];
    std::lock_guard<std::mutex> lck(_shard.mtx);
    _shard._container[key] 
        = CacheItem(std::forward<Val_t>(val),expiration);
}

template<typename Key_t,typename Val_t ,std::size_t Shard_size>
bool
    Fastcache<Key_t,Val_t,Shard_size>::
    exists(const Key_t& key)
{
    std::size_t index = calc_index(key);
    auto & _shard = m_shards[index];
    std::lock_guard<std::mutex> lck(_shard.mtx);
    return _shard._container.find(key) != _shard._container.end();
}

template<typename Key_t,typename Val_t ,std::size_t Shard_size>
std::size_t
    Fastcache<Key_t,Val_t,Shard_size>::
    del(const Key_t& key)
{
    std::size_t index = calc_index(key);
    auto & _shard = m_shards[index];
    std::lock_guard<std::mutex> lck(_shard.mtx);
    return _shard._container.erase(key);
}

template<typename Key_t,typename Val_t ,std::size_t Shard_size>
void
    Fastcache<Key_t,Val_t,Shard_size>::
    new_expire(const Key_t& key,Duration_type expiration )
{
    std::size_t index = calc_index(key);
    auto & _shard = m_shards[index];
    std::lock_guard<std::mutex> lck(_shard.mtx);
    auto it = _shard._container.find(key);
    if( it != _shard._container.end()){
        it->second.expiration = getNowSeconds() + expiration;
    }
}

template<typename Key_t,typename Val_t ,std::size_t Shard_size>
std::optional<Val_t > 
    Fastcache<Key_t,Val_t,Shard_size>::
    get(const Key_t& key)
{
    std::size_t index = calc_index(key);
    auto & _shard = m_shards[index];
    std::lock_guard<std::mutex> lck(_shard.mtx);
    try {
        auto &val = _shard._container.at(key);
        //std::cout << val.data << " : expired at : " << val.expiration  << std::endl;
        //std::cout << "now :" << getNowSeconds() << std::endl;
        if( val.expired() ) {
            _shard._container.erase(key);
            // return std::make_tuple(false,Val_t{});
            return std::nullopt;
        }
        return val.data;
    }
    catch(std::out_of_range & oor){
        //对应的key已经不存在
        return std::nullopt;
    }
    // catch(std::exception & e){
    //     //std::cout << e.what() << std::endl;
    //     return std::nullopt;
    // }
}

template<typename Key_t,typename Val_t ,std::size_t Shard_size>
void
    Fastcache<Key_t,Val_t,Shard_size>::
    expired_check_func()
{
    using namespace std::chrono_literals;
    while( run_flag.load() == true) {
        std::this_thread::sleep_for(1s);

        auto now_time = getNowSeconds();
        for (auto& e : m_shards) {
            e.del_expired_keys(now_time);
        }
    }
}

template<typename Key_t,typename Val_t ,std::size_t Shard_size>
void
    Fastcache<Key_t,Val_t,Shard_size>::
    append(const Key_t& key,const Val_t & val,Duration_type expiration)
{
    // std::cout << "in append exists : " << exists(std::string(key)) << std::endl;
    std::size_t index = calc_index(key);
    auto & _shard = m_shards[index];
    std::lock_guard<std::mutex> lck(_shard.mtx);

    auto iter = _shard._container.find(key);
    // 没有找到,创建一个
    if ( iter == _shard._container.end() ) 
    {
        _shard._container[key] 
            = CacheItem(val,expiration);
    }
    else { //找到了,追加
        iter->second.append(val, getNowSeconds() + expiration);
    }
}

//单例模式的Cache
class Cache {
public:
    
    // del other ctor
    Cache(const Cache &) = delete;
    Cache(Cache &&) = delete;
    void operator=(const Cache& ) = delete;
    void operator=(Cache&& ) = delete;

    using cacheType = rojcpp::Fastcache<
        std::string,    //key type
        std::string,    //value type
        32>;            //shard size 分片数量

    static cacheType& get() {
        static cacheType catche;
        return catche;
    }

private:
    Cache() = default;
};


} // end namespace rojcpp



