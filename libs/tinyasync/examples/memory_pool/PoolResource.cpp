// code by Rainboy at 2022-11-24 17:07
// 用来理解 PoolImpl 内部的原理
#include <iostream>
#include <bitset>
#include <tinyasync/memory_pool.h>
#include <random>
#include <chrono>
#include <set>
#include <thread>
#include <algorithm>


using namespace tinyasync;


int main(){
    //定义一个 PoolResource;
    tinyasync::PoolResource myPool;

    //申请内存

    // auto mem1 = myPool.do_allocate(10*sizeof(int),alignof(int));
    // myPool.do_deallocate(mem1,10*sizeof(int),alignof(int));

    //申请内存
    const int size1 = 32768; // 32kb;
    const int size2 = tinyasync::PoolImpl::block_size(
            tinyasync::PoolImpl::k_malloc_order
            ) - 40; // 32kb;
    auto mem1 = myPool.do_allocate(size2,alignof(int));
    myPool.do_deallocate(mem1,size2,alignof(int));

    auto mem2 = myPool.do_allocate(size1,alignof(int));
    myPool.do_deallocate(mem2,size1,alignof(int));



    return 0;
}
