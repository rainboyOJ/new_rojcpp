# awaiter的解析

各种awaiter的作用就是配合协程,使协程挂起,当然挂起时,把回调(`resume`)
注册到`IoCtx`事件中心上,当相应的事件发生的时候,就会唤醒对应的协程.

核心的`Awaiter`如下

- Acceptor,等待连接,产生`Connect`
- Connect,
  - `AsyncRecv`,发送
  - `AsyncSend`,接收

## 类图谱

地址相关
```plaintext
struct Protocol //v4 v6 协议
enum class AddressType 协议类型
struct Address 地址抽象
struct Endpoint 地址+端口
```

辅助函数

```plaintext
void setnonblocking(fd) 设定socket无阻塞
TODO 什么是无阻塞

address_v4_from_string 转换,工厂函数
address_v6_from_string 转换,工厂函数

bind_socket(socket,Endpoint) socket与端口,地址的绑定,
  用于Acceptor的创建

```

```plaintext
DataAwaiterMixin 一个Awaiter需要的数据 
  Awaiter* m_next    下一个awaiter指针
  IoCtxBase * m_ctx  ioctx 指针
  m_conn connect指针
  m_suspend_coroutine 协程句柄
  m_buffer_addr  buff地址,buff的作用????
  m_buffer_size  buff大小
  m_bytes_transfer 字节接收/发送的数量
  m_suspend_return return时是否挂起?
  static constexpr k_closed_socket_ready 已经关闭socket?
```

异步的接收
```plaintext
class TINYASYNC_NODISCARD AsyncReceiveAwaiter :
    public DataAwaiterMixin<AsyncReceiveAwaiter, void*>
```
作用

如果`connect`已经准备好接收了,那就接收,不会挂起协程,否则挂起协程,等待接收.接收的数据存在Awaiter的buff里

需要注意的是`connect->next`的含义


异步的发送`AsyncSendAwaiter`


## Connection

`struct ConnImpl`底层实现

- `register()` 把`connect`注册到`IoCtx上`

这里使用了`EPOLLONESHOT`,为什么要使用这个,看下面的解释

[epoll的LT和ET使用EPOLLONESHOT - 走看看](http://t.zoukankan.com/kex1n-p-7451069.html)

一个conn可能有多个`Awaiter`

```plaintext
- m_ctx --> 
- m_conn_handle --> 
- m_callback --> 
- m_recv_awaiter --> 
- m_send_awaiter --> 
- m_post_task --> 
- m_ref_cnt --> 引用的数量,当为0时,会删除conn的占用的内存
- m_added_to_event_pool --> 
- m_recv_shutdown --> 
- m_send_shutdown --> 
- m_ready_to_send --> 
- m_ready_to_recv --> 
- m_tcp_nodelay --> 
+ native_handle() --> 
+ register_() --> 
+ reset() --> 
+ on_callback(Callback * callback,IoEvent & evt) --> 
+ ConnImpl(IoCtxBase & ctx,NativeSocket conn_sock,bool added_event_poll) --> 
+ ConnImpl(ConnImpl const &) --> 
+ operator =(ConnImpl const &) --> 
+ set_tcp_no_delay(bool b=true) --> 
+ async_read(void * buffer,std::size_t bytes) --> 
+ async_send(void const * buffer,std::size_t bytes) --> 
+ is_any_shutdown() const --> 
+ is_recv_shutdown() const --> 
+ is_send_shutdown() const --> 
+ is_recv_send_shutdown() const --> 
+ safe_shutdown_recv() --> 
+ safe_shutdown_send() --> 
+ safe_shutdown_recv_send() --> 
+ shutdown_recv() -->  调用::shutdown(conn_handle,SHUT_RD)
+ shutdown_send() -->  调用::shutdown(conn_handle,SHUT_WR)
+ shutdown_recv_send() --> 
+ safe_close() --> 
+ close() --> 
+ ~ConnImpl() --> 
 on_callback(Callback * callback,IoEvent & evt) --> 
+ wakeup_awaiter_on_close(PostTask * posttask) --> 
   this_type --> 
```

`Connection`的定义,基本上是对`ConnImpl`的wrapper


```plaintext
- m_impl --> ConnImpl's unique_ptr
+ Connection() --> 
+ Connection(Connection &&) --> 
+ operator =(Connection &&) --> 
+ Connection(IoCtxBase & ctx,NativeSocket conn_sock,bool added_event_poll) --> 
+ ~Connection() --> 
+ set_tcp_no_delay(bool b=true) --> 
+ is_closed() --> 
+ is_recv_shutdown() --> 
+ is_send_shutdown() --> 
+ is_recv_send_shutdown() --> 
+ shutdown_recv() --> 
+ shutdown_send() --> 
+ shutdown_recv_send() --> 
+ safe_shutdown_recv() --> 
+ safe_shutdown_send() --> 
+ safe_shutdown_recv_send() --> 
+ safe_close() --> 
+ close() --> 
+ native_handle() --> 
+ async_read(void * buffer,std::size_t bytes) --> 
+ async_read(Buffer const & buffer) --> 
+ async_send(void const * buffer,std::size_t bytes) --> 
+ async_send(ConstBuffer const & buffer) --> 
```

## 需要SocketMixin的类

`SocketMixin`一些`socket`需要的数据

```
    ~ m_ctx -->  //ioctx
    ~ m_protocol --> 协议
    ~ m_endpoint --> 端口与地址
    ~ m_socket --> socket df
    ~ m_added_to_event_pool --> 标志
    + reset_io_context(IoCtxBase & ctx,SocketMixin & socket) --> 重置
    + native_handle() const --> socket df
    + SocketMixin(IoCtxBase & ctx) --> 
    + SocketMixin() --> 
    + open(Protocol const & protocol,bool blocking=false) -->  //创建一个socket fd
    + reset() --> 清空

```
- `AccopterImpl`
- `ConnectorImpl`

## `Acceptor`相关
定义
```

class AcceptorCallback : public CallbackImplBase


class AcceptorImpl : SocketMixin
    - m_awaiter_que --> awaiter 队列
    - m_callback --> event回调
    + AcceptorImpl(IoCtxBase & ctx) --> 
    + AcceptorImpl() --> 
    + AcceptorImpl(Protocol const & protocol,Endpoint const & endpoint) --> 
    + AcceptorImpl(IoCtxBase & ctx,Protocol const & protocol,Endpoint const & endpoint) --> 
    + AcceptorImpl(AcceptorImpl const &) --> 
    + operator =(AcceptorImpl const &) --> 
    + ~AcceptorImpl() --> 
    + init(IoCtxBase *,Protocol const & protocol,Endpoint const & endpoint) --> 
    + reset_io_context(IoCtxBase & ctx,AcceptorImpl & r) --> 
    + listen() -->  调用::listen
    + async_accept() --> 

```
`Acceptor`是对`AccopterImpl`的wrapper

```plaintext

class TINYASYNC_NODISCARD AcceptorAwaiter

resume时返回 Connect
```

## Connection的实现
