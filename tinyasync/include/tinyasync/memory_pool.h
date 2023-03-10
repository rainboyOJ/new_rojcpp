#ifndef TINYASYNC_MEMORY_POOL_H
#define TINYASYNC_MEMORY_POOL_H

#include <cstddef>   // std::size_t, std::byte
#include <bit>       // std::countx_zeros
#include <vector>
#include <cstring>
#include <stdlib.h>
#include <memory>
#include <assert.h>

#if defined(__clang__)

#include <experimental/memory_resource>
namespace std {
    namespace pmr {
        using std::experimental::pmr::memory_resource;
        using std::experimental::pmr::get_default_resource;
        using std::experimental::pmr::set_default_resource;
        using std::experimental::pmr::new_delete_resource;
    }
}
#else

#include <memory_resource>

#endif

namespace tinyasync
{

    struct PoolNode {
        PoolNode *m_next;
    };

    class Pool
    {

    public:
        Pool() noexcept : m_block_size(0), m_block_per_chunk(0), m_head(nullptr)
        {
        }

        Pool(std::size_t block_size, std::size_t block_per_chunk) noexcept
        {
            initialize(block_size, block_per_chunk);
        }

        Pool(Pool &&r)
        {
            m_block_size = r.m_block_size;
            m_block_per_chunk = r.m_block_per_chunk;
            m_head = r.m_head;
            m_chunks = std::move(r.m_chunks);
            r.m_head = nullptr;
            r.m_block_size = 0;
            r.m_block_per_chunk = 0;
        }

        void swap(Pool &r)
        {
            std::swap(m_block_size, r.m_block_size);
            std::swap(m_block_per_chunk, r.m_block_per_chunk);
            std::swap(m_head, r.m_head);
            std::swap(m_chunks, r.m_chunks);
        }

        Pool &operator=(Pool &&r)
        {
            Pool pool(std::move(r));
            return *this;
        }

        void initialize(std::size_t block_size, std::size_t block_per_chunk)
        {
            m_block_size = std::max(sizeof(void *), block_size);
            m_block_per_chunk = block_per_chunk;
            m_head = nullptr;
        }

        size_t m_block_size;
        size_t m_block_per_chunk;
        PoolNode *m_head;

        std::vector<void *> m_chunks; //?????????????????????

        void *alloc() noexcept
        {
            auto head = m_head;
            if (!head) {
                    auto block_per_chunk = m_block_per_chunk;
                    auto block_size = m_block_size;
                    std::size_t memsize = block_size * block_per_chunk;
                    if (!memsize)
                    {
                        return nullptr;
                    }
                    // max alignment
                    auto *h = ::malloc(memsize);
                    if (!h)
                    {
                        return h;
                    }
                    m_chunks.push_back(h);
                    for (std::size_t i = 0; i < block_per_chunk - 1; ++i)
                    {
                        PoolNode *p = (PoolNode *)(((char *)h) + block_size * i);
                        PoolNode *p2 = (PoolNode *)(((char *)h) + block_size * (i + 1));
                        p->m_next = p2;
                    }
                    PoolNode *p = (PoolNode *)(((char *)h) + block_size * (block_per_chunk - 1));
                    p->m_next = nullptr;
                    m_head = (PoolNode *)h;
                    head = m_head;
            }
            m_head = head->m_next;
            return head;
        }

        void free(void *node_) noexcept
        {
            auto node = (PoolNode *)node_;
            node->m_next = m_head;
            m_head = node;
        }

        ~Pool() noexcept
        {
            for (auto *p : m_chunks)
            {
                ::free(p);
            }
        }
    };

    struct FreeNode
    {
        FreeNode *m_next;
        FreeNode *m_prev;

        void init()
        {
            m_next = this;
            m_prev = this;
        }

        bool remove_self()
        {
            auto prev = this->m_prev;
            auto next = this->m_next;
            next->m_prev = prev;
            prev->m_next = next;
            return prev == next;
        }

        void push(FreeNode *node)
        {
            auto next = m_next;
            node->m_next = next;
            node->m_prev = this;
            this->m_next = node;
            next->m_prev = node;
        }
    };

    struct PoolBlock
    {
        static const uint32_t k_free_mask = 1<<7; // 1000 0000
        static const uint32_t k_order_mask = 0x7F; // 0111 1111

