#include <iostream>

#include "./src/concurrentqueue.h"
#include "./src/judgeQueue.hpp"

//#define log(...) std::cout << __VAR_ARGS__ << std::endl;
int main(){
    

    //moodycamel::ConcurrentQueue<int> q;     // 存接入的socket队列
    //int a=-1;
    ////q.enqueue(11);
    //bool ret = q.try_dequeue(a);
    //std::cout << ret << std::endl;
    //std::cout << a << std::endl;

    judge_Queue_node jn;
    //for(int i=1;i<=10000;++i){
        //;
        //int a = 1+1;
    //}
    MessageSendJudge msj("key","code","cpp","pid",100,123);
    judge_Queue::get().enque(std::move(msj), 42);

    for(int i=1;i<=100;++i){
        bool ret = judge_Queue::get().try_deque(jn);
        std::cout << i << " " << ret << std::endl;
        std::cout << jn.key << std::endl;
        std::cout << jn.code << std::endl;
        std::cout << jn.timeLimit << std::endl;
        std::cout << jn.memoryLimit << std::endl;
    }
    moodycamel::ConcurrentQueue<judge_Queue_node> q;     // 存接入的socket队列
    bool ret = q.try_dequeue(jn);
    std::cout << ret << std::endl;
    return 0;
}

