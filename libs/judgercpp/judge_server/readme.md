# 评测服务器

原理/流程图

```
->client 发送评测数据
->解码(状态机)
->放入评测线程池,执行评测函数
->得到结果
->回写结果到socket
```

## 文件说明

```plaintext

src
├── base64.hpp          base64的编码与解码,没有用到
├── Cache.hpp           对评测结果进行缓存,没有用到
├── check.hpp           输出的结果就行对比,判断是否一样
├── Client.hpp          客户端封装
├── concurrentqueue.h   多线程无锁队列,没有用到
├── define.hpp          一些默认定义的值
├── judgeArgs.hpp       生成评测参数
├── judgeQueue.hpp      评测队列,没有用到
├── judgeWorkPool.hpp   评测队列
├── MessageBuffer.hpp   基础类,用来传递消息
├── Problem.hpp         对题目的进行检测
├── Result.hpp          发送给Clinet的结果数据封装
├── Send.hpp            发磅给Server的评测数据的封装
├── Server.hpp          Server端封装
├── socketBase.hpp      基类,socket通信
├── socketManager.hpp   对socket进行管理的类
├── threadPool.hpp      线程池,没有用到
└── utils.hpp           常用函数

0 directories, 18 files
```

## 安装

1. 先安装 `judger`
2. 修改`USER_CONFIG.hpp`中的值,
3. 编译并运行
    ```
    mkdir build && cd build
    cmake ...
    # 如果想开启Debug(会输出调试信息到屏幕)
    # cmake -DJUDGE_SERVER_DEBUG=ON
    make
    ./server
    ```

## 使用方式

客户端,具体看`client.cpp`

向服务端发送一个评测时,当服务端面对下面的评测数据时

```
p1.in p1.ans
p2.in p2.ans
p3.in p3.ans
```

一共有三条评测数据,那么服务端返回的数据为

```cpp
enum judgeResult_id {
    SUCCESS             = 0,
    INVALID_CONFIG      = -1,
    FORK_FAILED         = -2,
    PTHREAD_FAILED      = -3,
    WAIT_FAILED         = -4,
    ROOT_REQUIRED       = -5,
    LOAD_SECCOMP_FAILED = -6,
    SETRLIMIT_FAILED    = -7,
    DUP2_FAILED         = -8,
    SETUID_FAILED       = -9,
    EXECVE_FAILED       = -10,
    SPJ_ERROR           = -11,
    COMPILE_FAIL        = -12 // TODO
};
```

注意发送的不是json数据,是MessageSendJudge这个类
在准备阶段,

```json
{
  "code": -1, // INVALID_CONFIG
  "msg": "unsupport language: haskell"
}
```

```json
{
  "code": -12, // COMPILE_FAIL
  "msg": "compile fail msg,eg: invalid #include"
}
```

```json
{
  "code": 0, //SUCCESS
  "msg": "allSize 3" //测试点的数量
}
```

发送每单个点的数据

```json
{
  code:"?"
  "msg":"1 p1.in p1.ans"
  "result" : [result]
}
```

最后发送一次全部的测试数据

```
{
  code:"?"
  "msg":"ALL",
  "result" : [result1,result2,result3]
}
```