        uint32_t m_prev_size;
        // ???8????????????:
        // ????????????: ???????????????
        uint32_t m_size;      // total size include header
        FreeNode m_free_node; //??????freeNode??????,???PoolBlock???????????????FreeNode?????????

        uint32_t prev_size() { // ?????????block?????????
            return m_prev_size >> 8; // ??? 2^8
        }

        uint32_t prev_free() {
            return m_prev_size & k_free_mask;
        }

        std::size_t prev_order() { // ?????????block??? order
            return m_prev_size & k_order_mask;
        }

        static uint32_t encode_size(uint32_t size, std::size_t order, bool free) {
            return (size<<8) | order | (free?k_free_mask:0);
        }

        uint32_t size() {
            return m_size >> 8;
        }

        uint32_t free_() { // ?????????free???,
            return m_size & k_free_mask;
        }

        std::size_t order() {
            return m_size & k_order_mask;
        }

        void *mem() // ???????????????
        {
            return (char *)this + 2*sizeof(uint32_t); // ??????,m_prev_size,m_size
        }

        //?????????????????? mem ????????????
        static PoolBlock *from_mem(void *p) //????????????????????????PoolBlock
        {
            assert((void *)p > (void *)100);
            return (PoolBlock *)((char *)p - 2*sizeof(uint32_t));
        }

        //?????????FreeNode??????PoolBlock
        static PoolBlock *from_free_node(FreeNode *node)
        {
            assert(node > (void *)100);
            return (PoolBlock *)((char *)node - offsetof(PoolBlock, m_free_node));
        }
    };

    // inline std::map<void *, bool> m_allocated;

    struct PoolImpl
    {
        static const int k_max_order = 48;
        static const int k_malloc_order = 44;

        uint64_t m_free_flags; // m_free?????????????????????free???Node
        FreeNode m_free[k_max_order + 1];

        FreeNode m_chuncks;

        PoolImpl()
        {
            for (auto &m : m_free) //???????????????FreeNode
            {
                m.m_next = &m;
                m.m_prev = &m;
            }
            m_free_flags = 0;
            m_chuncks.init(); // ???????????????freeNode
        }

        ~PoolImpl() //??????
        {            
            for(auto i = 0; i < k_max_order; ++i) {
                assert(m_free[i].m_next == &m_free[i]);
            }

            auto c = m_chuncks.m_next;
            for(;c != &m_chuncks;) {
                auto next = c->m_next;
                auto p = PoolBlock::from_free_node(c);
                ::free(p);
                c = next;
            }
        }

        static constexpr uint16_t s1(std::size_t s) {
            return s + s/4;
        }
        static constexpr uint16_t s2(std::size_t s) {
            return s + s/2;
        }
        static constexpr uint16_t s3(std::size_t s) {
            return s + 3*s/4;
        }

        //????????????????????? ????????????table???????????????
        //?????? 8->0  9->1 10->1 11->2
        // ??????table????????????????????????
        //8 10 12 14 16 20 24 28 32 40 48 56 64 80 96 112 128 160 192 224 256 320 384 448 512 640 768 896 1024 1280 1536 1792 2048 2560 3072 3584 4096 5120 6144 7168 8192 10240 12288 14336 16384 20480 24576 28672 32768
        static uint16_t block_order(std::size_t block_size)
        {
#if 0
            static constexpr uint16_t table[] = {
                                    8, s1(8), s2(8), s3(8),
                                    16, s1(16), s2(16), s3(16),
                                    32, s1(32), s2(32), s3(32),
                                    64, s1(64), s2(64), s3(64),
                                    128, s1(128), s2(128), s3(128),
                                    256, s1(256), s2(256), s3(256),
                                    512, s1(512), s2(512), s3(512),
                                    1<<10, s1(1<<10), s2(1<<10), s3(1<<10),
                                    1<<11, s1(1<<11), s2(1<<11), s3(1<<11),
                                    1<<12, s1(1<<12), s2(1<<12), s3(1<<12),
                                    1<<13, s1(1<<13), s2(1<<13), s3(1<<13),
                                    1<<14, s1(1<<14), s2(1<<14), s3(1<<14),
                                    1<<15, 
                                    };

            auto end = table + sizeof(table)/sizeof(table[0]);
            auto p = std::lower_bound(table, end, block_size);
            assert(p != end);
            return p - table;
#else
            const size_t block_size_ = block_size;
            const size_t shift = sizeof(block_size_)*8 - std::countl_zero(block_size_) - 3;
            const size_t block_size__ = block_size_ >> shift;
            const size_t block_size_2 = block_size__ & 3;
            const size_t block_size_3 = (shift-1)*4 + block_size_2;
            const size_t order = block_size_3 + bool(block_size_ & ((1<<shift)-1) );
            return order;
#endif
        }

