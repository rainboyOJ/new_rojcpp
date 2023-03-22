
#ifndef TINYASYNC_MUTEX_H
#define TINYASYNC_MUTEX_H

namespace tinyasync
{


    class LockCore
    {
    public:
        static constexpr int k_mtx_locked = 1;  // 锁1: mtx锁
        static constexpr int k_que_locked = 2;  // 锁2: 队列锁
        static constexpr int k_que_notempty = 4;// 标记锁,队列为空

        Queue m_que; // 基于LinkNode的队列
        std::atomic<int> m_flags = 0;   // 标记锁,包含上面3个锁

#ifndef NDEBUG
        std::atomic<int> que_thd_cnt = 0;
        std::atomic<int> mtx_thd_cnt = 0;
        std::atomic<int> mtx_lock_cnt = 0; //  mtx锁的次数
#endif

        LockCore()
        {
        }

        LockCore(LockCore &&) = delete;
        LockCore operator=(LockCore &&) = delete;

        std::size_t _count()    // 队列内的项目数
        {
            return m_que.count();
        }

        static int bit_set(int flags, int bit) //bit 操作
        {
            return flags | bit;
        }
        static int bit_clear(int flags, int bit)
        {
            return flags & (~bit);
        }
        static int bit_test(int flags, int bit)
        {
            return flags & bit;
        }

        // correct if you own the lock
        // just a hit if you don't have the lock
        bool is_locked()
        {
            return bit_test(m_flags.load(std::memory_order_relaxed), k_mtx_locked);
        }

        // try lock
        // atomiclly do following
        // if mutex locked
        //     return true
        // else
        //     enqueue p
        //     return false
        bool try_lock(ListNode *p)
        {

            // lock either queue or mutex
            // lock queue is for push
            // lock queue only when mutex is really (not false) locked
            // ensure following:
            // push queue  -before-   mutex unlock   -before-   pop queue
            int old_flags = m_flags.load(std::memory_order_relaxed);
            int flags = 0;
            int enqueue = false;

            //下面的操作,保证运行结束后
            //要么 单独的mtx被锁上,que不被锁, 0,0,1
            //要么 mtx,que都被锁   0,1,1
            do
            {
                flags = bit_set(old_flags, k_mtx_locked); // 标记,加入mtx锁
                enqueue = flags == old_flags;
                if (enqueue) // mtx锁已经锁上,
                {
                    // mutex already locked
                    // assume queue is not locked
                    flags = bit_set(old_flags, k_que_locked); // 标记,加入que锁
                    old_flags = bit_clear(old_flags, k_que_locked);
                }
                else
                {
                    // mutex not locked
                    // we lock it
                }
            } while (!m_flags.compare_exchange_strong(old_flags, flags,
                                                      std::memory_order_acq_rel,
                                                      std::memory_order_relaxed));

            // we have locked something
            if (enqueue)
            {

#ifndef NDEBUG
                //enqueue==true 表明前一状态必须是 mtx_lock
                // 后面 que 必须被locked
                // enqueue 为true 就表示 从 001 -> 011 状态改变,
                // 这只能发生一次
                assert(bit_test(m_flags.load(), k_que_locked));
                // queue locked
                que_thd_cnt += 1;
                if (que_thd_cnt > 1) //被锁的次数必须是1次
                {
                    exit(1);
                }
#endif

                m_que.push(p); //加入 p 到队列

#ifndef NDEBUG
                que_thd_cnt -= 1; // 取出cnt
#endif

                // unlock queue
                // note: you can't assume mutex is locked or not
                int old_flags = m_flags.load(std::memory_order_relaxed);
                int flags = 0;
                do
                {
                    flags = bit_clear(old_flags, k_que_locked); // 去除队列锁
                    flags = bit_set(flags, k_que_notempty); // 队列非空标记
                } while (!m_flags.compare_exchange_strong(old_flags, flags,
                                                          std::memory_order_acq_rel,
                                                          std::memory_order_relaxed));

                return false; // false, 表示加入队列成功
            }
            else // 原来 mtx 没有锁, 现在mtx已经锁上
            {
#ifndef NDEBUG
                assert(bit_test(m_flags.load(), k_mtx_locked));

                // mutex locked
                mtx_thd_cnt += 1;
                if (mtx_thd_cnt == 2) //只能发生一次
                {
                    exit(1);
                }

                ++mtx_lock_cnt;
#endif
            }

            return !enqueue; // !enqueue 也就是true,表示mtx已经锁上,但是p没有加入队列
        }

