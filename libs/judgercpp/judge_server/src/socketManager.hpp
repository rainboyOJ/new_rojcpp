/**
 * 对socket进行管理,
 * - 添加
 * - 删除
 * - 获取
 */
#pragma once
#include <atomic>
#include <vector>
#include <map>
#include <mutex>

class socketManager {
public:
    //单例模式
    static socketManager & Instance(){
        static socketManager sm;
        return sm;
    }
    void insert(int fd);       //插入
    bool get(int &fd);         //获得随机的socket
    bool get_specified_fd(int fd);  //获取指定的socket
    bool remove(int fd);       //删除
    bool unuse(int fd);        //设末使用
    void removeAll();
    auto size() const { return socket_map.size();}
private:
    socketManager() = default;
    std::map<int,bool> socket_map; // {fd,use?}
    std::mutex mtx;
};

class socketManagerRAII{
public:
    socketManagerRAII()
    { //随机获取
        while(socketManager::Instance().get(fd)  == false) ;
    }
    //指定获取
    explicit socketManagerRAII(int _fd)
    { 
        while( socketManager::Instance().get_specified_fd(_fd)  == false) ;
        fd = _fd;
    }
    ~socketManagerRAII(){ 
        socketManager::Instance().unuse(fd); 
    }
    int get() { return fd; }
private:
    int fd{-1};
};

void socketManager::insert(int fd){
    std::lock_guard<std::mutex> lock(mtx);
    socket_map.emplace(fd,false);
}

bool socketManager::get(int &fd){
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& e : socket_map) {
        if( e.second == false){
            e.second = true;
            fd = e.first;
            return true;
        }
    }
    return false;
}

bool socketManager::remove(int fd){
    std::lock_guard<std::mutex> lock(mtx);
    auto it = socket_map.find(fd);
    if( it == socket_map.end() )
        return false;       //dont find
    if( it->second == true)
        return false;        //using
    else{
        socket_map.erase(it);
        return true;
    }
    return false;
}

bool socketManager::unuse(int fd){
    std::lock_guard<std::mutex> lock(mtx);
    auto it = socket_map.find(fd);
    if( it == socket_map.end())
        return true;       //dont find
    it->second = false; //一定把这个设为 false
    return true;       //using
}

void socketManager::removeAll(){
    std::lock_guard<std::mutex> lock(mtx);
    for( auto it = socket_map.begin() ;it != socket_map.end() ;){
        it = socket_map.erase(it);
    }
}

bool socketManager::get_specified_fd(int fd){
    std::lock_guard<std::mutex> lock(mtx);
    auto it = socket_map.find(fd);
    if( it == socket_map.end() || it->second == true)
        return false;       //dont find
    else{
        it->second = true; //设为正在使用
        return true;    //获取成功
    }
}

