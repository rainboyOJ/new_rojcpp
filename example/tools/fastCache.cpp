// tools/fastCache.hpp 的测试代码
#include <iostream>
#include <thread>
#include <iomanip>
#include "tools/fastCache.hpp"

using namespace std::literals::chrono_literals;

template<typename T>
void log_one(T && a) {
}

void log_one(bool a) {
    std::cout << std::boolalpha << a << "\n";
}

void delay_one_seconds(){
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

#define log(one) std::cout << #one "= "; log_one(one);


int main(){
    //创建一个fastCache


    auto & myCache = rojcpp::Cache::get();

    const std::string key1 = "key1";
    bool exists_key1 = myCache.exists(key1);
    log(exists_key1);

    // 创建key
    myCache.set(key1, "hello", 3s);
    exists_key1 = myCache.exists(key1);
    log(exists_key1);
    
    //得到key对应的值
    auto val = myCache.get(key1);
    if( val.has_value() )
        std::cout << "key1 -> " << val.value() << "\n";
    else
        std::cout << " key1 not exists\n";


    for(int i=1;i<=4;++i){
        delay_one_seconds();
        std::cout << "delay " << i << " seconds" << "\n";
    }

    //检查 key1 不存在
    exists_key1 = myCache.exists(key1);
    log(exists_key1);

    auto val1 = myCache.get(key1);
    if( val1.has_value() )
        std::cout << "key1 -> " << val1.value() << "\n";
    else
        std::cout << " key1 not exists\n";
    return 0;
}
