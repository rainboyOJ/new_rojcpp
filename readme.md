# RojCpp 

基于 c++20 的oj服务器后端,整个OJ由三个部分组成,这里是`后端cpp`部分


```plaintext

+---------+         +---------+       +---------+
|         |         |         |       |         |
| 前端vue | <------>| 后端cpp | <---->| 评测cpp |
|         |         |         |       |         |
+---------+         +---------+-      +---------+

```

## 快速安装

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
# 如果你想调试 : cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## 模块

- http功能实现 `include/`
- 日志 `libs/logger/`
- 网络IO库 `libs/tinyasync/`
- mysql连接池与操作封装
- 简易Cache
- 简单反射,json与struct互转

## 目录结构

```plaintext
.
├── include 头文件
│   ├── all_headers.h
│   ├── debug.h
│   ├── define.h
│   ├── http_request.h
│   ├── http_response.h
│   ├── http_session.h
│   ├── multipart_parser.hpp
│   ├── server.h
│   ├── site_conf.hpp
│   └── tools
│       ├── buffers.h
│       ├── cookie.h
│       ├── fastCache.hpp
│       ├── gzip.hpp
│       ├── mime_types.hpp
│       ├── picohttpparser.h
│       ├── response_cv.hpp
│       ├── url_encode_decode.hpp
│       └── utils.hpp
├── src  头文件的实现
└── libs
    └── tinyasync IO框架,基于c++20 coroutine
```


## 头文件的包含关系

```mermaid
flowchart TB
    http_session.h--> http_request.h & http_response.h
    http_response.h --> tools/cookie.h & tools/response_cv.hpp & all_headers.h 
    http_request.h --> tools/picohttpparser.h & define.h
    tools/response_cv.hpp --> define.h
    server.h --> http_session.h & libs/tinyasync.h
```

## 文档

- [路由设计](./docs/route/readme.md)
- [tinyasync的文档](./libs/tinyasync/doc)
- [工具库utils.hpp](./docs/tools/utils.md)
