#pragma once

//对发送的数据进行封装

#include <arpa/inet.h>
#include <cstring>
#include <vector>
#include <string_view>
#include <string>

class MessageBuffer {
    public:
        using MessageBuffer_t = std::vector<uint8_t>;

        MessageBuffer() = default;

        MessageBuffer(MessageBuffer&& rval) : buff(std::move(rval.buff))
        {}

        void appendMessge(std::string_view msg){
            push_back(msg.length());
            push_back(msg);
        }

        void push_back(uint32_t __val ){
            uint32_t val = htonl(__val); //转成net字节序
            char * ptr = (char *)&val;
            for(int i=0 ;i<sizeof(val);++i){
                buff.push_back(*(ptr+i) & 0xff);
            }
        }

        void push_back(std::string_view str){
            for (const auto& e : str) {
                buff.push_back(e);
            }
        }

        //导出message
        template<typename T>
        T dumpsMessage();

        std::string to_string(){
            std::string str;
            for (const auto& e : buff) {
                str+=e;
            }
            return str;
        }

        //导出message,从外部的字符串
        template<typename T>
        static T dumpsMessageFromStr(const char * ptr,int & cur);


        std::size_t size() const{
            return buff.size();
        }
        const char * data() const {
            return (const char *)&buff[0];
        }

        auto begin() { return buff.begin(); }
        auto end() { return buff.end(); }

    private:
        /**
         * @得到当前位置代表的数字
         */
        static uint32_t getInt(const char * ptr){
            uint32_t val;
            memcpy(&val,ptr,sizeof(val));
            return ntohl(val); //转成主机序
        }
        MessageBuffer_t buff;
        int cur{0}; //导出message时使用的下标
};

template<>
int MessageBuffer::dumpsMessageFromStr<int>(const char * ptr,int & cur) {
    auto val = getInt(ptr+cur);
    cur+=4;
    return val;
}

template<>
int MessageBuffer::dumpsMessage<int>(){ 
    return dumpsMessageFromStr<int>((char *)buff.data(), cur); 
}

template<>
std::string MessageBuffer::dumpsMessageFromStr<std::string>(const char * ptr,int & cur) {
    auto size = getInt(ptr+cur);
    auto old = cur+4;
    cur+=(4+size);
    std::string ret(ptr+old,ptr+cur);
    return ret;
}

template<>
std::string MessageBuffer::dumpsMessage<std::string>(){
    return dumpsMessageFromStr<std::string>((char *)buff.data() , cur);
}
