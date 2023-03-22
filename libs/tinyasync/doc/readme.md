# tinyasync

仓库地址: https://github.com/lhprojects/tinyasync

fork到我的仓库: https://github.com/rainboyOJ/tinyasync

一个由c++20协程编写的网络io库,想学习它的原理

## 文件结构

```plaintext
include
└── tinyasync
    ├── awaiters.h  各种的等待器,实现的协程的暂停
    ├── basics.h    所需的头文件,基础类,工具类的定义
    ├── buffer.h    buffer数组
    ├── dns_resolver.h  hostName 转 ip
    ├── io_context.h    核心,IO事件中心
    ├── memory_pool.h   内存池,内存分配
    ├── mutex.h         锁,队列锁,无锁队列
    ├── task.h          协程的Return Object 实现
    └── tinyasync.h     包含其它头文件
```

## 框架/组成/功能模块

### 功能模块

- [`IoContext`事件中心](./io_context.md)
- [awaiter](./awaiter.md),协程等待器,用于暂停协程

- [`tash.h`],功能库,设定了协程的相关,来控制协程的运行行为,包括,
  - 1.内存申请
  - 2.生命周期
  - 3.协程句柄,基类
  - 4.协程生命结束的行为
  - 非常有用的[`co_spawn`功能](./co_spawn.md)


## 概念/功能


整个代码由下面几个功能组合来完成整体的功能


`io_context`事件中心或者叫IO中心,基本上需要的任务或事件
都注册到它身上.

`buffer`,定义了两个类型的buffer来,提供内存buffer

- 1. `Buffer`,本质是真正buffer的wrapper
- 1. `ConstBuffer`,只读的buffer

各种`Aawaiter`:

- `AsyncReceiveAwaiter` 异步接收数据
- `AsyncSendAwaiter` 异步发送数据
- `Connector` 连接服务端
- `AcceptAwaiter` 接收连接
- `ConnectorAwaiter` 发送连接
- `TimerAwaiter` 异步等待

它们的作用,向ctx中心注册事件,等对应的事件发生时,使协程resume


`mutex.h`,实现了如下

无锁队列,如果你需要某些任务,不能现时的执行,那么你需要它
同时还实现了一些学用的锁
具体看[`mutex.h`解析](./mutext.md)

`dns_resolve` 异步的dns解析

`memory_pool`基于`pmr`的内存池

## 解析

- [`io_context`解析](./io_context.md)
- [`co_spawn`抛出不管协程](./co_spawn.md)


## 原理1: 不需要虚函数,实现多态

前提：

1. 只需要一个函数进行虚化，所以可以直接定义一个成员变量来存储指针


类的布局图

```
+--------------------+            +--------------------+    
|m_callback          |            |m_callback          |    
|                    | A          |                    | A  
+--------------------+            +--------------------+    
|b_do_call<type C1>()|            |b_do_call<type C1>()|    
|b_do_call<type C3>()|            |b_do_call<type C3>()|    
|b_do_call<type C2>()| B          |b_do_call<type C2>()| B  
|   ......           |            |   ......           |    
+--------------------+            +--------------------+    
|                    |            |                    |    
|on_callback()       | C1         |on_callback()       | C2 
|                    |            |                    |    
+--------------------+            +--------------------+    
```

代码:`./code_by_self/callback.cpp`

可以把所有的`c1,c2,c3`转成


TODO 补充完整

## PostTask

加入任务的队列的有哪些

- post_task 函数

`io_context`里有
`dns_resolve`里
`mutex.h`里有

需要压入任务的
`awaiter.h`
`mutex.h`
`dns_resolve.h`
