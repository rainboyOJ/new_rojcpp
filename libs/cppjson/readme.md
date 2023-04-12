# cppjson

cpp的类与json的互相转换,一个类对象可以`dumps`成一json,也可以`load`一个jsonString创建一个类的对象,代码抄自[shijunfeng00/CppJson: 轻量级C++对象序列化框架，同时支持部分运行时反射](https://github.com/shijunfeng00/CppJson)

## 项目结构

```plaintext
src
├── configPair.hpp  存某个类的单个成员的信息
├── config.hpp      包装了configPair,存某个类的所有成员信息
├── reflectable.hpp     反射(类对象转json)的静态方法集合
├── serializable.hpp    序列化(json转类对象)的静态方法集合,核心函数decode(解析JSON)
├── exception.hpp       一些异常
└── utils.hpp           常用工具函数
```

## 编译与测试

```sh
mkdir build && cd build
cmake ..
make
```

## BUG

- 嵌套的循环依赖的类型 如:`std::tuple<std::pair<int,int>>`
