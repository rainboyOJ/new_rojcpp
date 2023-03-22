#include <iostream>
#include "tinyasync/tinyasync.h"


using namespace tinyasync;

Task<int> mygenerator(){
    std::cout << "run int line:" << __LINE__ << " in mygenerator" << "\n";
    co_await std::suspend_always{};

    //下面这一行到达不了
    std::cout << "run int line:" << __LINE__ << " in mygenerator" << "\n";
    co_return 42;
}


int main(){
    //mygenerator启动协程,但是协程在init_suspend是挂起
    //co_await mygenerator() 启动 mygenerator;
    tinyasync::co_spawn(mygenerator());
    std::cout << "at main return" << "\n";
    return 0;
}
