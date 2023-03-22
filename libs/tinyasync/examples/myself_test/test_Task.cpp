#include <iostream>
#include "tinyasync/tinyasync.h"


using namespace tinyasync;

Task<int> mygenerator(){
    std::cout << "run int line:" << __LINE__ << " in mygenerator" << "\n";
    co_return 42;
}


int main(){
    //启动协程,但是协程在init_suspend是挂起
    auto coro = mygenerator(); 

    //resume协程
    auto resume_ret = coro.resume();
    std::cout << "after resume, coro is done = " << resume_ret << "\n";
    std::cout << "result = " << coro.result() << "\n";

    //协程到达 final_suspend
    // std::cout << "coro.done() = "
    //     << coro.promise.done()
    //     << "\n";
    return 0;
}
