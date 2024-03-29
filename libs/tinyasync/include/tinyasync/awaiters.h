#ifndef TINYASYNC_AWAITERS_H
#define TINYASYNC_AWAITERS_H

namespace tinyasync {

    struct Protocol
    {

        static Protocol ip_v4()
        {
            return {};
        }
    };

    static_assert(is_trivial_parameter_in_itanium_abi<Protocol>::value);

    enum class AddressType {
        IpV4,
        IpV6,
    };

    struct Address
    {

        // ip_v32 host byte order
        Address(uint32_t v32)
        {
            m_int4 = htonl(v32);
            m_address_type = AddressType::IpV4;
        }

        // if we bind on this address,
        // We will listen to all network card(s)
        Address()
        {

            if(INADDR_ANY != 0) {
                m_int4 = htonl(INADDR_ANY);
            } else {
                m_int4 = 0;
            }
            m_address_type = AddressType::IpV4;
        }

        std::string to_string() const
        {
            char buf[256];
#if _WIN32
            //
#elif defined(__unix__)
            if (m_address_type == AddressType::IpV4) {
                inet_ntop(AF_INET, &m_addr4, buf, sizeof(buf));
                return buf;
            } else if (m_address_type == AddressType::IpV6) {
                inet_ntop(AF_INET6, &m_addr6, buf, sizeof(buf));
                return buf;
            }
#endif
            return "";
        }

        static Address Any()
        {
            return {};
        }

        union
        {
            uint32_t m_int4;
            in_addr m_addr4;
            in6_addr m_addr6;

        };
        AddressType m_address_type;
    };
    static_assert(is_trivial_parameter_in_itanium_abi<Address>::value);
    static_assert(has_trivial_five<Address>::value);

    struct Endpoint
    {
        Endpoint(Address address, uint16_t port)
        {
            m_address = address;
            m_port = port;
        }
        Endpoint() : Endpoint(Address(), 0)
        {
        }

        Address address() const noexcept {
            return m_address;
        }

        uint16_t port() const noexcept { 
            return m_port;
        }
        
    private:
        Address m_address;
        uint16_t m_port;
    };
    static_assert(is_trivial_parameter_in_itanium_abi<Endpoint>::value);
    static_assert(has_trivial_five<Endpoint>::value);

    inline void setnonblocking(NativeSocket fd)
    {
#ifdef __WIN32
        //
#elif defined(__unix__)
        //int status = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
        int status = fcntl(fd, F_SETFL, O_NONBLOCK);

        if (status == -1) {
            throw std::system_error(errno, std::system_category(), "can't set fd nonblocking");
        }
#endif
    }


    inline Address address_v4_from_string(char const *str)
    {
        in_addr addr;        
        if(inet_pton(AF_INET, str, &addr) < 0) {
            throw_errno("address_v4_from_string");
        }
        Address addr_;
        addr_.m_addr4 = addr;
        addr_.m_address_type = AddressType::IpV4;
        return addr_;
    }
    inline Address address_v6_from_string(char const *str)
    {
        in6_addr addr;      
        if(inet_pton(AF_INET6, str, &addr) < 0) {
            throw_errno("address_v6_from_string");
        }
        Address addr_;
        addr_.m_addr6 = addr;
        addr_.m_address_type = AddressType::IpV6;
        return addr_;
    }

    inline NativeSocket open_socket(Protocol const& protocol, bool blocking = false)
    {
#ifdef TINYASYNC_THROW_ON_OPEN
        throw_error("throw on open", 0);
#endif
        TINYASYNC_GUARD("open_socket(): ");


#ifdef _WIN32

        DWORD flags = 0;
        if (!blocking) {
            flags = WSA_FLAG_OVERLAPPED;
        }

        auto socket_ = ::WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, flags);
        if (socket_ == INVALID_SOCKET) {
            throw_WASError("can't create socket");
        }

#elif defined(__unix__)
        // PF means protocol
        auto socket_ = ::socket(PF_INET, SOCK_STREAM, 0);
        if (socket_ == -1) {
            throw_errno("can't create socket");
        }
        if (!blocking)
            setnonblocking(socket_);
#endif

