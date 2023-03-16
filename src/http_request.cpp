#include "http_request.h"


namespace rojcpp {
    

void request::feed_read_datas(void *src,std::size_t siz) {
    if( m_buff.left_size() < siz ) {
        //expand siz -> 2^n
        //num是要向上取整的数，b为2^n
        //(num + (b - 1)) & ~(b-1)
        // new_size 是 8的倍数
        std::size_t new_size = (m_buff.tot_size() + 8-1) & ~(8-1);
        m_buff.expand_size(new_size);
    }

    memcpy(m_buff.write_data(),src,siz);
    m_buff.update_used_sized(siz);
}


int request::parse_header() {

    m_header_len = sizeof(m_phr_headers) / sizeof(m_phr_headers[0]);
    int parser_ret = phr_parse_request(
            (char *)m_buff.data(),
            m_buff.used_size(),
            &m_method,          // set value to point the address of method starting in read buffer
            &m_method_len ,     // set value that method length in read buffer
            &m_url,             // the pointer address of url path in read buffer
            &m_url_len,         // the length of url path in read buffer
            &m_minor_version,   // http_request minor version
            m_phr_headers,      // set each value of array of struct phr_header point to each value in read buffer after header analyzed
            &m_header_len,      // set numbers of headers after analyzed
            0);                 // i don't understand usefulness 
                                
    if(parser_ret < 0 ) // -1 or -2 
        return parser_ret;
    
    return parser_ret;

}


} // end namespace rojcpp
