#include "http_session.h"
#include "logger/logger.h"

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
    
    //2 处理http数据
    handle_read(m_req,m_res);
    
    // 设定的response的内容
    // m_res.set_status_and_content<status_type::accepted,content_type::string>("ok");

}

int http_session::handle_read(request& req,response & res) {
    //1 检查路由,不需要检查

    if( m_req.has_body() ) {
        
        //TODO
        //handle gzip

        content_type type = m_req.get_content_type();
        m_req.set_content_type(type);

        switch(type) {
            case rojcpp::content_type::string:
            case rojcpp::content_type::json:
            case rojcpp::content_type::unknown:
                handle_string_body(); 
                break;
            default: //not support ,error
                throw std::runtime_error("not support content-type");

        }
        route(); // 执行路由

    }
    else { // handle only has header
        route(); // 执行路由
    }


    return 0;
}


std::tuple<std::byte *,std::size_t> http_session::res_buff(){
    if( m_res.m_const_respone.length() !=0 )
        return {(std::byte *)m_res.m_const_respone.data(),m_res.m_const_respone.length()};
    return {m_res.m_buff.data(),m_res.m_buff.used_size() };
}

//执行路由
void http_session::route() {
    if((*m_http_handle_check)(m_req,m_res) == false) {
        // res.set_status_and_content<status_type::not_found, content_type::json>();
        // res.set_status(status_type::not_found);
        m_res.send_not_found();
    }
}


int http_session::handle_string_body() {
    LOG_DEBUG << __FUNCTION__ ;

    //TODO do some check work

    // if( m_req.has_recived_all() ) { 不应该在这里检查,应该在读取的地方检查
        // handle_body();
    // }
    // else {
    //
    // }

    return 0;
}



} // end namespace rojcpp
