// response 的逻辑
#pragma  once

#include <vector>
#include <array>
#include <chrono>
#include "tools/cookie.h"
#include "tools/response_cv.hpp"
#include "all_headers.h"
#include "tools/buffers.h"

class http_session;


namespace rojcpp {

class response {
    friend class http_session;

public:

    
    explicit
    response(std::pmr::memory_resource * mpr) 
        :m_buff(mpr)
    {}

    template<status_type status>
    constexpr auto set_status_and_header() 
    {
        // 类似 "HTTP/1.1 200 OK\r\n"
        // constexpr std::string_view const status_str = to_rep_string(status);

        // return sv_join_v<not,rep_server,header_end_sv>;
    }

    /**
     * constexpr 编译期函数
     */
    template<status_type status,content_type content_type, size_t N>
    constexpr auto set_status_and_content
        (
            const char(&content)[N],
            content_encoding encoding = content_encoding::none 
        )
    {
        // 类似 "HTTP/1.1 200 OK\r\n"
        constexpr std::string_view status_str = to_rep_string(status);

        // 类似 Content-type:application/json:
        constexpr std::string_view type_str   = to_content_type_str(content_type);

        // 类似 Content-length: 100
        // type is std::array<char,int = 19>
        constexpr auto len_str = num_to_string<N-1>::value;

        m_buff.clear();
        
        append_buff(status_str);
        append_buff(type_str);
        append_buff(rep_server);
        append_buff(len_str.data(),len_str.size());
        append_buff("\r\n");
        append_buff(content);

    }

    template<status_type status,content_type content_type>
    void set_status_and_content
        (
            const std::string & str,
            content_encoding encoding = content_encoding::none 
        )
     {
        // 类似 "HTTP/1.1 200 OK\r\n"
        constexpr std::string_view status_str = to_rep_string(status);

        // 类似 Content-type:application/json:
        constexpr std::string_view type_str   = to_content_type_str(content_type);

        // 类似 Content-length: 100
        // type is std::array<char,int = 19>
        int len_str = str.length();

        m_buff.clear();
        
        append_buff(status_str);
        append_buff(type_str);
        append_buff(rep_server);
        append_buff("content_length: ");
        append_buff_number(len_str);
        append_buff("\r\n\r\n");
        append_buff(str.data(), len_str);
     }

    // >>>>  common API
    void send_not_found() {
        m_const_respone = sv_join_v<rep_not_found,rep_server,header_end_sv>;
    }
    // <<<< common API END

    //返回的数据
    std::string response_str();

    void enable_respone_time();

    //核心 TODO
    std::string_view build_response_buffer();

    std::vector<char> to_buffers();

    //添加响应头
    void add_header(std::string&& key ,std::string && value);
    void clear_headers();

    
    void set_status(status_type status);
    status_type get_status() const;

    std::string_view get_content_type() const;

    //重要:创建cookie
    void create_cookie(const std::string & uuid);

    void set_domain();
    std::string_view get_domain();
    std::string_view get_path();

    void set_url();
    std::string_view get_url() const;

    void redirect();

    void redirect_post();

    bool need_delay () const;

    //重置
    void reset();

private:

    //push data to m_buff
    void append_buff(const char * buff,const std::size_t buff_size);

    inline void append_buff(std::string_view buff) {
        append_buff(buff.data(), buff.length());
    }

    void append_buff_number(std::size_t num) {
        bool flag = 1;
        char buf[512];
        int idx = 0;
        do {
            buf[idx++] = num % 10;
            num /= 10;
        }while(num != 0);
        std::reverse(buf, buf+idx);
        append_buff(buf, idx);
    }

    /**
     * 得到
     */
    std::string_view get_header_value(std::string_view key) const;

    __vector<std::pair<__string,__string>> m_headers;
    __string m_content;

    //返回的content的类型
    content_type m_body_type = content_type::unknown;
    status_type  m_status    = status_type::init;

    std::string_view m_raw_url;
    std::string_view m_domain;
    std::string_view m_path;
    __string m_rep_str;     //这里最后要用buffer替换

    std::shared_ptr<cookie> cookie_sh_ptr = nullptr;

    rojcpp::Buffer<> m_buff;

    std::string_view m_const_respone;
};


} // end namespace rojcpp
