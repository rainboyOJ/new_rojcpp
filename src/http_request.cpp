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

    m_num_headers = sizeof(m_phr_headers) / sizeof(m_phr_headers[0]);
    // int parser_ret = phr_parse_request(
    //if return value > 0,it represent header bytes length
    int header_len = phr_parse_request(
            (const char *)m_buff.data(),
            m_buff.used_size(),
            &m_method,          // set value to point the address of method starting in read buffer
            &m_method_len ,     // set value that method length in read buffer
            &m_url,             // the pointer address of url path in read buffer
            &m_url_len,         // the length of url path in read buffer
            &m_minor_version,   // http_request minor version
            m_phr_headers,      // set each value of array of struct phr_header point to each value in read buffer after header analyzed
            &m_num_headers,      // set numbers of headers after analyzed
            0);                 // i don't understand usefulness 
                                
    if(header_len < 0 ) // -1 mean parse erro or -2  mean not read completely
        return header_len;

    //check_request();

    //check_gzip();

    //step : set body length
    std::string_view header_value = get_header_value("content-length");
    if( header_value.empty()) {
        set_body_len(0);
    }
    else {
        set_body_len( atoll(header_value.data()) );
    }

    // step 2: cookie的设置
    std::string_view cookie = get_header_value("cookie");
    // TODO, 需要设置cookie str吗
    // if( !cookie.empty() ) {
    //     ;
    // }

    // step 3: get raw_url && parse queries
    std::string_view raw_url_ = {m_url,m_url_len};
    std::size_t npos = raw_url_.find('/');
    if(npos == std::string_view::npos) // parse error ,not right url partten
        return -1;

    npos = raw_url_.find('?');
    if( npos != std::string_view::npos) {
        m_path = raw_url_.substr(0,npos);
        std::string_view queries_str = raw_url_.substr(npos+1,m_url_len-npos-1);
        m_query_map = parse_query(queries_str);
    }
    else{
        m_path = raw_url_;
    }

    return header_len;
}



std::string_view request::get_header_value(std::string_view key) const {
    for(int i = 0 ; i < m_num_headers ;i++) {
        if(iequal(m_phr_headers[i].name , m_phr_headers[i].name_len ,key.data(),key.size()))
            return std::string_view(m_phr_headers[i].value,m_phr_headers[i].value_len);
    }

    return {};

}


// === private api
std::map<std::string_view,std::string_view> request::parse_query(std::string_view str) const
{
    std::map<std::string_view,std::string_view> query_map;
    std::string_view key,val;
    int pos = 0, length = str.length();

    for(int i=0 ; i<length;i++) {
        char c = str[i];
        if( c == '='){
            key = {&str[pos],std::size_t(i-pos)};
            key = trim(key);
            pos = i+1;
        }
        else if ( c == '&') {
            val = {&str[pos],std::size_t(i-pos)};
            val = trim(val);
            pos = i+1;
            query_map.emplace(key,val);
        }
    }

    if( pos == 0) return {};

    if( length - pos > 0) {
        val = {&str[pos],std::size_t(length-pos)};
        val = trim(val);
        query_map.emplace(key,val);
    }
    else if( length- pos == 0) {
        query_map.emplace(key,"");
    }

    return query_map;
}


content_type request::get_content_type() const {
    if( m_content_type != content_type::unknown )
        return m_content_type;

    std::string_view header_value = get_header_value("content-type");
    std::size_t header_len = header_value.length();

    if( header_value.empty()) return m_content_type;

    if( header_value.find("application/x-www-form-url_encoded") != std::string_view::npos)
        return content_type::urlencoded;

    //Content-Type:	multipart/form-data; boundary=----WebKitFormBoundaryX4zzs3sYStDhVBUX
    if (header_value.find("multipart/form-data") != std::string_view::npos)
    {
        // TODO
        // int pos = header_value.find("=");
        // std::string_view boundary = header_value.substr(pos+1,header_len-pos);
        //
        // if( boundary.front() == '"' && boundary.back() == '"')
        //     boundary = boundary.substr(1,boundary.length()-1);

        // multipart_parser_.set_boundary("\r\n--" + std::move(boundary));
        return content_type::multipart;
    }

    if( header_value.find("application/octet-stream") != std::string_view::npos)
        return content_type::octet_stream;

    if( header_value.find("application/json") != std::string_view::npos)
        return content_type::json;

    return content_type::unknown;
}

} // end namespace rojcpp