        static std::size_t block_size(std::size_t block_order)
        {
            static constexpr uint16_t table[] = {
                                    8, s1(8), s2(8), s3(8),
                                    16, s1(16), s2(16), s3(16),
                                    32, s1(32), s2(32), s3(32),
                                    64, s1(64), s2(64), s3(64),
                                    128, s1(128), s2(128), s3(128),
                                    256, s1(256), s2(256), s3(256),
                                    512, s1(512), s2(512), s3(512),
                                    1<<10, s1(1<<10), s2(1<<10), s3(1<<10),
                                    1<<11, s1(1<<11), s2(1<<11), s3(1<<11),
                                    1<<12, s1(1<<12), s2(1<<12), s3(1<<12),
                                    1<<13, s1(1<<13), s2(1<<13), s3(1<<13),
                                    1<<14, s1(1<<14), s2(1<<14), s3(1<<14),
                                    1<<15, 
                                    };
            return table[block_order];
        }

        //??????0?????????
        static std::size_t ffs64(uint64_t v)
        {
            return std::countr_zero(v);
        }

        //?????????order ?????? ????????????
        void add_free_block(PoolBlock *block, std::size_t order)
        {
            std::size_t idx = order;
            m_free[idx].push(&(block->m_free_node));
            m_free_flags |= (uint64_t(1) << idx); // m_free????????????????????????
        }

        // Block???PoolImpl???????????????,???????????????????????????
        void remove_free_block(PoolBlock *block, std::size_t order)
        {
            if (block->m_free_node.remove_self())
            {
                std::size_t idx = order;
                m_free_flags &= ~(uint64_t(1) << idx); //????????????????????????,???????????????
            }
        }

        // * block ??????
        // * block_size,??????block??????????????????
        // align ??????,??????block???????????????
        PoolBlock *break_(PoolBlock *block, std::size_t const block_size, std::size_t align)
        {

            auto old_size = block->size(); //??????block?????????
            assert(block_size <= old_size); // block_size ????????????????????? ???

            //??????????????? ??????block
            void *ptr = (char*)block + old_size - block_size + 2*sizeof(uint32_t);
            // + 2*sizeof(uint32_t) ?????? ?????? block_size ???????????? 2??? uint32_t ????????????

            //??????????????????,????????????
            void *ptr_align = (void*)((std::ptrdiff_t)ptr & ~(align-1));

            //????????? ???????????? uint32_t(m_prev_size,m_size)
            PoolBlock* block2 = (PoolBlock*)((char*)ptr_align - 2*sizeof(uint32_t));
            // ??????block2->mem() ??????????????????

            //???????????????
            std::size_t new_size = (char*)block2 - (char*)block;


            //????????? ?????? PoolBlock ???
            if (new_size < sizeof(PoolBlock))
            {
                // use whole block
                remove_free_block(block, block->order());

                auto block_next = next_block(block, old_size); //?????????????????????
                auto encode_size = block->m_size & ~PoolBlock::k_free_mask;
                block->m_size = encode_size;
                block_next->m_prev_size =  encode_size;
                return block;
            } else  { // ??????????????? PoolBlock ???
                change_block_size(this, block, new_size, block2); // ????????????

                auto block2_size = old_size - new_size;
                assert(block2_size >= block_size); //

                auto block2_next = next_block(block2, block2_size); // ??????block2_next???????????? ?
                auto new_ord = block_order(block2_size);
                auto encode_size = PoolBlock::encode_size(block2_size, new_ord, false);
                block2->m_size = encode_size;
                block2_next->m_prev_size = encode_size;

                return block2;
            }
        }