        TINYASYNC_LOG("create socket %s, nonblocking = %d", socket_c_str(socket_), int(!blocking));
        return socket_;

    }

    inline void bind_socket(NativeSocket socket, Endpoint const& endpoint)
    {

        TINYASYNC_GUARD("bind_socket(): ");
        TINYASYNC_LOG("socket = %s", socket_c_str(socket));


        int binderr;
        if(endpoint.address().m_address_type == AddressType::IpV4)
        {
            sockaddr_in serveraddr;
            memset(&serveraddr, 0, sizeof(serveraddr));
            serveraddr.sin_family = AF_INET;
            serveraddr.sin_port = htons(endpoint.port());
            serveraddr.sin_addr.s_addr = endpoint.address().m_addr4.s_addr;
            binderr = ::bind(socket, (sockaddr*)&serveraddr, sizeof(serveraddr));

        } else if(endpoint.address().m_address_type == AddressType::IpV6) {
            sockaddr_in6 serveraddr;
            memset(&serveraddr, 0, sizeof(serveraddr));
            serveraddr.sin6_family = AF_INET6;
            serveraddr.sin6_port = htons(endpoint.port());
            serveraddr.sin6_addr = endpoint.address().m_addr6;
            binderr = ::bind(socket, (sockaddr*)&serveraddr, sizeof(serveraddr));
        }

#ifdef _WIN32

        if (binderr == SOCKET_ERROR) {
            throw_WASError(format("can't bind socket, socket = %x", socket));
        }

#elif defined(__unix__)

        if (binderr == -1) {
            throw_errno(format("can't bind socket, fd = %x", socket));
        }
#endif
    }


    template<class Awaiter, class Buffer>
    class DataAwaiterMixin {
    public:
        friend class ConnImpl;
        friend class ConnCallback;

        Awaiter* m_next;
        IoCtxBase* m_ctx;
        ConnImpl* m_conn;
        std::coroutine_handle<TaskPromiseBase> m_suspend_coroutine;
        Buffer m_buffer_addr;
        std::size_t m_buffer_size;
        std::size_t m_bytes_transfer;
        bool m_suspend_return;
        
#ifdef _WIN32
        WSABUF win32_single_buffer;
#endif

        static constexpr std::ptrdiff_t k_closed_socket_ready = -2;
        static constexpr std::ptrdiff_t k_time_out = -3;

    };


    class TINYASYNC_NODISCARD AsyncReceiveAwaiter :
        public DataAwaiterMixin<AsyncReceiveAwaiter, void*>
    {
    public:
        friend class ConnImpl;
        bool m_timeout_flag = false; //是否是一个timeoutAsyncAwaiter
        PostTask m_post_task;
        // ListNode m_node;
        timeNode m_timenode;
        AsyncReceiveAwaiter(ConnImpl& conn, void* b, std::size_t n);

        AsyncReceiveAwaiter(ConnImpl& conn, void* b, std::size_t n,bool timeOutFlag);

        // static AsyncReceiveAwaiter *from_node(ListNode *node) {
        //     return (AsyncReceiveAwaiter *)((char*)node - offsetof(AsyncReceiveAwaiter, m_node));
        // }

        // 超时 post_task 回调
        static void on_timeout_outcallbak(PostTask * post_task);

        bool await_ready();

        template<class Promise>
        inline bool await_suspend(std::coroutine_handle<Promise> suspend_coroutine) {
            std::coroutine_handle<TaskPromiseBase> h = suspend_coroutine.promise().coroutine_handle_base();
            return await_suspend(h);
        }

        bool await_suspend(std::coroutine_handle<TaskPromiseBase> h);
        std::size_t await_resume();

        /*
        //超时应该做的事情
        static void time_out(AsyncReceiveAwaiter * self){
            //1 从conn->AsyncReceiveAwaiter 列表删除自己
            AsyncReceiveAwaiter* * pre_ptr = &(m_conn->AsyncReceiveAwaiter);
            for(auto awaiter_ptr =  m_conn->AsyncReceiveAwaiter ;
                    awaiter_ptr != nullptr;
                    awaiter_ptr = awaiter_ptr->m_next)
            {
                if( awaiter_ptr == self){
                    *pre_ptr = awaiter_ptr->m_next;
                    return;
                }
                pre_ptr = &(awaiter_ptr->m_next);
            }
        }
        */

        //把自己从timeOut 上删除
        static void del_from_time_queue(){
        }

    };

    class TINYASYNC_NODISCARD AsyncSendAwaiter : public std::suspend_always,
        public DataAwaiterMixin<AsyncSendAwaiter, void const*>
    {
    public:
        friend class ConnImpl;
        AsyncSendAwaiter(ConnImpl& conn, void const* b, std::size_t n);

        bool await_ready();

        template<class Promise>
        inline bool await_suspend(std::coroutine_handle<Promise> suspend_coroutine) {
            std::coroutine_handle<TaskPromiseBase> h = suspend_coroutine.promise().coroutine_handle_base();
            return await_suspend(h);
        }

        bool await_suspend(std::coroutine_handle<TaskPromiseBase> h);
        std::size_t await_resume();


    };

    class ConnImpl
    {
        friend class Connection;
        friend class AsyncReceiveAwaiter;
        friend class AsyncSendAwaiter;
        friend class AsyncCloseAwaiter;
        friend class ConnCallback;


        IoCtxBase* m_ctx;
        NativeSocket m_conn_handle;        
        Callback m_callback;
        AsyncReceiveAwaiter* m_recv_awaiter = nullptr; // 一个链表
        AsyncSendAwaiter* m_send_awaiter = nullptr;
        PostTask m_post_task;

        // one for connection
        // one for close
        int m_ref_cnt = 2;
        bool m_added_to_event_pool = false;

        bool m_recv_shutdown = false;
        bool m_send_shutdown = false;
        bool m_ready_to_send = true;
        bool m_ready_to_recv = true;
        bool m_tcp_nodelay = false;

    public:


        NativeSocket native_handle() {
            return m_conn_handle;
        }

        void register_()
        {
            auto m_conn = this;
            epoll_event evt;
            evt.events = 0;
            if(m_conn->m_recv_awaiter) {
                evt.events |= EPOLLIN;
            }
            if(m_conn->m_send_awaiter) {
                evt.events |= EPOLLOUT;
            }
            if(!evt.events) {
                return;
            }
            evt.events |= EPOLLONESHOT;
            evt.data.ptr = &m_conn->m_callback;

            int epoll_clt_addmod;
            if(!m_conn->m_added_to_event_pool) {
                epoll_clt_addmod = EPOLL_CTL_ADD;
                TINYASYNC_LOG("epoll_ctl(EPOLL_CTL_ADD, %s) for conn_handle = %d",
                    ioe2str(evt).c_str(), m_conn->m_conn_handle);
                m_conn->m_added_to_event_pool = true;
            } else {
                epoll_clt_addmod = EPOLL_CTL_MOD;
                TINYASYNC_LOG("epoll_ctl(EPOLL_CTL_MOD, %s) for conn_handle = %d",
                    ioe2str(evt).c_str(), m_conn->m_conn_handle);
            }

            int clterr = epoll_ctl(m_ctx->event_poll_handle(), epoll_clt_addmod, m_conn->m_conn_handle, &evt);
            if(clterr == -1) {
                TINYASYNC_LOG("epoll_ctl failed for conn_handle = %d", m_conn->m_conn_handle);
                throw_errno(format("can't epoll_clt for conn_handle = %s", socket_c_str(m_conn->m_conn_handle)));
            }
        }

        void reset()
        {
            if (m_conn_handle) {
                // ubind from epool ...
                if(close_socket(m_conn_handle) < 0) {
                    throw_errno(format("can't close socket fd  = %s", socket_c_str(m_conn_handle)));
                }
                m_conn_handle = NULL_SOCKET;
            }
        }

        static void on_callback(Callback *callback, IoEvent &evt);


        ConnImpl(IoCtxBase &ctx, NativeSocket conn_sock, bool added_event_poll)
        {
            TINYASYNC_GUARD("Connection.Connection(): ");
            TINYASYNC_LOG("conn_socket %p", conn_sock);

            m_ctx = &ctx;
            m_conn_handle = conn_sock;
            m_callback.m_callback = on_callback;


            auto m_conn = this;
            epoll_event evt;
            evt.events = EPOLLIN | EPOLLOUT | EPOLLEXCLUSIVE | EPOLLET;
            evt.data.ptr = &m_conn->m_callback;
            int epoll_clt_addmod;
            if(!added_event_poll) {
                epoll_clt_addmod = EPOLL_CTL_ADD;
                TINYASYNC_LOG("epoll_ctl(EPOLL_CTL_ADD, %s) for conn_handle = %d",
                    ioe2str(evt).c_str(), m_conn->m_conn_handle);
            } else {
                epoll_clt_addmod = EPOLL_CTL_MOD;
                TINYASYNC_LOG("epoll_ctl(EPOLL_CTL_MOD, %s) for conn_handle = %d",
                    ioe2str(evt).c_str(), m_conn->m_conn_handle);
            }
            m_conn->m_added_to_event_pool = true;

            int clterr = epoll_ctl(m_ctx->event_poll_handle(), epoll_clt_addmod, m_conn->m_conn_handle, &evt);
            if(clterr == -1) {
                TINYASYNC_LOG("epoll_ctl failed for conn_handle = %d", m_conn->m_conn_handle);
                throw_errno(format("can't epoll_clt for conn_handle = %s", socket_c_str(m_conn->m_conn_handle)));
            }

        }

        ConnImpl(ConnImpl const&) = delete;
        ConnImpl& operator=(ConnImpl const&) = delete;


        void set_tcp_no_delay(bool b = true) {
            int flags = b;
            if(::setsockopt(m_conn_handle, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(flags)) < 0) {
                throw_errno("can't set tcp no delay");
            }
        }

        AsyncReceiveAwaiter async_read(void* buffer, std::size_t bytes)
        {
            return { *this, buffer, bytes };
        }

        AsyncReceiveAwaiter async_read_timeout(void * buffer,std::size_t bytes)
        {
            return { *this, buffer, bytes ,true};
        }

        AsyncSendAwaiter async_send(void const* buffer, std::size_t bytes)
        {
            TINYASYNC_GUARD("Connection.send(): ");
            TINYASYNC_LOG("try to send %d bytes", bytes);
            return { *this, buffer, bytes };
        }

        static void wakeup_awaiter_on_close(PostTask *posttask)
        {
            using this_type = ConnImpl;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
            auto *conn = (this_type*)((char*)posttask - offsetof(this_type, m_post_task));
#pragma GCC diagnostic pop

            for(auto awaiter = conn->m_recv_awaiter; awaiter;)
            {
                auto next = awaiter->m_next;
                awaiter->m_bytes_transfer = (std::uintptr_t)(-1);
                errno = ENOTSOCK;
                TINYASYNC_RESUME(awaiter->m_suspend_coroutine);
                awaiter = next;
            }

            for(auto awaiter = conn->m_send_awaiter; awaiter;)
            {
                auto next = awaiter->m_next;
                awaiter->m_bytes_transfer = (std::uintptr_t)(-1);
                errno = ENOTSOCK;
                TINYASYNC_RESUME(awaiter->m_suspend_coroutine);
                awaiter = next;
            }

            conn->m_ref_cnt--;
            if(!conn->m_ref_cnt) {
                delete conn;
            }
        }

        bool is_any_shutdown() const
        {
            return !m_recv_shutdown && !m_send_shutdown;
        }

        bool is_recv_shutdown() const
        {
            return m_recv_shutdown;
        }

        bool is_send_shutdown() const
        {
            return m_send_shutdown;
        }

        bool is_recv_send_shutdown() const
        {
            return m_recv_shutdown && m_send_shutdown;
        }

        void safe_shutdown_recv()
        {
            if(!is_recv_shutdown()) {
                shutdown_recv();
            }
        }

        void safe_shutdown_send()
        {
            if(!is_send_shutdown()) {
                shutdown_send();
            }
        }

        void safe_shutdown_recv_send()
        {
            if(!is_recv_shutdown() && !is_send_shutdown()) {
                shutdown_recv_send();
            } else if(!is_send_shutdown()) {
                shutdown_send();
            } else {
                shutdown_recv();
            } 
        }

        void shutdown_recv()
        {
            TINYASYNC_ASSERT(m_conn_handle);
            TINYASYNC_ASSERT(!m_recv_shutdown);
            auto conn_handle = m_conn_handle;
            ::shutdown(conn_handle, SHUT_RD);            
            m_recv_shutdown = true;
        }

        void shutdown_send()
        {
            TINYASYNC_ASSERT(m_conn_handle);
            TINYASYNC_ASSERT(!m_recv_shutdown);
            auto conn_handle = m_conn_handle;
            ::shutdown(conn_handle, SHUT_WR);            
            m_send_shutdown = true;
        }

        void shutdown_recv_send()
        {
            TINYASYNC_ASSERT(m_conn_handle);
            TINYASYNC_ASSERT(!m_recv_shutdown);
            TINYASYNC_ASSERT(!m_send_shutdown);
            auto conn_handle = m_conn_handle;
            ::shutdown(conn_handle, SHUT_RDWR);            
            m_send_shutdown = true;
            m_recv_shutdown = true;
        }

        void safe_close() {
            auto conn_handle = m_conn_handle;
            if(conn_handle != NULL_SOCKET) {
                this->close();
            }
        }

        void close()
        {
            TINYASYNC_GUARD("Connection:close(): ");
            auto conn_handle = m_conn_handle;

            if(close_socket(conn_handle) < 0) {
                TINYASYNC_LOG("close error");
                throw_errno("close error");
            }
            m_conn_handle = NULL_SOCKET;            
            m_send_shutdown = true;
            m_recv_shutdown = true;
            m_added_to_event_pool = false;

            // post tasks at the end of queue
            // wakeup all awaiters
            // remain epoll_event should happen before this task
            // callback will not response to these epoll_event
            // after we finish the task
            // we should never recv any epoll_event
            // thus we can delete connection
            m_post_task.set_callback(wakeup_awaiter_on_close);
            m_ctx->post_task(&m_post_task);

        }

        ~ConnImpl() noexcept
        {
            TINYASYNC_GUARD("ConnImpl::~ConnImpl() ");
            TINYASYNC_ASSERT(!this->m_recv_awaiter && !this->m_send_awaiter);
            TINYASYNC_LOG("destruct");
        }

    };

    inline void ConnImpl::on_callback(Callback *callback, IoEvent& evt)
    {
        TINYASYNC_GUARD("ConnCallback.callback(): ");
        ConnImpl* conn = (ConnImpl*)((char*)callback - offsetof(ConnImpl, m_callback));
        auto conn_handle = conn->m_conn_handle;        

#ifdef _WIN32

        if (m_conn->m_recv_awaiter) {
            assert(!m_conn->m_send_awaiter);
            auto awaiter = m_conn->m_recv_awaiter;
            awaiter->m_bytes_transfer = (size_t)evt.transfered_bytes;
            TINYASYNC_RESUME(awaiter->m_suspend_coroutine);

        } else if (m_conn->m_send_awaiter) {
            assert(!m_conn->m_recv_awaiter);
            auto awaiter = m_conn->m_send_awaiter;
            awaiter->m_bytes_transfer = (size_t)evt.transfered_bytes;
            TINYASYNC_RESUME(awaiter->m_suspend_coroutine);
        } else {
            assert(false);
            throw std::runtime_error("Why conn callback called");
        }

#elif defined(__unix__)

        if(conn_handle == NULL_SOCKET)
        {
            // connection have been closed
            // use post task to wakeup awaiters
            return;
        }

        // now conn must be alive
        // so pre-load them

        int events = evt.events;
        if(events & EPOLLIN) {
            conn->m_ready_to_recv = true;
        }
        if(events & EPOLLOUT) {
            conn->m_ready_to_send = true;
        }

        auto recv_awaiter = conn->m_recv_awaiter;
        auto send_awaiter = conn->m_send_awaiter;

        if ((events & EPOLLIN) && recv_awaiter) {
            // we want to read and it's ready to read

            auto awaiter = recv_awaiter;

            do {
                std::size_t desired_bytes = awaiter->m_buffer_size;
                TINYASYNC_LOG("ready to read for conn_handle %d, %d bytes at %p reading",
                    conn_handle, (int)awaiter->m_buffer_size, awaiter->m_buffer_addr);

                int nbytes = ::recv(conn_handle, awaiter->m_buffer_addr, (int)awaiter->m_buffer_size, 0);

                TINYASYNC_LOG("recv %d bytes", nbytes);

                if(nbytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) { 
                    // try again latter ...
                    conn->m_ready_to_recv = false;
                    break;
                } else {
                    // may cause self deleted
                    awaiter->m_bytes_transfer = (std::ptrdiff_t)nbytes;
                    TINYASYNC_RESUME(awaiter->m_suspend_coroutine);
                }

                if(nbytes < desired_bytes) {
                    conn->m_ready_to_recv = false;
                    break;
                }

                awaiter = awaiter->m_next;
            } while(awaiter);

        }
        
        if((events & EPOLLOUT) && send_awaiter)
        {
            // we want to send and it's ready to read

            // send_awaiter is not nullptr
            // the connection must be alive

            auto awaiter = send_awaiter;

            do {
                std::size_t desired_bytes = awaiter->m_buffer_size;
                TINYASYNC_LOG("ready to send for conn_handle %d, %d bytes at %p sending",
                    conn_handle, (int)awaiter->m_buffer_size, awaiter->m_buffer_addr);

                int nbytes = ::send(conn_handle, awaiter->m_buffer_addr, (int)awaiter->m_buffer_size, 0);

                TINYASYNC_LOG("sent %d bytes", nbytes);

                if(nbytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) { 
                    // try again latter ...
                    conn->m_ready_to_send = false;
                    break;
                } else {
                    awaiter->m_bytes_transfer = (std::ptrdiff_t)nbytes;
                    TINYASYNC_RESUME(awaiter->m_suspend_coroutine);
                }

                if(nbytes < desired_bytes) {
                    conn->m_ready_to_send = false;
                    break;
                }

                awaiter = awaiter->m_next;
            } while(awaiter);
        }
        
        if(!(   events & (EPOLLIN|EPOLLOUT) )) {
            TINYASYNC_LOG("not processed event for conn_handle %x", errno, conn->m_conn_handle);
            fprintf(stderr, "%s\n", ioe2str(evt).c_str());
            exit(1);
        }

#endif

    }

    AsyncReceiveAwaiter::AsyncReceiveAwaiter(ConnImpl& conn, void* b, std::size_t n)
    {
        TINYASYNC_GUARD("AsyncReceiveAwaiter.AsyncReceiveAwaiter(): ");
        m_next = nullptr;
        m_ctx = conn.m_ctx;
        m_conn = &conn;
        m_buffer_addr = b;
        m_buffer_size = n;
        m_bytes_transfer = 0;
        TINYASYNC_LOG("conn: %p", m_conn);
    }

    AsyncReceiveAwaiter::AsyncReceiveAwaiter(ConnImpl& conn, void* b, std::size_t n,bool timeOutFlag)
        : AsyncReceiveAwaiter::AsyncReceiveAwaiter(conn,b,n)
    {
        m_timeout_flag = timeOutFlag;
    }


    bool AsyncReceiveAwaiter::await_ready()
    {
        auto conn = m_conn;
        if(!conn->native_handle() || conn->is_recv_shutdown())
        {            
            m_bytes_transfer = k_closed_socket_ready;
            return true;
        }
        return false;
    }

    bool AsyncReceiveAwaiter::await_suspend(std::coroutine_handle<TaskPromiseBase> h)
    {
        TINYASYNC_GUARD("AsyncReceiveAwaiter.await_suspend(): ");
        TINYASYNC_LOG("try to receive %zu bytes from %s", m_buffer_size, socket_c_str(m_conn->m_conn_handle));
        TINYASYNC_ASSERT(m_conn->m_conn_handle);
        auto conn =  m_conn;
        NativeSocket conn_handle = conn->m_conn_handle;

#ifdef _WIN32

        DWORD flags = 0;
        win32_single_buffer.buf = (CHAR*)m_buffer_addr;
        win32_single_buffer.len = (ULONG)m_buffer_size;
        LPWSABUF                           lpBuffers = &win32_single_buffer;
        DWORD                              dwBufferCount = 1;
        LPDWORD                            lpNumberOfBytesRecvd = NULL;
        LPDWORD                            lpFlags = &flags;
        LPWSAOVERLAPPED                    lpOverlapped = &m_conn->m_callback.m_overlapped;
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine = NULL;

        memset(&m_conn->m_callback.m_overlapped, 0, sizeof(m_conn->m_callback.m_overlapped));

        if (::WSARecv(m_conn->m_conn_handle, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd,
            lpFlags, lpOverlapped, lpCompletionRoutine) == 0) {
            TINYASYNC_LOG("WSARecv completed");
            // completed, luckly
            // always use GetQueuedCompletionStatus
        } else {
            if (WSAGetLastError() == ERROR_IO_PENDING) {
                TINYASYNC_LOG("WSARecv ERROR_IO_PENDING");
            } else {
                throw_LastError("WSARecv failed:");
            }
        }
        return true;

#elif defined(__unix__)

    if(conn->m_ready_to_recv) {
        auto nbytes = ::recv(conn_handle, m_buffer_addr, m_buffer_size, 0);
        if(nbytes == -1) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                conn->m_ready_to_recv = false;
            } else {
                throw_errno("recv error");
            }
        } else {
            m_suspend_return = false;
            m_bytes_transfer = (std::ptrdiff_t)nbytes;
            return false;
        }
    }

    if(m_timeout_flag){ //此时,加入ctx Time OUT Queue
        auto * tnode = &m_timenode;
        m_post_task.set_callback(on_timeout_outcallbak);
        tnode->m_post_task = &m_post_task;
        m_ctx->post_time_out(tnode);
    }

    m_suspend_coroutine = h;
    // insert into front of list
    this->m_next = m_conn->m_recv_awaiter;
    m_conn->m_recv_awaiter = this;
    TINYASYNC_LOG("set recv_awaiter of conn(%p) to %p", m_conn, m_conn->m_recv_awaiter);
    m_suspend_return = true;
    return true;
