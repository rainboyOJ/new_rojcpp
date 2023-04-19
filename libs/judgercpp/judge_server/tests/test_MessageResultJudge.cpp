/**
 * 返回的结果数据的dump 与 load
 */

#include <cstdio>
#include <iostream>
#include "../src/Result.hpp"
#include "../src/utils.hpp"

int main(){

    MessageResultJudge msg_res("key",judgeResult_id::SUCCESS,"hello world");
    msg_res.push_back(1,2,3,4,5,6,7);
    msg_res.push_back(1,2,3,4,5,6,7);
    std::cout << msg_res;
    std::cout  << std::endl;

    auto dumps = msg_res.dumps();
    MessageResultJudge msg2;
    show_hex_code(dumps);
    show_hex_code(dumps.to_string());
    msg2.loads(dumps.to_string());
    std::cout <<  msg2<< std::endl;

    return 0;
}