        // ????????????
        // size ?????????????????????
        // alignof ??????
        static void *alloc(PoolImpl *pool, std::size_t size, std::size_t align = alignof(std::max_align_t))
        {
#if __USE_ASYNC_UTILS__
            printf("in PoolImpl alloc ,size = %lld, \n",size);
#endif


            //????????????????????????????????????
            std::size_t size_ = std::max(size, 2*sizeof(void*)); // ?????????16
            align = std::max(2*sizeof(uint32_t), align); // ??????????????? 8

            // size_ + 2???uint32_t( m_prev_size,m_size) ,??????block?????????
            const std::size_t block_size = 2*sizeof(uint32_t) + size_; // ???????????????,????????????uint32_t,?????????block_size

            //??????????????? align ???
            const std::size_t block_size_ = size_ + align; 
            //??????????????????2,??????????????????align?????????
            
            if (block_size_ >= PoolImpl::block_size(k_malloc_order)) //?????????????????????16384byte
            {
                return ::aligned_alloc(align, size); // ???????????????
            }


            //             v--- allocated block
            //             [prev_size size][          size_         ]
            //             [           block_size                   ]
            // [    align                 ]
            // [              block_size_                           ]

            auto idx_ = PoolImpl::block_order(block_size_); //???????????????????????????

            std::size_t idx;
            PoolBlock *block;

            // ffs64????????????, ??????????????????0
            // ?????????????????????64,????????? m_free_flags ???????????????????????????(??????m_free?????????????????????)
            // ?????????????????????,???????????????>= ???????????????
            //
            // pool->m_free_flags & ~((uint64_t(1) << idx_) - 1)
            //  ???m_free_flags idx????????????????????????
            idx = ffs64(pool->m_free_flags & ~((uint64_t(1) << idx_) - 1)); 

            if (idx != 64) // ???????????????????????????
            {
                FreeNode &node = pool->m_free[idx];
                auto next = node.m_next;
                if (next == &node)
                {
                    // empty
                    assert(next != &node); // ???? ???????????????
                }
                block = PoolBlock::from_free_node(next);
            }
            else // idx?????????64???0,m_free????????????,??????????????????
            {
                auto size = PoolImpl::block_size(k_max_order); // ??????????????????????????????,32kb
                auto head = (PoolBlock *)::malloc(size + 2*sizeof(PoolBlock));// ??????????????????,sizeof(PoolBlock)=24
                // auto tail = (PoolBlock *)((char*)head + size); // ?? ????????????+size
                auto tail = (PoolBlock *)((char*)head + size + sizeof(PoolBlock) ); // ????????????,by rainboy
                block = (PoolBlock *)(head + 1); // block
                // auto tail = block + size;

                block->m_prev_size = PoolBlock::encode_size(0, 0, false); // ????????? : ??????0, order,0,free=false
                block->m_size = PoolBlock::encode_size(size, k_max_order, true); //??????,??????,size,order,free=true

                tail->m_size = PoolBlock::encode_size(0, 0, false);

                pool->m_chuncks.push(&head->m_free_node); //?????????????????????,free????????????
                pool->add_free_block(block, k_max_order);
            }

            //break_: ????????????????????????,????????????
            block = pool->break_(block, block_size, align);

            assert(!block->free_());
            return block->mem(); //????????????
        }

        //Block ?????? size?????????????????????Block
        static PoolBlock *next_block(PoolBlock* block, std::size_t size) {
            return (PoolBlock*)((char*)block + size);
        }

        static PoolBlock *prev_block(PoolBlock* block, std::size_t size) {
            return (PoolBlock*)((char*)block - size);
        }

        // ??? block ?????? new_size??????
        // new_size ????????????
        // next ?????????block
        static void change_block_size(PoolImpl *pool, PoolBlock *block, std::size_t new_size, PoolBlock *next)
        {
            auto old_ord = block->order(); //???????????????
            std::size_t order = block_order(new_size);  // ??????order
            uint32_t encode_size = PoolBlock::encode_size(new_size, order, true);
            block->m_size = encode_size;
            next->m_prev_size = encode_size;

            if (old_ord != order) // ???????????????order
            {
                pool->remove_free_block(block, old_ord); // ??????
                pool->add_free_block(block, order); // ??????
            }
        }

        static void change_block_size(PoolImpl *pool, PoolBlock *block, std::size_t new_size, PoolBlock *next,
            PoolBlock *new_block)
        {
            auto old_ord = block->order();
            std::size_t order = block_order(new_size);
            uint32_t encode_size = PoolBlock::encode_size(new_size, order, true);
            new_block->m_size = encode_size;
            next->m_prev_size = encode_size;

            pool->remove_free_block(block, old_ord);
            pool->add_free_block(new_block, order);
        }

