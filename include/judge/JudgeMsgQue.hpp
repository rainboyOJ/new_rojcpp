//用来传递信息的有锁队列

#include <mutex>
#include <optional>
#include <list>
#include <queue>



template<typename T>
class MultiThdQueue {
    private:
        std::queue<T,std::list<T>> m_que;
        std::mutex m_mtx;
    public:
        
        void push(T && val) {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_que.push(std::move(val));
        }

        void push( T & val) {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_que.push(val);
        }

        std::optional<T> pop() {
            std::lock_guard<std::mutex> lock(m_mtx);
            if( m_que.empty())
                return std::nullopt;
            auto & t = m_que.front();
            m_que.pop();
            return {std::move(t)};
        }
};
