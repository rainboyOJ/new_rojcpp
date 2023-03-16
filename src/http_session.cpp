#include "http_session.h"

namespace rojcpp {
    

std::tuple<std::byte *,std::size_t> 
    http_session::req_buff() {
        static constexpr std::size_t need_alloc_size = 1024 * 1024; // 1024kb
        if( m_req.m_buff.empty() ) {
            m_req.m_buff.expand_size(need_alloc_size);
        }

        if( m_req.m_buff.left_size() <= 0 )
            m_req.m_buff.expand_twice_size();

        return { 
            m_req.m_buff.write_data(),
            m_req.m_buff.left_size()
        };
    }

void http_session::update_req_buff_used_size(std::size_t siz) {
    m_req.m_buff.update_used_sized(siz);
}


void http_session::process() {
    //1. [check] 是否超过的最大容量
    
    //2 
    
    // 设定的response的内容
    m_res.set_status_and_content<status_type::accepted,content_type::string>("ok");

}

std::tuple<std::byte *,std::size_t> http_session::res_buff(){
    return {m_res.m_buff.data(),m_res.m_buff.used_size() };
}

int http_session::handle_read() {
    return m_req.parse_header();
}



} // end namespace rojcpp
