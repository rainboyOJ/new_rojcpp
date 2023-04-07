#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "tools/picohttpparser.h"


//from https://stackoverflow.com/a/2602258
//read all text from file
std::string read_file(const char * filename) {
    std::ifstream myfile;
    myfile.open(filename);
    std::stringstream buffer;
    buffer << myfile.rdbuf();
    return buffer.str();
}

const char *method, *path;
int pret, minor_version;
struct phr_header headers[100];
std::size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
ssize_t rret;


const char buff[] = "GET /hoge HTTP/1.1\r\n\r\n";

//带有header
const char buff2[] = "GET /hoge HTTP/1.1\r\naccept: */*\r\n\r\n";
//带有content
const char buff3[] = "GET /hoge HTTP/1.1\r\ncontent-length: 5\r\n\r\n";
const char buff4[] = "GET /hog HTTP/1.1\r\ncontent-length: 5\r\n\r\n12345\r\n\r\n";

const char buff5[] = 
"GET /usr/login HTTP/1.1\r\n"
"Host: 127.0.0.1:8899\r\n"
"User-Agent: curl/7.87.0\r\n"
"Accept: */*\r\n\r\n";


void test(const char * buff,int buff_size) {
    num_headers = sizeof(headers) /sizeof(headers[0]);
    int parsed_head_nums = phr_parse_request(
            // file1.data() , 
            // file1.length(),
            buff,
            buff_size,
            &method, &method_len, &path, &path_len,
                             &minor_version, headers, &num_headers, 0);

    std::cout << "parsed result : " << parsed_head_nums << "\n";

}

int main(){
    
    std::string file1 = read_file("header1.txt");

    test(buff,sizeof(buff)-1);
    test(buff2,sizeof(buff2)-1);
    test(buff3,sizeof(buff3)-1);
    test(buff4,sizeof(buff4)-1);
    std::cout << "buff5 size = " << sizeof(buff5)-1 << "\n";
    test(buff5,sizeof(buff5)-1);
    /**
     * 结论: 由上面的buff1,buff2,buff3,buff4
     *      测试可以知:
     *      phr_parse_request 当传递的过来的数据到http数据头结束的部分时,
     *      就可以解析成功,不需要content部分
     *      返回的值为http header部分的长度
     */
    return 0;
}
