## Buffer

使用一块内存指针,转制为Buffer类型指针,方便管理这块内存的使用

buffer的种类

- Buffer
- BufferRef
- ConstBuffer
- ConstBufferRef

## Buffer功能

```
1.
Buffer sub_buffer(size_t offset) const

Buffer向后移动 offset 个byte,产生新的Buffer

Buffer sub_buffer(size_t offset, std::size_t cnt) const
Buffer向后移动 offset 个byte,产生新的Buffer,且新的Buffer有cnt个byte元素

data

size

```

## Buffer使用