        // unlock a locked mutex
        // atomiclly do following
        // if queue is not empty
        //     if !unlock_mtx_only_if_que_empty
        //        unlock mutex
        //     p = dequeue
        //     return p
        //  else
        //    unlock mutex
        //    return nullptr
        ListNode *unlock(bool unlock_mtx_only_if_que_empty)
        {

            bool que_empty = false; //空标记
            int flags = 0;
            int old_flags = m_flags.load(std::memory_order_relaxed);
            do
            {
#ifndef NDEBUG
                mtx_thd_cnt -= 1;
                assert(bit_set(old_flags, k_mtx_locked));  // ?? bit_set?
#endif
                que_empty = !bit_test(old_flags, k_que_notempty); // 队列是否空
                if (que_empty) //队列空
                {
                    // empty queue?
                    // unlock mutex
                    flags = bit_clear(old_flags, k_mtx_locked);
                }
                else //队列非空
                {
                    // not empty queue?
                    // and lock queue

                    flags = bit_set(old_flags, k_que_locked); // 锁定queue
                    if (!unlock_mtx_only_if_que_empty)
                    {
                        // unlock mutex
                        flags = bit_clear(flags, k_mtx_locked);
                    }
                    else
                    {
                        // keep locked
#ifndef NDEBUG
                        mtx_thd_cnt += 1;
#endif
                    }
                    old_flags = bit_clear(old_flags, k_que_locked); // 队列列锁
                }
            } while (!m_flags.compare_exchange_strong(old_flags, flags,
                                                      std::memory_order_acq_rel,
                                                      std::memory_order_relaxed));

            if (que_empty)
            {
                return nullptr;
            }

            // queue locked
#ifndef NDEBUG
            if (unlock_mtx_only_if_que_empty)
            {
                assert(bit_test(m_flags.load(), k_mtx_locked));
            }
            assert(bit_test(m_flags.load(), k_que_locked));

            que_thd_cnt += 1;
            if (que_thd_cnt > 1)
            {
                exit(1);
            }
#endif

            // dequeue
            auto head = m_que.pop_nocheck(que_empty);
            // 调用pop_nocheck 会改变 que_empty的值

#ifndef NDEBUG
            que_thd_cnt -= 1;
#endif

            // unlock queue
            // note: you can't assume mutex is locked or not
            old_flags = m_flags.load(std::memory_order_relaxed);
            flags = 0;
            do
            {
                flags = bit_clear(old_flags, k_que_locked);
                if (que_empty)
                {
                    flags = bit_clear(flags, k_que_notempty);
                }
            } while (!m_flags.compare_exchange_strong(old_flags, flags,
                                                      std::memory_order_acq_rel,
                                                      std::memory_order_relaxed));

            //printf("que unlocked(unlock)\n");

            return head;
        }
    };

    class Mutex;
    class MutexLockAwaiter;
 
    class MutexLockAwaiter
    {
    public:
        ListNode m_node; //存到队列上
        Mutex *m_mutex; // Mutex指针
        IoCtxBase *m_ctx;
        std::coroutine_handle<TaskPromiseBase> m_suspended_coroutine = nullptr;
        PostTask m_posttask;

        static MutexLockAwaiter *from_node(ListNode *node)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
            return (MutexLockAwaiter *)((char *)node - offsetof(MutexLockAwaiter, m_node));
#pragma GCC diagnostic pop
        }

