#include <iostream>
#include <tinyasync/memory_pool.h>

using namespace tinyasync;


int main(){
    //定义一个内存池 
    PoolResource MyReseouce;

    vector_buffer<char> myvec(100,&MyReseouce);
    std::cout << "tot_size = "
        << myvec.get_tot_size() << "\n";
    auto t  = myvec.get_raw_buff();
    for(int i = 1;i<=10;i++  ){
        t[i] = 1;
    }

    myvec.expand();
    std::cout << "tot_size = "
        << myvec.get_tot_size() << "\n";

    t  = myvec.get_raw_buff();
    for(int i = 1;i<=10;i++  ){
        std::cout << i << " " << t[i]  << "\n";
    }

    return 0;
}
