## 简介

`io_context`是整个库的核心,它有两个主要的功能:

1. 任务队列`queue listNode PostTask`,所有的异步任务,都存在这个队列里,等待被执行
2. 事件中心,通过`epoll`实现,监听事件

本质是一个大循环,当事件发生的时候调用对应的方法


## 类关系

```plaintext
IoCtxBase
  from_node_to_post_task()
  ListNode * get_node(PostTask * task)

  virtual void run()
  virtual void post_task()
  virtual void request_abort()
  virtual ~IoCtxBase()

  成员变量
  m_epoll_handle
  * m_meory_resource

IoCtx: public IoCtxBase 真正实现

  m_thread_wating
  m_task_queue_size
  m_task_queue !!任务队列PostTask

IoContext
  std::unique_ptr<IoCtxBase> m_ctx; //ctx的指针
  + IoContext(std::integral_constant<bool,multiple_thread>=std::true_type ()):159
  + get_io_ctx_base() 得到m_ctx
  + run() 开始事件循环
  + post_task(PostTask * task) 加入PostTask任务
  + request_abort() 不运行
  + get_memory_resource_for_task() 得到给task return object 使用的memory_resource
  + event_poll_handle() 得到event_poll 的 handle
  + IoContext(std::integral_constant<bool,multiple_thread>) 构造函数
```
## 简介

`io_context`是整个库的核心,它有两个主要的功能:

1. 任务队列`queue libstNode PostTask`,所有的异步任务,都存在这个队列里,等待被执行
2. 事件中心,通过`epoll`实现,监听事件


## 类关系

```plaintext
IoCtxBase
IoCtx: public IoContext
IoContext
  std::unique_ptr<IoCtxBase> m_ctx;
  run() 开始事件循环
  post_task() 
    - m_ctx:154
    + IoContext(std::integral_constant<bool,multiple_thread>=std::true_type ()):159
    + get_io_ctx_base():161
    + run():165
    + post_task(PostTask * task):171
    + request_abort():177
    + get_memory_resource_for_task():183
    + event_poll_handle():189
     IoContext(std::integral_constant<bool,multiple_thread>):243

```

## 核心的执行的过程:

在不考虑多线程的情况下,执行过程如下

- 取`m_task_queue`头`callback`
- 如果头不空, 调用`callback(task)`,执行task任务
- 否则:
  - 如果是多线程
    - 检查epoll事件中是否有`wake_event,effective event`
    - 如果`m_thread_waiting` --> `wakeup_a_thread()` ???
  - 调用`epoll`事件的回调

TODO

`m_thread_waiting`有什么用