        static void on_callback(PostTask *);

        template<class Promise>
        auto await_suspend(std::coroutine_handle<Promise> suspended_coroutine)
        {
            auto suspended_coroutine_base = suspended_coroutine.promise().coroutine_handle_base();
            return await_suspend(suspended_coroutine_base);
        }

        MutexLockAwaiter(Mutex &mutex, IoCtxBase &ctx);
        // 返回 false 直接挂起协程
        bool await_ready() noexcept;
        bool await_suspend(std::coroutine_handle<TaskPromiseBase> suspend_coroutine);
        void await_resume();
    };

    class Mutex
    {
    public:
        LockCore m_lockcore; //一个无锁队列


        // 发生lock时,返回一个MutexLockAwaiter
        MutexLockAwaiter lock(IoContext &ctx)
        {
            return {*this, *ctx.get_io_ctx_base()};
        }

        // LockCore 的 mtx 是否被锁上
        bool is_locked()
        {
            return m_lockcore.is_locked();
        }
        void unlock(); //解锁

    };

    inline bool MutexLockAwaiter::await_ready() noexcept
    {
        return false;
    }

    //返回值
    inline bool MutexLockAwaiter::await_suspend(std::coroutine_handle<TaskPromiseBase> suspend_coroutine)
    {
        m_suspended_coroutine = suspend_coroutine;
        auto mutex = m_mutex;

        // (1) if we don't acquire the mutex
        // this coroutine will be enqueued

        // if the owner of the lock unlock the mutex
        // then this coroutine would be poped out
        // then this coroutine would be resumed

        // if owner of the mutex unlock the mutex before this function return
        // the coroutine will be resumed before this function return

        // thus we should invoke the lock in await_suspend instead of await_ready
        // In await_ready the state of this cocoutine is not saved

        // (2) if we acquire the mutex
        // then this coroutuine continue to run

        bool own_mutex = mutex->m_lockcore.try_lock(&m_node);
        return !own_mutex;
    }

    inline void MutexLockAwaiter::on_callback(PostTask *posttask)
    {
        TINYASYNC_GUARD("MutexTaskCallback::on_callback");
        auto awaiter = (MutexLockAwaiter*)((char*)posttask - offsetof(MutexLockAwaiter, m_posttask));
        TINYASYNC_ASSERT(awaiter->m_mutex->is_locked());

        //
        TINYASYNC_RESUME(awaiter->m_suspended_coroutine);
    }

    inline void MutexLockAwaiter::await_resume()
    {
        TINYASYNC_ASSERT(m_mutex->is_locked());
    }

    inline MutexLockAwaiter::MutexLockAwaiter(Mutex &mutex, IoCtxBase &ctx) : m_mutex(&mutex), m_ctx(&ctx)
    {            
    }

    inline void Mutex::unlock()
    {
        TINYASYNC_GUARD("Mutex::unlock(): ");
        auto *node = m_lockcore.unlock(true);
        TINYASYNC_LOG("node = %p", node);
        if (node)
        {
            // still locked
            // pop from mutex list
            MutexLockAwaiter *awaiter = MutexLockAwaiter::from_node(node);            
            // insert into ctx's task list
            awaiter->m_posttask.set_callback(MutexLockAwaiter::on_callback);  
            awaiter->m_ctx->post_task(&awaiter->m_posttask);
        }
        else
        {
            // unlokced
        }
    }

    template <typename _Mutex>
    class AdoptUniqueLock
    {
    public:
        typedef _Mutex mutex_type;

        AdoptUniqueLock(_Mutex &__m) noexcept : m_mtx(&__m)
        {
            m_owned = true;
        }

        ~AdoptUniqueLock()
        {
            if (m_owned)
                m_mtx->unlock();
        }

        void unlock()
        {
            if (m_owned) {
                m_owned = false;
                m_mtx->unlock();
            }
        }

        AdoptUniqueLock(const AdoptUniqueLock &) = delete;
        AdoptUniqueLock &operator=(const AdoptUniqueLock &) = delete;

        AdoptUniqueLock(AdoptUniqueLock &&r)
        {
            m_mtx = r.m_mtx;
            m_owned = r.m_owned;
            r.m_owned = false;
        }

        AdoptUniqueLock &operator=(AdoptUniqueLock &&r)
        {
            if (m_owned) {
                m_mtx->unlock();
            }
            m_mtx = r.m_mtx;
            m_owned = r.m_owned;
            r.m_owned = false;
        }

    private:
        _Mutex *m_mtx;
        bool m_owned;
    };

    AdoptUniqueLock<Mutex> auto_unlock(Mutex &mtx)
    {
        return mtx;
    }

    template <class Awaiter>
    class WaitAwaiter;

    template <class Awaiter>
    struct WaitCallback : CallbackImplBase // 给ctx的 event.data.ptr 使用
    {

        typename Awaiter::callback_type *m_callback;
        WaitAwaiter<Awaiter> *m_awaiter;
        MutexLockAwaiter m_mutex_lock_awaiter;
        std::coroutine_handle<TaskPromiseBase> m_suspended_coroutine;

        WaitCallback(WaitAwaiter<Awaiter> *awaiter) :
            CallbackImplBase(this),
            m_mutex_lock_awaiter(*awaiter->m_mtx)
        {
        }

        void on_callback(IoEvent &evt)
        {
            TINYASYNC_GUARD("WaitCallback:on_callback(): ");
            auto suspend = m_mutex_lock_awaiter.await_suspend(m_suspended_coroutine);
            if (!suspend)
            {
                TINYASYNC_LOG("locked");                
                // locked
                // invoke on_callback directly
                m_callback->on_callback(evt);
            }
        }
    };


    template <class Awaiter>
    class WaitAwaiter
    {
    public:
        Awaiter *m_awaiter;
        Mutex *m_mtx;
        //^v^v^v initialization order barrier ^v^v^v
        WaitCallback<Awaiter> m_callback_ {this};
        Callback *m_callback = &m_callback_;

        using callback_type = WaitCallback<Awaiter>;

        WaitCallback<Awaiter> *get_callback() {
            return &m_callback_;
        }

        void set_callback(Callback *c) {
            m_callback = c;
        }

        std::atomic<int> m_flags = false;

        WaitAwaiter(Awaiter &awaiter, Mutex &mtx)
            : m_awaiter(&awaiter),
              m_mtx(&mtx)
        {

        }

        WaitAwaiter(WaitAwaiter &&) = delete;
        WaitAwaiter operator=(WaitAwaiter &&) = delete;

        bool await_ready() const
        {
            return m_awaiter->await_ready();
        }

        template<class Promise>
        void await_suspend(std::coroutine_handle<Promise> h) {
            auto base = h.promise().coroutine_handle_base();
            await_suspend(base);
        }

        void await_suspend(std::coroutine_handle<TaskPromiseBase> h)
        {
            auto awaiter = m_awaiter;

            this->m_callback_.m_suspended_coroutine = h;
            // hook
            // epoll_wait -> ... -> this->m_ballback_   -> awaiter->m_callback  ->  awaiter->await_resume()
            this->m_callback_.m_callback = awaiter->get_callback();

            awaiter->set_callback(this->m_callback);
            awaiter->await_suspend(h);

            m_mtx->unlock();
        }

        size_t await_resume()
        {
            m_awaiter->await_resume();
            return 0;
        }
    };

    template <class Awaiter>
    auto wait(Mutex &mtx, Awaiter &&awaiter) -> WaitAwaiter<std::remove_reference_t<Awaiter> >
    {
        return {awaiter, mtx};
    }

    class Event;
    class EventAwaiter;

    template<class Trait>
    class Condv;

    template<class Condv>
    class CondvAwaiter;

    struct PostTaskEvent : PostTask
    {
        ListNode *m_awaiters;
    };

    class Event
    {
    public:
        Queue m_awaiter_que;        // 队列
        IoContext *m_ctx = nullptr; // ctx

        Event(IoContext &ctx)
        {
            m_ctx = &ctx;
        }

        EventAwaiter operator co_await();   // 调用 EventAwaiter


        static void on_notify(PostTask *postask);

        void notify_one() // 通知一个
        {
            TINYASYNC_GUARD("Event::notify_one(): ");
            TINYASYNC_LOG("");
    
            auto awaiter = this->m_awaiter_que.pop();

            if(awaiter) {
                auto posttask = new PostTaskEvent();
                awaiter->m_next = nullptr;
                posttask->m_awaiters = awaiter; // 指向 队列中取出的 ListNode
                posttask->set_callback(on_notify); // callback 回调
                m_ctx->post_task(posttask); // 加入ctx 任务队列
            }
        }

        void notify_all() // 
        {
            TINYASYNC_GUARD("Event::notify_all(): ");
            TINYASYNC_LOG("");

            auto posttask = new PostTaskEvent();

            posttask->m_awaiters = this->m_awaiter_que.m_before_head.m_next;

            // 清空队列
            this->m_awaiter_que.m_before_head.m_next = nullptr; // 清空队列
            this->m_awaiter_que.m_tail = nullptr;

            posttask->set_callback(on_notify);
            m_ctx->post_task(posttask);
        }

    private:
    };

    class EventAwaiter
    {
        friend class Event;
        ListNode m_node;
        Event *m_event = nullptr;
        std::coroutine_handle<TaskPromiseBase> m_resume_coroutine = nullptr;

    public:

        static EventAwaiter *from_node(ListNode *node) {
            return (EventAwaiter*)((char*)node - offsetof(EventAwaiter,m_node));
        }

        EventAwaiter(Event &evt)
        {
            m_event = &evt;
        }

        bool await_ready()
        {
            return false;
        }

        template <class Promise>
        void await_suspend(std::coroutine_handle<Promise> h)
        {
            await_suspend(h.promise().coroutine_handle_base());
        }

        void await_suspend(std::coroutine_handle<TaskPromiseBase> h);

        void await_resume();
    };

    //ctx任务执行
    inline void Event::on_notify(PostTask *postask)
    {
        // 记录 指针 m_awaiters
        ListNode *node = ((PostTaskEvent*)postask)->m_awaiters;
        delete postask; // 删除 内存

        for(;node; node = node->m_next) {
            EventAwaiter *awaiter = EventAwaiter::from_node(node); // 转换成 EventAwaiter
            TINYASYNC_RESUME(awaiter->m_resume_coroutine); // resume
        }
    }

    inline void EventAwaiter::await_suspend(std::coroutine_handle<TaskPromiseBase> h)
    {
        TINYASYNC_GUARD("EventAwaiter::await_suspend(): ");
        TINYASYNC_LOG("");

        auto evt = m_event;

        // insert into the head of awaiter list
        evt->m_awaiter_que.push(&this->m_node);
        m_resume_coroutine = h;
    }

    inline void EventAwaiter::await_resume()
    {
        TINYASYNC_GUARD("EventAwaiter::await_resume(): ");
        TINYASYNC_LOG("");
    }

    inline EventAwaiter Event::operator co_await()
    {
        return {*this};
    }



   template<class Trait = MultiThreadTrait>
   class Condv
{
    public:
        using mutex_type = typename Trait::spinlock_type;
        Queue m_awaiter_que;
        mutex_type m_native_mutex;
        IoCtxBase *m_ctx = nullptr;

