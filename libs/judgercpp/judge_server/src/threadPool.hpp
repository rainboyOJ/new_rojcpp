/**
 * @线程池
 */
#pragma once
#ifndef __MYTHREAD_POOL_HPP
#define __MYTHREAD_POOL_HPP
#include <queue>
#include <iostream>

#include <vector>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <functional>
#include <stdexcept>
#include <algorithm>


namespace THREAD_POOL {

const int THREADPOOL_MAX_NUM = 16; //线程池最大容量

using std::function;
using std::vector;
using std::queue;
using std::mutex;
using std::atomic;
using std::thread;
using std::condition_variable;
using std::unique_lock;
using std::lock_guard;
using std::runtime_error;
using std::forward;


//不直接支持类成员函数, 支持类静态成员函数或全局函数,operator(),lambda函数等
class threadpool {
    using TaskType = function<void()>; //任务的类型
    vector<thread>     _pool;       //线程池
    queue<TaskType>    _tasks;     //任务队列
    condition_variable _task_cv;//条件阻塞
    mutex              _lock;
    atomic<bool>       _run{true};    //线程池是否执行
    atomic<int>        _idlThrNum{0}; //空闲线程数量

public:
    //不允许 拷贝构造与赋值构造
    threadpool(const thread &)            = delete;
    threadpool& operator=(const thread &) = delete;

    explicit threadpool(unsigned int size = 
            std::max( std::thread::hardware_concurrency(), 1u)
            ) { addThread(size);}
    ~threadpool(){
        //std::cout << "dcotr" << std::endl;
        _run = false;
        _task_cv.notify_all(); // 唤醒所有线程执行
        //等待所有的线程执行完
        for (auto& th : _pool) {
            if( th.joinable() ){
                th.join();
            }
        }
    }

public:
    // 提交一个任务,不能获取任务的结果
    template<typename  F,typename... Args>
    auto commit(F&& f,Args&&... args)-> void //decltype(f(args...))
    {
        if( !_run ) throw runtime_error("ThreadPool was stopped!");
        
        auto task = std::make_shared<TaskType>( std::bind( forward<F>(f),forward<Args>(args)...) );
        //[ff=forward<F>(f),... args = std::forward<Args>(args)](){ ff(args...); } //这个只有c++20支持
        { //添加任务到队列
            lock_guard<mutex> lck{_lock};
            _tasks.emplace( [task](){ (*task)(); } );
        }
        _task_cv.notify_one();
        //std::cout << "ok" << std::endl;
    }

    unsigned int idlCount() const { return _idlThrNum;}     //空闲线程数量
    unsigned int thrCount() const { return  _pool.size();} //线程池大小

private:
    //线程池大小增加
    void addThread(unsigned int size){
        for(; _pool.size() < THREADPOOL_MAX_NUM && size >0;--size){
            auto f = [this](){
                while ( _run ) { // 可以运行,获取一个任务
                    TaskType task;
                    {
                        unique_lock<mutex> lck{_lock};
                        //wait 当第二参数返回false时，休眠当前线程，释放锁(unlock)
                        //     当第二参数返回true时，获取锁(lock)
                        //<==> while(!pred()) wait(lck);
                        _task_cv.wait(lck,[this]{ return !_run or !_tasks.empty(); });
                        // 线程池不执行了 且 队列为空
                        if( !_run && _tasks.empty() ) return;
                        task = _tasks.front();
                        _tasks.pop();
                    } //锁释放
                    _idlThrNum.fetch_sub(1);
                    task(); //执行任务
                    _idlThrNum.fetch_add(1);
                }
            };
            _pool.emplace_back(std::move(f));
            _idlThrNum.fetch_add(1);
        }
    }

};

}; // end namespace threadpool

#endif

