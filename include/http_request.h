#include "tools/picohttpparser.h"
#include "all_headers.h"
#include "tools/buffers.h"
#include "define.h"


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
    
    private:

        std::size_t m_num_headers; // http头数量
        struct phr_header m_phr_headers[32]; // 头信息指针

        const char * m_method = nullptr;
        std::size_t m_method_len;

        const char * m_url = nullptr;
        std::size_t m_url_len;

        int m_minor_version = 0;
        std::size_t m_header_len;   // header 占用了多少字节的长度
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

        rojcpp::Buffer<std::byte> m_buff;
};


} // end namespace rojcpp