        Condv(IoContext &ctx)
        {
            m_ctx = ctx.get_io_ctx_base();
        }


        CondvAwaiter<Condv> wait(Mutex &mtx);


        static void on_notify(PostTask *postask);

        void notify_one()
        {
            TINYASYNC_GUARD("Event::notify_one(): ");
            TINYASYNC_LOG("");
    

            auto posttask = new PostTaskEvent();

            m_native_mutex.lock();
            auto awaiter = this->m_awaiter_que.pop();
            m_native_mutex.unlock();

            if(awaiter) {
                TINYASYNC_LOG("has awaiter");
                awaiter->m_next = nullptr;
                posttask->m_awaiters = awaiter;
                posttask->set_callback(on_notify);
                m_ctx->post_task(posttask);
            } else {
                TINYASYNC_LOG("no awaiter");
            }
            

        }

        void notify_all()
        {
            TINYASYNC_GUARD("Event::notify_all(): ");
            TINYASYNC_LOG("");

            auto posttask = new PostTaskEvent();

            m_native_mutex.lock();
            auto awaiter = this->m_awaiter_que.m_before_head.m_next;
            this->m_awaiter_que.m_before_head.m_next = nullptr;
            this->m_awaiter_que.m_tail = nullptr;
            m_native_mutex.unlock();

            if(awaiter) {
                posttask->m_awaiters = awaiter;
                posttask->set_callback(on_notify);
                m_ctx->post_task(posttask);
            }

            
        }

    };

    using ConditionVariable = Condv<>;

    template<class Condv>
    class CondvAwaiter
    {
    public:
        ListNode m_node;
        Condv *m_condv = nullptr;
        Mutex *m_mtx;
        MutexLockAwaiter m_mutex_lock_awaiter;
        std::coroutine_handle<TaskPromiseBase> m_resume_coroutine = nullptr;


        static CondvAwaiter *from_node(ListNode *node) {
            return (CondvAwaiter*)((char*)node - offsetof(CondvAwaiter<Condv>,m_node));
        }

        CondvAwaiter(Condv &evt, Mutex &mtx) : m_mutex_lock_awaiter(mtx, *evt.m_ctx)
        {
            m_condv = &evt;
            m_mtx = &mtx;
        }

        bool await_ready()
        {
            return false;
        }

        template <class Promise>
        void await_suspend(std::coroutine_handle<Promise> suspend_coroutine)
        {
            auto h = suspend_coroutine.promise().coroutine_handle_base();
            await_suspend(h);
        }

        void await_suspend(std::coroutine_handle<TaskPromiseBase> h);

        void await_resume();
    };

    template<class Trait>
    inline void Condv<Trait>::on_notify(PostTask *postask)
    {
        using Condv = CondvAwaiter<Trait>;
        ListNode *node = ((PostTaskEvent*)postask)->m_awaiters;
        delete postask;

        for(;node; node = node->m_next) {
            CondvAwaiter<Condv> *awaiter = CondvAwaiter<Condv>::from_node(node);
            if(!awaiter->m_mutex_lock_awaiter.await_suspend(awaiter->m_resume_coroutine)) {
                TINYASYNC_RESUME(awaiter->m_resume_coroutine);
            }
        }
    }

    template<class Trait>
    CondvAwaiter<Condv<Trait> > Condv<Trait>::wait(Mutex &mtx)
    {
        return {*this, mtx};
    }

    template<class Condv>
    inline void CondvAwaiter<Condv>::await_suspend(std::coroutine_handle<TaskPromiseBase> h)
    {
        TINYASYNC_GUARD("EventAwaiter::await_suspend(): ");
        TINYASYNC_LOG("");

        m_resume_coroutine = h;

        auto evt = m_condv;
        // insert into the head of awaiter list
        evt->m_native_mutex.lock();
        evt->m_awaiter_que.push(&this->m_node);
        evt->m_native_mutex.unlock();
        
        Mutex *mtx = this->m_mtx;
        TINYASYNC_ASSERT(mtx->is_locked());
        mtx->unlock();
    }

    template<class Condv>
    inline void CondvAwaiter<Condv>::await_resume()
    {
        TINYASYNC_GUARD("EventAwaiter::await_resume(): ");
        TINYASYNC_LOG("");
    }



} // namespace tinyasync

#endif
