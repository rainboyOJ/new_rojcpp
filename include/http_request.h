#include "tools/picohttpparser.h"
#include "all_headers.h"
#include "define.h"

namespace rojcpp {
    
class request {
    public:
        
        /**
         * 核心: 解析http头信息
         */
        void parse_header();
    
    private:

        std::size_t m_num_headers; // http头数量
        struct phr_header m_phr_headers[32]; // 头信息指针

        const char * m_method = nullptr;
        std::size_t m_method_len;

        const char * m_url = nullptr;
        std::size_t m_url_len;

        int m_minor_version = 0;
        int m_header_len;   // header 占用了多少字节的长度
        std::size_t m_body_len; // body 占用了多少字节的长度

        __string m_cookie_str;

        std::size_t m_cur_size = 0;
        std::size_t m_left_body_len= 0;


        content_type m_content_type = content_type::unknown;

//===== validate
        static constexpr std::size_t max_header_len = 1024*1024; //1024kb;
        // check_header_cb  check_headers_;
//===== validate end

        // 一个特别的值,设定这条连接对应的用户的id值(sql的记录id)
        std::size_t m_user_id = 0;

};

} // end namespace rojcpp
