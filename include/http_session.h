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
// #include "tinyasync/tinyasync.h"


namespace rojcpp {
    

// 处
enum class PROCESS_STATE  {
    TO_READ,
    READING,
    READ_END,
    TO_WRITE,
    WRITING,
    WRITE_END,
};


class http_session;

// 一个指向http_session内部的 成员函数指针
using continue_work_fptr = PROCESS_STATE(http_session::*)();


class http_session {
    private:
        std::pmr::memory_resource * m_pool; //内存池
        continue_work_fptr m_continue_work = nullptr;
        // tinyasync::Connection m_ta_conn; //tinyasync的 Connection

        // http_handle //
    public:

        http_session(std::pmr::memory_resource * mr) : m_pool{mr}
        {}

        // http_session(tinyasync::Connection && conn) : m_ta_conn( std::move() )
        // {}

        //核心1 : 读取
        // tinyasync::Task<> headle_read();

        //核心2 : 异步写入
        // tinyasync::Task<> headle_write();

        //核心3: 数据处理与路由
        void process();


        
};

} // end namespace rojcpp
