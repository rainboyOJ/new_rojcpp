#include "site_conf.hpp"
#include "http_response.h"

namespace rojcpp {

void response::add_header(std::string &&key, std::string &&value)
{
    m_headers.emplace_back (std::move(key),std::move(value));
}

void response::clear_headers() {
    m_headers.clear();
}

void response::set_status(status_type status) {
    m_status = status;
}

status_type response::get_status() const {
    return m_status;
}


std::string_view response::get_content_type() const {
    return  to_content_type_str(m_body_type);
}

void response::create_cookie(const std::string & uuid) {

    constexpr auto expire_time = __config__::session_expire;
    auto cok_ptr = std::make_shared<cookie>(CSESSIONID,uuid);
    cok_ptr -> set_domain("/");
    std::time_t now = std::time(nullptr); //创建时间
    auto time_stamp_     = expire_time + now;
    cok_ptr -> set_max_age(expire_time == -1 ? -1 : time_stamp_);
    cok_ptr -> set_domain("");
    cok_ptr -> set_path("/");
    cok_ptr -> set_version(0);
    // 不需要用cache
    //Cache::get().set(std::string("session_") + uuid, cok_ptr -> to_string());
    cookie_sh_ptr = cok_ptr;

}


void response::reset() {
    m_headers.clear();
    m_content.clear();
    m_rep_str.clear();
}


} // end namespace rojcpp
