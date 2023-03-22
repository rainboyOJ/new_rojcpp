# task.h

主要功能:实现了`coroutine`协程的`return object` 实现,使对应的协程具有以下的性质

- 内存申请

类图
```plaintext

    struct TaskPromiseBase

    template<class Result, class Alloc = std::allocator<std::byte> >
    struct TaskPromise : TaskPromiseWithResult<Result>, TaskPromiseWithAllocator<Alloc> {


    template<class Result = void>
    class TINYASYNC_NODISCARD Task
        using promise_type = TaskPromiseWithResult<Result>;
        using coroutine_handle_type =  std::coroutine_handle<promise_type>;
        using result_type = Result;
```


## task的能力, `co_await`

因为Task重载了`co_await`,所以它可以像
一个`Awaiter`一样.

- `await_ready() -> false`直接挂起对应的协程
- `await_suspend`,设置了`m_h.promise().m_continuation`,返回了一个`m_h`
那么`m_h`对应的主协程,会resume