        //????????????
        // pool ???????????????, p ????????????????????? size ??????, align ??????
        static void free(PoolImpl *pool, void *p, std::size_t size, std::size_t align)
        {

#if __USE_ASYNC_UTILS__
            printf("in PoolImpl free ,size = %lld, \n",size);
#endif
            std::size_t size_ = std::max(size, 2*sizeof(void*)); // ????????????16tybe
            align = std::max(2*sizeof(uint32_t), align); // ???????????????8
            const std::size_t block_size_ = size_ + align;// ????????????????????? size_ + align
            if (block_size_ >= block_size(k_malloc_order)) // ???????????????????????????
            {
                ::free(p); // ???????????????
                return;
            }
            
            PoolBlock *cur = PoolBlock::from_mem(p); // ??????????????? PoolBlock
            auto cur_free = cur->free_();
            assert(!cur_free); // ????????????????????? no free, ?????????????????????,????????????????????????

            auto cur_size = cur->size(); // ????????????????????????
            auto prev_free = cur->prev_free(); // ?????????????????????free

            PoolBlock *next = next_block(cur, cur_size);
            auto next_free = next->free_();

            // ?????????????????????free???,???????????????????????????
            if (prev_free && next_free) 
            {
                auto next_size = next->size();
                auto prev_size = cur->prev_size();
                
                auto size = prev_size + cur_size + next_size;
                auto prev = prev_block(cur, prev_size);

                pool->remove_free_block(next, next->order());

                auto nn = next_block(next, next_size);
                //??????????????????????????????,?????????????????????????
                change_block_size(pool, prev, size, nn); 
            }
            else if (prev_free) // ???????????????free???,??????????????????
            {
                auto prev_size = cur->prev_size();

                auto size = prev_size + cur_size;
                auto prev = prev_block(cur, prev_size);

                change_block_size(pool, prev, size, next);
            }
            else if (next_free) //??????????????????
            {
                auto next_size = next->size();
                auto size = cur_size + next_size;

                auto nn = next_block(next, next_size);
                change_block_size(pool, next, size, nn, cur);       
            }
            else //????????????????????????
            {
                auto order = cur->order();
                uint32_t encode_size = cur->m_size | PoolBlock::k_free_mask;

                next->m_prev_size = encode_size;
                cur->m_size = encode_size;
                pool->add_free_block(cur, order);
            }
        }
    };

    // ???StackfulPool?????????
    struct StackfulPoolArg {
        char *m_base;
        char *m_guard;       
    };
    
    //???????????????????????????,??????????????????FILO?????????dealloc??????
    struct StackfulPool
    {
        char *m_base; // ?
        char *m_guard;// ?

        template<class T>
        struct StackfulAllocator //?????????
        {
            using value_type = T;

            template<class U>
            StackfulAllocator(StackfulAllocator<U> r) {
                m_pool = r.m_pool;
            }

            StackfulAllocator() {
                m_pool = nullptr;
            }

            StackfulPool *m_pool; // ??????

            //?????? sz ??? T ?????????????????????
            void *allocate(std::size_t sz) {
                return m_pool->allocate(sz * sizeof(T), alignof(T));
            }
            void deallocate(void *p, std::size_t sz) {
                m_pool->deallocate(p, sz * sizeof(T), alignof(T));
            }
        };

        auto get_allocator_for_task() { //????????????????????????
            StackfulAllocator<std::byte> alloc;
            alloc.m_pool = this;
            return alloc;
        }
        
        // ???????????????????????????
        StackfulPool(std::size_t sz) {
            sz = up_round(sz, alignof(std::max_align_t));
            m_guard = (char*)::malloc(sz);
            m_base = m_guard + sz;
        }

        StackfulPoolArg as_arg() {
            StackfulPoolArg arg;
            arg.m_base = this->m_base;
            arg.m_guard = this->m_guard;
            return arg;            
        }

        StackfulPool(StackfulPoolArg arg) {
            m_base = arg.m_base;
            m_guard = arg.m_guard;
        }

        ~StackfulPool() {
            ::free(m_guard);
        }

        [[noreturn]]
        static void throw_exception()
        {
            throw std::bad_alloc();
        }

        // ????????????
        static std::size_t up_round(std::size_t sz, std::size_t align) {
            return (sz + align - 1) & ~(align-1);
        }