#endif

    }


    std::size_t AsyncReceiveAwaiter::await_resume()
    {
        TINYASYNC_GUARD("AsyncReceiveAwaiter.await_resume(): ");

        auto nbytes = (std::ptrdiff_t)m_bytes_transfer;

        if(nbytes == k_closed_socket_ready) {
            TINYASYNC_LOG("ERROR = %d, fd = %d", errno, m_conn->m_conn_handle);
            errno = ENOTSOCK;
            throw_errno("AsyncReceiveAwaiter::await_resume(): recv error");
        }

        if(nbytes == k_time_out){ //超时
            TINYASYNC_LOG("ERROR = TIMEOUT, fd = %d",  m_conn->m_conn_handle);
            // throw_errno("AsyncReceiveAwaiter::await_resume(): recv error,timeOut");
            throw AsyncRecvTimeOutError{};
        }

        if(m_suspend_return) {
            // pop from front of list
            m_conn->m_recv_awaiter = m_conn->m_recv_awaiter->m_next;
        }

        if (nbytes < 0) {
            TINYASYNC_LOG("ERROR = %d, fd = %d", errno, m_conn->m_conn_handle);
            throw_errno("AsyncReceiveAwaiter::await_resume(): recv error");
        } else {
            if (nbytes == 0) {
                if (errno == ESHUTDOWN) {
                    TINYASYNC_LOG("ESHUTDOWN, fd = %d",  m_conn->m_conn_handle);
                }
            }
            TINYASYNC_LOG("fd = %d, %d bytes read", m_conn->m_conn_handle, nbytes);
        }

        //正常结束
        //删除,timeNode
        if( m_timeout_flag )
            m_timenode.remove_self();

        return m_bytes_transfer;
    }

    void AsyncReceiveAwaiter::on_timeout_outcallbak(PostTask * post_task)
    {

        using this_type = AsyncReceiveAwaiter;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
        auto * awaiter = (this_type*)((char*)post_task - offsetof(this_type, m_post_task));
#pragma GCC diagnostic pop

        // auto * awaiter = from_node(post_task->listNode);
        //1 设置awaiter的返回值
        awaiter->m_bytes_transfer = k_time_out; // 超时标记

        //2 删除conn Awaiter -> m_next
        AsyncReceiveAwaiter ** ptr_pre = &(awaiter->m_conn->m_recv_awaiter);
        for( auto curr_awaiter = awaiter->m_conn->m_recv_awaiter ; 
                curr_awaiter !=nullptr
                ; curr_awaiter = curr_awaiter->m_next)
        {
            if( curr_awaiter == awaiter){
#ifdef  __USE_ASYNC_UTILS__
                printf("delete m_conn->m_recv_awaiter ,%p\n",curr_awaiter);
                printf("delete m_conn->m_recv_awaiter awaiter ,%p\n",awaiter);
#endif
                *ptr_pre = curr_awaiter->m_next;
                break;
            }
            ptr_pre = &(curr_awaiter->m_next);
        }
        //3. 恢复 协程
        TINYASYNC_RESUME(awaiter->m_suspend_coroutine);
    }

    AsyncSendAwaiter::AsyncSendAwaiter(ConnImpl& conn, void const* b, std::size_t n)
    {
        m_next = nullptr;
        m_ctx = conn.m_ctx;
        m_conn = &conn;
        m_buffer_addr = b;
        m_buffer_size = n;
        m_bytes_transfer = 0;
    }

    bool AsyncSendAwaiter::await_ready()
    {
        auto conn = m_conn;
        if(!conn->native_handle() || conn->is_send_shutdown())
        {
            m_bytes_transfer = k_closed_socket_ready;
            return true;
        }
        return false;
    }

    bool AsyncSendAwaiter::await_suspend(std::coroutine_handle<TaskPromiseBase> h)
    {
        TINYASYNC_LOG("AsyncSendAwaiter::await_suspend(): ");
        TINYASYNC_ASSERT(m_conn->m_conn_handle);

        auto conn =  m_conn;
        NativeSocket conn_handle = conn->m_conn_handle;


#ifdef _WIN32

        DWORD flags = 0;
        win32_single_buffer.buf = (CHAR*)m_buffer_addr;
        win32_single_buffer.len = (ULONG)m_buffer_size;
        LPWSABUF                           lpBuffers = &win32_single_buffer;
        DWORD                              dwBufferCount = 1;
        LPDWORD                            lpNumberOfBytesRecvd = NULL;
        LPDWORD                            lpFlags = &flags;
        LPWSAOVERLAPPED                    lpOverlapped = &m_conn->m_callback.m_overlapped;
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine = NULL;

        memset(&m_conn->m_callback.m_overlapped, 0, sizeof(m_conn->m_callback.m_overlapped));

        if (::WSASend(m_conn->m_conn_handle, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd,
            flags, lpOverlapped, lpCompletionRoutine) == 0) {
            TINYASYNC_LOG("WSASend completed");
            // completed, luckly
            // always use GetQueuedCompletionStatus
            //return false;
        } else {
            if (WSAGetLastError() == ERROR_IO_PENDING) {
                TINYASYNC_LOG("WSASend ERROR_IO_PENDING");
            } else {
                throw_LastError("WSASend failed:");
            }
        }
        return true;

#elif defined(__unix__)
        if(conn->m_ready_to_send) {
            auto nbytes = ::send(conn_handle, m_buffer_addr, m_buffer_size, 0);
            if(nbytes == -1) {
                if(errno == EAGAIN) {
                    conn->m_ready_to_send = false;
                } else {
                    throw_errno("send error");
                }
            } else {
                m_bytes_transfer = (std::ptrdiff_t)nbytes;
                m_suspend_return = false;
                return false;
            }
        }

        m_suspend_coroutine = h;
        // insert front of list
        this->m_next = m_conn->m_send_awaiter;
        m_conn->m_send_awaiter = this;
        TINYASYNC_LOG("set send_awaiter of conn(%p) to %p", m_conn, m_conn->m_send_awaiter);
        m_suspend_return = true;
        return true;
#endif
    }

    std::size_t AsyncSendAwaiter::await_resume()
    {
        TINYASYNC_GUARD("AsyncSendAwaiter.await_resume(): ");

        auto nbytes = (std::ptrdiff_t)m_bytes_transfer;

        if(nbytes == k_closed_socket_ready) {
            TINYASYNC_LOG("ERROR = %d, fd = %d", errno, m_conn->m_conn_handle);
            errno = ENOTSOCK;
            throw_errno("AsyncReceiveAwaiter::await_resume(): recv error");
        }

        if(m_suspend_return) {
            // pop from front of list
            m_conn->m_send_awaiter = m_conn->m_send_awaiter->m_next;
        }

        if (nbytes < 0) {
            TINYASYNC_LOG("ERROR = %d, fd = %d", errno, m_conn->m_conn_handle);
            throw_errno("AsyncSendAwaiter::await_resume(): send error");
        } else {
            if (nbytes == 0) {
                if (errno == ESHUTDOWN) {
                    TINYASYNC_LOG("ESHUTDOWN, fd = %d", m_conn->m_conn_handle);
                }
            }
            TINYASYNC_LOG("fd = %d, %d bytes read", m_conn->m_conn_handle, nbytes);
        }


        return m_bytes_transfer;
    }

    class Connection
    {
        // use unique_ptr because
        // 1. fast move/swap
        // 2. reference stable, usefull for callbacks
        std::unique_ptr<ConnImpl> m_impl;
    public:
    
        Connection() = default;
        Connection(Connection &&) = default;
        Connection &operator=(Connection &&) = default;

        Connection(IoCtxBase &ctx, NativeSocket conn_sock, bool added_event_poll)
        {
            m_impl.reset(new ConnImpl(ctx, conn_sock, added_event_poll));
        }

        ~Connection()
        {
            auto impl = m_impl.release();
            if(impl) {
                impl->safe_shutdown_recv_send();
                impl->safe_close();
                
                impl->m_ref_cnt--;
                if(!impl->m_ref_cnt) {      
                    // ensure no epoll_event
                    // we can free connimpl          
                    delete impl;
                }
            }
        }

        void set_tcp_no_delay(bool b = true) {
            auto impl = m_impl.get();
            impl->set_tcp_no_delay(b);
        }


        bool is_closed() {
            auto impl = m_impl.get();
            return !impl->m_conn_handle;
        }

        bool is_recv_shutdown() {
            auto impl = m_impl.get();
            return impl->is_recv_shutdown();
        }
        
        bool is_send_shutdown() {
            auto impl = m_impl.get();
            return impl->is_send_shutdown();
        }

        bool is_recv_send_shutdown() {
            auto impl = m_impl.get();
            return impl->is_recv_send_shutdown();
        }

        void shutdown_recv() {
            auto impl = m_impl.get();
            impl->shutdown_recv();
        }

        void shutdown_send() {
            auto impl = m_impl.get();
            impl->shutdown_send();
        }

        void shutdown_recv_send() {
            auto impl = m_impl.get();
            impl->shutdown_send();
        }

        void safe_shutdown_recv() {
            auto impl = m_impl.get();
            if(impl)
                impl->safe_shutdown_recv();
        }

        void safe_shutdown_send() {
            auto impl = m_impl.get();
            if(impl)
                impl->safe_shutdown_send();
        }

        void safe_shutdown_recv_send() {
            auto impl = m_impl.get();
            if(impl)
                impl->safe_shutdown_recv_send();
        }

        void safe_close()
        {
            auto impl = m_impl.get();
            if(impl) {
                impl->safe_close();
            }
        }

        void close()
        {
            auto impl = m_impl.get();
            TINYASYNC_ASSERT(impl);
            TINYASYNC_ASSERT(impl->m_conn_handle);
            impl->close();
        }

        NativeSocket native_handle() {
            auto impl = m_impl.get();
            return impl->m_conn_handle;
        }

        AsyncReceiveAwaiter async_read(void* buffer, std::size_t bytes)
        {
            auto impl = m_impl.get();
            TINYASYNC_ASSERT(m_impl.get());
            return impl->async_read(buffer, bytes);
        }

        AsyncReceiveAwaiter async_read(Buffer const &buffer)
        {
            auto impl = m_impl.get();
            TINYASYNC_ASSERT(m_impl.get());
            return impl->async_read(buffer.data(), buffer.size());
        }

        AsyncReceiveAwaiter async_read_timeout(void * buffer,std::size_t bytes)
        {
            auto impl = m_impl.get();
            TINYASYNC_ASSERT(m_impl.get());
            return impl->async_read_timeout(buffer , bytes);
        }

        AsyncSendAwaiter async_send(void const* buffer, std::size_t bytes)
        {
            auto impl = m_impl.get();
            TINYASYNC_ASSERT(m_impl.get());
            return impl->async_send(buffer, bytes);
        }

        AsyncSendAwaiter async_send(ConstBuffer const &buffer)
        {
            auto impl = m_impl.get();
            TINYASYNC_ASSERT(m_impl.get());
            return impl->async_send(buffer.data(), buffer.size());
        }        

        
    };



    class SocketMixin {

    protected:
        friend class Acceptor;
        friend class AcceptorImpl;
        friend class AsyncReceiveAwaiter;
        friend class AsyncSendAwaiter;

        IoCtxBase* m_ctx;
        Protocol m_protocol;
        Endpoint m_endpoint;
        NativeSocket m_socket;
        bool m_added_to_event_pool;

    public:


        void reset_io_context(IoCtxBase &ctx, SocketMixin &socket)
        {
            m_ctx = &ctx;
            m_protocol = socket.m_protocol;
            m_endpoint = socket.m_endpoint;
            m_socket = socket.m_socket;
            m_added_to_event_pool = false;
        }

        NativeSocket native_handle() const noexcept
        {
            return m_socket;

        }

        SocketMixin(IoCtxBase &ctx)
        {
            m_ctx = &ctx;
            m_socket = NULL_SOCKET;
            m_added_to_event_pool = false;

        }

        SocketMixin()
        {
            m_ctx = nullptr;
            m_socket = NULL_SOCKET;
            m_added_to_event_pool = false;
        }

        void open(Protocol const& protocol, bool blocking = false)
        {
            m_socket = open_socket(protocol, blocking);
            m_protocol = protocol;
        }


        void reset()
        {
            TINYASYNC_GUARD("SocketMixin.reset(): ");
            if (m_socket) {
#ifdef _WIN32
                //
#elif defined(__unix__)

                if (m_ctx && m_added_to_event_pool) {
                    // this socket may be added to many pool
                    // ::close can't atomatically remove it from event pool
                    auto ctlerr = epoll_ctl(m_ctx->event_poll_handle(), EPOLL_CTL_DEL, m_socket, NULL);
                    if (ctlerr == -1) {
                        auto what = format("can't remove (from epoll) socket %x", m_socket);
                        throw_errno(what);
                    }
                    m_added_to_event_pool = false;
                }
#endif
                TINYASYNC_LOG("close socket = %s", socket_c_str(m_socket));
                if(close_socket(m_socket) < 0) {
                    printf("%d\n", errno);
                    std::exit(1);
                }
                m_socket = NULL_SOCKET;
            }
        }
    };


    class AcceptorCallback : public CallbackImplBase
    {
        friend class AcceptorImpl;
        AcceptorImpl* m_acceptor;
    public:
        AcceptorCallback(AcceptorImpl* acceptor) : CallbackImplBase(this)
        {
            m_acceptor = acceptor;
        };

        void on_callback(IoEvent& evt);

    };

    class TINYASYNC_NODISCARD AcceptorAwaiter
    {

        friend class AcceptorImpl;
        friend class AcceptorCallback;

        ListNode m_node;
        AcceptorImpl* m_acceptor;
        NativeSocket m_conn_socket;
        std::coroutine_handle<TaskPromiseBase> m_suspend_coroutine;

    public:

        static AcceptorAwaiter *from_node(ListNode *node) {
            return (AcceptorAwaiter *)((char*)node - offsetof(AcceptorAwaiter, m_node));
        }

        bool await_ready() { return false; }
        AcceptorAwaiter(AcceptorImpl& acceptor);

        template<class Promise>
        inline void await_suspend(std::coroutine_handle<Promise> suspend_coroutine) {
            auto h = suspend_coroutine.promise().coroutine_handle_base();
            await_suspend(h);
        }
        bool await_suspend(std::coroutine_handle<TaskPromiseBase> h);
        Connection await_resume();
    };

    class AcceptorImpl : SocketMixin
    {
        friend class Acceptor;
        friend class AcceptorAwaiter;
        friend class AcceptorCallback;
        Queue m_awaiter_que;
        AcceptorCallback m_callback = this;

#ifdef _WIN32
        NativeSocket m_accept_socket = NULL_SOCKET;
        LPFN_ACCEPTEX m_lpfnAcceptEx = NULL;
        char m_accept_buffer[(sizeof(sockaddr_in) + 16) * 2];
#endif


    public:
        AcceptorImpl(IoCtxBase &ctx) : SocketMixin(ctx)
        {
        }
        AcceptorImpl() : SocketMixin()
        {
        }

        AcceptorImpl(Protocol const& protocol, Endpoint const& endpoint) : AcceptorImpl()
        {
            init(nullptr, protocol, endpoint);
        }

        AcceptorImpl(IoCtxBase& ctx, Protocol const& protocol, Endpoint const& endpoint) : AcceptorImpl(ctx)
        {
            init(&ctx, protocol, endpoint);
        }

        AcceptorImpl(AcceptorImpl const&) = delete;
        AcceptorImpl& operator=(AcceptorImpl const&) = delete;

        ~AcceptorImpl()
        {
            TINYASYNC_GUARD("AcceptorImpl::~AcceptorImpl(): ");
            reset();            
        }

        void init(IoCtxBase *, Protocol const& protocol, Endpoint const& endpoint)
        {
            try {
                // one effort triple successes
                open(protocol);
                // set reuse addr 
                int on  =1;
                // 使用 ctrl + c 停止程序的运行也不会出现 bind error
                ::setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
                bind_socket(m_socket, endpoint);
                m_endpoint = endpoint;
                listen();
            }
            catch (...) {
                reset();
                auto what = format("open/bind/listen failed %s:%d", endpoint.address().to_string().c_str(), (int)(unsigned)endpoint.port());
                std::throw_with_nested(std::runtime_error(what));
            }
        }


        void reset_io_context(IoCtxBase &ctx, AcceptorImpl &r)
        {
            ((SocketMixin*)this)->reset_io_context(ctx, r);
        }


        void listen()
        {
            TINYASYNC_GUARD("Acceptor.listen(): ");
            TINYASYNC_LOG("socket = %s, address = %s, port = %d", socket_c_str(m_socket), m_endpoint.address().to_string().c_str(),
                m_endpoint.port());

            int max_pendding_connection = 5;
            int err = ::listen(m_socket, max_pendding_connection);

#ifdef _WIN32
            if (err == SOCKET_ERROR) {
                throw_WASError(format("can't listen socket, socket = %s", socket_c_str(m_socket)));
            }

            // prepare Accept function
            NativeSocket listen_socket = m_socket;
            DWORD dwBytes;
            GUID GuidAcceptEx = WSAID_ACCEPTEX;
            int iResult = ::WSAIoctl(listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                &GuidAcceptEx, sizeof(GuidAcceptEx),
                &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx),
                &dwBytes, NULL, NULL);

            if (iResult == SOCKET_ERROR) {
                throw_WASError("can't get AcceptEx function pointer");
            }
            assert(m_lpfnAcceptEx);


            ULONG_PTR key = m_socket;
            HANDLE iocp = ::CreateIoCompletionPort((NativeHandle)listen_socket, m_ctx->handle(), key, 0);
            if (iocp == NULL) {
                throw_LastError("can't create IoCompletionPort");
            }


#elif defined(__unix__)

            if (err == -1) {
                throw_errno("can't listen socket");
            }
#endif

        }


        AcceptorAwaiter async_accept()
        {
            return { *this };
        }
    };


    AcceptorAwaiter::AcceptorAwaiter(AcceptorImpl& acceptor) : m_acceptor(&acceptor)
    {
    }

    bool AcceptorAwaiter::await_suspend(std::coroutine_handle<TaskPromiseBase> h)
    {
        TINYASYNC_ASSERT(m_acceptor);
        auto acceptor = m_acceptor;
        acceptor->m_awaiter_que.push(&this->m_node);
        m_suspend_coroutine = h;
        TINYASYNC_GUARD("AcceptorAwaiter::await_suspend(): ");

#ifdef _WIN32

        NativeSocket listen_socket = m_acceptor->m_socket;
        NativeSocket conn_socket = open_socket(m_acceptor->m_protocol, false);
        m_acceptor->m_accept_socket = conn_socket;

        PVOID        lpOutputBuffer = m_acceptor->m_accept_buffer;
        DWORD        dwReceiveDataLength = 0;
        DWORD        dwLocalAddressLength = sizeof(sockaddr_in) + 16;
        DWORD        dwRemoteAddressLength = sizeof(sockaddr_in) + 16;
        LPDWORD      lpdwBytesReceived = NULL;

        memset(&m_acceptor->m_callback.m_overlapped, 0, sizeof(m_acceptor->m_callback.m_overlapped));
        BOOL accret = m_acceptor->m_lpfnAcceptEx(listen_socket,
            conn_socket,
            lpOutputBuffer,
            dwReceiveDataLength,
            dwLocalAddressLength,
            dwRemoteAddressLength,
            lpdwBytesReceived,
            &m_acceptor->m_callback.m_overlapped);

        if (accret) TINYASYNC_UNLIKELY {
                TINYASYNC_LOG("AcceptEx completed");
        } else {
            if (WSAGetLastError() == ERROR_IO_PENDING) TINYASYNC_LIKELY {
                TINYASYNC_LOG("AcceptEx io_pending");
            } else {
                throw_LastError("AcceptEx failed");
            }
        }
        return true;
#elif defined(__unix__)

        if (!m_acceptor->m_added_to_event_pool) {
            int listenfd = m_acceptor->m_socket;
            epoll_event evt;
            evt.data.ptr = &m_acceptor->m_callback;

            // level triger by default
            // one thread one event
            evt.events = EPOLLIN | EPOLLEXCLUSIVE;
            auto epfd = m_acceptor->m_ctx->event_poll_handle();
            auto ctlerr = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &evt);
            if (ctlerr == -1) {
                TINYASYNC_LOG("can't await accept %s (epoll %s)", socket_c_str(listenfd), handle_c_str(epfd));
                throw_errno(format("can't await accept %x (epoll %s)", socket_c_str(listenfd), handle_c_str(epfd)));
            }
            m_acceptor->m_added_to_event_pool = true;
        }

        return true;
