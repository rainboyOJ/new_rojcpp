/**
 * 产生所需要的Buffer
 * 基于pmr
 */

#pragma once
#include <memory>
#include <cstring>
#include <memory_resource>

namespace rojcpp {
    
    template<typename T = std::byte ,typename Alloc =  std::pmr::polymorphic_allocator<T>>
    class Buffer {
        public:

            Buffer(std::pmr::memory_resource * mem_resource_ptr);

            void expand_size(std::size_t size_); //增加空间

            void expand_twice_size(); //增加两倍的空间

            bool empty(); //是否为空

            T * data(); //数据的起始位置

            T * write_data(); //可以写入的空间的位置

            void clear();

            void update_used_sized(std::size_t size_);
            std::size_t used_size() const; //已经这写入的空间
            std::size_t left_size() const; //剩余可写入空间
            std::size_t tot_size() const; //总空间,包括已经写入

        private:
            
        // std::bytes;
            Alloc m_alloctor; //内存申请器
            std::size_t m_tot_mem = 0;
            std::size_t used_mem  = 0;
            T * m_raw_mem = nullptr;
    };

    //转换成buffer
    void to_buffer();
} // end namespace rojcpp

namespace rojcpp {


    template<typename T ,typename Alloc>
    Buffer<T,Alloc>::Buffer(std::pmr::memory_resource * mem_resource_ptr ) 
        : m_alloctor(mem_resource_ptr)
    {}

    template<typename T ,typename Alloc>
    void Buffer<T,Alloc>::expand_size(std::size_t size_){
        T * tmp_mem = m_alloctor.allocate(m_tot_mem + size_);
        if( m_raw_mem == nullptr){
            m_tot_mem = size_;
            m_raw_mem = tmp_mem;
        }
        else {
            memcpy(tmp_mem, m_raw_mem, m_tot_mem);
            m_alloctor.deallocate(m_raw_mem,m_tot_mem);
            m_tot_mem += size_;
            m_raw_mem = tmp_mem;
        }
    }

    template<typename T ,typename Alloc>
    void Buffer<T,Alloc>::expand_twice_size(){
        expand_size(m_tot_mem == 0 ? 1 : (m_tot_mem << 1));
    }

    template<typename T ,typename Alloc>
    bool Buffer<T,Alloc>::empty() {
        return m_raw_mem == nullptr || m_tot_mem == 0;

    }

    template<typename T ,typename Alloc>
    T * Buffer<T,Alloc>::data() {
        return m_raw_mem;
    }

    template<typename T ,typename Alloc>
    T * Buffer<T,Alloc>::write_data() {
        return m_raw_mem + used_mem;
    }

    template<typename T ,typename Alloc>
    std::size_t Buffer<T,Alloc>::left_size() const {
        return m_tot_mem - used_mem > 0 ?m_tot_mem - used_mem : 0;
    }

    template<typename T ,typename Alloc>
    std::size_t Buffer<T,Alloc>::used_size() const {
        return used_mem;
    }

    template<typename T ,typename Alloc>
    std::size_t Buffer<T,Alloc>::tot_size() const {
        return m_tot_mem;
    }

    template<typename T ,typename Alloc>
    void Buffer<T,Alloc>::update_used_sized(std::size_t size_) {
        used_mem += size_;
    }

    template<typename T ,typename Alloc>
    void Buffer<T,Alloc>::clear() {
        used_mem = 0;
    }


} //end namespace rojcpp
