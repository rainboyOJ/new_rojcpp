#include <iostream>
#include "tinyasync/time_queue.h"
#include "tinyasync/tinyasync.h"


//此代码证明
// from_node的正确性
int main(){
    std::cout << "hello" << "\n";

    tinyasync::IoCtx<tinyasync::SingleThreadTrait> myctx;
    char buffer[10];
    auto mysock = ::socket(PF_INET,SOCK_STREAM,0);
    tinyasync::ConnImpl connImpl(myctx,mysock,false);
    auto * ptr = new tinyasync::AsyncReceiveAwaiter(connImpl,buffer,10,true);
    printf("ptr addr %p\n",ptr);
    auto * m_node_addr  = &ptr->m_node;
    printf("ptr->m_node addr  %p\n",m_node_addr);
    auto * ptr2 = tinyasync::AsyncReceiveAwaiter::from_node(m_node_addr);
    printf("ptr2 addr %p\n",ptr2);
    delete ptr;
    return 0;
}
