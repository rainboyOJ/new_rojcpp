# log模块

好的日志调试事半功倍,有些时候,相比使用gdb,节省很多的时间.

tinyasync的日志源码在`basics.h`头文件里, 被`#ifdef TINYASYNC_TRACE #endif`包含的就是源码.

一些宏

```plaintext
TINYASYNC_GUARD
TINYASYNC_LOG
TINYASYNC_LOG_NNL
```
- 一个线程级变量 `thread_local std::vecotr<std::string> log_prefix`,按顺序存字符串.
- 一个类`log_prefix_guard`,类似于RAII的设计,在构造时,向`log_prefix`里存入字符串,析构时,从`log_prefix`里pop数据
- 宏`TINYASYNC_LOG`,取出`log_prefix`里的所有的字符串,加上自己的字符串,一起输出.

如果你需要调试tinyasync,需要定义`TINYASYNC_TRACE`这个宏


因为每一个线程都有自己的`log_prefix`,所以不用担心data race的问题.
但是,输出到标准输出的时候,可能有问题.

`TINYASYNC_GUARD`可以在一个local内定义一个`log_prefix_guard`.这样相当于添加了log的"前缀".当调用`TINYASYNC_LOG`
时一起输出.当自己的生命周期结束时,从`log_prefix`这个容器里删除自己.


它的使用参考代码 [doc/functional/code/log.cpp](./code/log.cpp)
