#pragma once

#include "tools/picohttpparser.h"
#include "all_headers.h"
#include "tools/buffers.h"
#include "define.h"
#include "tools/utils.hpp"


class http_session;

namespace rojcpp {

enum class PARSE_HEADER_STATE : int {
    HAS_ERROR,
    NEED_READ,
};
    
class request {

    friend class http_session;

    public:
        
        request(std::pmr::memory_resource * mrp )
            : m_buff(mrp) 
        {}

        /**
         * 核心: 解析http头信息
         *
         * will get information of read data
         * -1 represent has error
         * -2 represent imcomplement
         *  > 0 respect numbers of headers in read data
         */
        int parse_header();

        /**
         * 填充读取的数据
         */
        void feed_read_datas(void * src,std::size_t siz);


        
        // @brief get the header value from key
        std::string_view get_header_value(std::string_view key) const;

        
        std::size_t header_len() const { return m_header_len; }
        void set_header_len(std::size_t val) { m_header_len = val;}

        std::size_t body_len()   const { return m_body_len ; }
        void  set_body_len(std::size_t val) {m_body_len = val;}

        std::size_t total_len() const { return m_header_len + m_body_len;}

        std::size_t total_len() { }

        //是否有content body
        bool has_body() const { return m_body_len !=0; } /* || is_chunked_*/

        //是否接收完了所有的数据
        bool has_recived_all() { return m_body_len == m_cur_size - m_header_len;}

        content_type get_content_type() const;
        void set_content_type(content_type type) { m_content_type = type;}

        std::string_view get_method() const { return {m_method,m_method_len}; }
        std::string_view get_url() const { return {m_url,m_url_len};}
        std::string_view get_path() const { return m_path;}

    
    private:

        //@brief only parse query string like this : ?foo=bar&fruit=apple
        std::map<std::string_view,std::string_view> parse_query(std::string_view str) const;

        std::size_t m_num_headers; // http头数量
        struct phr_header m_phr_headers[32]; // 头信息指针

        const char * m_method = nullptr;
        std::size_t m_method_len;

        const char * m_url = nullptr;
        std::size_t m_url_len;
        std::string_view m_path;

        int m_minor_version = 0;
        std::size_t m_header_len; // header 占用了多少字节的长度
        std::size_t m_body_len;   // body 占用了多少字节的长度

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

        std::map<std::string_view,std::string_view> m_query_map;

        rojcpp::Buffer<std::byte> m_buff;
};


} // end namespace rojcpp
