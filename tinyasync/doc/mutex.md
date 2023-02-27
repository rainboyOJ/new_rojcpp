# mutex.h

主要功能,实现了一个无锁队列,保证那些需要按顺序执行的任务,按顺序执行

- Mutex
- AdoptUniqueLock
- WaitAwaiter
- Event,事件
- Condv

## LockCore

本质上是一个无锁队列,但它特殊在,如果是第一次添加项目,也就是在队列是空的条件下添加项目

它的核心操作如下

它是一个三元状态锁,对比于一般的无锁队列,有一点区别,**如何保证状态一样的时候,不会出错?**


### `try_lock`

函数原型:`bool try_lock(ListNode * p)`

前置知识`m_flags.comapre_exchange_strong(A,B)`,如果`m_flags == A`,那么
`m_flags`就会原子性的替换成B,这整个操作是原子的,无论成功与否,A都会变面`m_flags`
的值

```
0,0,0   初始状态
  |
  v
0,0,1   mtx锁上,que无锁
  |
  v
0,1,1   mtx,que都锁上
```

```plaintext
如果 mtx locked
  返回 true ,表明已经锁上
否则
  加入 把p加入队列
  return false
```

### unlock

解锁与取数据

函数原型

```
ListNode *unlock(bool unlock_mtx_only_if_que_empty)
```

```
如果 队列 非空
  如果 unlock_mtx_only_if_que_empty == false
    也就是不需要空的时候才删除mtx lock
    unlock mtx lock
  取出队列头 p
  返回 p
否则
  解锁 mtx lock  
  返回 nullptr
```

## Mutex

- `MutexLockAwaiter`
- `Mutex`

建立在`LockCore`上的一个锁,主要功能,

如果你在多个协程上`co_await Mutex.lock()`,那么这些协程会按顺序的`resume`

像队列一样,按顺序执行

那它是如果实现的呢?

`MutexLockAwaiter`挂起协程后

```plaintext
(1) if we don't acquire the mutex
this coroutine will be enqueued
如果我们没有获得mtx锁,那么这个协程句柄就会加入队列中

if the owner of the lock unlock the mutex
then this crooutine would be poped out
then this coroutine would be resumed

如果锁的拥有都unlock了锁,那么协程就会resume

if owner of the mutex unlock the mutex before this function return
如果 mutex的拥有者在这个函数返回前 unlock了
the coroutine will be resumed before this function return
那么这个协程会在这个函数返回前,resume

thus we should invoke the lock in await_suspend instead of await_ready
In await_ready the state of this cocoutine is not saved
因此,我们应该在 await_suspend 里执行lock,而不是在`await_ready`里执行
,因为 await_ready 的时候,协程的状态还没有保存

(2) if we acquire the mutex
then this coroutuine continue to run
如果我们获得锁,那协程继续运行
```

一个awaiter的`await_suspend`
- 如果返回true,挂起协程
- 返回false,恢复协程

返回

`try_lock` 加入了p,也就是没有获得锁,那么`await_suspend return !false`,
也就是挂起协程,如果`try_lock`返回`true`,也就是获得了锁,那就是`await_suspend return !true`
恢复协程,继续执行

## AdoptUniqueLock

TODO

## Awaiter,WaitCallback

- `wait(Mutex &mtx, Awaiter & awaiter)`函数

## Event,EventAwaiter

对于事件,我们应该关心下面的东西

- 事件如何定义:`Event myevent(ctx)`
- 事件如何监听:`co_await myevent`
- 事件如何发生:`myevent.notify_one()`
- 事件如何回调,没有,需要做的是等待事件的发生,才能继续执行


- `PostTaskEvent`
- `EventAwaiter`

核心功能:

0. `Event`队列中存有`EventAwaiter`队列,如果`co_await EventAwaiter`,会把`EventAwaiter`加入到队列中
1. `notify_one`,从队列中取出一个`awaiter`,放入到ctx的`post_task`队列中,这样就会执行一件事件
2. `notify_all`


作用?

## Condv

- `Condv`
- `CondvAwaiter`
