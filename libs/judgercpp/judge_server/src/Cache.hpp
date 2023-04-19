/**
 * @desc 缓存
 * 实现添加与删除
 * 过期检查
 */

#include <vector>
#include <memory>
#include <map>
#include <atomic>
#include <ctime>

#include "define.hpp"
#include "Result.hpp"


/**
 * 结果的集合
 */
struct judgeResult {
    explicit judgeResult(std::time_t expire)
    {
        time_stamp_ = std::time(nullptr) + expire;
    }

    //压入评测的结果
    void push_back( int cpu_time,  int real_time, long memory, int signal,
                    int exit_code, int error, int result) {
        std::lock_guard<std::mutex> lock(mtx_);
        res.push_back( cpu_time,  real_time, memory,  signal,  exit_code,  error,  result);
    }

    //设定执行的结果和消息
    void set_code_and_msg(judgeResult_id _code,std::string_view _msg){
        std::lock_guard<std::mutex> lock(mtx_);
        res.set_code(_code);
        res.set_msg(_msg);
    }

    std::time_t time_stamp_; //过期时间
    std::mutex mtx_;         //锁
    bool end{false};        //是否评测结束
    MessageResultJudge res;
};


struct Cache {
    Cache& get();       //单例模式
    void check_expire();
    std::shared_ptr<judgeResult> create_Results(std::string_view id); //创建一个结果集合
    std::weak_ptr<judgeResult> get_Results(); //得到一个结果集
    void remove(std::string_view id); //删除一个
private:
    Cache() = default;
    Cache(const Cache &) = delete;
    Cache(Cache &&)      = delete;

    std::map<std::string,std::shared_ptr<judgeResult>> map_;
    std::mutex mtx_;            //锁
    int max_age_ = 60*60; //最多60分钟
    std::atomic_int64_t id_ = 0;
};

std::shared_ptr<judgeResult> 
Cache::create_Results(std::string_view id) //创建一个结果集合
{
    auto s = std::make_shared<judgeResult>(max_age_);
    {
        std::lock_guard<std::mutex> lock(mtx_);
        map_.emplace(std::string(id),s);
    }
    return s;
}

std::weak_ptr<judgeResult>
Cache::get_Results(std::string id) //得到一个结果集
{
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = map_.find(std::string(id));
    return it != map_.end() ? it->second : nullptr;
}

void Cache::remove(std::string_view id) //删除一个
{
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = map_.find(std::string(id));
    if( it != map_.end())
        map_.erase(it);
}

void Cache::check_expire(){
    if(map_.empty()) return;
    auto now = std::time(nullptr);
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto it = map_.begin(); it != map_.end();) {
        if (now - it->second->time_stamp()>= 0) {
            it = map_.erase(it);
        }
        else {
            ++it;
        }
    }
}
