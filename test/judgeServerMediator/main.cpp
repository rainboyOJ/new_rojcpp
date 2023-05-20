#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "judge/judgeServerMediator.hpp"

std::string codeAplusB = R"(/* author: Rainboy email: rainboylvx@qq.com  time: 2022年 03月 21日 星期一 08:56:19 CST */
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const int maxn = 1e6+5,maxe = 1e6+5; //点与边的数量

int n,m;
/* 定义全局变量 */

int main(int argc,char * argv[]){
    int a,b;
    std::cin >> a >> b;
    std::cout << a+b ;
    return 0;
}
)";

//测试与judgeServer的连接
int main (int argc, char *argv[]) {
    // 创建一个judgeServerMediator
    auto & myJudgeMidator = JSM();

    //set function that handle result of judge 
    result_handler my_handle = [](MessageResultJudge & msg){
        std::cout << "result.code = "<<  msg.get_code() << "\n";
    };
    myJudgeMidator.set_result_handle(my_handle);

    // TODO : 是否已经连接 judgeServer
    
    //发送评测数据
    myJudgeMidator.send("test_key",codeAplusB,"cpp","1000",1000,128);
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}
