# 基于pmr的内存池实现

基础
```plaintext
PoolNode 指向自己的指针,用于链接
Pool
FreeNode 自由链接,具体看代码,有remove_self和push_node(FreeNode*)操作
PoolBlock
PoolImpl
```

- StackfulPool ,栈型内存池,FIFO
- FixPoolResource,基于Pool实现的,每次申请的内存大小都一样的pmr
- PoolResource  基于PoolImpl实现的,通用内存池.使用table,避免内存碎片

## Pool

```plaintext
+ Pool() --> 
+ Pool(std::size_t block_size,std::size_t block_per_chunk) --> 
+ Pool(Pool && r) --> 
+ swap(Pool & r) --> 
+ operator =(Pool && r) --> 
初始化
+ initialize(std::size_t block_size,std::size_t block_per_chunk) --> 
+ m_block_size --> block的大小,也就是block的占用的字节
+ m_block_per_chunk --> 每一个chunk内block的数量
+ m_head --> 链的开头指针
+ m_chunks --> vector<void *> 数组,每个元素是指向一个chunk的指针
+ alloc() --> 申请内存 + free(void * node_) --> 释放内存
+ ~Pool() --> 
```
为了避免申请小块的内存产生内存碎片
每一次申请固定的`m_block_per_chunk*block_size`大的一块内存

这个内存池的特点是每次只能申请固定大小的内存,大小是`block_size`

free不是真的free,而是把这个固定大小的内存再压回去

真正的free是在析构时


## PoolResource

`FreeNode`一个环形的链

一个存了两个指针的双向链表
```plaintext
    +--------++--------+ 
    | m_prev || m_next | 
    +--------++--------+ 
```

有如下的操作

1.`init()`初始化,两个指针都指向自己

```plaintext

 +------+           +-----+
 |      v           v     |
 |  +--------++--------+  |
 +--| m_prev || m_next |--+
    +--------++--------+ 
```

2.`remove_self()`,在链表上删除自己本身,如果返回值为`true`,表示链表上只有一个点,也就是空

3.`push(FreeNode *)`添加一个freeNode,在自己后面插入一个结点


```plaintext
+--------++--------+        +--------++--------+  
| m_prev || m_next | -----> | m_prev || m_next |  
+--------++--------+        +--------++--------+  
```


`PoolBlock`,要理解下面三个概念,size,free,order

size是由bit组成的,由三个部分组成的,大小,是否free,order,具体看代码

free标志,表示当前内存没有被使用

`prev_free_size` 前一块内存的标记,那什么是前一块内存呢??

```plaintext

0000 0000 | 1 |         000 0000
  大小      free标志    order
```
## PoolBlock

```plaintext
  + k_free_mask --> 
    + k_order_mask --> 
    + m_prev_size --> 
    + m_size --> 
    + m_free_node --> 
    + prev_size() --> 
    + prev_free() --> 
    + prev_order() --> 
    + encode_size(uint32_t size,std::size_t order,bool free) --> 
    + size() --> 
    + free_() --> 
    + order() --> 
    + mem() --> 
    + from_mem(void * p) --> 
    + from_free_node(FreeNode * node) --> 
```


## PoolImpl

PoolImpl是整个内存池的核心,它实现了一个类似于`C++ STL 2.9`(见侯捷——STL源码剖析中相应的章节)基于表的内存
池结构,但表更加细粒度,多了内存分割与合并的功能.

整个内存池最核心的功能就是对一片内存就进分片与合并.


```plaintext
    + k_max_order -->  48
    + k_malloc_order --> 44
    + m_free_flags --> 标志,对应位置的是否有free的freeNode
    + FreeNode m_free[k_max_order+1] --> 
    + FreeNode m_chuncks --> 
    + PoolImpl() --> 
    + ~PoolImpl() --> 
    + s1(std::size_t s) --> ?
    + s2(std::size_t s) --> ?
    + s3(std::size_t s) --> ?
    + block_order(std::size_t block_size) --> 得到block_size 对应的table中的下标
    + block_size(std::size_t block_order) --> order 对应的 block_size
    + ffs64(uint64_t v) --> 
    + add_free_block(PoolBlock * block,std::size_t order) --> 
    + remove_free_block(PoolBlock * block,std::size_t order) --> 
    + break_(PoolBlock * block,std::size_t const block_size,std::size_t align) --> 
    + alloc(PoolImpl * pool,std::size_t size,std::size_t align=alignof (std::max_align_t)) --> 
    + next_block(PoolBlock * block,std::size_t size) --> 
    + prev_block(PoolBlock * block,std::size_t size) --> 
    + change_block_size(PoolImpl * pool,PoolBlock * block,std::size_t new_size,PoolBlock * next) --> 
    + change_block_size(PoolImpl * pool,PoolBlock * block,std::size_t new_size,PoolBlock * next,PoolBlock * new_block) --> 
    + free(PoolImpl * pool,void * p,std::size_t size,std::size_t align) --> 
```

分片的原理

合并的原理
