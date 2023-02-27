一个用来解析 multipart 的工具类 ,核心使用的一个简单的状态机(见下面的图)
在某个状态的时候调用对应的处理函数。


## 什么是 multipart


媒体类型multipart/form-data遵循multipart MIME数据流定义（该定义可以参考Section 5.1 - RFC2046），大概含义就是：媒体类型multipart/form-data的数据体由多个部分组成，这些部分由一个固定边界值（Boundary）分隔。

multipart/form-data请求体的布局如下：

```
# 请求头 - 这个是必须的，需要指定Content-Type为multipart/form-data，指定唯一边界值
Content-Type: multipart/form-data; boundary=${Boundary}

# 请求体
--${Boundary}
Content-Disposition: form-data; name="name of file"
Content-Type: application/octet-stream

bytes of file
--${Boundary}
Content-Disposition: form-data; name="name of pdf"; filename="pdf-file.pdf"
Content-Type: application/octet-stream

bytes of pdf file
--${Boundary}
Content-Disposition: form-data; name="key"
Content-Type: text/plain;charset=UTF-8

text encoded in UTF-8
--${Boundary}--
```

利用`curl`,`netcat`来测试

- 创建一个文本文件`echo hello > 1.txt`
- `netcat`创建一个服务器`netcat -l -p 8899`
- `curl`发送`multipart`数据,`curl -v -F key1=value1 -F upload=@1.txt http://127.0.0.1:8899`

那么`netcat`得到的数据如下

```
POST / HTTP/1.1
Host: 127.0.0.1:8899
User-Agent: curl/7.87.0
Accept: */*
Content-Length: 290
Content-Type: multipart/form-data; boundary=------------------------27aa49dd4137e76c

--------------------------27aa49dd4137e76c
Content-Disposition: form-data; name="key1"

value1
--------------------------27aa49dd4137e76c
Content-Disposition: form-data; name="upload"; filename="1.txt"
Content-Type: text/plain

hello

--------------------------27aa49dd4137e76c--
```

整个`multipart_parser.hpp`代码的作用的就是来解析这个`multipart/form-data`请求

## 核心思路

一个状态机,全部的状态如下

```
PARSE_ERROR,
START,
START_BOUNDARY,
HEADER_FIELD_START,
HEADER_FIELD,
HEADER_VALUE_START,
HEADER_VALUE,
HEADER_VALUE_ALMOST_DONE,
HEADERS_ALMOST_DONE,
PART_DATA_START,
PART_DATA,
PART_END,
END
```
HEADER_FIELD_START
-->
HEADER_FIELD    -> 读到CR?HEADERS_ALMOST_DONE -> PART_DATA_START
-->
HEADER_VALUE_START
-->
HEADER_VALUE
->
HEADERS_ALMOST_DONE
->
HEADER_FIELD_START


END  怎么转移到的?