        //?????? bytes ???????????????,???alignment ??????
        void* allocate(size_t bytes, size_t alignment = alignof(std::max_align_t))
        {

            if(alignment > alignof(std::max_align_t)) {
                auto base = m_base;
                base -= bytes;
                base = (char*)(std::uintptr_t(base) & ~(alignment-1));
                
                if(base < m_guard) {                
                    throw_exception();
                }

                auto bytes1 = up_round(bytes, sizeof(std::size_t));
                auto bytes2 = up_round(bytes, alignment);
                if(bytes1 == bytes2) {
                    auto size = m_base - base;
                    if(size < sizeof(std::size_t)) {
                        base -= alignment;
                    }
                }
                auto pbase = (char**)((char*)base + bytes1);
                *pbase = m_base;

                m_base = base;
                return base;

            } else {
                bytes = (bytes + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t)-1);
                auto base = m_base;
                base -= bytes;
                
                if(base < m_guard) {                
                    throw_exception();
                }

                m_base = base;
                return base;
            }
        }

        void deallocate(void* p, size_t bytes, size_t alignment = alignof(std::max_align_t))
        {
            if(alignment > alignof(std::max_align_t)) {
                auto bytes1 = up_round(bytes, sizeof(std::size_t));
                m_base = *(char**)((char*)p + bytes1);
            } else {
                bytes = (bytes + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t)-1);
                m_base = (char*)p + bytes;
            }
        }
    };

    // ??????????????????????????????
    class FixPoolResource : public std::pmr::memory_resource
    {
        Pool m_impl;
    public:

        FixPoolResource(size_t block_size) {
            m_impl.initialize(block_size, 100);
        }
        FixPoolResource(FixPoolResource&&) = delete;
        FixPoolResource &operator=(FixPoolResource&&) = delete;

        virtual void *
        do_allocate(size_t __bytes, size_t __alignment) override
        {
            return m_impl.alloc();
        }

        virtual void
        do_deallocate(void *__p, size_t __bytes, size_t __alignment) override
        {
            m_impl.free(__p);
        }

        virtual bool
        do_is_equal(const std::pmr::memory_resource &__other)  const noexcept override
        {
            return this == &__other;
        }
    };

    // ?????????????????????
    class PoolResource : public std::pmr::memory_resource
    {
        PoolImpl m_impl;
    public:

        PoolResource() = default;
        PoolResource(PoolResource&&) = delete;
        PoolResource &operator=(PoolResource&&) = delete;

        virtual void *
        do_allocate(size_t __bytes, size_t __alignment) override
        {
            return PoolImpl::alloc(&m_impl, __bytes, __alignment);
        }

        virtual void
        do_deallocate(void *__p, size_t __bytes, size_t __alignment) override
        {
            PoolImpl::free(&m_impl, __p, __bytes, __alignment);
        }

        virtual bool
        do_is_equal(const std::pmr::memory_resource &__other)  const noexcept override
        {
            return this == &__other;
        }
    };

    // ??????????????????buffer ??????
    template<typename T = std::byte>
    class vector_buffer {

        std::pmr::polymorphic_allocator<T> m_poly_alloc; //?????????
        std::size_t m_tot_size; //??????????????????
        std::size_t m_size;     //?????????????????????
        T * buff = nullptr;       //????????????????????????

    public:
        
        vector_buffer(std::size_t base_size,std::pmr::memory_resource * pmr_res)
            : m_poly_alloc(pmr_res),
            m_size{0}, m_tot_size{base_size}
        {
            buff = m_poly_alloc.allocate(base_size);
        }

        //???????????? buff
        T * get_raw_buff() const { return buff;}
        T * get_write_buff() { return buff + m_size;}

        std::size_t get_tot_size() const  { return  m_tot_size;}
        std::size_t get_write_size() const  { return  m_tot_size - m_size;}

        void update_use_size(std::size_t use_size) {
            m_size += use_size;
        }

        //?????????????????????
        void expand(){
            auto new_tot_size = m_tot_size * 2;
            T * new_buff = m_poly_alloc.allocate(new_tot_size);


            if( buff) {
                std::memcpy(new_buff,buff,sizeof(T) * m_tot_size );
                m_poly_alloc.deallocate(buff,m_tot_size);
            }
            m_tot_size = new_tot_size;
            buff = new_buff;
        }


        ~vector_buffer(){
            if( buff)
                m_poly_alloc.deallocate(buff,m_tot_size);
        }

    };
} // namespace tinyasync

#endif

