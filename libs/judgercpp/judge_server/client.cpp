#include <bits/stdc++.h>
#include "./src/Client.hpp"


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

//接收结果的数量
std::atomic_int recv_res_count{0};

int main(){
    

    // args1 与服务器建立4个连接
    // args2 judge_server ip
    // args3  judge_server port
    Client myclient(4,"127.0.0.1",9000); 

    //返回信息的处理函数
    myclient.set_result_handle([](MessageResultJudge & res){
                if(res.get_msg() == "all") {
                    recv_res_count+=1;
                    std::cout << recv_res_count.load() << std::endl;
                }
                //std::cout << res << std::endl;
            });
    //发送 10 次 评测数据
    for(int i=1;i<=10;++i){
        myclient.send("test key"
                ,codeAplusB
                ,"cpp"
                ,"1000"
                ,1000
                ,128
                );
    }


    while ( 1 ) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "wait..." << std::endl;
        if( recv_res_count.load() >= 10 )
            break;
    }
    return 0;
}
