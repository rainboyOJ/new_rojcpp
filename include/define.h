/* 需要的全局常量 */
#pragma once
#include <filesystem>
namespace fs = std::filesystem;

namespace rojcpp {

    // 解析请求数据的状态
    enum parse_status {
        complete = 0,       //完成
        has_error = -1,     //出错
        not_complete = -2,  //未完成
    };

    //响应头 content_type 内部的类型
    enum class content_type {
        string,
        json,
        multipart,
        urlencoded,
        chunked,
        octet_stream,
        websocket,
        unknown,
    };

    //请求头 内的数据类型
    enum class req_content_type{
        html,
        json,
        string,
        multipart,
        none
    };

    constexpr inline auto HTML = req_content_type::html;
    constexpr inline auto JSON = req_content_type::json;
    constexpr inline auto TEXT = req_content_type::string;
    constexpr inline auto NONE = req_content_type::none;

    inline const std::string_view STATIC_RESOURCE = "rojcpp_static_resource";
    inline const std::string CSESSIONID = "CSESSIONID";
    inline const std::string CSESSIONIDWithEQU = "CSESSIONID=";

    const static inline std::string CRCF = "\r\n";
    const static inline std::string TWO_CRCF = "\r\n\r\n";
    const static inline std::string BOUNDARY = "--rojcppBoundary2B8FAF4A80EDB307";
    const static inline std::string MULTIPART_END = CRCF + "--" + BOUNDARY + "--" + TWO_CRCF;


    constexpr static int SESSION_DEFAULT_EXPIRE_{7*24*60*60}; // 7 days

    struct NonSSL {};
    struct SSL {};
}
