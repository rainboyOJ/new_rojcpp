// response 的逻辑
#pragma  once

#include <vector>
#include <array>
#include <chrono>
#include "tools/cookie.h"
#include "tools/response_cv.hpp"
#include "all_headers.h"


namespace rojcpp {

class response {
public:
    response() {}

    template<status_type status,content_type content_type, size_t N>
    constexpr auto set_status_and_content
        (
            const char(&content)[N],
            content_encoding encoding = content_encoding::none 
        )
    {
        // 类似 "HTTP/1.1 200 OK\r\n"
        constexpr auto status_str = to_rep_string(status);

        // 类似 Content-type:application/json:
        constexpr auto type_str   = to_content_type_str(content_type);

        // 类似 Content-length: 100
        constexpr auto len_str = num_to_string<N-1>::value;

        m_rep_str.append(status_str)
                 .append(type_str)
                 .append(rep_server) // "Server: rojcpp\r\n";
                 .append(len_str.data(),len_str.sizel());

        m_rep_str.append("\r\n");

        m_rep_str.append(content);

    }

    //返回的数据
    std::string response_str();

    void enable_respone_time();

    //核心 TODO
    std::string build_response_buffer();

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
};


} // end namespace rojcpp
