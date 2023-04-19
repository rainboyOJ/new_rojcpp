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
