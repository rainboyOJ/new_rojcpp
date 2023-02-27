## `co_spawn`

如何管理协程的生命周期,当前是它自己管理自己了是好最的了.于是,我们希望协程可以做到如下的功能:

一个函数A(可以是普通函数或协程)内,可以**抛出不管**式的运行另一个协程B,那我们就需要这个`co_spawn`辅助协程函数.

那的实现代码在`tinyasync/task.h`内

`SpawnTask`这个`return_object`本质就是一个`initial_suspend() and final_suspend()`都是`nerver`.
所以`co_spawn`就是一个普通的协程.

我们知道如果一个普通函数A的内部,执行了一个协程C,本质上是在堆上申请了一块内存来存放协程的相关的状态.
如果协程C暂停了,那么可以拿到C的句柄,句柄本质上是上面申请的内存的指针.我们可以通过句柄来操作C协程:resume,destroy等.

这也是协程的本质(最简单的理解),如果我们创建了协程C,但不获取C的句柄(也就是不操作C),
那么这个协程不就是**自生自灭**的吗?当协程自己本身执行完毕的时候,协程C就消失了.


```cpp
Task<> myTask(){

}
co_spawn(myTask())
```

执行的流程

```plaintext

1. myTask() 执行 `initial_suspend` 挂起,产生一个return object : A
2. co_spawn拿到参数A,开始执行
3. co_spawn 执行 suspend_never initial_suspend(),不挂起
4. co_spawn 执行 co_await A
5. 执行 A.await_ready() -> false,挂起 co_spawn
6. 执行 A.await_suspend(co_spawn_handle) 设置 A.m_h.promise().m_continuation = co_spawn_handle
7. 执行 return A.m_h,挂起co_spawn
8. 执行 resume(A)
9. A 整个执行结束
10. 执行 A 的 finish_suspend(), 返回 m_continuation,挂起A,
11. co_spawn 执行 A await_resume(),得到结果(没有用这个结果)
12. co_spawn ,suspend_never final_suspend, co_spawn 结束,A,析构,整个A协程删除
13. co_spawn 协程句柄删除
```
