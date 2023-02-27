
工具库文档

## utils.hpp
位于 `include/tools/utils.hpp`

提供了各种的工具函数

- `struct ci_less`,转小写后的字典序比较仿函数
- `class noncopyable`,基类,禁止拷贝
- `sv_char_trait`  对`std::char_trait`进行hook,
- `trim_left(string_view v)` 
- `trim_right(string_view v)` 
- `trim(string_view v)` 
- `get_domain_url(string_view path)` 得到域名
- `remove_www(string_view path)`  删除字符串中的www
- `get_host_port(string_view path)` 得到链接中的port
- ` GET, POST, DEL, HEAD, PUT, CONNECT, TRACE, OPTIONS` 常量enum
- `method_name(http_method mthd)`   将`http_method`转成string
- `get_content_type_str(req_content_type type)` 将`req_content_type`宏转成string 

未完成

## `mime_types.hpp`

文件后缀对应的`mime_type`

## `response_cv.hpp`

response const variable,创建respone时需要的一些常量,具体看源码
