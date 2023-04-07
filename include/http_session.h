// 一个处理 http 请求的 类
// 包含 http_request, http_response
// 所有http的处理都在这里
// 1. 路由与路由检查
// 2. http 协议处理
// 3. 生成response数据
// 4. 其它功能
//      上传(只运行上传文本文件,也就是代码文件,也可能不支持)
//      下载(暂无)
//      静态文件托管(暂无)
#pragma once

#include <memory_resource>

#include "http_request.h"
#include "http_response.h"
#include "http_route.h"

namespace rojcpp {
    

// 处理状态
enum class PROCESS_STATE  {
    TO_READ,
    READING,
    READ_END,
    TO_WRITE,
    WRITING,
    WRITE_END,
};


// 一个指向http_session内部的 成员函数指针
using continue_work_fptr = PROCESS_STATE(http_session::*)();

//调用route的函数
using http_handle_func = std::function<void(request&,response&)> ;

//调用route检查 的函数
using http_handle_check_func = std::function<bool(request&,response&)>;

class http_session {
    private:
        std::pmr::memory_resource * m_pool; //内存池
        continue_work_fptr m_continue_work = nullptr;
        request  m_req;
        response m_res;

        http_handle_func * m_http_handle = nullptr;
        http_handle_check_func *  m_http_handle_check = nullptr;

        //接口函数

        // http_handle //

    public:


        explicit
        http_session(std::pmr::memory_resource *mr ,
                http_handle_func *  http_handle, // 指针
                http_handle_check_func *  http_handle_check //指针
                )
        : m_http_handle(http_handle),
          m_http_handle_check(http_handle_check),
          m_pool{mr},m_req(mr),m_res(mr)
        {

        }


        // API
        // API end

        // http_session(tinyasync::Connection && conn) : m_ta_conn( std::move() )
        // {}

        //核心1 : 读取 -> 由外部写入Buffer里
        //得到req buffer的[写入地址,可写入的大小]
        std::tuple<std::byte *,std::size_t> req_buff() ;

        //update buff used size after use function req_buff()
        void update_req_buff_used_size(std::size_t siz);


        //核心2 : 异步回应 --> 由外部读取 Res里的Buffer
        std::tuple<std::byte *,std::size_t> res_buff();

        int parse_raw_request_data() { return m_req.parse_header();}

        //核心3: 数据处理与路由
        void process();

        /**
         * @brief 解析读取的数据
         *
         * @return 
         */
        int handle_read(request&,response&);

        //处理字符串类型的body
        int handle_string_body();

        //执行路由
        void route();

        
};

} // end namespace rojcpp
