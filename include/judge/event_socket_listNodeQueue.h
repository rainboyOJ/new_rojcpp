/**
 * 创建一个map
 * map<socket,Queue> 用来存event的listNode
 */
#pragma once

#include <map>
#include <memory>
#include "tinyasync/basics.h"
#include "tinyasync/io_context.h"

    using _SID_ = int; //sid == submit id
class Event_listNodeQue_map {
    using Queue_ptr = std::shared_ptr<tinyasync::Queue>;

    private:

        std::map<_SID_,Queue_ptr> __map;

    public:
        
        //是否有对应的事件
        bool has_Event(_SID_);

        Queue_ptr get_Event(_SID_);

        void push_Event(_SID_,tinyasync::ListNode * ln);
};

//定义websocket事件,当有对应的judgeServer 返回事件发生的时候
//judgeCenter 得到返回的评测信息,并通知对应的ctx中心,处理对应的webSocketEvent
//
class webSocketEvent;
class webSocketEventAwaiter;

namespace  tinyasync {


class webSocketEvent {
    public:
        IoContext * m_ctx = nullptr;

        webSocketEvent(IoContext & ctx,_SID_ sid);

        webSocketEvent operator co_await();

        static void on_notify(PostTask * posttask);



        //通知所有的Event信息
        static void notify_all(int socket);

};


class webSocketEventAwaiter {
    friend webSocketEvent;

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

} // end namespace tinyasync
