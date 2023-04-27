
#pragma once

#include <list>
#include <bitset>
#include <vector>
#include <mutex>
#include <optional>
#include <memory>
#include <map>
#include "ReadWriteLock.hpp"
#include "judge_server/src/Client.hpp"
#include "server.h"
#include "__config.h"

/**
 * @desc 用来存评测结果的缓存容器
 * 自带有一个读写锁
 */
template<typename Key,typename Value>
class judgeCache
{
public:

    using ShareValue = std::shared_ptr<Value>;

private:
    RWLock _rwLock; //读写锁

    std::map<Key,ShareValue> _map;

public:

    bool has_key(const Key key) {
        _rwLock.lock_read();
        bool exits  =  _map.find(key) != _map.end();
        _rwLock.unlock_read();
        return exits;
    }

    ShareValue get(const Key key) {
        _rwLock.lock_read();
        auto v_it =_map.find(key);
        if( v_it != _map.end() ) {
            ShareValue ret_v = v_it->second;
            _rwLock.unlock_read();
            return ret_v;
        }
        _rwLock.lock_read();
        return nullptr;
    }

    void Delete(const Key key) {
        _rwLock.lock_write();
        auto v_it = _map.find(key);
        if( v_it != _map.end())
            _map.erase(v_it);
        _rwLock.unlock_write();
    }

    void set(const Key k,ShareValue val) {
        _rwLock.lock_write();
        _map[k] = val;
        _rwLock.unlock_write();
    }
};


//容器2: sid 与 多个Ser 对应,表示要通知对应的Serv
// 无锁容器
// 理论上最多只支持最多32个Ser注册
// cid == SerId
struct SidMapSer {
    using ctx_id = int;
    using Value = unsigned int;
    //设计: 每个sid 对应了一个32位的bit集合
    // Value的每一个位表示对应的Ser是否注册了
    std::map<std::size_t,Value> _map;

private:

    Value clear(int pos,Value v) {
        return ( v & ~(1<<pos) );
    }

    Value set(int pos,Value v) {
        return (v | (1 << pos) );
    }

public:
    

    //对应位置是否有cid这个服务器
    static
    inline bool has_sid(Value v, int cid) {
        return (v & (1<<cid));
    }


    //加入一个数据
    void push(std::size_t sid,ctx_id cid) {
        _map[sid] = set(_map[sid],cid) ;
    }

    //取出,一个sid对应的bitset集合
    Value get(std::size_t sid) const {
        auto v_it = _map.find(sid);
        if( v_it == _map.end())
            return 0;
        return v_it->second;
    }

    //清除
    void pop(std::size_t sid,ctx_id cid) {
        auto v_it = _map.find(sid);
        if( v_it != _map.end())
            _map[sid]= clear(cid, v_it->second);
    }
};


struct judgeMsg {};


/**
 * @desc 评测服务器中介者,功能
 * 1. 与评测服务器进行通信
 * 2. 分发信息给客户端
 *
 *  设计模型:
 *
 *  与judgeServer 进行连接
 *
 *
 *     Client1 <---------> Mediator <-----------------> JudgeServer
 *     Client2 <--------->
 *
 *
 */

//Ser,就是服务器
template<typename Ser>
class judgeServerMediator
{
public:
    static constexpr int MaxSer = 32; //最多能接受32个Ser注册
    using SerPtr = Ser *; //服务器指针
    std::atomic<bool> _abort{false};
    std::condition_variable t; //通知工作评测线程去评测

private:
    const int ctx_cnt; //连接ctx的数量
    SerPtr _ser[MaxSer];

    //1. 注册sid 对应哪个Ser 的容器
    SidMapSer _sidMapSer;
    
    //2. 存评测结果的缓存容器
    judgeCache<std::size_t,judgeMsg> _cache;

    //3: 评测客户端
    Client _client; //(std::size_t connect_size,std::string_view judge_server_ip,int port)

private:

    //通知Server
    void notify_ser(int ser_id,int sid);

    //得到sid对应的那SerId
    unsigned int get_sid_corr_sers(std::size_t sid);


public:
    //构造函数
    judgeServerMediator(int _ctx_cnt)
        : ctx_cnt(_ctx_cnt),

        _client (
                __config__::JUDGE_SERVER_CONNECT_SIZE,
                __config__::JUDGE_SERVER_IP,
                __config__::JUDGE_SERVER_PORT
                )
    {
        //清空
        for(int i = 0 ;i< MaxSer ;++i)
            _ser[i] = nullptr;
    }
    
    //使用 SerId 与 SerPtr 产生映射
    void Register(int SerId,SerPtr ser);

    //得到评测信息的时候,会通知sid对应的SerId
    void add_listen(int SerId,std::size_t sid);


    // 添加评测
    // void push_judge(std::size_t sid,JudgeInfo)
    // std::optional<Result> get_judge_result(std::size_t sid)

    // Result get_judge_result(std::size_t sid)
    // 是否对应sid的评测信息
    bool has_judge_result(std::size_t sid);

    //停止运行
    void stop() {
        _abort.store(true);
    }

private:

    //工作线程循环
    void work( ) {
        while(_abort.load() == false) {

            // 1. 读取评测队列中的值
            // 2. 发送评测
            // 3. 
        }
    }
};



judgeServerMediator<rojcpp::server> & JSM() ;
