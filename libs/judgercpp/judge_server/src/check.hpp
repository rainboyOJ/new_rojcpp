/**
 * 测试两个文件是否一样
 * 使用方式
 * bool check::noipstyle_check(file1,file2)
 */
#pragma once

#include <iostream>
#include <fstream>
#include <string_view>


namespace cyaron {

const char CR = '\r';
const char LF = '\n';

struct Reader {
    Reader() = delete;
    Reader(std::string_view file_name)
    { fs.open( std::string(file_name).c_str());}
    virtual ~Reader(){ fs.close(); }
        //当前是否到了eof
    bool eof(){ return cur == EOF; }
    bool isEofn(char c) { return c == CR || c == LF ;}
    void readEoln(){
        for(;;)
            if( isEofn(peekNextChar()) )
                getNextChar();
            else break;
    }

    std::string readLine(){
        std::string __ret;
        for(;;){
            char c = getNextChar();
            if( isEofn(c)) break;
            if( c == EOF ) break;
            __ret += c;
        }
        readEoln();
        return __ret;
    }
private:
    char cur{-2}; //当前的char
    std::ifstream fs;

    //读取下一个字符
    inline char getNextChar(){
        cur = static_cast<char>(fs.get());
        return cur;
    }

    //读取下一个字符
    inline char peekNextChar(){
        return static_cast<char>(fs.peek());
    }
};

struct Check {

    // 返回 true 表示一样
    // false 表示不一样
    static bool noipstyle_check(
            const std::string_view user_output_path,
            const std::string_view std_output_path )
    {
        auto del_line_back_space = [](std::string &str){
            while(!str.empty()){
                auto ec = str.back();
                if( ec == '\n' || ec == ' ' || ec == '\r') str.pop_back();
                else break;
            }
        };

        Reader ouf(user_output_path);
        Reader ans(std_output_path);
        int n=0;
        while ( !ans.eof() ) {
            std::string j = ans.readLine();
            del_line_back_space(j);            //去除末尾空白
            if (j.empty() && ans.eof()) break; // ans 读到最后一个空白行
            std::string p = ouf.readLine();
            del_line_back_space(p);
            n++;
            if (j != p) return false;
        }
        return true;
    }
};

} //namespace cyaron 