#endif

    }

    // connection is set nonblocking
    Connection AcceptorAwaiter::await_resume()
    {
        TINYASYNC_GUARD("AcceptorAwaiter.await_resume(): ");
        
        auto acceptor = m_acceptor;
        acceptor->m_awaiter_que.pop();
        NativeSocket conn_sock = m_conn_socket;

#ifdef _WIN32
        conn_sock = acceptor->m_accept_socket;

        ULONG_PTR key = conn_sock;
        HANDLE iocp = ::CreateIoCompletionPort((NativeHandle)conn_sock, m_acceptor->m_ctx->handle(), key, 0);
        if (iocp == NULL) {
            TINYASYNC_LOG("can't create iocp for accepted connection");
            throw_LastError("can't create IoCompletionPort");
        }
#elif defined(__unix__)

        TINYASYNC_LOG("accepted, socket = %s", socket_c_str(conn_sock));
        if(conn_sock == -1) {
            throw_errno(format("can't accept, socket = %s", socket_c_str(conn_sock)).c_str());
        }

        TINYASYNC_LOG("setnonblocking, socket = %s", socket_c_str(conn_sock));
        setnonblocking(conn_sock);
#endif
                
        TINYASYNC_ASSERT(conn_sock != NULL_SOCKET);
        return { *acceptor->m_ctx, conn_sock, false };
    }



    void AcceptorCallback::on_callback(IoEvent& evt)
    {
        TINYASYNC_GUARD("AcceptorCallback.callback(): ");
        auto acceptor = m_acceptor;
        ListNode* node = acceptor->m_awaiter_que.pop();
        if (node) {
            // it's ready to accept
            auto conn_sock = ::accept(acceptor->m_socket, NULL, NULL);
            if (conn_sock == -1) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    return;                    
                } else {
                    //real error
                    // wakeup awaiter
                }
            }

            AcceptorAwaiter *awaiter = AcceptorAwaiter::from_node(node);
            awaiter->m_conn_socket = conn_sock;
            TINYASYNC_RESUME(awaiter->m_suspend_coroutine);
        } else {
            // this will happen when after accetor.listen() but not yet co_await acceptor.async_accept()
            // you should have been using level triger to get this event the next time
            TINYASYNC_LOG("No awaiter found, event ignored");
        }
    }



    class Acceptor
    {

    public:
        Acceptor(IoContext& ctx, Protocol protocol, Endpoint endpoint)
        {
            m_impl.reset(new AcceptorImpl(*ctx.get_io_ctx_base(), protocol, endpoint));
        }

        Acceptor(Protocol protocol, Endpoint endpoint)
        {
            m_impl.reset(new AcceptorImpl(protocol, endpoint));
        }

        Acceptor() = default;

        AcceptorAwaiter async_accept()
        {
            auto impl = m_impl.get();
            return impl->async_accept();
        }

        Endpoint endpoint() {
            auto impl = m_impl.get();
            return impl->m_endpoint;
        }

        Acceptor reset_io_context(IoContext &ctx)
        {
            auto r = this->m_impl.get();
            auto impl = new AcceptorImpl();
            impl->reset_io_context(*ctx.get_io_ctx_base(), *r);
            
            Acceptor acc;
            acc.m_impl.reset(impl);
            return std::move(acc);
        }

    private:
        std::unique_ptr<AcceptorImpl> m_impl;
    };
























    class ConnectorCallback : public CallbackImplBase
    {
        friend class ConnectorAwaiter;
        friend class ConnectorImpl;
        ConnectorImpl* m_connector;


    public:
        ConnectorCallback(ConnectorImpl& connector) : CallbackImplBase(this),
            m_connector(&connector)
        {
        }
        void on_callback(IoEvent& evt);

    };


    class TINYASYNC_NODISCARD ConnectorAwaiter {
        friend class ConnectorCallback;

        ConnectorImpl* m_connector;
        std::coroutine_handle<TaskPromiseBase> m_suspend_coroutine;
        void unregister(NativeSocket conn_handle);

    public:
        ConnectorAwaiter(ConnectorImpl& connector) : m_connector(&connector)
        {
        }

        constexpr bool await_ready() const { return false; }

        template<class Promise>
        inline void await_suspend(std::coroutine_handle<Promise> suspend_coroutine) {
            auto h = suspend_coroutine.promise().coroutine_handle_base();
            await_suspend(h);
        }

        bool await_suspend(std::coroutine_handle<TaskPromiseBase> suspend_coroutine);

        Connection await_resume();

    };


    class ConnectorImpl : public SocketMixin {

        friend class ConnectorAwaiter;
        friend class ConnectorCallback;
        ConnectorAwaiter* m_awaiter = nullptr;
        ConnectorCallback m_callback = { *this };
    public:

        ConnectorImpl(IoCtxBase& ctx) : SocketMixin(ctx)
        {
        }

        ConnectorImpl(IoCtxBase& ctx, Protocol const& protocol, Endpoint const& endpoint) : ConnectorImpl(ctx)
        {
            try {
                this->open(protocol);
                m_endpoint = endpoint;
            }
            catch (...) {
                this->reset();
                throw;
            }
        }

        ConnectorImpl(ConnectorImpl const& r) :
            SocketMixin(static_cast<SocketMixin const&>(r)),
            m_callback(*this)
        {
            m_awaiter = r.m_awaiter;
            // don't copy, m_callback is fine
            // m_callback = r.m_callback;
        }

        ConnectorImpl& operator=(ConnectorImpl const& r)
        {
            static_cast<SocketMixin&>(*this) = static_cast<SocketMixin const&>(r);
            m_awaiter = r.m_awaiter;
            m_callback = r.m_callback;
            m_callback.m_connector = this;
            return *this;
        }

        // don't reset
        ~ConnectorImpl() = default;

        ConnectorAwaiter async_connect()
        {
            return { *this };
        }

        ConnectorAwaiter operator co_await()
        {
            return async_connect();
        }

    private:

        inline static std::mutex s_mutex;

        bool connect(Endpoint const& endpoint)
        {

            TINYASYNC_GUARD("Connector::connect():");

            auto connfd = m_socket;

            int connerr;
            if(endpoint.address().m_address_type == AddressType::IpV4) {

                sockaddr_in serveraddr;
                memset(&serveraddr, 0, sizeof(serveraddr));
                serveraddr.sin_family = AF_INET;
                serveraddr.sin_port = htons(endpoint.port());
                serveraddr.sin_addr = endpoint.address().m_addr4;
                NativeSocket connfd = m_socket;
                connerr = ::connect(connfd, (sockaddr*)&serveraddr, sizeof(serveraddr));

            } else if(endpoint.address().m_address_type == AddressType::IpV6) {

                sockaddr_in6 serveraddr;
                memset(&serveraddr, 0, sizeof(serveraddr));
                serveraddr.sin6_family = AF_INET6;
                serveraddr.sin6_port = htons(endpoint.port());
                serveraddr.sin6_addr = endpoint.address().m_addr6;
                NativeSocket connfd = m_socket;
                connerr = ::connect(connfd, (sockaddr*)&serveraddr, sizeof(serveraddr));
            }


            bool connected = false;

#ifdef _WIN32

            NativeSocket conn_socket = m_socket;

            bind_socket(conn_socket, Endpoint(Address::Any(), 0));

            static std::atomic<LPFN_CONNECTEX> s_ConnectEx;
            auto LpfnConnectex = initialize_once(s_ConnectEx, (LPFN_CONNECTEX)nullptr, s_mutex, [&]() -> LPFN_CONNECTEX {

                // prepare Accept function
                DWORD dwBytes;
                GUID GuidConnectEx = WSAID_CONNECTEX;
                LPFN_CONNECTEX LpfnConnectex;
                int iResult = ::WSAIoctl(conn_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &GuidConnectEx, sizeof(GuidConnectEx),
                    &LpfnConnectex, sizeof(LpfnConnectex),
                    &dwBytes, NULL, NULL);

                if (iResult == SOCKET_ERROR) {
                    throw_WASError("can't get AcceptEx function pointer");
                }
                return LpfnConnectex;
            });

            assert(LpfnConnectex);

            ULONG_PTR key = m_socket;
            HANDLE iocp = ::CreateIoCompletionPort((NativeHandle)m_socket, m_ctx->handle(), key, 0);
            if (iocp == NULL) {
                TINYASYNC_LOG("can't create iocp");
                throw_LastError("can't create IoCompletionPort");
            }

            memset(&m_callback.m_overlapped, 0, sizeof(m_callback.m_overlapped));

            PVOID lpSendBuffer = NULL;
            DWORD dwSendDataLength = 0;
            LPDWORD lpdwBytesSent = NULL;
            BOOL connerr = LpfnConnectex(conn_socket, (sockaddr*)&serveraddr, sizeof(serveraddr),
                lpSendBuffer, dwSendDataLength, lpdwBytesSent,
                &m_callback.m_overlapped);


            if (connerr) TINYASYNC_UNLIKELY {
                TINYASYNC_LOG("ConnectEx complemented");
            } else {
                int err;
                if ((err=WSAGetLastError()) == ERROR_IO_PENDING) TINYASYNC_LIKELY{
                    TINYASYNC_LOG("ConnectEx io_pending");
                } else {
                    throw_WASError("ConnectEx failed:");
                }
            }

#elif defined(__unix__)
            if (connerr == -1) {
                if (errno == EINPROGRESS) {
                    TINYASYNC_LOG("EINPROGRESS, conn_handle = %X", connfd);
                    // just ok ...
                } else {
                    throw_errno(format("can't connect, conn_handle = %X", connfd));
                }
            } else if (connerr == 0) {
                TINYASYNC_LOG("connected, conn_handle = %X", connfd);
                // it's connected ??!!
                connected = true;
            }
#endif


            TINYASYNC_LOG("conn_handle = %s", socket_c_str(m_socket));
            m_endpoint = endpoint;
            return connected;
        }


    };


    ConnectorImpl async_connect(IoContext& ctx, Protocol const& protocol, Endpoint const& endpoint)
    {
        return ConnectorImpl(*ctx.get_io_ctx_base(), protocol, endpoint);
    }

    void ConnectorCallback::on_callback(IoEvent& evt)
    {
        TINYASYNC_GUARD("ConnectorCallback::callback():");
        assert(m_connector);
        assert(m_connector->m_awaiter);

        auto connfd = m_connector->m_socket;

#ifdef _WIN32
        //
#elif defined(__unix__)
        if (evt.events & (EPOLLERR | EPOLLHUP)) {
            throw_errno(format("error, fd = %d", connfd));
        }

        int result;
        socklen_t result_len = sizeof(result);
        if (getsockopt(connfd, SOL_SOCKET, SO_ERROR, &result, &result_len) < 0) {
            throw_errno(format("getsockopt failed for conn_handle = %d", m_connector->m_socket));
        }
        if (result != 0) {
            if (errno == EINPROGRESS) {
                TINYASYNC_LOG("EINPROGRESS, fd = %d", connfd);
                return;
            } else {
                TINYASYNC_LOG("bad!, fd = %d", connfd);
                throw_errno(format("connect failed for conn_handle = %d", m_connector->m_socket));
            }
        }
#endif

        TINYASYNC_LOG("connected");
        TINYASYNC_RESUME(m_connector->m_awaiter->m_suspend_coroutine);

    }


    void ConnectorAwaiter::unregister(NativeSocket conn_handle)
    {
        auto connfd = conn_handle;
        m_connector->m_awaiter = nullptr;

#ifdef _WIN32
        //
#elif defined(__unix__)
        // epoll_event evt;
        // evt.data.ptr = nullptr;
        // evt.events = 0;
        // we can't handle error?
        // so we remove it from epoll
        auto clterr = epoll_ctl(m_connector->m_ctx->event_poll_handle(), EPOLL_CTL_DEL, connfd, NULL);
        if (clterr == -1) {
            throw_errno(format("can't unregister conn_handle = %d", connfd));
        }
#endif

        TINYASYNC_LOG("unregister connect for conn_handle = %s", socket_c_str(connfd));
    }

    bool ConnectorAwaiter::await_suspend(std::coroutine_handle<TaskPromiseBase> suspend_coroutine)
    {
        TINYASYNC_LOG("ConnectorAwaiter::await_suspend():");
        m_suspend_coroutine = suspend_coroutine;
        NativeSocket connfd = m_connector->m_socket;

        assert(!m_connector->m_awaiter);
        m_connector->m_awaiter = this;

#ifdef _WIN32
        //
#elif defined(__unix__)
        epoll_event evt;
        evt.data.ptr = &m_connector->m_callback;
        // level triger by default
        evt.events = EPOLLOUT;
        auto clterr = epoll_ctl(m_connector->m_ctx->event_poll_handle(), EPOLL_CTL_ADD, m_connector->m_socket, &evt);
        if (clterr == -1) {
            throw_errno(format("epoll_ctl(EPOLLOUT) failed for conn_handle = %d", m_connector->m_socket));
        }
#endif

        TINYASYNC_LOG("register connect for conn_handle = %d", m_connector->m_socket);

        try {
            if (m_connector->connect(m_connector->m_endpoint)) {
                return false;
            }
            return true;
        } catch (...) {
            std::throw_with_nested(std::runtime_error("can't connect"));
        }
    }


    Connection ConnectorAwaiter::await_resume()
    {
        TINYASYNC_GUARD("ConnectorAwaiter::await_resume():");
        auto connfd = m_connector->m_socket;
        this->unregister(connfd);
        // added in event poll, because of connect
        return { *m_connector->m_ctx, m_connector->m_socket, false};
    }

    class TimerAwaiter;

    class TimerCallback : public CallbackImplBase
    {
    public:
        friend class TimerAwaiter;
        TimerAwaiter* m_awaiter;

        TimerCallback(TimerAwaiter* awaiter) : CallbackImplBase(this), m_awaiter(awaiter)
        {
        }
        void on_callback(IoEvent& evt);

    };

    class TINYASYNC_NODISCARD TimerAwaiter
    {
    public:
        friend class TimerCallback;
    private:
        IoCtxBase* m_ctx;
        std::chrono::nanoseconds m_elapse; // mili-second
        NativeHandle m_timer_handle = NULL_HANDLE;
        std::coroutine_handle<TaskPromiseBase> m_suspend_coroutine;
        TimerCallback m_callback_ = this;
        Callback *m_callback = &m_callback_;

    public:
        // interface to wait
        using callback_type = TimerCallback;

        TimerCallback *get_callback() {
            return &m_callback_;
        }

        void set_callback(Callback *c) {
            m_callback = c;
        }


        // elapse in micro-second
        TimerAwaiter(IoContext& ctx, std::chrono::nanoseconds elapse) : m_ctx(ctx.get_io_ctx_base()), m_elapse(elapse)
        {
        }

        constexpr bool await_ready() const noexcept { return false; }

        template<class Promise>
        inline void await_suspend(std::coroutine_handle<Promise> suspend_coroutine)
        {
            auto h = suspend_coroutine.promise().coroutine_handle_base();
            await_suspend(h);
        }

        void await_resume() const noexcept {
#ifdef _WIN32
            // timer thread have done cleaning up
#elif defined(__unix__)
            // remove from epoll list
            epoll_ctl(m_ctx->event_poll_handle(), m_timer_handle, EPOLL_CTL_DEL, NULL);
            close(m_timer_handle);
#endif
        }

#ifdef _WIN32

        static void CALLBACK onTimeOut(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
        {
            UNREFERENCED_PARAMETER(dwTimerLowValue);
            UNREFERENCED_PARAMETER(dwTimerHighValue);

            auto awaiter = (TimerAwaiter*)lpArg;
            CloseHandle(awaiter->m_timer_handle);

            constexpr DWORD numDataTransfered = 0;
            const auto poverlapped = &awaiter->m_callback.m_overlapped;
            constexpr ULONG_PTR key = 0;

            memset(&awaiter->m_callback.m_overlapped, 0, sizeof(awaiter->m_callback.m_overlapped));
            // tell the io thread time out
            PostQueuedCompletionStatus(awaiter->m_ctx->handle(), numDataTransfered, key, poverlapped);
        };

        // run on timer thread
        static void CALLBACK onQueueUserAPC(ULONG_PTR Parameter)
        {
            TimerAwaiter* awaiter = (TimerAwaiter*)Parameter;

            HANDLE timer_handle = CreateWaitableTimerW(NULL, FALSE, NULL);
            // false sharing
            awaiter->m_timer_handle = timer_handle;

            if (timer_handle != NULL_HANDLE) {
                LARGE_INTEGER elapse;
                // QuadPart is in 100ns
                elapse.QuadPart = -awaiter->m_elapse.count()/100;

                SetWaitableTimer(timer_handle, &elapse, 0, onTimeOut, (PVOID)awaiter, FALSE);
            } else {
                throw_LastError("can't create timer");
            }
        }
        static DWORD TimerThradProc(LPVOID lpParameter)
        {
            TINYASYNC_GUARD("[timer thread]: ");


            for (;;) {
                TINYASYNC_LOG("sleeping ...");
                // get thread into sleep
                // in thi state, the callback get a chance to be invoked
                constexpr bool alertable = true;
                ::SleepEx(INFINITE, alertable);
                TINYASYNC_LOG("wake up");
            }
        };

        struct TimerThread {

            std::mutex m_mutex;
            std::atomic<void*> m_timer_thread_handle = NULL_HANDLE;

            HANDLE initialize() noexcept
            {
                HANDLE thread_handle = ::CreateThread(
                    NULL,                   // default security attributes
                    0,                      // use default stack size  
                    TimerThradProc,         // thread function name
                    NULL,                   // argument to thread function 
                    0,                      // use default creation flags 
                    NULL);                  // returns the thread identifier
                return thread_handle;
            }

            inline HANDLE get_timer_thread_handle()
            {
                return initialize_once(m_timer_thread_handle, (void*)NULL_HANDLE, m_mutex, [&]() {
                    return initialize();
                });
            }

        };

        inline static TimerThread timer_thread;

        inline void await_suspend(std::coroutine_handle<TaskPromiseBase> h)
        {
            HANDLE thread_handle = timer_thread.get_timer_thread_handle();
            if (thread_handle == NULL_HANDLE) {
                throw_LastError("can't create thread");
            }

            m_suspend_coroutine = h;
            if (QueueUserAPC(onQueueUserAPC, thread_handle, (ULONG_PTR)this) == 0) [[unlikely]] {
                throw_LastError("can't post timer to timer thread");
            }
        }

#elif defined(__unix__)

        inline void await_suspend(std::coroutine_handle<TaskPromiseBase> h)
        {

            // create a timer
            m_suspend_coroutine = h;
            itimerspec time;
            time.it_value = to_timespec(m_elapse);
            time.it_interval = to_timespec(std::chrono::nanoseconds{0});

            auto fd = timerfd_create(CLOCK_REALTIME, 0);
            if (fd == -1) {
                throw_errno("can't create timer");
            }

            try {
                auto fd2 = timerfd_settime(fd, 0, &time, NULL);
                if (fd2 == -1) {
                    throw_errno("can't set timer");
                }

                auto epfd = m_ctx->event_poll_handle();

                epoll_event evt;
                evt.data.ptr = m_callback;
                evt.events = EPOLLIN;
                auto fd3 = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &evt);

                if (fd3 == -1) {
                    throw_errno("can't bind timer fd with epoll");
                }

            }
            catch (...) {
                close_handle(m_timer_handle);
                throw;
            }
            m_timer_handle = fd;
        }

#endif
    };


    void TimerCallback::on_callback(IoEvent& evt)
    {
        TINYASYNC_RESUME(m_awaiter->m_suspend_coroutine);
    }




    TimerAwaiter async_sleep(IoContext& ctx, std::chrono::nanoseconds elapse)
    {
        return TimerAwaiter(ctx, elapse);
    }




} // namespace tinyasync


#endif
